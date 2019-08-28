#pragma once

#include <iterator>
#include <memory>
#include <variant>

#include "internal/ringbuffer.hh"
#include "internal/unbuffered.hh"

namespace krc {

template <typename T>
class channel
{
public:
    class iterator;

    explicit channel(size_t buffer_size = 0);

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
    using buffer_t = std::variant<internal::unbuffered<T>, internal::ringbuffer<T>>;

    static std::shared_ptr<buffer_t> make_buffer(size_t buffer_size);

    std::shared_ptr<buffer_t> d_buffer;
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
channel<T>::channel(size_t buffer_size)
    : d_buffer(make_buffer(buffer_size))
{}

template <typename T>
std::shared_ptr<typename channel<T>::buffer_t> channel<T>::make_buffer(size_t buffer_size)
{
    if(buffer_size)
        return std::make_shared<buffer_t>(std::in_place_type<internal::ringbuffer<T>>, buffer_size);
    else
        return std::make_shared<buffer_t>(std::in_place_type<internal::unbuffered<T>>);
}

template <typename T> template<typename U>
bool channel<T>::push(U&& item)
{
    static_assert(std::is_convertible<U, T>::value);
    assert(d_buffer);

    auto visitor = [&item](auto&& b) {
        return b.push(std::forward<U>(item));
    };

    return std::visit(visitor, *d_buffer);
}

template <typename T>
std::optional<T> channel<T>::pull()
{
    assert(d_buffer);

    auto visitor = [](auto&& b) {
        return b.pull();
    };

    return std::visit(visitor, *d_buffer);
}

template <typename T>
void channel<T>::close()
{
    assert(d_buffer);

    auto visitor = [](auto&& b) {
        b.close();
    };

    std::visit(visitor, *d_buffer);
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
