/**
 * @file display_debug.h
 *
 */
 /* Copyright (C) 2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DISPLAY_DEBUG_H_
#define DISPLAY_DEBUG_H_

#ifdef DEBUG_DISPLAY
#define DISPLAY_DEBUG_ENTRY() DEBUG_ENTRY()
#define DISPLAY_DEBUG_EXIT() DEBUG_EXIT()
#define DISPLAY_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#define DISPLAY_DEBUG_PUTS(...) DEBUG_PUTS(__VA_ARGS__)
#else
#define DISPLAY_DEBUG_ENTRY() \
    do {                      \
    } while (false)
#define DISPLAY_DEBUG_EXIT() \
    do {                     \
    } while (false)
#define DISPLAY_DEBUG_PRINTF(...) \
    do {                          \
    } while (false)
#define DISPLAY_DEBUG_PUTS(...) \
    do {                        \
    } while (false)
#endif

#include "firmware/debug/debug_debug.h"

#endif // DISPLAY_DEBUG_H_
