#pragma once

#include <cassert>
#include <condition_variable>
#include <optional>
#include <queue>
#include <type_traits>

#include "mutex.hh"

namespace krc {
namespace internal {

template <typename T>
class queue : private no_copy
{
public:
    explicit queue(size_t max_size);

    size_t size() const;

    bool empty() const;

    size_t max_size() const;

    template <typename U>
    bool push(U&& item);

    std::optional<T> pull();

    void close();

private:
    typedef std::unique_lock<mutex> lock_t;

    bool closed() const;

    bool not_full() const;

    bool not_empty() const;

    size_t        d_max_size;
    std::queue<T> d_base;

    mutable mutex               d_mutex;
    std::condition_variable_any d_not_full;
    std::condition_variable_any d_not_empty;
    bool                        d_closed{false};
};

template <bool, typename T>
struct pull_helper
{
    static_assert(!std::is_move_constructible<T>::value);

    T operator()(std::queue<T>& q) const
    {
        assert(!q.empty());

        T item = q.front();
        q.pop();
        return item;
    }
};

template <typename T>
struct pull_helper<true, T>
{
    static_assert(std::is_move_constructible<T>::value);

    T operator()(std::queue<T>& q) const
    {
        assert(!q.empty());

        T item(std::move(q.front()));
        q.pop();
        return item;
    }
};

template <typename T>
queue<T>::queue(size_t max_size)
    : d_max_size(max_size)
{
    assert(d_max_size > 0);
}

template <typename T>
size_t queue<T>::max_size() const
{
    return d_max_size;
}

template <typename T>
template <typename U>
bool queue<T>::push(U&& item)
{
    static_assert(std::is_same<typename std::decay<T>::type, typename std::decay<U>::type>::value);

    lock_t l(d_mutex);
    d_not_full.wait(l, [=] { return closed() || this->not_full(); });

    if(closed())
        return false;

    d_base.emplace(std::forward<U>(item));

    l.unlock();
    d_not_empty.notify_one();

    return true;
}

template <typename T>
std::optional<T> queue<T>::pull()
{
    lock_t l(d_mutex);

    d_not_empty.wait(l, [=] { return closed() || this->not_empty(); });

    if(!not_empty())
        return std::optional<T>();

    auto item = pull_helper<std::is_move_assignable<T>::value, T>()(d_base);

    l.unlock();
    d_not_full.notify_one();

    return item;
}

template <typename T>
bool queue<T>::not_full() const
{
    return d_base.size() < max_size();
}

template <typename T>
bool queue<T>::not_empty() const
{
    return !d_base.empty();
}

template <typename T>
void queue<T>::close()
{
    lock_t l(d_mutex);
    d_closed = true;
    l.unlock();

    d_not_full.notify_all();
    d_not_empty.notify_all();
}

template <typename T>
bool queue<T>::closed() const
{
    return d_closed;
}

template <typename T>
size_t queue<T>::size() const
{
    lock_t l(d_mutex);
    return d_base.size();
}

template <typename T>
bool queue<T>::empty() const
{
    lock_t l(d_mutex);
    return d_base.empty();
}

} // namespace internal
} // namespace krc
