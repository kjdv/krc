#pragma once

#include <vector>
#include <thread>

#include "executor.hh"
#include "single_executor.hh"
#include <channel.hh>

namespace krc {

class multi_executor : public executor_impl
{
public:
    explicit multi_executor(size_t num_threads);

    void dispatch(target_t target) override;

    void run(target_t target) override;

    void yield() override;

    routine_id get_id() const override;

private:
    size_t d_num_threads;
};

}
