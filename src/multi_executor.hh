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

    ~multi_executor();

    void dispatch(target_t target) override;

    void run(target_t target) override;

    void yield() override;

    routine_id get_id() const override;

private:
    struct sub {
        single_executor exec;
        std::thread thread;
    };

    const sub *this_sub() const;
    sub *this_sub();

    channel<target_t> d_dispatcher;
    sub d_main;
    std::vector<sub> d_subs;
};

}
