#include <executor.hh>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <io.hh>
#include <unistd.h>
#include <vector>

// annoyingly there is no obvious portable way to figure out the size of pipe buffers
#ifdef __linux__
#define KNOWN_PIPE_BUFSIZE
#endif

namespace krc {
namespace {

#ifdef __linux__
size_t fill_write_buffer(int fd, char val)
{
    int               bufsize = fcntl(fd, F_GETPIPE_SZ);
    std::vector<char> data(bufsize, val);

    auto w = ::write(fd, data.data(), data.size());
    assert(bufsize == w);

    return w;
}
#else  // __linux__
size_t fill_write_buffer(int fd, char val)
{
    assert(false && "find a portable way");
    return 0;
}
#endif // __linux__

class io_test : public testing::Test
{
public:
    void SetUp() override
    {
        int rc = ::pipe(d_fds);
        ASSERT_EQ(0, rc);
    }

    void TearDown() override
    {
        close(d_fds[0]);
        close(d_fds[1]);
    }

    int& read()
    {
        return d_fds[0];
    }

    int& write()
    {
        return d_fds[1];
    }

    void close(int& fd)
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
    int  p = write();
    int  l = read();

    dispatch([&r, l] {
        krc::io::read(l, &r, 1);
    });
    run([p] {
        char c = 'a';
        krc::io::write(p, &c, 1);
    });

    EXPECT_EQ('a', r);
}

TEST_F(io_test, read_returns_on_close)
{
    int     l = read();
    ssize_t r;

    close(write());
    run([l, &r] {
        char c;
        r = krc::io::read(l, &c, 1);
    });

    EXPECT_EQ(0, r);
}

TEST_F(io_test, read_returns_on_error)
{
    int     l = read();
    ssize_t r;

    close(read());
    run([l, &r] {
        char c;
        r = krc::io::read(l, &c, 1);
    });

    EXPECT_EQ(-1, r);
}

// only enabled on linux, exact behaviour is platform-dependent
#ifdef __linux__
TEST_F(io_test, write_returns_on_error)
#else
TEST_F(io_test, DISABLED_write_returns_on_error)
#endif
{
    int     l = read();
    ssize_t r;

    close(write());
    run([l, &r] {
        char c = 'a';
        r      = krc::io::write(l, &c, 1);
    });

    EXPECT_EQ(-1, r);
}

#ifdef KNOWN_PIPE_BUFSIZE
TEST_F(io_test, yields_when_blocking_write)
#else
TEST_F(io_test, DISABLED_yields_when_blocking_write)
#endif
{
    char r;
    int  p = read();
    int  l = write();

    // first fill the write buffer, else the test wont be valid
    auto size = fill_write_buffer(l, 'a');

    dispatch([l] {
        char c = 'a';
        krc::io::write(l, &c, 1);
    });
    run([p, &r, size] {
        krc::io::read(p, &r, 1);

        std::vector<char> buf(size);
        krc::io::read(p, buf.data(), size);
    });

    EXPECT_EQ('a', r);
}

} // namespace
} // namespace krc
