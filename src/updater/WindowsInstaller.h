#pragma once

#include <filesystem>

namespace solis {

class WindowsInstaller {
public:
    static bool launch(const std::filesystem::path& installer);
};

}
