#pragma once

#include <atomic>

namespace krc {

class mutex
{
public:
    void lock();

    void unlock();

    bool try_lock();

private:
    std::atomic<bool> d_held{false};
};

} // namespace krc
