#pragma once

#include "context.hh"
#include "internal/no_copy.hh"
#include "executor.hh"
#include <queue>
#include <vector>

namespace krc {

class single_executor : public executor_impl
{
public:
    explicit single_executor();
    ~single_executor();

    void dispatch(target_t target) override;

    void run(target_t target) override;

    void yield() override;

    routine_id get_id() const override;

private:
    void next();
    void cleanup();
    void gc();

    context<>::handle              d_main{nullptr};
    std::queue<context<>::handle>  d_schedule;
    std::vector<context<>::handle> d_garbage;
};

}
