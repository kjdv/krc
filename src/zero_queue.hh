#pragma once

#include <condition_variable>
#include <optional>

#include "closed_exception.hh"
#include "mutex.hh"

namespace krc {

template <typename T>
class zero_queue
{
  public:
    explicit zero_queue();

    void push(T&& item);

    std::optional<T> pull();

    void close();

  private:
    typedef std::unique_lock<mutex> lock_t;

    bool is_closed() const;

    bool available() const;

    bool can_push() const;

    bool can_pull() const;

    std::optional<T>            d_item;
    mutable mutex               d_mutex;
    std::condition_variable_any d_can_push;
    std::condition_variable_any d_can_pull;
    bool                        d_closed{false};
};

template <typename T>
zero_queue<T>::zero_queue()
{
}

template <typename T>
void zero_queue<T>::push(T&& item)
{
    lock_t l(d_mutex);

    d_can_push.wait(l, [=] { return is_closed() || this->can_push(); });

    if(is_closed())
        throw channel_closed("push on a closed channel");

    d_item = item;

    l.unlock();
    d_can_pull.notify_one();
}

template <typename T>
std::optional<T> zero_queue<T>::pull()
{
    lock_t l(d_mutex);

    d_can_pull.wait(l, [=] { return is_closed() || this->can_pull(); });

    if(!can_pull())
        return std::optional<T>();

    std::optional<T> item;
    std::swap(item, d_item);

    l.unlock();
    d_can_push.notify_one();

    return item;
}

template <typename T>
bool zero_queue<T>::can_push() const
{
    return !d_item.has_value();
}

template <typename T>
bool zero_queue<T>::can_pull() const
{
    return d_item.has_value();
}

template <typename T>
void zero_queue<T>::close()
{
    lock_t l(d_mutex);
    d_closed = true;
    l.unlock();

    d_can_push.notify_all();
    d_can_pull.notify_all();
}

template <typename T>
bool zero_queue<T>::is_closed() const
{
    return d_closed;
}

} // namespace krc
