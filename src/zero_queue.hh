#pragma once

#include <condition_variable>
#include <optional>

#include "mutex.hh"

namespace krc {

template <typename T>
class zero_queue : private no_copy
{
public:
    explicit zero_queue();

    bool push(T&& item);

    std::optional<T> pull();

    void close();

private:
    typedef std::unique_lock<mutex> lock_t;

    bool is_closed() const;

    bool can_push() const;

    bool can_pull() const;

    T*                          d_item{nullptr};
    mutable mutex               d_mutex;
    std::condition_variable_any d_pull_ready;
    std::condition_variable_any d_push_ready;
    bool                        d_closed{false};
};

template <typename T>
zero_queue<T>::zero_queue()
{
}

template <typename T>
bool zero_queue<T>::push(T&& item)
{
    lock_t l(d_mutex);

    d_push_ready.wait(l, [this] { return is_closed() || can_push(); });

    if(is_closed())
        return false;

    d_item = &item;

    l.unlock();
    d_pull_ready.notify_one();

    return true;
}

template <typename T>
std::optional<T> zero_queue<T>::pull()
{
    d_push_ready.notify_one();

    lock_t l(d_mutex);
    d_pull_ready.wait(l, [this] { return is_closed() || can_pull(); });

    if(d_item == nullptr)
        return std::optional<T>();

    T item = *d_item;
    d_item = nullptr;

    l.unlock();

    return item;
}

template <typename T>
void zero_queue<T>::close()
{
    lock_t l(d_mutex);
    d_closed = true;
    l.unlock();

    d_push_ready.notify_all();
    d_pull_ready.notify_all();
}

template <typename T>
bool zero_queue<T>::can_push() const
{
    return d_item == nullptr;
}

template <typename T>
bool zero_queue<T>::can_pull() const
{
    return d_item != nullptr;
}

template <typename T>
bool zero_queue<T>::is_closed() const
{
    return d_closed;
}

} // namespace krc
