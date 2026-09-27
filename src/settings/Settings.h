#pragma once
#include "core/Module.h"
#include <string>
namespace solis {
struct Settings {
 std::string engine="None";
 int depth=5;
 int lines=3;
 int elo=1500;
 int threads=2;
 int hash=128;
 bool showEval=true;
 bool accuracy=true;
 bool autoMove=false;
};
class SettingsStore: public Module {
public:
 bool initialize() override;
 void shutdown() override;
 const Settings& get() const;
 Settings& get();
private:
 Settings settings_;
};
}
