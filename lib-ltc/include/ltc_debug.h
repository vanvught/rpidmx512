/**
 * @file ltc_debug.h
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
 
#ifndef LTC_DEBUG_H_
#define LTC_DEBUG_H_

#ifdef DEBUG_LTC
#define LTC_DEBUG_ENTRY() DEBUG_ENTRY()
#define LTC_DEBUG_EXIT() DEBUG_EXIT()
#define LTC_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#define LTC_DEBUG_PUTS(...) DEBUG_PUTS(__VA_ARGS__)
#else
#define LTC_DEBUG_ENTRY() \
    do {                     \
    } while (false)
#define LTC_DEBUG_EXIT() \
    do {                    \
    } while (false)
#define LTC_DEBUG_PRINTF(...) \
    do {                         \
    } while (false)
#define LTC_DEBUG_PUTS(...) \
    do {                       \
    } while (false)
#endif

#ifdef DEBUG_LTC_ETC
#define LTC_ETC_DEBUG_ENTRY() DEBUG_ENTRY()
#define LTC_ETC_DEBUG_EXIT() DEBUG_EXIT()
#define LTC_ETC_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#define LTC_ETC_DEBUG_PUTS(...) DEBUG_PUTS(__VA_ARGS__)
#else
#define LTC_ETC_DEBUG_ENTRY() \
    do {                     \
    } while (false)
#define LTC_ETC_DEBUG_EXIT() \
    do {                    \
    } while (false)
#define LTC_ETC_DEBUG_PRINTF(...) \
    do {                         \
    } while (false)
#define LTC_ETC_DEBUG_PUTS(...) \
    do {                       \
    } while (false)
#endif

#ifdef DEBUG_NTP_SERVER
#define NTP_SERVER_DEBUG_ENTRY() DEBUG_ENTRY()
#define NTP_SERVER_DEBUG_EXIT() DEBUG_EXIT()
#define NTP_SERVER_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#define NTP_SERVER_DEBUG_PUTS(...) DEBUG_PUTS(__VA_ARGS__)
#else
#define NTP_SERVER_DEBUG_ENTRY() \
    do {                     \
    } while (false)
#define NTP_SERVER_DEBUG_EXIT() \
    do {                    \
    } while (false)
#define NTP_SERVER_DEBUG_PRINTF(...) \
    do {                         \
    } while (false)
#define NTP_SERVER_DEBUG_PUTS(...) \
    do {                       \
    } while (false)
#endif

#endif // LTC_DEBUG_H_
