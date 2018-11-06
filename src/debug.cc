#include "debug.hh"
#include "context.hh"
#include <thread>
#include <iostream>
#include <sstream>

namespace krc {

using namespace std;

#ifdef DEBUG_THIS
void debug(string_view msg)
{
    ostringstream s;
    s << "thread=" << this_thread::get_id() << " routine=" << context<>::get_id() << " " << msg << "\n";

    std::cout << s.str() << std::endl;
}
#endif

}
