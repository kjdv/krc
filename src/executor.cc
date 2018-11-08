#include "executor.hh"
#include <cassert>
#include <channel.hh>
#include "debug.hh"

using namespace std;

namespace krc {
namespace {

// the purists will hate this use of singletons, but for this domain with
// stacks changing underneath you and the (single) executor having by its
// nature an effect on a global or thread scope it seems like the right thing
thread_local single_executor *t_exec{nullptr};

struct defer
{
    std::function<void()> func;
    ~defer()
    {
        func();
    }
};

void consume(channel<target_t> ch)
{
    assert(t_exec);
    for(auto& item : ch)
        t_exec->dispatch(item);
    debug("done consuming");
}

}

executor executor::s_instance;

executor& executor::instance()
{
    return s_instance;
}

void executor::dispatch(const target_t &target)
{
    assert(d_dispatcher && "called from outside a krc execution scope");
    d_dispatcher(target);
}

void executor::run(const target_t &target, size_t num_threads)
{
    assert(t_exec == nullptr && "run() called more than once");
    assert(num_threads > 0 && "need at least one thread");

    if(num_threads == 1)
        run_single(target);
    else
        run_multi(target, num_threads);
}

void executor::run_single(const target_t &target)
{
    single_executor se;
    t_exec = &se;
    d_dispatcher = [](const target_t &item) {
        assert(t_exec != nullptr);
        t_exec->dispatch(item);
    };

    defer cleanup{[this] {
            d_dispatcher = std::function<void(target_t)>();
            t_exec = nullptr;
    }};

    se.run(target);
}

void executor::run_multi(const target_t &target, size_t num_threads)
{
    assert(num_threads >= 2);

    channel<target_t> dispatch_channel;
    vector<thread> subthreads;
    defer joiner{[&subthreads]{
        debug("joining threads");
        for(auto&& t : subthreads)
            t.join();
    }};

    for (size_t i = 0; i < num_threads - 1; ++i)
    {
        subthreads.emplace_back([&dispatch_channel]{
            defer cleanup {[]{
                    debug("setting sub to null");
                    t_exec = nullptr;
            }};
            single_executor se;
            t_exec = &se;

            debug(string("run on ") + to_string((intptr_t)t_exec));
            se.run([&dispatch_channel] { consume(dispatch_channel); });
            t_exec = nullptr;
        });
    }

    single_executor se;
    t_exec = &se;
    d_dispatcher = [&dispatch_channel](target_t item) {
        debug("dispatching");
        dispatch_channel.push(move(item));
        debug("done dispatching");
    };

    defer cleanup{[this] {
           debug("setting main to null");
           d_dispatcher = std::function<void(target_t)>();
           t_exec = nullptr;
    }};

    auto wrapped = [&target, &dispatch_channel] {
        debug("main");
        debug(string("run on ") + to_string((intptr_t)t_exec));
        target.target();
        debug("done with main");
        dispatch_channel.close();
        debug("fully done");
    };

    se.run(target_t(wrapped, target.stack_size));
    debug("unwinding");
}

void executor::yield()
{
    debug("yielding");
    if (t_exec != nullptr)
        t_exec->yield();
    else
        this_thread::yield();
    debug("done yield");
}

routine_id executor::get_id()
{
    return t_exec ? t_exec->get_id() : no_routine_id;
}

executor::executor()
{}

} // namespace krc
