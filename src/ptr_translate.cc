#include "ptr_translate.hh"
#include <cstring>

namespace krc {

static_assert(sizeof(int) * 2 >= sizeof(void*), "you run an unusual system");

void from_ptr(void* p, int& p1, int& p2)
{
    int buf[] = {0, 0};
    memcpy(buf, &p, 2 * sizeof(int));
    p1 = buf[0];
    p2 = buf[1];
}

void* to_ptr(int p1, int p2)
{
    int   buf[] = {p1, p2};
    void* p;
    memcpy(&p, buf, 2 * sizeof(int));
    return p;
}

} // namespace krc
