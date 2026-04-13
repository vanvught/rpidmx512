/*
 * Copyright (c) 2013-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

extern "C" int memcmp(const void* s1, const void* s2, size_t len) { // NOLINT

    auto* s = reinterpret_cast<const unsigned char*>(s1);
    auto* d = reinterpret_cast<const unsigned char*>(s2);

    unsigned char sc;
    unsigned char dc;

    while (len--) {
        sc = *s++;
        dc = *d++;
        if (sc - dc) return (sc - dc);
    }

    return 0;
}
