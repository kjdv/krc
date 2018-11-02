#pragma once

#include "runtime.hh"
#include <queue>
#include <vector>
#include "no_copy.hh"
#include "context.hh"

namespace krc {

class executor : private no_copy
{
public:
    static executor& instance();

    void dispatch(const std::function<void()>& target, size_t stack_size);

    void run(const std::function<void()>& target, size_t stack_size);

    void yield();

    routine_id get_id();

private:
    static executor s_instance;

    executor();
    ~executor();

    void next();
    void cleanup();
    void gc();

    context<>::handle d_main{nullptr};
    std::queue<context<>::handle> d_schedule;
    std::vector<context<>::handle> d_garbage;
};

} // namespace krc
