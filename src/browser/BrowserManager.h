#pragma once
#include "browser/IBrowserSession.h"
#include "core/Module.h"
#include <memory>
namespace solis {
class BrowserManager: public Module {
public:
 bool initialize() override;
 void shutdown() override;
 bool setSession(std::unique_ptr<IBrowserSession>);
 IBrowserSession* session();
private:
 std::unique_ptr<IBrowserSession> session_;
};
}
