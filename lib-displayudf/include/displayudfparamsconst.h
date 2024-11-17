/**
 * @file displayudfparamsconst.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DISPLAYUDFPARAMSCONST_H_
#define DISPLAYUDFPARAMSCONST_H_

struct DisplayUdfParamsConst {
	static inline const char FILE_NAME[] = "display.txt";
	static inline const char INTENSITY[] = "intensity";
	static inline const char SLEEP_TIMEOUT[] = "sleep_timeout";
	static inline const char FLIP_VERTICALLY[] = "flip_vertically";
	static inline const char TITLE[] = "title";
	static inline const char BOARD_NAME[] = "board_name";
	static inline const char VERSION[] = "version";
	static inline const char ACTIVE_PORTS[] = "active_ports";
	static inline const char DMX_DIRECTION[] = "dmx_direction";
};

#endif /* DISPLAYUDFPARAMSCONST_H_ */
