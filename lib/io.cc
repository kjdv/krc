#include "io.hh"
#include "runtime.hh"
#include <poll.h>
#include <unistd.h>

namespace krc {
namespace io {

ssize_t read(int fd, void* buf, size_t n)
{
    pollfd pfd;
    pfd.fd     = fd;
    pfd.events = POLLIN | POLLNVAL;

    int rc;
    while((rc = ::poll(&pfd, 1, 0)) == 0)
        krc::yield();

    if(rc > 0) // data ready
        return ::read(fd, buf, n);
    else // poll error
        return -1;
}

ssize_t write(int fd, const void* buf, size_t n)
{
    pollfd pfd;
    pfd.fd     = fd;
    pfd.events = POLLOUT | POLLNVAL;

    int rc;
    while((rc = ::poll(&pfd, 1, 0)) == 0)
        krc::yield();

    if(rc > 0) // ready
        return ::write(fd, buf, n);
    else // poll error
        return -1;
}

} // namespace io
} // namespace krc
