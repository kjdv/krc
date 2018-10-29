#include <iostream>
#include <cstdlib>
#include <runtime.hh>
#include <channel.hh>

using namespace std;
using namespace krc;

namespace {

void print(channel<int> ch)
{
    while (true) {
        auto p = ch.pop();

        if (!p.has_value())
            return;

        cout << p.value() << endl;
    }
}

void fibonacci(int n)
{
    channel<int> ch;

    krc::dispatch([=]{ print(ch); });

    int a = 0;
    int b = 1;
    for (int i = 0; i < n; ++i) {
        ch.push(forward<int>(a));

        int t = a;
        a = b;
        b = t + b;
    }

    ch.close();
}

}

int main(int argc, char **argv)
{
    if (argc < 2) {
        cerr << "usage: " << argv[0] << " N" << endl;
        return 1;
    }

    int n = atoi(argv[1]);
    if (n < 1) {
        cerr << "please enter a number >= 0, not " << argv[1] << endl;
        return 1;
    }

    krc::run([=]{ fibonacci(n); });

    return 0;
}
