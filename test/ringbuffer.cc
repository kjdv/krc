#include <ringbuffer.hh>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace krc {
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

}
}
