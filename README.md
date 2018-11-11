# KRC

Klaas' Routine and Channel Library.

An attempt to bring concurrency in the style of Go to C++, aiming at both the ease of use of their channels and the user-space cooperative multithreading that (besides a marginal and theoretical performance advantage) mostly does away with the need for more complicated asynchronous i/o.

This has to be seen as an exploration mostly to satisfy the curiosity of the author. For real-world production usage you are probably better of using existing implementations like [Boost.Fiber](https://www.boost.org/doc/libs/1_66_0/libs/fiber/doc/html/fiber/overview.html).

## Important caveats

At the time of writing only message passing within a single thread is supported. This is an obvious and likely show-stopping limitation for any serious application.

Another caveat is that started routines must not leak exceptions, doing so is undefined behaviour.

## Usage

### Starting, dispatching and yielding to jobs

C.f. `examples/executor.cc`.

To start using the user-land contexts, call `run()` with the top-level function:

```c++
#include <runtime.hh>

void foo();

/* ... */

krc::run(foo);
// run will return once foo() (and alls its subroutines, if any) has finished
```

To start another subroutine, use `dispatch()`. Note this is undefined behaviour unless `dispatch()` is called from within a krc-managed routine.

```c++
void bar();

void foo()
{
    krc::dispatch(bar);
    /* ... */
}
```

An important difference between these user-land 'threads' and the traditional kernel-land threads is that, unlike kernel-land threads, these are cooperative. This is a bit of a misnomer as they are actually uncooperative by default. Control will not be yielded to other routines unless you explicitly say so.

In the above example, `bar()` will not run until an explicit event yields control. These include acquiring or releasing mutexes or waiting for an i/o operation to complete (more on this later). This can also be done explicitely:

```c++

void bar()
{
  /* ... */
  krc::yield(); // allows foo() to run
  /* ... */
}

void foo()
{
  krc::dispatch(bar);
  /* ... */
  krc::yield(); // allows bar() to run
  /* ... */
}

```

### Channels

C.f. `examples/fibonacci.cc`

The recommended mechanism to communicate between routines are channels. This is, effectively, just an abstraction over the well-known synchronized queue.

Channels can be created buffered or unbuffered:

```c++
channel<int> unbuffered; // no buffer, each push() will block until another routine pull()s
channel<int> buffered(10); // buffered, the channel can be push()ed to 10 times without pull()ing before it blocks
```

Pushing and pulling takes this form
```c++
channel<int> ch;
bool rc = ch.push(1);    // returns true, unless ch.close() has been called
optional<int> ch.pull(); // returns the push()ed to value, in fifo-order. Returns no-value is the channel is closed.
ch.close();              // this indicates no more push() operations are expected.
```

When consuming from a channel, range-based for loops are supported:

```c++
channel<int> ch;
for (int i : ch) // loops until ch.close() is called by the producing end
{
  // use i
}
```

An important aspect of channels is that pushing to and pulling from them can block. If this block control is implicitely yielded to another routine.

### I/O

C.f. `examples/io.cc`.

The user-land threads are most useful when they can do away with most needs for more complicated asynchronous i/o. The well known unix [read()](https://www.freebsd.org/cgi/man.cgi?sektion=2&query=read) and [write](https://www.freebsd.org/cgi/man.cgi?sektion=2&query=write) system calls can be replaced with equivalents with the same signature and semantics in the `krc::io` namespace:

```c++
namespace krc {
namespace io {
ssize_t read(int fd, void* buf, size_t n);
ssize_t write(int fd, const void* buf, size_t n);
}
}
```

When these are called, rather than (potentially) blocking control will be yielded to other routines until the requested operation completes (or fails with an error).

Future improvements will include wrapping these in custom `streambuf`s so they can be used with C++ iostreams.
