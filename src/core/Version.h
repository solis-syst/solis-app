#pragma once

#include <string>

namespace solis {

class Version {
public:
    static std::string current();
    static int compare(const std::string& left, const std::string& right);
};

}
