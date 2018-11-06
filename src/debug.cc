#include "debug.hh"
#include "context.hh"
#include <thread>
#include <string>
#include <iostream>
#include <sstream>
#include <mutex>
#include <queue>
#include <atomic>

namespace krc {

namespace {

using namespace std;

class logger
{
public:
    explicit logger()
        : d_thread([this] { this->run(); })
    {}

    ~logger()
    {
        d_closed.store(true);
        d_thread.join();
    }

    void post(string_view msg)
    {
        lock_t l(d_mut);
        d_msg.push(string(msg));
    }

private:
    typedef unique_lock<std::mutex> lock_t;

    void run()
    {
        while(!d_closed.load())
        {
            string msg;
            {
                lock_t l(d_mut);
                if (d_msg.empty())
                {
                    l.unlock();
                    this_thread::sleep_for(chrono::milliseconds(10));
                    continue;
                }

                msg = d_msg.front(); d_msg.pop();
            }
            cout << msg << flush;
        }
    }

    queue<string> d_msg;
    std::mutex d_mut;
    thread d_thread;
    atomic<bool> d_closed{false};
};

logger s_logger;

}

#ifdef DEBUG_THIS
void debug(string_view msg)
{
    ostringstream s;
    s << "thread=" << this_thread::get_id() << " routine=" << context<>::get_id() << " " << msg << "\n";

    s_logger.post(s.str());
}
#endif

}
