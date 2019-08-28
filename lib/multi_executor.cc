#include "multi_executor.hh"
#include "internal/utils.hh"

namespace krc {

using namespace std;
using internal::defer;

namespace {

// the purists will hate me, but semi-global (thread local) variables
// feel like the right thing, as the whole nature of this domain is that
// the stack can change underneath you, and calls to yield() or get_id() need
// to be directed to the right instance where the right instance is defined by
// the thread you are running on
thread_local single_executor *t_exec{nullptr};

void consume(channel<target_t> ch)
{
    assert(t_exec);
    for(auto&& t : ch)
        t_exec->dispatch(t);
}

void consume_runner(channel<target_t> ch)
{
    single_executor se;
    t_exec = &se;
    se.run([ch] { consume(ch); });
    t_exec = nullptr;
}

}

multi_executor::multi_executor(size_t num_threads)
    : d_num_threads{num_threads}
{
    assert(d_num_threads >= 2);
}

void multi_executor::dispatch(target_t target)
{
    assert(t_exec && "dispatch called from outside of a managed thread");
    t_exec->dispatch(move(target));
}

void multi_executor::run(target_t target)
{
    channel<target_t> dispatcher;

    vector<thread> subthreads;
    defer joiner{[&subthreads] {
        for(auto& st : subthreads)
            st.join();
    }};

    for(size_t i = 0; i < d_num_threads; ++i)
        subthreads.emplace_back([dispatcher] { consume_runner(dispatcher); });

    single_executor se;
    defer nuller{[] { t_exec = nullptr; }};
    t_exec = &se;

    auto wrapped = [dispatcher, &target]() mutable {
        t_exec->dispatch([dispatcher] {
            consume(dispatcher);
        });
        target.target();

        dispatcher.close();
    };

    se.run(target_t(wrapped, target.stack_size));
}

void multi_executor::yield()
{
    if(t_exec == nullptr)
        this_thread::yield(); // probably reasonable
    else
        t_exec->yield();
}

routine_id multi_executor::get_id() const
{
    return t_exec == nullptr ? no_routine_id : t_exec->get_id();
}

}
