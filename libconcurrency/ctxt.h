#ifndef __CTXT_H__
#define __CTXT_H__

#include <setjmp.h>
#include <stdint.h>

/* context management definitions */
typedef jmp_buf _ctxt;

#ifndef _setjmp
#define _setjmp setjmp
#endif

#ifndef _longjmp
#define _longjmp longjmp
#endif

#define _save_and_resumed(c) _setjmp(c)
#define _rstr_and_jmp(c) _longjmp(c, 1)

/* true if stack grows up, false if down */
static int _stack_grows_up;

static void _infer_direction_from(int *first_addr) {
	int second;
	_stack_grows_up = (first_addr < &second);
}

static void _infer_stack_direction() {
	int first;
	_infer_direction_from(&first);
}

#endif /*__CTXT_H__*/