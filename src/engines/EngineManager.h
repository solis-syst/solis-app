#pragma once
#include "core/Module.h"
#include "engines/IEngine.h"
#include <memory>
#include <string>
#include <unordered_map>
namespace solis {
class EngineManager: public Module {
public:
 bool initialize() override;
 void shutdown() override;
 bool registerEngine(std::unique_ptr<IEngine>);
 bool selectEngine(const std::string&);
 IEngine* activeEngine();
private:
 std::unordered_map<std::string,std::unique_ptr<IEngine>> engines_;
 std::string activeEngineId_;
};
}
