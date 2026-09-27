#pragma once

#include <filesystem>
#include <functional>
#include <string>

namespace solis {

class ManagedBrowserDownloader {
public:
    using ProgressCallback = std::function<void(std::size_t, std::size_t)>;

    bool download(
        const std::string& url,
        const std::filesystem::path& destination,
        ProgressCallback progress = {}) const;
};

}
