/*
 * cxa_atexit.cpp
 */

#include <cstddef>

typedef void (*exitfunc_t)();

static exitfunc_t atexit_funcs[32];
static size_t atexit_count = 0;

extern "C" int __cxa_atexit(exitfunc_t func, [[maybe_unused]] void *arg, [[maybe_unused]] void *dso_handle) {
	if (atexit_count >= sizeof(atexit_funcs) / sizeof(atexit_funcs[0]))
		return -1;

	atexit_funcs[atexit_count++] = func;
	return 0; // Success
}

extern "C" void __call_atexit_funcs() {
	for (size_t i = atexit_count; i > 0; --i) {
		exitfunc_t func = atexit_funcs[i - 1];
		if (func)
			func();
	}
}
