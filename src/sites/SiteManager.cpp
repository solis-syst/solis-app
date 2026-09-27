#include "sites/SiteManager.h"
namespace solis {
bool SiteManager::initialize(){for(auto& [id,s]:sites_)if(!s->initialize())return false;return true;}
void SiteManager::shutdown(){for(auto& [id,s]:sites_)s->shutdown();}
bool SiteManager::registerSite(std::unique_ptr<ISiteAdapter> s){if(!s)return false;auto id=s->id();if(id.empty()||sites_.contains(id))return false;sites_.emplace(id,std::move(s));return true;}
bool SiteManager::selectSiteForUrl(const std::string& url){for(auto& [id,s]:sites_)if(s->matchesUrl(url)){activeSiteId_=id;return true;}activeSiteId_.clear();return false;}
ISiteAdapter* SiteManager::activeSite(){auto it=sites_.find(activeSiteId_);return it==sites_.end()?nullptr:it->second.get();}
}
