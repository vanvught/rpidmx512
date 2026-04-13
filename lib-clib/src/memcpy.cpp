#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC optimize("-fno-strict-aliasing")

#include <stddef.h>
#include <stdint.h>

extern "C" void* memcpy(void* dst, const void* src, size_t n) { // NOLINT
    unsigned char* d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;

    // Align to word boundary
    while (n && (((uintptr_t)d | (uintptr_t)s) & (sizeof(uint32_t) - 1))) {
        *d++ = *s++;
        --n;
    }

    uint32_t* dw = (uint32_t*)d;
    const uint32_t* sw = (const uint32_t*)s;

    while (n >= 16) { // unroll
        dw[0] = sw[0];
        dw[1] = sw[1];
        dw[2] = sw[2];
        dw[3] = sw[3];
        dw += 4;
        sw += 4;
        n -= 16;
    }

    while (n >= 4) {
        *dw++ = *sw++;
        n -= 4;
    }

    d = (unsigned char*)dw;
    s = (const unsigned char*)sw;

    while (n--) *d++ = *s++;

    return dst;
}

#pragma GCC pop_options