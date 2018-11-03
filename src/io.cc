#include "io.hh"
#include <unistd.h>
#include <poll.h>
#include <executor.hh>

namespace krc {
namespace io {

ssize_t read(int fd, void *buf, size_t n)
{
    pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN | POLLHUP | POLLNVAL;

    auto &exec = executor::instance();

    int rc;
    while((rc = ::poll(&pfd, 1, 0)) == 0)
        exec.yield();

    if(rc > 0) // data ready
        return ::read(fd, buf, n);
    else  // poll error
        return -1;
}

ssize_t write(int fd, const void *buf, size_t n)
{
    pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLOUT | POLLHUP | POLLNVAL;

    auto &exec = executor::instance();

    int rc;
    while((rc = ::poll(&pfd, 1, 0)) == 0)
        exec.yield();

    if(rc > 0) // ready
        return ::write(fd, buf, n);
    else  // poll error
        return -1;
}



}
}
