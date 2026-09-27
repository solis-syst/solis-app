#pragma once

#include "core/Module.h"
#include "updater/UpdateInfo.h"

#include <string>

namespace solis {

class Updater : public Module {
public:
    Updater(std::string owner, std::string repository);

    bool initialize() override;
    void shutdown() override;

    bool checkForUpdates(bool promptUser = true);
    bool restartRequested() const;

private:
    std::string owner_;
    std::string repository_;
    bool restartRequested_ = false;
};

}
