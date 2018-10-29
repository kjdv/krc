#pragma once

#include "runtime.hh"
#include <memory>

namespace krc {

class executor
{
public:
    executor(const executor&) = delete;
    executor& operator=(const executor&) = delete;

    static executor& instance();

    void dispatch(const std::function<void()>& target, size_t stack_size);

    void run(const std::function<void()>& target, size_t stack_size);

    // returns true if control was yielded, false if this could not be done (no other routines waiting)
    bool yield();

private:
    executor();

    class impl;
    std::unique_ptr<impl> d_pimpl;

    static executor s_instance;
};

} // namespace krc
