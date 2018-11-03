#include <io.hh>
#include <executor.hh>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>

namespace krc {
namespace {

class io_test : public testing::Test
{
public:
    void SetUp() override
    {
        int rc = socketpair(AF_LOCAL, SOCK_STREAM, 0, d_fds);
        ASSERT_EQ(0, rc);
    }

    void TearDown() override
    {
       close(d_fds[0]);
       close(d_fds[1]);
    }

    int &local()
    {
        return d_fds[0];
    }

    int &peer()
    {
        return d_fds[1];
    }

    void close(int &fd)
    {
        if(fd >= 0)
        {
            ::close(fd);
            fd = 0;
        }
    }
private:
   int d_fds[2];
};

TEST_F(io_test, yields_when_blocking_read)
{
    char r;
    int p = peer();
    int l = local();

    dispatch([&r,l]{
        krc::io::read(l, &r, 1);
    });
    run([p]{
        char c = 'a';
        ::write(p, &c, 1);
    });

    EXPECT_EQ('a', r);
}

TEST_F(io_test, read_returns_on_close)
{
    int l = local();
    ssize_t r;

    close(peer());
    run([l, &r]{
        char c;
        r = ::read(l, &c, 1);
    });

    EXPECT_EQ(0, r);
}

TEST_F(io_test, read_returns_on_error)
{
    int l = local();
    ssize_t r;

    close(local());
    run([l, &r]{
        char c;
        r = ::read(l, &c, 1);
    });

    EXPECT_EQ(-1, r);
}

TEST_F(io_test, write_returns_on_error)
{
    int l = local();
    ssize_t r;

    close(local());
    run([l, &r]{
        char c = 'a';
        r = ::write(l, &c, 1);
    });

    EXPECT_EQ(-1, r);
}

}
}
