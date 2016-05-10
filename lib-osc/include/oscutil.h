//
// oscutil.h
//
#ifndef _oscutil_h
#define _oscutil_h

#include <circle/types.h>
#include <circle/alloc.h>
#include <circle/util.h>

#define NULL 	0		// TODO: should use 0 instead

void *calloc (size_t nmemb, size_t size);

void *realloc (void *ptr, size_t size);

void printf (const char *fmt, ...);

#endif
