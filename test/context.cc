#include <context.hh>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace krc {
namespace {

using testing::ElementsAre;

class context_test : public testing::Test
{
public:
    enum { stack_size = 1<<16 };

    void TearDown() override
    {
        for(auto h : d_handles)
            context<>::release(h);
    }

    context<>::handle make(const target_t target)
    {
        auto h = context<>::make(target, stack_size);
        d_handles.push_back(h);
        return h;
    }

    context<>::handle main()
    {
        auto h = context<>::main();
        d_handles.push_back(h);
        return h;
    }

    void swap(context<>::handle old_ctx, context<>::handle new_ctx)
    {
        context<>::swap(old_ctx, new_ctx);
    }

private:
    std::vector<context<>::handle> d_handles;
};

TEST_F(context_test, calls)
{
    std::vector<int> items;
    context<>::handle foo, bar, mn;

    foo = make([&]{
        items.push_back(1);
        swap(foo, bar);
        items.push_back(3);
        swap(foo, mn);
        assert(false && "never reached");
    });

    bar = make([&]{
        items.push_back(2);
        swap(bar, foo);
        assert(false && "never reached");
    });

    mn = main();

    swap(mn, foo);
    items.push_back(4);

    EXPECT_THAT(items, ElementsAre(1, 2, 3, 4));
}

}
}
