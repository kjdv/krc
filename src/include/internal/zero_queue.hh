#pragma once

#include <optional>

#include "condition_variable.hh"
#include "mutex.hh"

namespace krc {
namespace internal {

template <typename T>
class zero_queue : private no_copy
{
public:
    explicit zero_queue();

    template <typename U>
    bool push(U&& item);

    std::optional<T> pull();

    void close();

private:
    typedef std::unique_lock<mutex> lock_t;

    bool is_closed() const;

    bool can_push() const;

    bool can_pull() const;

    std::optional<T>            d_item;
    mutable mutex               d_mutex;
    condition_variable d_pull_ready;
    condition_variable d_push_ready;
    bool                        d_closed{false};
};

template <typename T>
zero_queue<T>::zero_queue()
{
}

template <typename T>
template <typename U>
bool zero_queue<T>::push(U&& item)
{
    static_assert(std::is_same<typename std::decay<T>::type, typename std::decay<U>::type>::value);

    lock_t l(d_mutex);

    d_push_ready.wait(l, [this] { return is_closed() || can_push(); });

    if(is_closed())
        return false;

    d_item = std::move(item);

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

    if(!d_item.has_value())
        return std::optional<T>();

    T item(std::move(*d_item));
    d_item.reset();

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
    return !d_item.has_value();
}

template <typename T>
bool zero_queue<T>::can_pull() const
{
    return d_item.has_value();
}

template <typename T>
bool zero_queue<T>::is_closed() const
{
    return d_closed;
}

} // namespace internal
} // namespace krc
