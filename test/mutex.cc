#include <mutex.hh>
#include <executor.hh>
#include <gtest/gtest.h>

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
    auto &exec = executor::instance();

    mutex m;
    m.lock();

    exec.run([&]{
        m.unlock();
    });

    EXPECT_TRUE(m.try_lock());
    m.unlock();
}

TEST(mutex, lock_yields_when_held)
{
    auto &exec = executor::instance();

    mutex m;
    m.lock();

    exec.push([&]{
        m.lock();
    });
    exec.push([&]{
        m.unlock();
    });

    exec.run();

    EXPECT_FALSE(m.try_lock());
}


}
}
