#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <zero_queue.hh>

namespace krc {
namespace {

using namespace std;
using namespace testing;

class zero_queue_test : public Test
{
public:
  vector<int>     sink;
  zero_queue<int> zq;

  void SetUp() override
  {
    d_thr = thread([=] {
      while(!zq.closed())
      {
        auto p = zq.pop();
        sink.push_back(p.value());
      }
    });
  }

  void TearDown() override
  {
    join();
  }

  void join()
  {
    zq.close();
    if(d_thr.joinable())
      d_thr.join();
  }

private:
  thread d_thr;
};

TEST_F(zero_queue_test, push_pop)
{
  zq.push(1);
  zq.push(2);
  zq.push(3);

  join();

  EXPECT_THAT(sink, ElementsAre(1, 2, 3));
}

TEST_F(zero_queue_test, push_on_closed_raises)
{
  zq.close();
  EXPECT_THROW(zq.push(1), channel_closed);
}

TEST_F(zero_queue_test, closed_indicator)
{
  zq.push(1);
  zq.push(2);
  zq.close();

  join();

  EXPECT_TRUE(zq.closed());
  EXPECT_THAT(sink, ElementsAre(1, 2));
}

} // namespace
} // namespace krc
