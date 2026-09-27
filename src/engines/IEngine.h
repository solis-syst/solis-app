#pragma once
#include "core/Module.h"
#include "core/Types.h"
#include <functional>
#include <string>
namespace solis {
struct EngineInfo { std::string id; std::string name; bool supportsUci=true; };
struct EngineRequest { std::string fen; int depth=5; int multiPv=1; };
using AnalysisCallback=std::function<void(const AnalysisLine&)>;
using AnalysisFinishedCallback=std::function<void(const AnalysisResult&)>;
class IEngine: public Module {
public:
 virtual ~IEngine()=default;
 virtual EngineInfo info() const=0;
 virtual bool setOption(const std::string&,const std::string&)=0;
 virtual bool analyze(const EngineRequest&,AnalysisCallback,AnalysisFinishedCallback)=0;
 virtual void stop()=0;
};
}
