/*
 * Co-routines in C
 *
 * Possible implementations:
 * 1. stack switching - need to constantly check stack usage (I use this).
 * 2. stack copying - essentially continuations.
 *
 * Notes:
 * * termination of a coroutine without explicit control transfer returns control
 *   to the coroutine which initialized the coro library.
 *
 * Todo:
 * 1. Co-routines must be integrated with any VProc/kernel-thread interface, since
 *    an invoked co-routine might be running on another cpu. A coro invoker must
 *    check that the target vproc is the same as the current vproc; if not, queue the
 *    invoker on the target vproc using an atomic op.
 * 2. VCpu should implement work-stealing, ie. when its run queue is exhausted, it
 *    should contact another VCpu and steal a few of its coros, after checking its
 *    migration queues of course. The rate of stealing should be tuned:
 *    http://www.cs.cmu.edu/~acw/15740/proposal.html
 * 3. Provide an interface to register a coroutine for any errors generated. This is
 *    a type of general Keeper, or exception handling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <libconcurrency/coro.h>
#include <libconcurrency/tls.h>
#include "ctxt.h"

/* the coroutine structure */
struct _coro {
	_ctxt ctxt;
	_entry start;
	void * env;
	size_t env_size;
};
static cvalue cnone = { NULL };

/*
 * Each of these are local to the kernel thread. Volatile storage is necessary
 * otherwise _value is often cached as a local when switching contexts, so
 * a sequence of calls will always return the first value!
 */
THREAD_LOCAL volatile coro _cur;
THREAD_LOCAL volatile cvalue _value;
THREAD_LOCAL struct _coro _on_exit;
THREAD_LOCAL intptr_t _sp_base;

/*
 * We probe the current machine and extract the data needed to modify the
 * machine context. The current thread is then initialized as the currently
 * executing coroutine.
 */
EXPORT
coro coro_init(void * sp_base)
{
	_infer_stack_direction();
	_sp_base = (intptr_t)sp_base;
	_cur = &_on_exit;
	return _cur;
}

void _coro_save(coro to)
{
	intptr_t mark = (intptr_t)&mark;
	void * sp = (void *)(_stack_grows_up ? _sp_base : mark);
	size_t sz = (_stack_grows_up ? mark - _sp_base : _sp_base - mark);
	/* copy stack to a save buffer */
	/*if (to->env != NULL) {
		perror("Needed to free buffer!\n");
		free(to->env);
	}*/
	to->env = malloc(sz);
	to->env_size = sz;
	memcpy(to->env, sp, sz);
}

void _coro_restore(size_t sz, intptr_t target)
{
	intptr_t top = (intptr_t)&top;
	if (_stack_grows_up && top > target || !_stack_grows_up && top < target) {
		void * sp = (void *)(_stack_grows_up ? _sp_base : _sp_base - sz);
		memcpy(sp, _cur->env, sz);
		/*if (_cur->env == NULL) {
			perror("Already freed!\n");
		} else {
			free(_cur->env);
		}
		_cur->env = NULL;*/
		free(_cur->env);
		_rstr_and_jmp(_cur->ctxt);
	} else {
		/* recurse until the stack depth is greater than target stack depth */
		void * padding[64];
		_coro_restore(sz, target);
	}
}

/*
 * This function invokes the start function of the coroutine when the
 * coroutine is first called. If it was called from coro_new, then it sets
 * up the stack and initializes the saved context.
 */
void _coro_enter(coro c)
{
	if (_save_and_resumed(c->ctxt))
	{	/* start the coroutine; stack is empty at this point. */
		cvalue _return;
		_return.p = _cur;
		_cur->start(_value);
		/* return the exited coroutine to the exit handler */
		coro_call(&_on_exit, _return);
	}
	else
	{
		_coro_save(c);
	}
}

EXPORT
coro coro_new(_entry fn)
{
	coro c = (coro)malloc(sizeof(struct _coro));
	c->env_size = 0;
	c->env = NULL;
	c->start = fn;
	_coro_enter(c);
	return c;
}

/*
 * First, set the value in the volatile global. If _value were not volatile, this value
 * would be cached on the stack, and hence saved and restored on every call. We then
 * save the context for the current coroutine, set the target coroutine as the current
 * one running, then restore it. The target is now running, and it simply returns _value.
 */
EXPORT
cvalue coro_call(coro target, cvalue value)
{
	/* FIXME: ensure target is on the same proc as cur, else, migrate cur to target->proc */

	_value = value; /* pass value to 'target' */
	if (!_save_and_resumed(_cur->ctxt))
	{
		/* we are calling someone else, so we set up the environment, and jump to target */
		intptr_t target_top = (_stack_grows_up
			? _sp_base + target->env_size
			: _sp_base - target->env_size);
		_coro_save(_cur);
		_cur = target;
		_coro_restore(_cur->env_size, target_top);
	}
	/* when someone called us, just return the value */
	return _value;
}

EXPORT
coro coro_clone(coro c)
{
	coro cnew = (coro)malloc(sizeof(struct _coro));
	size_t sz = c->env_size;
	void * env = malloc(sz);
	cnew->env = env;
	cnew->env_size = sz;
	/* copy the context then the stack data */
	memcpy(cnew, c, sizeof(struct _coro));
	memcpy(env, c->env, sz);
	return cnew;
}

EXPORT
void coro_free(coro c)
{
	if (c->env != NULL)
	{
		free((void *)c->env);
	}
	free(c);
}

EXPORT
void coro_poll()
{
	/* no-op */
}
