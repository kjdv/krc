#include <iostream>
#include <ucontext.h>
#include <queue>

using namespace std;

struct context
{
  ucontext_t ctx;
  bool needs_push{true};

  context()
  {
    getcontext(&ctx);
  }
};

queue<context> g_context_queue;

void yield()
{
  cout << "yielding" << endl;

  context ctx;

  if (ctx.needs_push) {
    ctx.needs_push = false;
    g_context_queue.push(ctx);
    context new_ctx = g_context_queue.front();
    g_context_queue.pop();

    setcontext(&new_ctx.ctx);
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

int main()
{
  foo();

  return 0;
}
