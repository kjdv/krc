#include <io.hh>
#include <runtime.hh>
#include <unistd.h>
#include <sys/socket.h>
#include <thread>
#include <iostream>

using namespace std;
using namespace krc;

namespace {

struct closer {
    int fd;
    ~closer()
    {
        ::close(fd);
    }
};

// echo whatever is send, slowly
void echo(int fd)
{
    enum { bufsize = 1024 };
    char buf[bufsize];

    while(true)
    {
        auto r = ::read(fd, buf, bufsize);
        if (r < 0)
            return;

        this_thread::sleep_for(chrono::seconds(1));


        ssize_t w = 0;
        while(r > 0)
        {
            auto t = ::write(fd, buf + w, r);
            if (t <= 0)
                break;
            w += t;
            r -= t;
        }
    }
}

int gcd(int a , int b)
{
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

}

int main()
{
    // create a socketpair
    int fds[2];
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fds) != 0)
    {
        cerr << "could not open socektpair" << endl;
        return 1;
    }

    thread echo_server([&]{
        int fd = fds[1];
        closer c{fd};
        echo(fd);
    });

    echo_server.join();

    return 0;
}
