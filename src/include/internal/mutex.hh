#pragma once

#include "no_copy.hh"
#include <atomic>

namespace krc {
namespace internal {

class spinlock : private no_copy
{
public:
    explicit spinlock();

    void lock();

    void unlock();

    bool try_lock();

private:
    std::atomic_flag d_lock;
};

class mutex : private no_copy
{
public:
    void lock();

    void unlock();

    bool try_lock();

private:
    spinlock d_lock;
};

} // namespace internal
} // namespace krc
