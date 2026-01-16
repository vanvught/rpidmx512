/**
 * @file if.h
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef NET_IF_H_
#define NET_IF_H_

/**
 * https://pubs.opengroup.org/onlinepubs/009695399/basedefs/net/if.h.html
 *
 * The <net/if.h> header shall define the following symbolic constant for the length of a buffer containing
 * an interface name (including the terminating NULL character):
 * IF_NAMESIZE Interface name length.
 */

#define IF_NAMESIZE		16	///< Length of interface name (including the terminating NULL character).

#ifndef IFNAMSIZ
# define IFNAMSIZ		IF_NAMESIZE
#endif

#endif /* NET_IF_H_ */
