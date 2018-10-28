#pragma once

#include <mutex>
#include <condition_variable>
#include <optional>
#include <stdexcept>

namespace krc {

class zero_queue_closed : public std::logic_error
{
public:
  using std::logic_error::logic_error;
};

template <typename T>
class zero_queue
{
public:
  explicit zero_queue();

  bool closed() const;

  void push(T && item);

  std::optional<T> pop();

  void close();

private:
  typedef std::unique_lock<std::mutex> lock_t;

  bool is_closed() const;

  bool available() const;

  bool can_push() const;

  bool can_pop() const;

  std::optional<T> d_item;
  mutable std::mutex d_mutex;
  std::condition_variable d_can_push;
  std::condition_variable d_can_pop;
  bool d_closed{false};
};

template <typename T>
zero_queue<T>::zero_queue()
{}

template <typename T>
void zero_queue<T>::push(T && item)
{
  lock_t l(d_mutex);
  d_can_push.wait(l, [=]{ return is_closed() || this->can_push(); });

  if(is_closed())
    throw zero_queue_closed("push on a closed queue");

  d_item = item;

  l.unlock();
  d_can_pop.notify_one();
}

template <typename T>
std::optional<T> zero_queue<T>::pop()
{
  lock_t l(d_mutex);

  d_can_pop.wait(l, [=]{ return is_closed() || this->can_pop(); });

  if(!can_pop())
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
bool zero_queue<T>::can_pop() const
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
  d_can_pop.notify_all();
}

template <typename T>
bool zero_queue<T>::is_closed() const
{
  return d_closed;
}

template <typename T>
bool zero_queue<T>::closed() const
{
  lock_t l(d_mutex);
  return d_closed;
}

}
