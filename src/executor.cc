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

struct deferA
{
    std::function<void()> func;
    ~deferA()
    {
        func();
    }
};

void consume(channel<target_t> ch)
{
    assert(t_exec);
    debug("start consuming on " + to_string((uintptr_t)t_exec));
    for(auto& item : ch)
        t_exec->dispatch(item);
    debug("done consuming "  + to_string((uintptr_t)t_exec));
}

void run_consumer(channel<target_t> ch)
{
    //defer cleanup {[]{
    //        debug("setting sub to null");
    //        t_exec = nullptr;
    //}};
    single_executor se;
    t_exec = &se;

    debug(string("run on ") + to_string((intptr_t)t_exec));
    se.run([ch] { consume(ch); });

    debug("setting sub to null");
    //t_exec = nullptr;
}

}

executor executor::s_instance;

executor& executor::instance()
{
    return s_instance;
}

void executor::dispatch(target_t target)
{
    assert(d_dispatcher && "called from outside a krc execution scope");
    d_dispatcher(move(target));
}

void executor::run(target_t target, size_t num_threads)
{
    assert(t_exec == nullptr && "run() called more than once");
    assert(num_threads > 0 && "need at least one thread");

    if(num_threads == 1)
        run_single(move(target));
    else
        run_multi(move(target), num_threads);
}

void executor::run_single(target_t target)
{
    single_executor se;
    d_dispatcher = [&se](target_t item) {
        assert(t_exec != nullptr);
        t_exec->dispatch(move(item));
    };

    se.run(move(target));
    d_dispatcher = std::function<void(target_t)>();
    t_exec = nullptr;
}

void executor::run_multi(target_t target, size_t num_threads)
{
    assert(num_threads >= 2);

    channel<target_t> dispatch_channel;
    vector<thread> subthreads;

    for (size_t i = 0; i < num_threads - 1; ++i)
        subthreads.emplace_back([dispatch_channel] { run_consumer(dispatch_channel); });

    single_executor se;
    t_exec = &se;
    d_dispatcher = [&dispatch_channel](target_t item) {
        debug("dispatching");
        dispatch_channel.push(move(item));
        debug("done dispatching");
    };

    auto wrapped = [fn = move(target.target), &dispatch_channel] {
        debug("main");
        debug(string("run on ") + to_string((intptr_t)t_exec));
        fn();
        debug("done with main");
        dispatch_channel.close();
        debug("fully done");
    };

    se.run(target_t(wrapped, target.stack_size));
    debug("unwinding");

    for(auto&& t : subthreads)
        t.join();

    d_dispatcher = std::function<void(target_t)>();
   // t_exec = nullptr;
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
