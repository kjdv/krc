#pragma once

#include <memory>
#include <optional>

#include "queue.hh"
#include "zero_queue.hh"

namespace krc {

template <typename T>
class channel
{
public:
  explicit channel(size_t queue_size = 0);

  void push(T&& item);

  std::optional<T> pop();

  void close();

private:
  class impl;
  class unbuffered;
  class buffered;

  static std::shared_ptr<impl> make_impl(size_t queue_size);

  std::shared_ptr<impl> d_pimpl;
};

template <typename T>
class channel<T>::impl
{
public:
  virtual ~impl()
  {
  }

  virtual void push(T&& item) = 0;

  virtual std::optional<T> pop() = 0;

  virtual void close() = 0;
};

template <typename T>
class channel<T>::unbuffered : public channel<T>::impl
{
public:
  void push(T&& item) override
  {
    d_impl.push(std::forward<T>(item));
  }

  std::optional<T> pop() override
  {
    return d_impl.pop();
  }

  void close() override
  {
    d_impl.close();
  }

private:
  zero_queue<T> d_impl;
};

template <typename T>
class channel<T>::buffered : public channel<T>::impl
{
public:
  explicit buffered(size_t max_size)
    : d_impl(max_size)
  {
  }

  void push(T&& item) override
  {
    d_impl.push(std::forward<T>(item));
  }

  std::optional<T> pop() override
  {
    return d_impl.pop();
  }

  void close() override
  {
    d_impl.close();
  }

private:
  queue<T> d_impl;
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
void channel<T>::push(T&& item)
{
  assert(d_pimpl);
  d_pimpl->push(std::forward<T>(item));
}

template <typename T>
std::optional<T> channel<T>::pop()
{
  assert(d_pimpl);
  return d_pimpl->pop();
}

template <typename T>
void channel<T>::close()
{
  assert(d_pimpl);
  return d_pimpl->close();
}

} // namespace krc
