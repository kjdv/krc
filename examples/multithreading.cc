#include <runtime.hh>
#include <channel.hh>
#include <iostream>

using namespace std;
using namespace krc;

namespace {

enum {
   NUM_THREADS = 4,
   NUM_PRODUCERS = NUM_THREADS * 2,
   STACK_SIZE = 1<<24,
   MAX_INT = 10000,
};

bool prime_test(unsigned p, unsigned i)
{
    assert(i > 0);
    if(i == 1)
        return true;
    else if(p % i == 0)
        return false;
    else
        return prime_test(p, i - 1);
}

bool is_prime(unsigned p)
{
    assert(p > 1);
    return prime_test(p, p - 1);
}

void print(channel<unsigned> ch, channel<bool> done)
{
    for(auto p : ch)
        cout << p << " is prime" << endl;
    done.push(true);
}

void checker(channel<unsigned> in, channel<unsigned> out, channel<bool> done)
{
    for(auto p : in)
    {
        if(is_prime(p))
            out.push(p);
    }
    done.push(true);
}

void check_for_primes()
{
    channel<unsigned> in;
    channel<unsigned> out;
    channel<bool> done;

    krc::dispatch([out, done] { print(out, done); });

    for(int i = 0; i < NUM_PRODUCERS; ++i)
        krc::dispatch(target_t([in, out, done] { checker(in, out, done); }, STACK_SIZE));

    for(unsigned p = 2; p < MAX_INT; ++p)
        in.push(p);

    in.close();

    // wait for subroutines to be done
    for(unsigned i = 0; i < NUM_PRODUCERS; ++i)
        done.pull();
    // print
    out.close();
    done.pull();
}

}

int main()
{
    krc::run(check_for_primes, NUM_THREADS);
    cout << "done" << endl;
    return 0;
}
