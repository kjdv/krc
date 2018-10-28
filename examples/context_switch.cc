#include <iostream>
#include <ucontext.h>
#include <queue>
#include <memory>

using namespace std;

queue<ucontext_t> g_context_queue;

void yield()
{
  cout << "yielding" << endl;

  if (!g_context_queue.empty()) {
    g_context_queue.emplace();

    ucontext_t new_ctx = g_context_queue.front();
    g_context_queue.pop();

    swapcontext(&g_context_queue.back(), &new_ctx);
  }
}

void join()
{
  cout << "joining" << endl;
  while(!g_context_queue.empty())
  {
    ucontext_t new_ctx = g_context_queue.front();
    g_context_queue.pop();
    setcontext(&new_ctx);
  }
}

void foo()
{
  for(int i = 0; i < 10; ++i)
  {
    cout << "foo " << i << endl;
    yield();
  }
}

void bar()
{
  for(int i = 0; i < 10; ++i)
  {
    cout << "bar " << i << endl;
    yield();
  }
}

ucontext_t main_ctx;

void push_foo()
{
  static char stack[16384];

  ucontext_t ctx;
  getcontext(&ctx);
  ctx.uc_stack.ss_sp = stack;
  ctx.uc_stack.ss_size = 16384;
  ctx.uc_stack.ss_flags = 0;
  ctx.uc_link = 0;
  makecontext(&ctx, &foo, 0);

  g_context_queue.push(ctx);
}

void push_bar()
{
  static char stack[16384];

  ucontext_t ctx;
  getcontext(&ctx);
  ctx.uc_stack.ss_sp = stack;
  ctx.uc_stack.ss_size = 16384;
  ctx.uc_stack.ss_flags = 0;
  ctx.uc_link = 0;
  makecontext(&ctx, &bar, 0);

  g_context_queue.push(ctx);
}


void run() {
  ucontext_t new_ctx = g_context_queue.front();
  g_context_queue.pop();
  swapcontext(&main_ctx, &new_ctx);
  join();

  setcontext(&main_ctx);
}

int main()
{
  push_foo();
  push_bar();
  run();

  return 0;
}
