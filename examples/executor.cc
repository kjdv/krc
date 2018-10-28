#include <executor.hh>
#include <iostream>

using namespace krc;
using namespace std;

namespace {

void bar()
{
  for (int i = 0; i < 10; ++i)
  {
    cout << "bar " << i << endl;
    executor::instance().yield();
  }

  cout << "bar done" << endl;
}

void foo()
{
  executor::instance().push(bar);

  for (int i = 0; i < 10; ++i)
  {
    cout << "foo " << i << endl;
    executor::instance().yield();
  }

  cout << "foo done" << endl;
}

}

int main()
{
  auto &exec = executor::instance();
  exec.run(foo);

  cout << "all done" << endl;

  return 0;
}
