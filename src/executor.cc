#include "executor.hh"
#include <cassert>
#include <ucontext.h>
#include <map>
#include <vector>
#include <queue>

using namespace std;

namespace krc {


// weird control ping-poing needed because makecontext targets need c-linkage
std::function<void(int)> g_execute;
void run_target(int routine_id)
{
  g_execute(routine_id);
}

class executor::impl
{
public:
  void push(const function<void ()> &target, size_t stack_size)
  {
    int routine_id = d_targets.empty() ? 0 : d_targets.rbegin()->first + 1;
    auto it = d_targets.emplace(std::piecewise_construct,
                                std::forward_as_tuple(routine_id),
                                std::forward_as_tuple(target, stack_size));
    assert(it.second);

    ucontext_t ctx;
    getcontext(&ctx);
    ctx.uc_stack.ss_sp = it.first->second.stack.data();
    ctx.uc_stack.ss_size = stack_size;
    ctx.uc_stack.ss_flags = 0;
    ctx.uc_link = 0;
    makecontext(&ctx, (void (*)())&run_target, 1, routine_id);

    d_routines.push(ctx);
  }

  void execute(int routine_id)
  {
    auto it = d_targets.find(routine_id);
    assert(it != d_targets.end());

    it->second.target();

    // cleanup
    d_targets.erase(it);

    next();
  }

  void run()
  {
    if (!d_routines.empty())
    {
      ucontext_t ctx = d_routines.front();
      d_routines.pop();

      swapcontext(&d_main, &ctx);
    }
  }

  void run(const function<void ()> &target, size_t stack_size)
  {
    push(target, stack_size);
    run();
  }

  void yield()
  {
    if (!d_routines.empty())
    {
      d_routines.emplace();

      ucontext_t new_ctx = d_routines.front();
      d_routines.pop();

      swapcontext(&d_routines.back(), &new_ctx);
    }
  }

private:
  void next()
  {
    if(d_routines.empty())
    {
      setcontext(&d_main);
    }
    else
    {
      ucontext_t ctx = d_routines.front();
      d_routines.pop();

      setcontext(&ctx);
    }
  }

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

    target_t(const target_t &) = delete;
    target_t &operator=(const target_t &) = delete;
  };

  std::map<int, target_t> d_targets;
};

executor executor::s_instance;

executor &executor::instance()
{
  return s_instance;
}

executor::executor()
  : d_pimpl(new impl)
{
  g_execute = [=](int routine_id){ d_pimpl->execute(routine_id); };
}

void executor::push(const std::function<void ()> &target, size_t stack_size)
{
  assert(d_pimpl);
  d_pimpl->push(target, stack_size);
}

void executor::run()
{
  assert(d_pimpl);
  d_pimpl->run();
}

void executor::run(const std::function<void ()> &target, size_t stack_size)
{
  assert(d_pimpl);
  d_pimpl->run(target, stack_size);
}

void executor::yield()
{
  assert(d_pimpl);
  d_pimpl->yield();
}

}
