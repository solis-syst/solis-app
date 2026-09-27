#pragma once

#include "analysis/Analyzer.h"
#include "browser/BrowserManager.h"
#include "core/EventBus.h"
#include "engines/EngineManager.h"
#include "settings/Settings.h"
#include "sites/SiteManager.h"
#include "updater/Updater.h"

namespace solis {

class Application {
public:
    Application();

    bool initialize();
    void shutdown();

    EngineManager& engines();
    SiteManager& sites();
    BrowserManager& browser();
    SettingsStore& settings();
    Analyzer& analyzer();
    EventBus& events();
    Updater& updater();

    bool restartRequested() const;

private:
    EventBus events_;
    SettingsStore settings_;
    BrowserManager browser_;
    EngineManager engines_;
    SiteManager sites_;
    Updater updater_;
    Analyzer analyzer_{engines_, sites_};
    bool initialized_ = false;
};

}
