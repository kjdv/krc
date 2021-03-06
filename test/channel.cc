#include <channel.hh>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <runtime.hh>
#include <vector>
#include <debug.hh>

namespace krc {
namespace {

using namespace std;
using namespace testing;

TEST(channel, resolved_to_unbuffered_by_default)
{
    channel<int> ch;

    vector<int> items;

    auto push = [&] {
        ch.push(1);
        ch.push(2);
        ch.push(3);
        ch.close();
    };
    auto pull = [&] {
        krc::dispatch(push);

        for(auto p : ch)
            items.push_back(p);
    };

    krc::run(pull);

    EXPECT_THAT(items, ElementsAre(1, 2, 3));
}

TEST(channel, resolve_to_buffered_on_nonzero_queue_size)
{
    channel<int> ch(3);
    ch.push(1);
    ch.push(2);
    ch.push(3);

    EXPECT_EQ(1, *ch.pull());
    EXPECT_EQ(2, *ch.pull());
    EXPECT_EQ(3, *ch.pull());
}

TEST(channel, range_support)
{
    channel<int> ch(3);
    ch.push(1);
    ch.push(2);
    ch.push(3);
    ch.close();

    vector<int> items;
    for(auto i : ch)
        items.push_back(i);

    EXPECT_THAT(items, ElementsAre(1, 2, 3));
}

} // namespace
} // namespace krc
