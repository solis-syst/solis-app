#include "app/Application.h"
namespace solis {
bool Application::initialize(){if(initialized_)return true;if(!settings_.initialize())return false;if(!browser_.initialize())return false;if(!engines_.initialize())return false;if(!sites_.initialize())return false;if(!analyzer_.initialize())return false;initialized_=true;return true;}
void Application::shutdown(){if(!initialized_)return;analyzer_.shutdown();sites_.shutdown();engines_.shutdown();browser_.shutdown();settings_.shutdown();initialized_=false;}
EngineManager& Application::engines(){return engines_;}
SiteManager& Application::sites(){return sites_;}
BrowserManager& Application::browser(){return browser_;}
SettingsStore& Application::settings(){return settings_;}
Analyzer& Application::analyzer(){return analyzer_;}
EventBus& Application::events(){return events_;}
}
