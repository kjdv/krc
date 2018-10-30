#include <ptr_translate.hh>
#include <gtest/gtest.h>

namespace krc {
namespace {

template <typename T>
void test(void *original)
{
    auto ints = from_ptr<T>(original);
    auto translated = to_ptr(ints);

    EXPECT_EQ(original, translated);
}

TEST(ptr_as_ints, int16)
{
    char p = 'a';
    test<int16_t>(&p);
}

TEST(ptr_as_ints, int32)
{
    char p = 'a';
    test<int32_t>(&p);
}

TEST(ptr_as_ints, int64)
{
    char p = 'a';
    test<int64_t>(&p);
}

}
}
