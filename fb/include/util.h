#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stddef.h>

void *qmemcpy(void *dest, void *src, size_t n);
void *quick_memcpy(void *dest, void *src, size_t n);

#define byte_swap __builtin_bswap32

#endif

