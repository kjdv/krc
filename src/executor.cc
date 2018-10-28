#include "executor.hh"
#include <cassert>

using namespace std;

namespace krc {

// weird control ping-poing needed because makecontext targets need c-linkage
void run_target(int routine_id)
{
  auto &exec = krc::executor::instance();

  exec.execute(routine_id);
}

executor executor::s_instance;

executor &executor::instance()
{
  return s_instance;
}

void executor::push(const function<void ()> &target, size_t stack_size)
{
  int routine_id = d_targets.empty() ? 0 : d_targets.rbegin()->first + 1;
  auto it = d_targets.emplace(routine_id, target_t{target, stack_size});
  assert(it.second);

  ucontext_t ctx;
  getcontext(&ctx);
  ctx.uc_stack.ss_sp = it.first->second.stack.data();
  ctx.uc_stack.ss_size = stack_size;
  ctx.uc_link = 0;
  makecontext(&ctx, (void (*)())&run_target, 1, routine_id);

  d_routines.push(ctx);
}

void executor::execute(int routine_id)
{
  auto it = d_targets.find(routine_id);
  assert(it != d_targets.end());

  it->second.target();

  // cleanup
  d_targets.erase(it);

  next();
}

void executor::next()
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

void executor::run()
{
  if (!d_routines.empty())
  {
    ucontext_t ctx = d_routines.front();
    d_routines.pop();

    swapcontext(&d_main, &ctx);
  }
}

void executor::yield()
{
  if (!d_routines.empty())
  {
    d_routines.emplace();

    ucontext_t new_ctx = d_routines.front();
    d_routines.pop();

    swapcontext(&d_routines.back(), &new_ctx);
  }
}

}
