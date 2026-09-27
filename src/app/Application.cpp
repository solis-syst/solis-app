#include "app/Application.h"

namespace solis {

Application::Application()
    : updater_("solis-syst", "solis-app") {
}

bool Application::initialize() {
    if (initialized_) {
        return true;
    }

    if (!settings_.initialize()) {
        return false;
    }

    if (!updater_.initialize()) {
        return false;
    }

    updater_.checkForUpdates(true);

    if (updater_.restartRequested()) {
        initialized_ = true;
        return true;
    }

    if (!browser_.initialize()) {
        return false;
    }

    if (!engines_.initialize()) {
        return false;
    }

    if (!sites_.initialize()) {
        return false;
    }

    if (!analyzer_.initialize()) {
        return false;
    }

    initialized_ = true;
    return true;
}

void Application::shutdown() {
    if (!initialized_) {
        return;
    }

    analyzer_.shutdown();
    sites_.shutdown();
    engines_.shutdown();
    browser_.shutdown();
    updater_.shutdown();
    settings_.shutdown();

    initialized_ = false;
}

EngineManager& Application::engines() { return engines_; }
SiteManager& Application::sites() { return sites_; }
BrowserManager& Application::browser() { return browser_; }
SettingsStore& Application::settings() { return settings_; }
Analyzer& Application::analyzer() { return analyzer_; }
EventBus& Application::events() { return events_; }
Updater& Application::updater() { return updater_; }

bool Application::restartRequested() const {
    return updater_.restartRequested();
}

}
