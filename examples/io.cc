#include <channel.hh>
#include <io.hh>
#include <runtime.hh>

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

using namespace std;
using namespace krc;

namespace {

enum { bufsize = 1024 };

chrono::duration sleep_time = chrono::milliseconds(10);

struct closer
{
    int fd;
    ~closer()
    {
        ::close(fd);
    }
};

// echo whatever is send, slowly
void echo(int fd)
{
    closer c{fd};
    char   buf[bufsize];

    while(true)
    {
        auto r = ::read(fd, buf, bufsize);
        if(r <= 0)
            return;

        this_thread::sleep_for(sleep_time);

        ssize_t w = ::write(fd, buf, r);
        if(w <= 0)
            break;
    }
}

void do_parallel_work()
{
    for(int i = 0; i < 10; ++i)
    {
        cout << "parallel work continues" << endl;
        yield();
        this_thread::sleep_for(sleep_time);
    }
}

void do_io_work(int fd)
{
    dispatch(do_parallel_work);

    closer c{fd};

    for(int i = 0; i < 10; ++i)
    {
        string msg = string("message number ") + to_string(i);

        auto w = io::write(fd, msg.data(), msg.size());
        assert(w == msg.size());

        char buf[bufsize];
        int  r = io::read(fd, buf, bufsize);
        if(r > 0)
        {
            string reply(buf, buf + r);
            cout << "reply: " << reply << endl;
        }
    }
}

} // namespace

int main()
{
    // create a socketpair
    int fds[2];
    if(socketpair(AF_LOCAL, SOCK_STREAM, 0, fds) != 0)
    {
        cerr << "could not open socektpair" << endl;
        return 1;
    }

    thread echo_thread([&] { echo(fds[1]); });

    run([&] { do_io_work(fds[0]); });

    echo_thread.join();

    return 0;
}
