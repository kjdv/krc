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

    bool try_wait();
private:
    std::atomic_char d_notified;
};

// small kludge: closable binary semaphore
class closeable_binary_semaphore
{
public:
    explicit closeable_binary_semaphore(bool init = false);

    void wait();

    void notify();

    void close();

    bool is_closed() const;

private:
    binary_semaphore d_sem;
    std::atomic<bool> d_closed{false};
};

}
}
