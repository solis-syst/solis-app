#pragma once
#include "core/Module.h"
#include <string>
namespace solis {
class IBrowserSession: public Module {
public:
 virtual ~IBrowserSession()=default;
 virtual bool navigate(const std::string&)=0;
 virtual std::string currentUrl() const=0;
 virtual std::string executeJavaScript(const std::string&)=0;
 virtual bool attachDebugger()=0;
 virtual void detachDebugger()=0;
};
}
