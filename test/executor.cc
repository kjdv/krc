#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <executor.hh>

namespace krc {
namespace {

using namespace std;
using namespace testing;

TEST(executor, basic_run)
{
    vector<int> events;

    executor::instance().run([&]{
      for(int i = 0; i < 3; ++i)
        events.push_back(i);
    });

    EXPECT_THAT(events, ElementsAre(0, 1, 2));
}

}
}
