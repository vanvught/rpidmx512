#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("no-tree-loop-distribute-patterns")

#include <stddef.h>

extern "C" void* memmove(void* dst, const void* src, size_t n) { // NOLINT
    char* dp = (char*)dst;
    const char* sp = (const char*)src;

    if (dp < sp) {
        while (n--) {
            *dp++ = *sp++;
        }
    } else {
        sp += n;
        dp += n;
        while (n--) {
            *--dp = *--sp;
        }
    }

    return dst;
}

#pragma GCC pop_options