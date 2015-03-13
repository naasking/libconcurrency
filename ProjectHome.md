# Concurrency in C #

A lightweight concurrency library for C, featuring symmetric [coroutines](http://en.wikipedia.org/wiki/Coroutine) as the main control flow abstraction. The library is similar to [State Threads](http://state-threads.sourceforge.net/), but using coroutines instead of green threads. This simplifies inter-procedural calls and largely eliminates the need for mutexes and semaphores for signaling.

Eventually, coroutine calls will also be able to safely migrate between kernel threads, so the achievable scalability is consequently much higher than State Threads, which is purposely single-threaded.

This library was inspired by Douglas W. Jones' ["minimal user-level thread package"](http://www.cs.uiowa.edu/~jones/opsys/threads/). The pseudo-platform-neutral probing algorithm on the svn trunk is derived from his code.

There is also a [safer, more portable coroutine implementation based on stack copying](http://code.google.com/p/libconcurrency/source/browse/branches/copying-cache-stacks), which was inspired by [sigfpe's page on portable continuations in C](http://homepage.mac.com/sigfpe/Computing/continuations.html). Copying is more portable and flexible than stack switching, and [making copying competitive with switching is being researched](http://higherlogics.blogspot.com/2008/07/coroutines-in-c-redux.html).

## Coroutines ##

Coroutine calls consist of only one operation: `coro_call`. This operation suspends the currently executing coroutine, and resumes the target coroutine passing it a given value. Passing a value in the other direction is exactly the same. Coroutines which use only one operation like this are called _symmetric_ coroutines.

_Asymmetric_ coroutines use two different operations, like call and yield, implying that one caller is subordinate to another. Lua supports asymmetric coroutines.

In many ways, such coroutines closely resemble the Inter-Process Communication (IPC) facilities of microkernel operating systems like [EROS](http://eros-os.org/), and they are also closely related to actors.

[Revisiting Coroutines](http://lambda-the-ultimate.org/node/2868) is a detailed paper explaining the advantages of coroutines as a control flow abstraction. While the authors prefer asymmetric coroutines, and use a coroutine label to avoid capture problems, I think it's simpler to simply use the coroutine reference itself in place of a label.

## Comparison to Existing Alternatives ##

Features libconcurrency provides that are not found in [libcoro](http://software.schmorp.de/pkg/libcoro.html), [libCoroutine](http://www.dekorte.com/projects/opensource/libCoroutine/docs/), [libpcl](http://www.xmailserver.org/libpcl.html), [coro](http://www.goron.de/~froese/coro/):
  * **Cloning a coroutine:** this enables a straightforward implementation of multishot continuations. Without the ability to clone, only one-shot continuations can be implemented in terms of coroutines. Note, cloning is not available on Windows since Windows uses the Fibers API. This was necessary to handle [some unfortunate problems](http://higherlogics.blogspot.com/2008/07/coroutines-in-c-redux.html). The [more portable stack copying implementation does support cloning](http://code.google.com/p/libconcurrency/source/browse/branches/copying-cache-stacks).
  * **Portable Speed:** libconcurrency is based on a portable technique that modifies `jmp_buf` and uses setjmp/longjmp to save and restore contexts. This is fast because setjmp/longjmp do not save and restore signals, whereas most other libraries are based on ucontext, which is very portable but inefficient, or assembler which is efficient but not portable. The library consists of entirely C99 compliant C and no exotic platform or OS features are used.
  * **Actual coroutine API:** most of the listed libraries implement green threads, not coroutines. You'll note that no values are passed between coroutines in these libraries. Passing values between coroutines is one of the primary reasons why they are more general than threads.
  * **[Simpler API](http://code.google.com/p/libconcurrency/source/browse/trunk/libconcurrency/coro.h):** the API consists of only 6 orthogonal functions: `coro_init, coro_call, coro_clone, coro_poll, coro_free`.
  * **Works on Windows:** libconcurrency is primarily developed on Windows, and uses Windows Fibers for coroutines.
  * **Resources are managed:** the `coro_poll` function must be called periodically to ensure a coroutine has enough resources to continue executing. This function can shrink or grow the stack using a hysteresis algorithm inspired by the description of one-shot continuations in [Representing Control in the Presence of One-Shot Continuations](http://citeseer.ist.psu.edu/bruggeman96representing.html).

## Status ##

This library is functional, but not nearly as well developed as something like [State Threads](http://state-threads.sourceforge.net/). If you're willing to do a little work, libconcurrency can provide a good foundation on which to build, but if you're looking for something that provides 95% of what you'll need out of the box, State Threads is by far the more mature project.

## Example ##

Here's a basic reader/writer that simply echoes characters read to the screen:
```
#include <stdio.h>
#include <coro.h>

coro cr;
coro cw;

void reader(cvalue r) {
  while (1) {
	printf("> ");
	r.c = getchar();
	if (r.c != '\n') {
		coro_call(cw, r);
	}
  }
}

void writer(cvalue w) {
  while(1) {
    printf(" echo: %c\n", w.c);
    w = coro_call(cr, cnone);
  }
}

int main(int argc, char **argv) {
  coro _main = coro_init();
  printf("Simple reader/writer echo...\n");
  cr = coro_new(reader);
  cw = coro_new(writer);
  coro_call(cw, cnone);
  return 0;
}
```