#pragma once

#include <functional>
#include <mutex>
#include "mutex.hh"

namespace krc {
namespace internal {

class condition_variable
{
public:
    typedef std::unique_lock<krc::internal::mutex> lock_t;
    typedef std::function<bool()> predicate_t;

    void wait(lock_t &l, const predicate_t &p);

    void notify_one()
    {} // n/a for busy waits

    void notify_all()
    {} // n/a for busy waits

private:
    void wait(lock_t &l);
};

}
}
