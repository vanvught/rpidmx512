/**
 * @file showfileparamsconst.h
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

#ifndef SHOWFILEPARAMSCONST_H_
#define SHOWFILEPARAMSCONST_H_

struct ShowFileParamsConst {
	static inline const char FILE_NAME[] = "show.txt";
	static inline const char SHOW[] = "show";
	static inline const char DMX_MASTER[] = "dmx_master";
	static inline const char OPTION_AUTO_PLAY[] = "auto_play";
	static inline const char OPTION_LOOP[] = "loop";
	static inline const char OPTION_DISABLE_SYNC[] = "disable_sync";
	static inline const char SACN_SYNC_UNIVERSE[] = "sync_universe";
	static inline const char ARTNET_DISABLE_UNICAST[] = "disable_unicast";
};

#endif /* SHOWFILEPARAMSCONST_H_ */
