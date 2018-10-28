#include "mutex.hh"
#include "executor.hh"

namespace krc {

mutex::mutex()
{
}

void mutex::lock()
{
  auto& exec = executor::instance();
  while(!try_lock())
  {
    if(!exec.yield())
    {
      // when there is nothing to yield to, save on battery and a regular lock
      std::lock_guard<std::mutex> l(d_base);
      d_held = true;
      return;
    }
  }
}

void mutex::unlock()
{
  {
    std::lock_guard<std::mutex> l(d_base);
    d_held = false;
  }

  auto& exec = executor::instance();
  exec.yield(); // this could well unblock someone else
}

bool mutex::try_lock()
{
  std::lock_guard<std::mutex> l(d_base);
  if(d_held)
    return false;

  d_held = true;
  return true;
}

} // namespace krc
