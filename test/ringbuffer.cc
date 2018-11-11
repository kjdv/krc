#include <internal/ringbuffer.hh>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace krc {
namespace internal {
namespace {

using namespace std;
using namespace testing;

TEST(ringbuffer_test, happy_push_pull)
{
    ringbuffer<int> rb(3);

    rb.push(1);
    rb.push(2);
    rb.push(3);

    EXPECT_EQ(1, rb.pull().value());
    EXPECT_EQ(2, rb.pull().value());
    EXPECT_EQ(3, rb.pull().value());
}

TEST(ringbuffer_test, flipover)
{
    ringbuffer<int> rb(3);

    rb.push(1);
    rb.push(2);
    rb.push(3);

    EXPECT_EQ(1, rb.pull().value());
    EXPECT_EQ(2, rb.pull().value());
    EXPECT_EQ(3, rb.pull().value());

    rb.push(4);
    rb.push(5);
    rb.push(6);

    EXPECT_EQ(4, rb.pull().value());
    EXPECT_EQ(5, rb.pull().value());
    EXPECT_EQ(6, rb.pull().value());
}

TEST(ringbuffer_test, close_disables_push)
{
    ringbuffer<int> rb(3);
    EXPECT_TRUE(rb.push(1));

    rb.close();
    EXPECT_FALSE(rb.push(2));
}

TEST(ringbuffer_test, close_disables_pull)
{
    ringbuffer<int> rb(3);
    rb.push(1);
    rb.close();

    EXPECT_EQ(1, rb.pull().value());
    EXPECT_FALSE(rb.pull().has_value());
}

TEST(ringbuffer_test, try_push_returns_closed_when_closed)
{
    using status = ringbuffer<int>::status;
    ringbuffer<int> rb(1);
    rb.close();
    EXPECT_EQ(status::closed, rb.try_push(1));
}

TEST(ringbuffer_test, try_push_returns_full_when_full)
{
    using status = ringbuffer<int>::status;
    ringbuffer<int> rb(1);
    rb.push(1);
    EXPECT_EQ(status::full, rb.try_push(2));
}

TEST(ringbuffer_test, try_push_returns_ok)
{
    using status = ringbuffer<int>::status;
    ringbuffer<int> rb(1);
    EXPECT_EQ(status::ok, rb.try_push(1));
}

TEST(ringbuffer_test, try_pull_returns_ok_and_closed)
{
    using status = ringbuffer<int>::status;
    ringbuffer<int> rb(3);
    rb.push(1);
    rb.push(2);
    rb.close();

    auto r = rb.try_pull();
    EXPECT_EQ(status::ok, r.second);
    EXPECT_EQ(1, r.first.value());

    r = rb.try_pull();
    EXPECT_EQ(status::ok, r.second);
    EXPECT_EQ(2, r.first.value());

    r = rb.try_pull();
    EXPECT_EQ(status::closed, r.second);
    EXPECT_FALSE(r.first.has_value());
}

TEST(ringbuffer_test, try_pull_returns_empty_when_empty)
{
    using status = ringbuffer<int>::status;
    ringbuffer<int> rb(1);

    auto r = rb.try_pull();
    EXPECT_EQ(status::empty, r.second);
    EXPECT_FALSE(r.first.has_value());
}

}
}
}
