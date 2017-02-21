/**
 * @file mcp7941x.h
 *
 */
/* Copyright (C) 2014, 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef MCP7941X_H_
#define MCP7941X_H_

#include <stdint.h>

#define MCP7941X_OK								0
#define MCP7941X_ERROR							1

#define MCP7941X_DEFAULT_SLAVE_ADDRESS			0x6F

struct rtc_time {
	int tm_sec;		///< Seconds.		[0-60]	(1 leap second)
	int tm_min;		///< Minutes.		[0-59]
	int tm_hour;	///< Hours.			[0-23]
	int tm_mday;	///< Day.		 	[1-31]
	int tm_mon;		///< Month.			[0-11]
	int tm_year;	///< Year - 1900
	int tm_wday;	///< Day of week.	[0-6]
	int tm_yday;	///< Days in year.	[0-365]
	int tm_isdst;	///< DST.			[-1/0/1]
};

extern uint8_t mcp7941x_start(uint8_t);
extern void mcp7941x_get_date_time(struct rtc_time *tm);
extern void mcp7941x_set_date_time(/*@out@*/const struct rtc_time *tm);

#endif /* MCP7941X_H_ */
