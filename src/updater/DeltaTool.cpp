#include "updater/Sha256Verifier.h"

#ifdef _WIN32

#include <windows.h>
#include <msdelta.h>

#include <filesystem>
#include <string>

namespace {

bool waitForParent(DWORD pid) {
    if (pid == 0) {
        return true;
    }

    const HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (!process) {
        return true;
    }

    const DWORD result = WaitForSingleObject(process, INFINITE);
    CloseHandle(process);

    return result == WAIT_OBJECT_0;
}

bool createDelta(
    const std::filesystem::path& source,
    const std::filesystem::path& target,
    const std::filesystem::path& delta) {
    std::filesystem::remove(delta);

    DELTA_INPUT globalOptions{};

    return CreateDeltaW(
        DELTA_FILE_TYPE_SET_EXECUTABLES,
        DELTA_FLAG_IGNORE_FILE_SIZE_LIMIT,
        DELTA_FLAG_NONE,
        source.c_str(),
        target.c_str(),
        nullptr,
        nullptr,
        globalOptions,
        nullptr,
        0,
        delta.c_str()) != FALSE;
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

    if (!ApplyDeltaW(
            DELTA_FLAG_NONE,
            source.c_str(),
            delta.c_str(),
            temporaryTarget.c_str())) {
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

    if (argc == 6 &&
        std::wstring(argv[1]) == L"--apply-delta") {
        return applyDelta(
            std::filesystem::path(argv[2]),
            std::filesystem::path(argv[3]),
            std::filesystem::path(argv[4]),
            std::filesystem::path(argv[5]),
            0)
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
