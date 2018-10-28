#pragma once

#include <queue>
#include <functional>
#include <vector>
#include <map>
#include <ucontext.h>

namespace krc {

enum { DEFAULT_STACK_SIZE = 16384};

class executor
{
public:
  executor(const executor &) = delete;
  executor &operator=(const executor &) = delete;

  static executor &instance();

  void push(const std::function<void()> &target, size_t stack_size = DEFAULT_STACK_SIZE);

  void run();

  void yield();

private:
  friend void run_target(int routine_id);

  void execute(int routine_id);

  executor()
  {}

  void next();

  ucontext_t d_main;

  std::queue<ucontext_t> d_routines;

  struct target_t
  {
    std::function<void()> target;
    std::vector<char> stack;

    target_t(const std::function<void()> &target_, size_t stack_size)
      : target(target_)
      , stack(stack_size)
    {}
  };

  std::map<int, target_t> d_targets;

  static executor s_instance;
};

}
