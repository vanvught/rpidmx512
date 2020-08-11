/**
 * @file rtc.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RTC_H_
#define RTC_H_

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

typedef enum rtc_types {
	RTC_MCP7941X,
	RTC_DS3231,
	RTC_PROBE
} rtc_types_t;

#define RTC_OK			0
#define RTC_ERROR		1

#ifdef __cplusplus
extern "C" {
#endif

extern bool rtc_start(rtc_types_t type);
extern bool rtc_is_connected(void);
extern rtc_types_t rtc_get_type(void);

extern void rtc_get_date_time(struct tm *tm);
extern void rtc_set_date_time(/*@out@*/const struct tm *tm);

#ifdef __cplusplus
}
#endif

#endif /* RTC_H_ */
