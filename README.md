# libconcurrency

A proof-of-concept lightweight concurrency library for C, featuring symmetric [coroutines](http://en.wikipedia.org/wiki/Coroutine) as the main control flow abstraction. The library is similar to [State Threads](http://state-threads.sourceforge.net/), but using coroutines instead of green threads. This simplifies inter-procedural calls and largely eliminates the need for mutexes and semaphores for signaling.

This library was inspired by [Douglas W. Jones' "minimal user-level thread package"](http://www.cs.uiowa.edu/~jones/opsys/threads/). The pseudo-platform-neutral probing algorithm on the svn trunk is derived from his code, but simplified.

There is also a safer, more portable coroutine implementation based on [stack copying in the copying branch](https://github.com/naasking/libconcurrency/tree/copying), which was inspired by sigfpe's page on portable continuations in C.

# Coroutines

Coroutine calls consist of only one operation: coro_call. This operation suspends the currently executing coroutine, and resumes the target coroutine passing it a given value. Passing a value in the other direction is exactly the same. Coroutines which use only one operation like this are called symmetric coroutines.

Asymmetric coroutines use two different operations, like call and yield, implying that one caller is subordinate to another. Lua supports asymmetric coroutines.

In many ways, such coroutines closely resemble the Inter-Process Communication (IPC) facilities of microkernel operating systems like EROS, and they are also closely related to actors.

[Revisiting Coroutines](http://lambda-the-ultimate.org/node/2868) is a detailed paper explaining the advantages of coroutines as a control flow abstraction. While the authors prefer asymmetric coroutines, and use a coroutine label to avoid capture problems, I think it's simpler to simply use the coroutine reference itself in place of a label.

# Status
This library is a functional prototype, but if you're looking for something to use in production, I recommend State Threads.

# Example
Here's a basic reader/writer that simply echoes characters read to the screen:

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
  
