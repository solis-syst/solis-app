#include "updater/Sha256Verifier.h"

#ifdef _WIN32

#include <windows.h>
#include <msdelta.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

DELTA_INPUT fileInput(const std::filesystem::path& path) {
    DELTA_INPUT input{};
    const auto value = path.wstring();
    auto* copy = new wchar_t[value.size() + 1]{};
    std::copy(value.begin(), value.end(), copy);
    copy[value.size()] = L'\0';

    input.Editable = FALSE;
    input.lpcStart = copy;
    input.uSize = static_cast<ULONG>(value.size() + 1);

    return input;
}

void freeInput(DELTA_INPUT& input) {
    delete[] static_cast<const wchar_t*>(input.lpcStart);
    input.lpcStart = nullptr;
    input.uSize = 0;
}

bool writeDelta(
    const std::filesystem::path& delta,
    const DELTA_OUTPUT& output) {
    std::ofstream file(delta, std::ios::binary | std::ios::trunc);

    if (!file || !output.lpStart || output.uSize == 0) {
        return false;
    }

    file.write(
        static_cast<const char*>(output.lpStart),
        static_cast<std::streamsize>(output.uSize));

    return static_cast<bool>(file);
}

bool writeTarget(
    const std::filesystem::path& target,
    const DELTA_OUTPUT& output) {
    std::ofstream file(target, std::ios::binary | std::ios::trunc);

    if (!file || !output.lpStart || output.uSize == 0) {
        return false;
    }

    file.write(
        static_cast<const char*>(output.lpStart),
        static_cast<std::streamsize>(output.uSize));

    return static_cast<bool>(file);
}

bool waitForParent(DWORD pid) {
    if (pid == 0) {
        return true;
    }

    const HANDLE process = OpenProcess(
        SYNCHRONIZE,
        FALSE,
        pid);

    if (!process) {
        return true;
    }

    const DWORD result = WaitForSingleObject(
        process,
        INFINITE);

    CloseHandle(process);

    return result == WAIT_OBJECT_0;
}

bool createDelta(
    const std::filesystem::path& source,
    const std::filesystem::path& target,
    const std::filesystem::path& delta) {
    std::filesystem::remove(delta);

    DELTA_INPUT sourceInput = fileInput(source);
    DELTA_INPUT targetInput = fileInput(target);
    DELTA_INPUT sourceOptions{};
    DELTA_INPUT targetOptions{};
    DELTA_INPUT globalOptions{};
    DELTA_OUTPUT output{};

    const BOOL success = CreateDeltaW(
        DELTA_FILE_TYPE_SET_EXECUTABLES,
        DELTA_FLAG_NONE,
        DELTA_FLAG_NONE,
        sourceInput,
        targetInput,
        sourceOptions,
        targetOptions,
        globalOptions,
        nullptr,
        0,
        &output);

    freeInput(sourceInput);
    freeInput(targetInput);

    if (!success) {
        return false;
    }

    const bool written = writeDelta(delta, output);
    DeltaFree(output.lpStart);

    return written;
}

bool applyDelta(
    const std::filesystem::path& source,
    const std::filesystem::path& delta,
    const std::filesystem::path& target,
    const std::filesystem::path& checksum,
    DWORD parentPid) {
    const auto temporaryTarget =
        std::filesystem::path(target.wstring() + L".solis-new");

    std::filesystem::remove(temporaryTarget);

    DELTA_INPUT sourceInput = fileInput(source);
    DELTA_INPUT deltaInput = fileInput(delta);
    DELTA_OUTPUT output{};

    const BOOL success = ApplyDeltaW(
        DELTA_FLAG_NONE,
        sourceInput,
        deltaInput,
        &output);

    freeInput(sourceInput);
    freeInput(deltaInput);

    if (!success) {
        return false;
    }

    const bool written = writeTarget(
        temporaryTarget,
        output);

    DeltaFree(output.lpStart);

    if (!written) {
        std::filesystem::remove(temporaryTarget);
        return false;
    }

    if (!solis::Sha256Verifier::verify(
            temporaryTarget,
            checksum)) {
        std::filesystem::remove(temporaryTarget);
        return false;
    }

    if (!waitForParent(parentPid)) {
        std::filesystem::remove(temporaryTarget);
        return false;
    }

    if (!MoveFileExW(
            temporaryTarget.c_str(),
            target.c_str(),
            MOVEFILE_REPLACE_EXISTING |
                MOVEFILE_WRITE_THROUGH)) {
        std::filesystem::remove(temporaryTarget);
        return false;
    }

    std::filesystem::remove(delta);
    std::filesystem::remove(checksum);

    const auto result = ShellExecuteW(
        nullptr,
        L"open",
        target.c_str(),
        nullptr,
        target.parent_path().c_str(),
        SW_SHOWNORMAL);

    return reinterpret_cast<INT_PTR>(result) > 32;
}

}

int wmain(int argc, wchar_t* argv[]) {
    if (argc == 5 &&
        std::wstring(argv[1]) == L"--create-delta") {
        return createDelta(
            std::filesystem::path(argv[2]),
            std::filesystem::path(argv[3]),
            std::filesystem::path(argv[4]))
            ? 0
            : 1;
    }

    if (argc == 7 &&
        std::wstring(argv[1]) == L"--apply-delta") {
        DWORD parentPid = 0;

        try {
            parentPid =
                static_cast<DWORD>(std::stoul(argv[6]));
        } catch (...) {
            return 1;
        }

        return applyDelta(
            std::filesystem::path(argv[2]),
            std::filesystem::path(argv[3]),
            std::filesystem::path(argv[4]),
            std::filesystem::path(argv[5]),
            parentPid)
            ? 0
            : 1;
    }

    return 1;
}

#else

int main() {
    return 1;
}

#endif
