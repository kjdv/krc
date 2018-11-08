#pragma once

#include "single_executor.hh"
#include <thread>
#include <vector>

namespace krc {

class executor : private internal::no_copy
{
public:
    static executor& instance();

    void dispatch(target_t target);

    void run(target_t target, size_t num_threads);

    void yield();

    routine_id get_id();

private:
    void run_single(target_t target);
    void run_multi(target_t target, size_t num_threads);

    static executor s_instance;

    executor();

    std::function<void(target_t)> d_dispatcher;
};

} // namespace krc
