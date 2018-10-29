#include <executor.hh>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace krc {
namespace {

using namespace std;
using namespace testing;

class executor_test : public Test
{
  public:
    vector<int> events;
    executor&   exec = executor::instance();

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
    exec.run(emitter(0, 3), DEFAULT_STACK_SIZE);
    EXPECT_THAT(events, ElementsAre(0, 1, 2));
}

TEST_F(executor_test, serial)
{
    exec.dispatch(emitter(0, 3), DEFAULT_STACK_SIZE);
    exec.run(emitter(3, 6), DEFAULT_STACK_SIZE);

    EXPECT_THAT(events, ElementsAre(0, 1, 2, 3, 4, 5));
}

TEST_F(executor_test, parallel)
{
    exec.dispatch(emitter(0, 3, true), DEFAULT_STACK_SIZE);
    exec.run(emitter(3, 6, true), DEFAULT_STACK_SIZE);

    EXPECT_THAT(events, ElementsAre(0, 3, 1, 4, 2, 5));
}

} // namespace
} // namespace krc
