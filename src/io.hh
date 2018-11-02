#pragma once

#include <sys/types.h>

namespace krc {
namespace io{

// variants of the well-known system calls that instead of blocking
// or returning will yield control to another routine if they
// can't complete inmidediately.
// todo: just read() and write() for now, extend with socket connects
//       and such later

ssize_t read(int fd, void* buf, size_t n);

ssize_t write(int fd, const void *buf, size_t n);

}
}
