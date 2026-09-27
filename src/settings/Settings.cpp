#include "settings/Settings.h"
namespace solis {
bool SettingsStore::initialize(){return true;}
void SettingsStore::shutdown(){}
const Settings& SettingsStore::get() const{return settings_;}
Settings& SettingsStore::get(){return settings_;}
}
