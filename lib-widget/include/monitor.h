/**
 * @file monitor.h
 *
 */
/* Copyright (C) 2015-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef MONITOR_H_
#define MONITOR_H_

#include <stdarg.h>

#define MONITOR_LINE_TIME			3	///<
#define MONITOR_LINE_WIDGET_PARMS	4	///<
#define MONITOR_LINE_LABEL			6	///<
#define MONITOR_LINE_INFO			7	///<
#define MONITOR_LINE_PORT_DIRECTION	9	///<
#define MONITOR_LINE_DMX_DATA		11	///<
#define MONITOR_LINE_PACKETS		14	///<
#define MONITOR_LINE_RDM_DATA		17	///<
#define MONITOR_LINE_RDM_CC			27	///<
#define MONITOR_LINE_STATUS			28	///<
#define MONITOR_LINE_STATS			29	///< last line when HEIGHT = 480 and CHAR_H = 16, 480/16 = 30, line starts at 0

#ifdef __cplusplus
extern "C" {
#endif

extern void monitor_line(int, /*@null@*/ const char *, ...) /*@modifies *stdout, errno@*/;
extern void monitor_time_uptime(int);

#ifdef __cplusplus
}
#endif

#endif /* MONITOR_H_ */
