#pragma once
#include "core/Module.h"
#include "engines/EngineManager.h"
#include "sites/SiteManager.h"
namespace solis {
class Analyzer: public Module {
public:
 Analyzer(EngineManager&,SiteManager&);
 bool initialize() override;
 void shutdown() override;
 bool analyzeCurrentPosition(const EngineRequest&,AnalysisFinishedCallback);
private:
 EngineManager& engines_;
 SiteManager& sites_;
};
}
