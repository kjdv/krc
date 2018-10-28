#pragma once

#include <mutex>

namespace krc {

class mutex
{
public:
    explicit mutex();

    void lock();

    void unlock();

    bool try_lock();

private:
    std::mutex d_base;
    bool d_held{false};
};

}
