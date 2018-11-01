#pragma once

#include "runtime.hh"
#include <map>
#include <queue>
#include <ucontext.h>
#include <vector>
#include "no_copy.hh"

namespace krc {

class executor_old : private no_copy
{
public:
    static executor_old& instance();

    void dispatch(const std::function<void()>& target, size_t stack_size);

    void run(const std::function<void()>& target, size_t stack_size);

    // returns true if control was yielded, false if this could not be done (no other routines waiting)
    bool yield();

private:
    executor_old();

    static executor_old s_instance;

    void run();

    void next();

    void execute(int routine_id);

    ucontext_t d_main;

    std::queue<ucontext_t> d_routines;

    struct target_t
    {
        std::function<void()> target;
        std::vector<char>     stack;

        target_t(const std::function<void()>& target_, size_t stack_size);

        target_t(const target_t&) = delete;
        target_t& operator=(const target_t&) = delete;
    };

    std::map<int, target_t> d_targets;
};

} // namespace krc
