#pragma once
#include "core/Module.h"
#include "sites/ISiteAdapter.h"
#include <memory>
#include <string>
#include <unordered_map>
namespace solis {
class SiteManager: public Module {
public:
 bool initialize() override;
 void shutdown() override;
 bool registerSite(std::unique_ptr<ISiteAdapter>);
 bool selectSiteForUrl(const std::string&);
 ISiteAdapter* activeSite();
private:
 std::unordered_map<std::string,std::unique_ptr<ISiteAdapter>> sites_;
 std::string activeSiteId_;
};
}
