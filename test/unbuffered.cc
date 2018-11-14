#include "internal/unbuffered.hh"
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

TEST(unbuffered, exchange)
{
    unbuffered<string> exch;
    thread p([&exch] {
        string s("foo");
        exch.push(s);
    });
    defer join{[&] { p.join(); }};

    EXPECT_EQ("foo", exch.pull().value());
}

TEST(unbuffered, exchange_multi_producer)
{
    unbuffered<string> exch;

    thread p1([&exch] {
        string s("foo");
        exch.push(s);
    });
    thread p2([&exch] {
        string s("bar");
        exch.push(s);
    });
    defer join{[&] {
        p1.join();
        p2.join();
    }};

    vector<string> items = {exch.pull().value(), exch.pull().value()};
    EXPECT_THAT(items, UnorderedElementsAre("foo", "bar"));
}

TEST(unbuffered, exchange_multi_producer_multi_consumer)
{
    unbuffered<string> exch;

    string s1, s2, s3;

    {
        thread p1([&exch] {
            string s("foo");
            exch.push(s);
        });
        thread p2([&exch] {
            string s("bar");
            exch.push(s);
        });
        thread p3([&exch] {
            string s("baz");
            exch.push(s);
        });

        thread c1([&s1, &exch] {
            s1 = exch.pull().value();
        });
        thread c2([&s2, &exch] {
            s2 = exch.pull().value();
        });
        thread c3([&s3, &exch] {
            s3 = exch.pull().value();
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

TEST(unbuffered, close_means_push_returns_false)
{
    unbuffered<string> ub;
    ub.close();

    EXPECT_FALSE(ub.push(string()));
}


TEST(unbuffered, close_means_pull_returns_no_value)
{
    unbuffered<string> ub;
    ub.close();

    EXPECT_FALSE(ub.pull().has_value());
}

TEST(unbuffered, close_cancels_pending_push)
{
    unbuffered<string> ub;

    thread t([&ub] {
        ub.push(string()); // note: return value is undefined
    });
    defer join{[&t] { t.join(); }};

    ub.close();
}

TEST(unbuffered, close_cancels_pending_pull)
{
    unbuffered<string> ub;

    thread t([&ub] {
        EXPECT_FALSE(ub.pull().has_value());
    });
    defer join{[&t] { t.join(); }};

    ub.close();
}

}
}
}
