#include "analysis/Analyzer.h"
namespace solis {
Analyzer::Analyzer(EngineManager& e,SiteManager& s):engines_(e),sites_(s){}
bool Analyzer::initialize(){return true;}
void Analyzer::shutdown(){}
bool Analyzer::analyzeCurrentPosition(const EngineRequest& request,AnalysisFinishedCallback done){
 auto* engine=engines_.activeEngine();
 if(!engine)return false;
 return engine->analyze(request,{},[this,done](const AnalysisResult& r){if(auto* site=sites_.activeSite())site->showAnalysis(r);if(done)done(r);});
}
}
