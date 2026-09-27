#pragma once

namespace solis {

class Module {
public:
    virtual ~Module() = default;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
};

}
