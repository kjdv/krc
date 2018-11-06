#include <executor.hh>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <mutex>
#include <vector>
#include "debug.hh"

namespace krc {
namespace {

using testing::ElementsAre;
using testing::UnorderedElementsAre;

class collector
{
public:
    std::vector<int> items;

    void push(int i)
    {
        lock_t l(d_mut);
        items.push_back(i);
    }

private:
    typedef std::lock_guard<std::mutex> lock_t;
    mutable std::mutex d_mut;
};

TEST(multi_executor, one_task) {
    auto& exec = executor::instance();
    collector c;

    auto sub = [&c]{
        debug("starting sub");
        c.push(1);
        c.push(2);
        c.push(3);

        debug("ending sub");
    };

    exec.run(sub, 2);

    EXPECT_THAT(c.items, ElementsAre(1, 2, 3));
}

}
}
