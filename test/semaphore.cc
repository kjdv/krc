#include "internal/semaphore.hh"
#include "internal/utils.hh"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <cassert>
#include <thread>

namespace krc {
namespace internal {
namespace {

using namespace std;

using testing::ElementsAre;
using testing::UnorderedElementsAre;

class exchange
{
public:
    void produce(string& item)
    {
        d_single_producer.wait();
        defer unlock{[this] { d_single_producer.notify(); }};

        assert(d_item == nullptr);
        d_item = &item;

        d_consume.notify();
        d_consume_done.wait();
    }

    string consume()
    {
        d_consume.wait();

        assert(d_item != nullptr);

        string result = *d_item;
        d_item = nullptr;

        d_consume_done.notify();

        return result;
    }

private:
    string *d_item{nullptr};

    binary_semaphore d_consume;
    binary_semaphore d_produce;
    binary_semaphore d_consume_done;

    binary_semaphore d_single_producer{true};
};

TEST(semaphore, exchange)
{
    exchange exch;
    thread p([&exch] {
        string s("foo");
        exch.produce(s);
    });

    EXPECT_EQ("foo", exch.consume());
    p.join();
}

TEST(semaphore, exchange_multi_producer)
{
    exchange exch;

    thread p1([&exch] {
        string s("foo");
        exch.produce(s);
    });
    thread p2([&exch] {
        string s("bar");
        exch.produce(s);
    });

    vector<string> items = {exch.consume(), exch.consume()};
    EXPECT_THAT(items, UnorderedElementsAre("foo", "bar"));

    p1.join();
    p2.join();
}


}
}
}
