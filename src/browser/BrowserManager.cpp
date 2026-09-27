#include "browser/BrowserManager.h"
#include "browser/ManagedBrowserBootstrapper.h"
#include "browser/ManagedBrowserDownloader.h"

namespace solis {
bool BrowserManager::initialize(){
    ManagedBrowserBootstrapper bootstrapper;
    if(!bootstrapper.ensureInstalled(managedBrowserExecutable_)) return false;
    return session_?session_->initialize():true;
}
void BrowserManager::shutdown(){if(session_)session_->shutdown();}
bool BrowserManager::setSession(std::unique_ptr<IBrowserSession> s){if(!s)return false;if(session_)session_->shutdown();session_=std::move(s);return true;}
IBrowserSession* BrowserManager::session(){return session_.get();}
bool BrowserManager::downloadManagedBrowser(
    const std::string& url,
    const std::filesystem::path& destination) {
    ManagedBrowserDownloader downloader;
    return downloader.download(url, destination);
}
const std::filesystem::path& BrowserManager::managedBrowserExecutable() const {
    return managedBrowserExecutable_;
}
}
