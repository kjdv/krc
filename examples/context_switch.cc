#include <iostream>
#include <ucontext.h>
#include <queue>
#include <memory>

using namespace std;

struct context
{
  shared_ptr<ucontext_t> ctx;
  bool needs_push{true};

  context()
    : ctx(make_shared<ucontext_t>())
  {}
};

queue<context> g_context_queue;

void yield()
{
  cout << "yielding" << endl;

  if (!g_context_queue.empty()) {
    context ctx;
    g_context_queue.push(ctx);

    context new_ctx = g_context_queue.front();
    g_context_queue.pop();

    swapcontext(ctx.ctx.get(), new_ctx.ctx.get());
  }

  return;

}

void join()
{
  cout << "joining" << endl;
  while(!g_context_queue.empty())
  {
    context new_ctx = g_context_queue.front();
    g_context_queue.pop();
    setcontext(new_ctx.ctx.get());
  }
   // yield();
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

  context ctx;
  getcontext(ctx.ctx.get());
  ctx.ctx->uc_stack.ss_sp = stack;
  ctx.ctx->uc_stack.ss_size = 16384;
  ctx.ctx->uc_stack.ss_flags = 0;
  ctx.ctx->uc_link = 0;
  makecontext(ctx.ctx.get(), &foo, 0);

  g_context_queue.push(ctx);
}

void push_bar()
{
  static char stack[16384];

  context ctx;
  getcontext(ctx.ctx.get());
  ctx.ctx->uc_stack.ss_sp = stack;
  ctx.ctx->uc_stack.ss_size = 16384;
  ctx.ctx->uc_stack.ss_flags = 0;
  ctx.ctx->uc_link = 0;
  makecontext(ctx.ctx.get(), &bar, 0);

  g_context_queue.push(ctx);
}


void run() {
  context new_ctx = g_context_queue.front();
  g_context_queue.pop();
  swapcontext(&main_ctx, new_ctx.ctx.get());
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
