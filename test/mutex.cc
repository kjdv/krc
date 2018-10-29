#include <gtest/gtest.h>
#include <mutex.hh>
#include <runtime.hh>

namespace krc {
namespace {

TEST(mutex, try_lock)
{
    mutex m;
    EXPECT_TRUE(m.try_lock());
    EXPECT_FALSE(m.try_lock());

    m.unlock();
}

TEST(mutex, lock_unlock)
{
    mutex m;
    m.lock();

    krc::run([&] {
        m.unlock();
    });

    EXPECT_TRUE(m.try_lock());
    m.unlock();
}

TEST(mutex, lock_yields_when_held)
{

    mutex m;
    m.lock();

    krc::dispatch([&] {
        m.lock();
    });
    krc::run([&] {
        m.unlock();
    });

    EXPECT_FALSE(m.try_lock());
}

} // namespace
} // namespace krc
