#include <iostream>
#include <runtime.hh>

using namespace krc;
using namespace std;

namespace {

void bar()
{
    for(int i = 0; i < 10; ++i)
    {
        cout << "bar " << i << endl;
        krc::yield();
    }

    cout << "bar done" << endl;
}

void foo()
{
    krc::dispatch(bar);

    for(int i = 0; i < 10; ++i)
    {
        cout << "foo " << i << endl;
        krc::yield();
    }

    cout << "foo done" << endl;
}

} // namespace

int main()
{
    krc::run(foo);

    cout << "all done" << endl;

    return 0;
}
