#pragma once

#include <atomic>
#include "no_copy.hh"

namespace krc {

class mutex : private no_copy
{
public:
    void lock();

    void unlock();

    bool try_lock();

private:
    std::atomic<bool> d_held{false};
};

} // namespace krc
