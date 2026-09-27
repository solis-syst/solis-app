#include "updater/WindowsInstaller.h"

#ifdef _WIN32

#include <windows.h>

#include <string>

namespace solis {

namespace {

std::wstring quoteArgument(const std::filesystem::path& value) {
    return L"\"" + value.wstring() + L"\"";
}

std::filesystem::path currentModulePath() {
    wchar_t buffer[32768]{};
    const DWORD length = GetModuleFileNameW(
        nullptr,
        buffer,
        static_cast<DWORD>(sizeof(buffer) / sizeof(buffer[0])));

    if (length == 0 || length >= sizeof(buffer) / sizeof(buffer[0])) {
        return {};
    }

    return std::filesystem::path(std::wstring(buffer, length));
}

}

bool WindowsInstaller::launch(const std::filesystem::path& installer) {
    const auto result = ShellExecuteW(
        nullptr,
        L"open",
        installer.c_str(),
        nullptr,
        nullptr,
        SW_SHOWNORMAL);

    return reinterpret_cast<INT_PTR>(result) > 32;
}

bool WindowsInstaller::launchDeltaUpdate(
    const std::filesystem::path& delta,
    const std::filesystem::path& checksum,
    const std::filesystem::path& targetExecutable) {
    const auto module = currentModulePath();
    if (module.empty()) {
        return false;
    }

    const auto helper = module.parent_path() / L"solis-updater.exe";
    if (!std::filesystem::exists(helper)) {
        return false;
    }

    const auto arguments =
        L"--apply-delta " +
        quoteArgument(targetExecutable) + L" " +
        quoteArgument(delta) + L" " +
        quoteArgument(targetExecutable) + L" " +
        quoteArgument(checksum) + L" " +
        std::to_wstring(GetCurrentProcessId());

    const auto result = ShellExecuteW(
        nullptr,
        L"runas",
        helper.c_str(),
        arguments.c_str(),
        nullptr,
        SW_SHOWNORMAL);

    return reinterpret_cast<INT_PTR>(result) > 32;
}

}

#else

namespace solis {

bool WindowsInstaller::launch(const std::filesystem::path&) {
    return false;
}

bool WindowsInstaller::launchDeltaUpdate(
    const std::filesystem::path&,
    const std::filesystem::path&,
    const std::filesystem::path&) {
    return false;
}

}

#endif
