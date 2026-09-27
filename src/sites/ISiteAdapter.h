#pragma once
#include "core/Module.h"
#include "core/Types.h"
#include <string>
namespace solis {
class ISiteAdapter: public Module {
public:
 virtual ~ISiteAdapter()=default;
 virtual std::string id() const=0;
 virtual bool matchesUrl(const std::string&) const=0;
 virtual bool getPosition(Position&)=0;
 virtual bool showAnalysis(const AnalysisResult&)=0;
 virtual void clearAnalysis()=0;
};
}
