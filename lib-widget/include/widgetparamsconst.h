/**
 * @file widgetparamsconst.h
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

#ifndef WIDGETPARAMSCONST_H_
#define WIDGETPARAMSCONST_H_

struct WidgetParamsConst {
	 static inline const char FILE_NAME[] = "params.txt";
	 static inline const char DMXUSBPRO_BREAK_TIME[] = "dmxusbpro_break_time";
	 static inline const char DMXUSBPRO_MAB_TIME[] = "dmxusbpro_mab_time";
	 static inline const char DMXUSBPRO_REFRESH_RATE[] = "dmxusbpro_refresh_rate";
	 static inline const char WIDGET_MODE[] = "widget_mode";
	 static inline const char DMX_SEND_TO_HOST_THROTTLE[] = "dmx_send_to_host_throttle";
};

#endif /* WIDGETPARAMSCONST_H_ */
