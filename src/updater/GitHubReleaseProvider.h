#pragma once

#include "updater/UpdateInfo.h"

#include <cstddef>
#include <functional>
#include <string>

namespace solis {

class GitHubReleaseProvider {
public:
    using ProgressCallback = std::function<void(std::size_t, std::size_t)>;

    GitHubReleaseProvider(std::string owner, std::string repository);

    bool fetchLatest(UpdateInfo& update) const;

    bool downloadInstaller(const UpdateInfo& update,
                           const std::filesystem::path& destination,
                           ProgressCallback progress) const;

    bool downloadChecksum(const UpdateInfo& update,
                          const std::filesystem::path& destination) const;

    bool downloadApplicationChecksum(const UpdateInfo& update,
                                     const std::filesystem::path& destination) const;

    bool downloadDelta(const UpdateInfo& update,
                       const std::filesystem::path& destination,
                       ProgressCallback progress) const;

    bool downloadDeltaChecksum(const UpdateInfo& update,
                               const std::filesystem::path& destination) const;

private:
    std::string owner_;
    std::string repository_;
};

}
