#include "multi_executor.hh"

namespace krc {

using namespace std;

namespace {

void consume(channel<target_t> ch, single_executor &exec)
{
    for(auto&& t : ch)
        exec.dispatch(t);
}

void consume_runner(channel<target_t> ch, single_executor &exec)
{
    exec.run([ch, &exec] { consume(ch, exec); });
}

struct defer
{
    std::function<void()> func;
    ~defer()
    {
        func();
    }
};

}

multi_executor::multi_executor(size_t num_threads)
    : d_subs(num_threads - 1)
{
    assert(num_threads >= 2);

    for(auto& s : d_subs)
    {
        single_executor &se = s.exec;
        s.thread = thread([this, &se] { consume_runner(d_dispatcher, se); });
    }
}

multi_executor::~multi_executor()
{
    for(auto& s: d_subs)
    {
        if(s.thread.joinable())
          s.thread.join();
    }
}

void multi_executor::dispatch(target_t target)
{
    d_dispatcher.push(move(target));
}

void multi_executor::run(target_t target)
{
    auto wrapped = [this, &target] {
        d_main.exec.dispatch([this] {
            consume(d_dispatcher, d_main.exec);
        });
        target.target();
        d_dispatcher.close();

        for (auto&& s : d_subs)
            s.thread.join();
    };

    d_main.exec.run(target_t(wrapped, target.stack_size));
}

void multi_executor::yield()
{
    auto s = this_sub();
    if(s == nullptr)
        this_thread::yield(); // probably reasonable
    else
        s->exec.yield();
}

routine_id multi_executor::get_id() const
{
    auto s = this_sub();
    return s == nullptr ? no_routine_id : s->exec.get_id();
}

const multi_executor::sub *multi_executor::this_sub() const
{
    auto this_id = this_thread::get_id();
    if (d_main.thread.get_id() == this_id)
        return &d_main;

    // assume the number of subthreads is small, no need to get smarter than linear search
    auto it = std::find_if(d_subs.begin(), d_subs.end(), [this_id](auto&& sub) {
        return sub.thread.get_id() == this_id;
    });

    return it == d_subs.end() ? nullptr : &(*it);
}

multi_executor::sub *multi_executor::this_sub()
{
    // Scott Meyers item 3
    return const_cast<sub *>(static_cast<const multi_executor &>(*this).this_sub());
}

}
