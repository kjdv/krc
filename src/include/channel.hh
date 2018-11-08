#pragma once

#include <iterator>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>

#include "internal/queue.hh"
#include "internal/zero_queue.hh"

namespace krc {

template <typename T>
class channel
{
public:
    class iterator;

    explicit channel(size_t queue_size = 0);

    // push the item on the channel, returns true on success and false if
    // the channel is closed and the item could not be pushed
    template <typename U>
    bool push(U&& item);

    // pull an item from the channel, returns no value if the channel is closed
    std::optional<T> pull();

    // close the channel
    void close();

    // range support
    iterator begin();
    iterator end();

private:
    typedef std::variant<internal::zero_queue<T>, internal::queue<T>> impl_t;
    typedef std::shared_ptr<impl_t> pimpl_t;

    static pimpl_t make_impl(size_t queue_size);

    pimpl_t d_pimpl;
};

template <typename T>
class channel<T>::iterator : public std::iterator<std::input_iterator_tag, T>
{
public:
    explicit iterator()
    {}

    explicit iterator(channel<T>* channel)
        : d_channel(channel)
    {
        pull();
    }

    iterator& operator++()
    {
        pull();
        return *this;
    }

    iterator operator++(int)
    {
        iterator result = *this;
        ++(*this);
        return result;
    }

    bool operator==(const iterator& other)
    {
        // the only equality that makes sense is if both iterators point to the end
        return (!this->d_item.has_value() && !other.d_item.has_value());
    }

    bool operator!=(const iterator& other)
    {
        return !(*this == other);
    }

    const T& operator*() const
    {
        assert(d_item.has_value());
        return *d_item;
    }

private:
    void pull()
    {
        assert(d_channel);
        d_item = d_channel->pull();
    }

    channel<T>*      d_channel{nullptr};
    std::optional<T> d_item;
};

template <typename T>
typename channel<T>::pimpl_t channel<T>::make_impl(size_t queue_size)
{
    if(queue_size)
        return std::make_shared<impl_t>(std::in_place_type<internal::queue<T>>, queue_size);
    else
        return std::make_shared<impl_t>(std::in_place_type<internal::zero_queue<T>>);
}

template <typename T>
channel<T>::channel(size_t queue_size)
    : d_pimpl(make_impl(queue_size))
{
}

template <typename T> template<typename U>
bool channel<T>::push(U&& item)
{
    assert(d_pimpl);
    static_assert(std::is_same<typename std::decay<T>::type, typename std::decay<U>::type>::value);

    auto visitor = [&item](auto&& q) {
        return q.push(std::forward<U>(item));
    };

    return std::visit(visitor, *d_pimpl);
}

template <typename T>
std::optional<T> channel<T>::pull()
{
    assert(d_pimpl);

    auto visitor = [](auto&& q) {
        return q.pull();
    };

    return std::visit(visitor, *d_pimpl);
}

template <typename T>
void channel<T>::close()
{
    assert(d_pimpl);

    auto visitor = [](auto&& q) {
        q.close();
    };

    std::visit(visitor, *d_pimpl);
}

template <typename T>
typename channel<T>::iterator channel<T>::begin()
{
    return iterator(this);
}

template <typename T>
typename channel<T>::iterator channel<T>::end()
{
    return iterator();
}

} // namespace krc
