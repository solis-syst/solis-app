#include "updater/WindowsInstaller.h"

#ifdef _WIN32

#include <windows.h>

namespace solis {

bool WindowsInstaller::launch(const std::filesystem::path& installer) {
    const auto result = ShellExecuteW(
        nullptr,
        L"open",
        installer.c_str(),
        nullptr,
        nullptr,
        SW_SHOWNORMAL
    );

    return reinterpret_cast<INT_PTR>(result) > 32;
}

}

#else

namespace solis {

bool WindowsInstaller::launch(const std::filesystem::path&) {
    return false;
}

}

#endif
