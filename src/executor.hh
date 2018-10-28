#pragma once

#include <functional>
#include <memory>

namespace krc {

enum {
    DEFAULT_STACK_SIZE = 1<<15,
    MIN_STACK_SIZE = 1<<15, // less than this does not play well on mac osx (todo: allow this to be smaller on other platforms)
};

class executor
{
public:
  executor(const executor &) = delete;
  executor &operator=(const executor &) = delete;

  static executor &instance();

  void push(const std::function<void()> &target, size_t stack_size = DEFAULT_STACK_SIZE);

  void run();

  void run(const std::function<void()> &target, size_t stack_size = DEFAULT_STACK_SIZE);

  void yield();

private:
  executor();

  class impl;
  std::unique_ptr<impl> d_pimpl;

  static executor s_instance;
};

}
