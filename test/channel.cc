#include <channel.hh>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <runtime.hh>
#include <vector>

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
    auto pop = [&] {
        while(true)
        {
            auto p = ch.pop();
            if(!p.has_value())
                return;

            items.push_back(p.value());
        }
    };

    krc::dispatch(push);
    krc::run(pop);

    EXPECT_THAT(items, ElementsAre(1, 2, 3));
}

TEST(channel, resolve_to_buffered_on_nonzero_queue_size)
{
    channel<int> ch(3);
    ch.push(1);
    ch.push(2);
    ch.push(3);

    EXPECT_EQ(1, ch.pop().value());
    EXPECT_EQ(2, ch.pop().value());
    EXPECT_EQ(3, ch.pop().value());
}

} // namespace
} // namespace krc
