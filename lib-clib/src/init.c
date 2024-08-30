/*
 * init.c
 */

#include "stddef.h"

extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));

void __libc_init_array(void) {
	  size_t count;
	  size_t i;

	  count = (size_t)(__preinit_array_end - __preinit_array_start);
	  for (i = 0; i < count; i++)
	    __preinit_array_start[i] ();

	  count = (size_t)(__init_array_end - __init_array_start);
	  for (i = 0; i < count; i++)
	    __init_array_start[i] ();
}
