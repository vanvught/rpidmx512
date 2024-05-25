/*
 * abort.c
 */


#ifdef NDEBUG
# undef NDEBUG
#endif

#include <assert.h>

void abort(void) {
	assert(0);
	for(;;);
}
