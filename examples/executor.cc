#include <executor.hh>
#include <iostream>

using namespace krc;
using namespace std;

namespace {

void foo()
{
  for (int i = 0; i < 10; ++i)
  {
    cout << "foo " << i << endl;
    executor::instance().yield();
  }

  cout << "foo done" << endl;
}

void bar()
{
  for (int i = 0; i < 10; ++i)
  {
    cout << "bar " << i << endl;
    executor::instance().yield();
  }

  cout << "bar done" << endl;
}

}

int main()
{
  auto &exec = executor::instance();

  exec.push(foo);
  exec.push(bar);
  exec.run();

  cout << "all done" << endl;

  return 0;
}
