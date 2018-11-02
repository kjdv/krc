#include "io.hh"
#include <unistd.h>

namespace krc {
namespace io {

ssize_t read(int fd, void *buf, size_t n)
{
    return ::read(fd, buf, n);
}

ssize_t write(int fd, const void *buf, size_t n)
{
    return ::write(fd, buf, n);
}



}
}
