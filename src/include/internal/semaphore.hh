#pragma once

#include <atomic>

namespace krc {
namespace internal {

class semaphore
{
public:
    explicit semaphore(int count = 0);

    void wait();

    bool try_wait();

    void notify();

private:
    std::atomic<int> d_count;
};

}
}
