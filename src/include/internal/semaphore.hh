#pragma once

#include <atomic>

namespace krc {
namespace internal {

class binary_semaphore
{
public:
    explicit binary_semaphore(bool init = false);

    void wait();

    void notify();

private:
    std::atomic_char d_notified;
};

}
}
