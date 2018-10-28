#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace krc {

template <typename T>
class queue
{
public:
  explicit queue(size_t max_size);

  size_t max_size() const;

  void push(T && item);

  std::optional<T> pop();

private:
  typedef std::unique_lock<std::mutex> lock_t;

  bool not_full() const;

  bool not_empty() const;

  size_t d_max_size;
  std::queue<T> d_base;

  std::mutex d_mutex;
  std::condition_variable d_not_full;
  std::condition_variable d_not_empty;
};

template <typename T>
queue<T>::queue(size_t max_size)
  : d_max_size(max_size)
{}

template <typename T>
size_t queue<T>::max_size() const
{
  return d_max_size;
}

template <typename T>
void queue<T>::push(T && item)
{
  lock_t l(d_mutex);
  d_not_full.wait(l, [=]{ return this->not_full(); });

  d_base.emplace(std::forward<T>(item));

  l.unlock();
  d_not_empty.notify_one();
}

template <typename T>
std::optional<T> queue<T>::pop()
{
  lock_t l(d_mutex);

  d_not_empty.wait(l, [=]{ return this->not_empty(); });

  auto item = d_base.front();
  d_base.pop();

  l.unlock();
  d_not_full.notify_one();

  return item;
}

template <typename T>
bool queue<T>::not_full() const
{
  return d_base.size() < max_size();
}

template <typename T>
bool queue<T>::not_empty() const
{
  return !d_base.empty();
}

}
