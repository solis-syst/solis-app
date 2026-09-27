#include "core/EventBus.h"
namespace solis {
void EventBus::subscribe(const std::string& e,Handler h){handlers_[e].push_back(std::move(h));}
void EventBus::publish(const std::string& e,const std::any& p){auto it=handlers_.find(e);if(it==handlers_.end())return;for(const auto& h:it->second)h(p);}
}
