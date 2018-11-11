#include <executor.hh>
#include <channel.hh>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <mutex>
#include <vector>

namespace krc {
namespace {

using testing::ElementsAre;
using testing::UnorderedElementsAre;

class collector
{
public:
    std::vector<int> items;

    void push(int i)
    {
        lock_t l(d_mut);
        items.push_back(i);
    }

    target_t::callable_t make_pusher(channel<int> &ch)
    {
        return [this, ch]() mutable {
            for (int i : ch)
                push(i);
        };
    }

private:
    typedef std::lock_guard<std::mutex> lock_t;
    mutable std::mutex d_mut;
};

struct multi_executor_test : public testing::Test
{
    executor &exec{executor::instance()};
    collector coll;
    channel<int> chan;
    channel<bool> done{100};

    target_t::callable_t make_consumer()
    {
        auto fn = coll.make_pusher(chan);
        return [this, fn] {
            fn();
            done.push(true);
        };
    }

    target_t::callable_t make_producer(int min, int max)
    {
        assert(max > min);
        return [this, min, max]{
            for(int i = min; i < max; ++i)
                this->chan.push(i);;
            this->done.push(true);
        };
    }

    void wait(unsigned n)
    {
        for(unsigned i = 0; i < n; ++i)
            done.pull();
        chan.close();
    }
};

TEST_F(multi_executor_test, one_task)
{
    auto sub = [this]{
        coll.push(1);
        coll.push(2);
        coll.push(3);
    };

    exec.run(sub, 2);

    EXPECT_THAT(coll.items, ElementsAre(1, 2, 3));
}

TEST_F(multi_executor_test, two_tasks)
{
    auto sub1 = make_consumer();
    auto sub2 = make_producer(0, 3);

    exec.run([&]{
        exec.dispatch(sub1);
        sub2();
        wait(1);
    }, 2);

    EXPECT_THAT(coll.items, ElementsAre(0, 1, 2));
}

TEST_F(multi_executor_test, multi_producer)
{
    auto sub1 = make_consumer();
    auto sub2 = make_producer(0, 3);
    auto sub3 = make_producer(3, 6);

    exec.run([&]{
        exec.dispatch(sub1);
        exec.dispatch(sub2);
        sub3();
        wait(2);
    }, 2);

    EXPECT_THAT(coll.items, UnorderedElementsAre(0, 1, 2, 3, 4, 5));
}

TEST_F(multi_executor_test, multi_producer_multi_consumer)
{
    auto sub1 = make_consumer();
    auto sub2 = make_consumer();
    auto sub3 = make_producer(0, 3);
    auto sub4 = make_producer(3, 6);

    exec.run([&]{
        exec.dispatch(sub1);
        exec.dispatch(sub2);
        exec.dispatch(sub3);
        sub4();
        wait(2);
    }, 2);

    EXPECT_THAT(coll.items, UnorderedElementsAre(0, 1, 2, 3, 4, 5));
}


}
}
