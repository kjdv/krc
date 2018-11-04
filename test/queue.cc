#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <internal/queue.hh>
#include <thread>
#include <vector>

namespace krc {
namespace {

TEST(queue, push_pull)
{
    queue<int> q(10);
    q.push(1);
    q.push(2);
    q.push(3);

    EXPECT_EQ(1, q.pull().value());
    EXPECT_EQ(2, q.pull().value());
    EXPECT_EQ(3, q.pull().value());
}

TEST(queue, push_beyond_max)
{
    enum { N = 10 };

    queue<int>       q(3);
    std::vector<int> items;

    std::thread t([&] {
        for(int i = 0; i < N; ++i)
            items.push_back(q.pull().value());
    });

    for(int i = 0; i < N; ++i)
        q.push(std::forward<int>(i));

    t.join();

    EXPECT_THAT(items, testing::ElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(queue, pull_on_closed_returns_none)
{
    enum { N = 10 };

    queue<int>                      q(3);
    std::vector<std::optional<int>> items;

    std::thread t([&] {
        for(int i = 0; i < N; ++i)
            items.push_back(q.pull());
    });

    q.push(1);
    q.push(2);
    q.push(3);

    q.close();

    t.join();

    std::optional<int> empty;
    EXPECT_THAT(items, testing::ElementsAre(1, 2, 3, empty, empty, empty, empty, empty, empty, empty));
}

TEST(queue, push_on_closed_returns_false)
{
    queue<int> q(10);

    EXPECT_TRUE(q.push(1));
    q.close();
    EXPECT_FALSE(q.push(2));
}

TEST(queue, size_indicator)
{
    queue<int> q(3);

    EXPECT_EQ(0, q.size());

    q.push(1);
    EXPECT_EQ(1, q.size());

    q.push(1);
    EXPECT_EQ(2, q.size());

    q.pull();
    EXPECT_EQ(1, q.size());

    q.pull();
    EXPECT_EQ(0, q.size());
}

TEST(queue, empty_indicator)
{
    queue<int> q(1);
    EXPECT_TRUE(q.empty());
    q.push(1);
    EXPECT_FALSE(q.empty());
    q.pull();
    EXPECT_TRUE(q.empty());
}

} // namespace
} // namespace krc
