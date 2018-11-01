#include <ptr_translate.hh>
#include <gtest/gtest.h>

namespace krc {
namespace {

TEST(ptr_as_ints, translate)
{
    char p = 'a';

    int p1, p2;
    from_ptr(&p, p1, p2);
    char *np = reinterpret_cast<char *>(to_ptr(p1, p2));

    EXPECT_EQ(&p, np);
    EXPECT_EQ(p, *np);
}

}
}
