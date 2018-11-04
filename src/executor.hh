#pragma once

#include "single_executor.hh"
#include <thread>
#include <vector>

namespace krc {

class executor : private internal::no_copy
{
public:
    static executor& instance();

    void dispatch(const target_t &target);

    void run(const target_t &target);

    void yield();

    routine_id get_id();

private:
    static executor s_instance;

    executor();

    single_executor d_exec;

    struct exec_info
    {
        single_executor exec;
        std::thread thread;
    };

    std::vector<exec_info> d_executors;

    struct pending_target
    {
        std::function<void()> target;
        size_t stack_size;
    };
};

} // namespace krc
