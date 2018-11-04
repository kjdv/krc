#pragma once

#include <iterator>
#include <memory>
#include <optional>

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
    bool push(T&& item);
    bool push(const T& item);

    // pull an item from the channel, returns no value if the channel is closed
    std::optional<T> pull();

    // close the channel
    void close();

    // range support
    iterator begin();
    iterator end();

private:
    class impl;
    class unbuffered;
    class buffered;

    static std::shared_ptr<impl> make_impl(size_t queue_size);

    std::shared_ptr<impl> d_pimpl;
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
class channel<T>::impl
{
public:
    virtual ~impl() = default;

    virtual bool push(T&& item) = 0;

    virtual std::optional<T> pull() = 0;

    virtual void close() = 0;
};

template <typename T>
class channel<T>::unbuffered : public channel<T>::impl
{
public:
    bool push(T&& item) override
    {
        return d_impl.push(std::forward<T>(item));
    }

    std::optional<T> pull() override
    {
        return d_impl.pull();
    }

    void close() override
    {
        d_impl.close();
    }

private:
    internal::zero_queue<T> d_impl;
};

template <typename T>
class channel<T>::buffered : public channel<T>::impl
{
public:
    explicit buffered(size_t max_size)
        : d_impl(max_size)
    {}

    bool push(T&& item) override
    {
        return d_impl.push(std::forward<T>(item));
    }

    std::optional<T> pull() override
    {
        return d_impl.pull();
    }

    void close() override
    {
        d_impl.close();
    }

private:
    internal::queue<T> d_impl;
};

template <typename T>
std::shared_ptr<typename channel<T>::impl> channel<T>::make_impl(size_t queue_size)
{
    if(queue_size)
        return std::make_shared<channel<T>::buffered>(queue_size);
    else
        return std::make_shared<channel<T>::unbuffered>();
}

template <typename T>
channel<T>::channel(size_t queue_size)
    : d_pimpl(make_impl(queue_size))
{
}

template <typename T>
bool channel<T>::push(T&& item)
{
    assert(d_pimpl);
    return d_pimpl->push(std::forward<T>(item));
}

template <typename T>
bool channel<T>::push(const T& item)
{
    assert(d_pimpl);
    return d_pimpl->push(T(item)); // only place where this odd duplication is needed, only copy needed
}

template <typename T>
std::optional<T> channel<T>::pull()
{
    assert(d_pimpl);
    return d_pimpl->pull();
}

template <typename T>
void channel<T>::close()
{
    assert(d_pimpl);
    return d_pimpl->close();
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