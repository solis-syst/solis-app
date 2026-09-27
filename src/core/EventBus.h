#pragma once
#include <any>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
namespace solis {
class EventBus {
public:
 using Handler=std::function<void(const std::any&)>;
 void subscribe(const std::string&,Handler);
 void publish(const std::string&,const std::any& payload={});
private:
 std::unordered_map<std::string,std::vector<Handler>> handlers_;
};
}
