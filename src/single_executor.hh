#pragma once

#include "context.hh"
#include "internal/no_copy.hh"
#include "runtime.hh"
#include <queue>
#include <vector>

namespace krc {

class single_executor : private internal::no_copy
{
public:
    explicit single_executor();
    ~single_executor();

    void dispatch(target_t target);

    void run(target_t target);

    void yield();

    routine_id get_id();

private:
    void next();
    void cleanup();
    void gc();

    context<>::handle              d_main{nullptr};
    std::queue<context<>::handle>  d_schedule;
    std::vector<context<>::handle> d_garbage;
};

}
