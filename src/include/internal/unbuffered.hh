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
        return false;
    }

    T *d_item{nullptr};

    binary_semaphore d_pull{false};
    binary_semaphore d_pull_done{false};

    binary_semaphore d_single_pusher{true};
};

template <typename T>
template <typename U>
bool unbuffered<T>::push(U&& item)
{
    static_assert(std::is_convertible<U, T>::value);

    d_single_pusher.wait();
    defer unlock{[this] { d_single_pusher.notify(); }};

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

    assert(d_item != nullptr);

    T result(std::move(*d_item));
    d_item = nullptr;

    d_pull_done.notify();

    return result;
}

template <typename T>
void unbuffered<T>::close()
{
  //  d_closed = true;
  //  d_single_pusher.close();
  //  d_pull.close();
  //  d_pull_done.notify();
}

}
}
