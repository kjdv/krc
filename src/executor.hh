#pragma once

#include "runtime.hh"
#include "internal/no_copy.hh"
#include <thread>
#include <vector>

namespace krc {

class executor_impl : private internal::no_copy
{
public:
    virtual ~executor_impl() = default;

    virtual void dispatch(target_t target) = 0;

    virtual void run(target_t target) = 0;

    virtual void yield() = 0;

    virtual routine_id get_id() const = 0;
};

class executor : private internal::no_copy
{
public:
    static executor& instance();

    void dispatch(target_t target);

    void run(target_t target, size_t num_threads);

    void yield();

    routine_id get_id() const;

private:
    static executor s_instance;

    executor();

    std::unique_ptr<executor_impl> d_delegate;
};

} // namespace krc
