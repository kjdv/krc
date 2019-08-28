#include <channel.hh>
#include <cstdlib>
#include <iostream>
#include <runtime.hh>

using namespace std;
using namespace krc;

namespace {

void print(channel<int> ch)
{
    for(auto i : ch)
        cout << i << endl;
}

void fibonacci(int n)
{
    channel<int> ch;

    krc::dispatch([=] { print(ch); });

    int a = 0;
    int b = 1;
    for(int i = 0; i < n; ++i)
    {
        ch.push(a);

        b = b + a;
        a = b - a;
    }

    ch.close();
}

} // namespace

int main(int argc, char** argv)
{
    int n = argc < 2 ? 10 : atoi(argv[1]);
    if(n < 1)
    {
        cerr << "please enter a number >= 0, not " << argv[1] << endl;
        return 1;
    }

    krc::run([=] { fibonacci(n); });

    return 0;
}
