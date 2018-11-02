#include <context.hh>
#include <iostream>
#include <cassert>

using namespace std;
using namespace krc;

namespace {

context<>::handle main_ctx, foo_ctx, bar_ctx;

void foo()
{
    cout << "enter foo" << " on " << context<>::get_id() << endl;
    cout << "switching to bar" << endl;
    context<>::swap(foo_ctx, bar_ctx);
    cout << "done with foo" << " on " << context<>::get_id() << endl;
    cout << "switching to main" << endl;
    context<>::set(main_ctx);

    assert(false && "never reached");
}

void bar()
{
    cout << "enter bar" << " on " << context<>::get_id() << endl;
    cout << "switching to foo" << endl;
    context<>::set(foo_ctx);

    assert(false && "never reached");
}

}

int main()
{
    enum {stack_size = 1<<16};

    main_ctx = context<>::main();
    foo_ctx = context<>::make(foo, stack_size);
    bar_ctx = context<>::make(bar, stack_size);

    cout << "switching to foo" << " on " << context<>::get_id() << endl;
    context<>::swap(main_ctx, foo_ctx);
    cout << "back to main" << " on " << context<>::get_id() << endl;

    cout << "freeing up" << endl;
    context<>::release(bar_ctx);
    context<>::release(foo_ctx);
    context<>::release(main_ctx);

    cout << "all done" << endl;

    return 0;
}