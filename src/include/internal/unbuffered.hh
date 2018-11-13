#pragma once

#include "semaphore.hh"
#include "utils.hh"
#include <optional>

namespace krc {
namespace internal {

template <typename T>
class unbuffered : private no_copy
{
public:
    template <typename U>
    bool push(U && item);

    std::optional<T> pull();

    void close();

private:
    bool is_closed() const
    {
        return d_single_pusher.is_closed();
    }

    T *d_item{nullptr};

    closeable_binary_semaphore d_pull{false};
    binary_semaphore d_pull_done{false};

    closeable_binary_semaphore d_single_pusher{true};
};

template <typename T>
template <typename U>
bool unbuffered<T>::push(U&& item)
{
    static_assert(std::is_convertible<U, T>::value);

    d_single_pusher.wait();
    defer unlock{[this] { d_single_pusher.notify(); }};

    if(is_closed())
        return false;

    assert(d_item == nullptr);
    d_item = &item;

    d_pull.notify();
    d_pull_done.wait();

    return true;
}

template <typename T>
std::optional<T> unbuffered<T>::pull()
{
    d_pull.wait();
    defer done{[this] { d_pull_done.notify(); }};

    if(is_closed())
        return std::optional<T>();

    assert(d_item != nullptr);

    T result(std::move(*d_item));
    d_item = nullptr;

    return result;
}

template <typename T>
void unbuffered<T>::close()
{
  //  d_closed = true;
  d_single_pusher.close();
  d_pull.close();
  d_pull_done.notify();
}

}
}
