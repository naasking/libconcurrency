/*
 * Compiler-provided Thread-Local Storage
 * TLS declarations for various compilers:
 * http://en.wikipedia.org/wiki/Thread-local_storage
 */

#ifndef __TLS_H__
#define __TLS_H__

/* Each #ifdef must define
 * THREAD_LOCAL
 * EXPORT
 */

#ifdef _MSC_VER

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define THREAD_LOCAL __declspec(thread)
#define EXPORT __declspec(dllexport)

#else
/* assume gcc or compatible compiler */
#define THREAD_LOCAL __thread
#define EXPORT

#endif /* _MSC_VER */

#endif /*__TLS_H__*/
