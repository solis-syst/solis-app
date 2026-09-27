#pragma once

#include <filesystem>
#include <string>

namespace solis {

class ManagedBrowserBootstrapper {
public:
    bool ensureInstalled(std::filesystem::path& executable) const;

private:
    bool fetchStableRelease(
        std::string& version,
        std::string& downloadUrl) const;

    bool extractArchive(
        const std::filesystem::path& archive,
        const std::filesystem::path& destination) const;

    std::filesystem::path browserRoot() const;
};

}
