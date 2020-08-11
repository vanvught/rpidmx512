/**
 * @file stdint.h
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef STDINT_H_
#define STDINT_H_

typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;
typedef unsigned long long	uint64_t;

typedef signed char			int8_t;
typedef signed short		int16_t;
typedef signed int			int32_t;
typedef signed long long	int64_t;

typedef int 				intptr_t;
typedef unsigned int 		uintptr_t;

#if !defined(UINT32_MAX)
 #ifdef __cplusplus
  #define UINT32_MAX	(static_cast<uint32_t>(-1))
 #else
  #define UINT32_MAX	((uint32_t)-1)
 #endif
#endif


#define INT16_MIN   (-0x7fff - 1)

#define INT16_MAX   0x7fff

#endif

