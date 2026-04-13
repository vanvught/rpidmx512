/*
 * abort.cpp
 */

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>

extern "C" void abort() { // NOLINT
    assert(0);
    for (;;);
}
