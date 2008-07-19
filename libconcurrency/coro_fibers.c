/* A coroutine implementation based on fibers */

/* minimum Windows version */
#define _WIN32_WINNT 0x0400

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <libconcurrency/stdint.h>
#include <libconcurrency/coro.h>
#include <libconcurrency/tls.h>
#include <windows.h>

/*
 * The default stack size for new fibers
 */
#define STACK_DEFAULT sizeof(intptr_t) * 4096

/* the coroutine structure */
struct _coro {
	void * fiber;
	_entry start;
};

/*
 * Each of these are local to the kernel thread. Volatile storage is necessary
 * otherwise _value is often cached as a local when switching contexts, so
 * a sequence of calls will always return the first value!
 */
THREAD_LOCAL volatile coro _cur;
THREAD_LOCAL volatile cvalue _value;
THREAD_LOCAL struct _coro _on_exit;

EXPORT
coro coro_init()
{
	_on_exit.fiber = ConvertThreadToFiber(NULL);
	_cur = &_on_exit;
	return _cur;
}

static void __stdcall _coro_enter(void * p)
{
	cvalue _return;
	_return.p = _cur;
	_cur->start(_value);
	coro_call(&_on_exit, _return);
}

EXPORT
coro coro_new(_entry start)
{
	coro c = (coro)malloc(sizeof(struct _coro));
	c->fiber = CreateFiber(STACK_DEFAULT, &_coro_enter, NULL);
	c->start = start;
	return c;
}

EXPORT
cvalue coro_call(coro target, cvalue value)
{
	_value = value;
	_cur = target;
	SwitchToFiber(target->fiber);
	return _value;
}

EXPORT
void coro_free(coro c)
{
	DeleteFiber(c->fiber);
	free(c);
}

EXPORT
void coro_poll()
{
	/* no-op with fibers */
}
