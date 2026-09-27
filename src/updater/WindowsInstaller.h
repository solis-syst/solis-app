#pragma once

#include <filesystem>

namespace solis {

class WindowsInstaller {
public:
    static bool launch(const std::filesystem::path& installer);

    static bool launchDeltaUpdate(
        const std::filesystem::path& delta,
        const std::filesystem::path& checksum,
        const std::filesystem::path& targetExecutable);
};

}
