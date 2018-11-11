#include <executor.hh>
#include <channel.hh>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <mutex>
#include <vector>
#include "debug.hh"

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
        debug("consume " + std::to_string(i));
        items.push_back(i);
    }

    target_t::callable_t make_pusher(channel<int> &ch)
    {
        return [this, ch]() mutable {
            for (int i : ch)
                push(i);
            debug("done consuming");
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

    target_t::callable_t make_consumer()
    {
        return coll.make_pusher(chan);
    }

    target_t::callable_t make_producer(int max)
    {
        return [this, max]{
            for(int i = 0; i < max; ++i)
                this->chan.push(i);;
            this->chan.close();
        };
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
    auto sub2 = make_producer(3);

    exec.run([&]{
        exec.dispatch(sub1);
        sub2();
    }, 2);

    EXPECT_THAT(coll.items, ElementsAre(0, 1, 2));
}

}
}
