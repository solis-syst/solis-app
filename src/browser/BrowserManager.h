#pragma once
#include "browser/IBrowserSession.h"
#include "core/Module.h"
#include <filesystem>
#include <memory>
#include <string>
namespace solis {
class BrowserManager: public Module {
public:
 bool initialize() override;
 void shutdown() override;
 bool setSession(std::unique_ptr<IBrowserSession>);
 IBrowserSession* session();
 bool downloadManagedBrowser(const std::string&, const std::filesystem::path&);
private:
 std::unique_ptr<IBrowserSession> session_;
};
}
