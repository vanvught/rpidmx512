/*
 * abort.cpp
 */

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>

extern "C" void abort() { // NOLINT
    assert(0);
    for (;;) {
    }
}
