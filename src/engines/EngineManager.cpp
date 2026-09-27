#include "engines/EngineManager.h"
namespace solis {
bool EngineManager::initialize(){for(auto& [id,e]:engines_)if(!e->initialize())return false;return true;}
void EngineManager::shutdown(){for(auto& [id,e]:engines_)e->shutdown();}
bool EngineManager::registerEngine(std::unique_ptr<IEngine> e){if(!e)return false;auto id=e->info().id;if(id.empty()||engines_.contains(id))return false;engines_.emplace(id,std::move(e));if(activeEngineId_.empty())activeEngineId_=id;return true;}
bool EngineManager::selectEngine(const std::string& id){if(!engines_.contains(id))return false;activeEngineId_=id;return true;}
IEngine* EngineManager::activeEngine(){auto it=engines_.find(activeEngineId_);return it==engines_.end()?nullptr:it->second.get();}
}
