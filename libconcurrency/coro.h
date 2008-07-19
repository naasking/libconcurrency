#ifndef __CORO_H__
#define __CORO_H__

/*
 * Portable coroutines for C. Caveats:
 *
 * 1. You should not take the address of a stack variable, since stack management
 *    could reallocate the stack, the new stack would reference a variable in the
 *    old stack. Also, cloning a coroutine would cause the cloned coroutine to
 *    reference a variable in the other stack.
 * 2. You must call coro_init for each kernel thread, since there are thread-local
 *    data structures. This will eventually be exploited to scale coroutines across
 *    CPUs.
 * 3. If setjmp/longjmp inspect the jmp_buf structure before executing a jump, this
 *    library probably will not work.
 *
 * Refs:
 * http://www.yl.is.s.u-tokyo.ac.jp/sthreads/
 */

#include <libconcurrency/tls.h>
#include <stdlib.h>

/* a coroutine handle */
typedef struct _coro *coro;

/* the type of value passed between coroutines */
typedef union _value {
	void *p;
	unsigned u;
	int i;
	char c;
} cvalue;

/* the type of entry function */
typedef void (*_entry)(cvalue v);

static cvalue cnone;

/*
 * Initialize the coroutine library, returning a coroutine for the thread that called init.
 */
EXPORT
extern coro coro_init();

/*
 * Create a new coroutine from the given function, and with the
 * given stack.
 */
EXPORT
extern coro coro_new(_entry fn);

/*
 * Invoke a coroutine passing the given value.
 */
EXPORT
extern cvalue coro_call(coro target, cvalue value);

/*
 * Clone a given coroutine. This can be used to implement multishot continuations.
 */
/*EXPORT
coro coro_clone(coro c);*/

/*
 * Free the coroutine and return the space for the stack.
 */
EXPORT
extern void coro_free(coro c);

/*
 * Poll the current coroutine to ensure sufficient resources are allocated. This
 * should be called periodically to ensure a coroutine doesn't segfault.
 */
EXPORT
extern void coro_poll();

#endif /* __CORO_H__ */
