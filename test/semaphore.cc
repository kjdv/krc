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

        string result(std::move(*d_item));
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
    defer join{[&] { p.join(); }};

    EXPECT_EQ("foo", exch.consume());
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
    defer join{[&] {
        p1.join();
        p2.join();
    }};

    vector<string> items = {exch.consume(), exch.consume()};
    EXPECT_THAT(items, UnorderedElementsAre("foo", "bar"));
}

TEST(semaphore, exchange_multi_producer_multi_consumer)
{
    exchange exch;

    string s1, s2, s3;

    {
        thread p1([&exch] {
            string s("foo");
            exch.produce(s);
        });
        thread p2([&exch] {
            string s("bar");
            exch.produce(s);
        });
        thread p3([&exch] {
            string s("baz");
            exch.produce(s);
        });

        thread c1([&s1, &exch] {
            s1 = exch.consume();
        });
        thread c2([&s2, &exch] {
            s2 = exch.consume();
        });
        thread c3([&s3, &exch] {
            s3 = exch.consume();
        });

        c1.join();
        c2.join();
        c3.join();
        p1.join();
        p2.join();
        p3.join();
    }

    EXPECT_THAT(vector<string>({s1, s2, s3}), UnorderedElementsAre("foo", "bar", "baz"));
}


}
}
}
