#include "browser/BrowserManager.h"
namespace solis {
bool BrowserManager::initialize(){return session_?session_->initialize():true;}
void BrowserManager::shutdown(){if(session_)session_->shutdown();}
bool BrowserManager::setSession(std::unique_ptr<IBrowserSession> s){if(!s)return false;if(session_)session_->shutdown();session_=std::move(s);return true;}
IBrowserSession* BrowserManager::session(){return session_.get();}
}
