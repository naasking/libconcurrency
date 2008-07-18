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

#include <tls.h>

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
typedef cvalue (*fun)(cvalue);

const cvalue cnone = { NULL };

/*
 * Initialize the coroutine library, returning a coroutine for the thread that called init.
 */
EXPORT
coro reset(fun f, cvalue v);

EXPORT
void shift(cvalue * v);

EXPORT
cvalue apply(dcont d, cvalue c);

dcont_free(dcont d);

#endif /* __CORO_H__ */