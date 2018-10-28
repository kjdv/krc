#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <executor.hh>

namespace krc {
namespace {

using namespace std;
using namespace testing;

class executor_test : public Test
{
public:
  vector<int> events;
  executor &exec = executor::instance();

  std::function<void()> emitter(int start, int end, bool interrupt = false)
  {
    return [=] {
      for(int i = start; i < end; ++i)
      {
        events.push_back(i);

        if(interrupt)
          exec.yield();
      }
    };
  }
};

TEST_F(executor_test, basic_run)
{
  exec.run(emitter(0, 3));
  EXPECT_THAT(events, ElementsAre(0, 1, 2));
}

TEST_F(executor_test, serial)
{
  exec.push(emitter(0, 3));
  exec.push(emitter(3, 6));
  exec.run();

  EXPECT_THAT(events, ElementsAre(0, 1, 2, 3, 4, 5));
}

TEST_F(executor_test, parallel)
{
  exec.push(emitter(0, 3, true));
  exec.push(emitter(3, 6, true));
  exec.run();

  EXPECT_THAT(events, ElementsAre(0, 3, 1, 4, 2, 5));
}

}
}
