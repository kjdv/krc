#pragma once

#include "single_executor.hh"

namespace krc {

class executor : private internal::no_copy
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

    single_executor d_exec;
};

} // namespace krc
