#pragma once

#include <filesystem>
#include <string>

namespace solis {

struct UpdateInfo {
    std::string version;
    std::string tag;
    std::string releaseUrl;
    std::string installerUrl;
    std::string checksumUrl;
    std::filesystem::path downloadedInstaller;
};

}
