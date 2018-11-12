#pragma once

#include <vector>
#include <atomic>
#include <optional>
#include <cassert>
#include <mutex>
#include <utility>
#include "internal/mutex.hh"
#include "internal/no_copy.hh"

namespace krc {
namespace internal {

template <typename T>
class ringbuffer : private internal::no_copy
{
public:
    explicit ringbuffer(size_t capacity);

    template <typename U>
    bool push(U && item);

    std::optional<T> pull();

    void close();

    // exposed for better testability
    enum class status {full, empty, closed, ok};

    template <typename U>
    status try_push(U && item);

    std::pair<std::optional<T>, status> try_pull();
private:
    using iter = typename std::vector<T>::iterator;

    size_t capacity() const
    {
        return d_buffer.size();
    }

    bool is_closed() const
    {
        return d_closed;
    }

    size_t size() const
    {
        return d_size;
    }

    bool is_empty() const
    {
        return size() == 0;
    }

    bool is_full()
    {
        return size() == capacity();
    }

    iter next(iter it)
    {
        return (it + 1) == end() ? begin() : (it + 1);
    }

    iter begin()
    {
        return d_buffer.begin();
    }

    iter end()
    {
        return d_buffer.end();
    }

    using lock_t = std::lock_guard<krc::internal::mutex>;

    mutable krc::internal::mutex d_mutex;
    std::vector<T> d_buffer;
    typename std::vector<T>::iterator d_read;
    typename std::vector<T>::iterator d_write;
    size_t d_size{0};
    bool d_closed{false};
};

template <typename T>
ringbuffer<T>::ringbuffer(size_t capacity)
    : d_buffer(capacity)
    , d_read(d_buffer.begin())
    , d_write(d_buffer.begin())
{
    assert(d_buffer.size() >= 1);
}

template <typename T>
template <typename U>
bool ringbuffer<T>::push(U && item)
{
    while(true)
    {
        auto s = try_push(std::forward<U>(item));
        switch(s)
        {
        case status::empty:
            assert(false && "unexpected status");
            // fall through
        case status::closed:
            return false;
        case status::ok:
            return true;
        case status::full:
            continue; // try again
        }
    }
}

template <typename T>
template <typename U>
typename ringbuffer<T>::status ringbuffer<T>::try_push(U && item)
{
    lock_t l(d_mutex);

    if(is_closed())
        return status::closed;

    if(is_full())
        return status::full;

    assert(size() < capacity());

    *d_write = std::move(item);
    d_write = next(d_write);
    ++d_size;

    return status::ok;
}

template <typename T>
std::optional<T> ringbuffer<T>::pull()
{
    while(true)
    {
        auto result = try_pull();
        switch(result.second)
        {
        case status::full:
            assert(false && "unexpected status");
            // fall through
        case status::closed:
            // fall through
        case status::ok:
            return result.first;
        case status::empty:
            continue; // try again
        }
    }
}

template <typename T>
std::pair<std::optional<T>, typename ringbuffer<T>::status> ringbuffer<T>::try_pull()
{
    lock_t l(d_mutex);

    if (is_empty())
    {
        if (is_closed())
            return std::make_pair(std::optional<T>(), status::closed);
        else
            return std::make_pair(std::optional<T>(), status::empty);
    }

    assert(size() > 0);

    T result = std::move(*d_read);
    d_read = next(d_read);
    --d_size;

    return std::make_pair<std::optional<T>, status>(result, status::ok);
}

template <typename T>
void ringbuffer<T>::close()
{
    lock_t l(d_mutex);
    d_closed = true;
}

}
}
