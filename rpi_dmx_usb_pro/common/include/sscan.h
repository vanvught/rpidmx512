/**
 * @file sscan.h
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifndef INCLUDE_SSCAN_H_
#define INCLUDE_SSCAN_H_

#include <stdint.h>

#if defined(RDM_RESPONDER) || defined(RDM_CONTROLLER)
extern int sscan_uint8_t(const char *, const char *, /*@out@*/uint8_t *);
extern int sscan_char_p(const char *, const char *, /*@out@*/char *, /*@out@*/uint8_t *);
#endif
#if defined(RDM_RESPONDER) || defined(DMX_SLAVE)
extern int sscan_spi(const char *, /*@out@*/char *, /*@out@*/char *, /*@out@*/uint8_t *, /*@out@*/uint8_t *, /*@out@*/uint16_t *);
#endif

#endif /* INCLUDE_SSCAN_H_ */
