/*
 * Copyright (c) 2013-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

extern "C" void* memchr(const void* src, int c, size_t len) { // NOLINT
    auto* s = reinterpret_cast<const unsigned char*>(src);

    while (len--) {
        if (*s == (unsigned char)c) return (void*)s;
        s++;
    }

    return nullptr;
}
