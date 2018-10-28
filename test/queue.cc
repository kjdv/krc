#include <queue.hh>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <vector>

namespace krc {
namespace {

TEST(queue, push_pop)
{
  queue<int> q(10);
  q.push(1);
  q.push(2);
  q.push(3);

  EXPECT_EQ(1, q.pop().value());
  EXPECT_EQ(2, q.pop().value());
  EXPECT_EQ(3, q.pop().value());
}

TEST(queue, push_beyond_max)
{
  enum { N = 10 };

  queue<int> q(3);
  std::vector<int> items;

  std::thread t([&]{
    for (int i = 0; i < N; ++i)
      items.push_back(q.pop().value());
  });

  for(int i = 0; i < N; ++i)
    q.push(std::forward<int>(i));

  t.join();

  EXPECT_THAT(items, testing::ElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
}

}
}
