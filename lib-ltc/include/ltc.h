/**
 * @file ltc.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef LTC_H_
#define LTC_H_

struct TLtcTimeCode {
	uint8_t nFrames;		///< Frames time. 0 – 29 depending on mode.
	uint8_t nSeconds;		///< Seconds. 0 - 59.
	uint8_t nMinutes;		///< Minutes. 0 - 59.
	uint8_t nHours;			///< Hours. 0 - 59.
	uint8_t nType;			///< 0 = Film (24fps) , 1 = EBU (25fps), 2 = DF (29.97fps), 3 = SMPTE (30fps)
};

enum TTimecodeTypes {
	TC_TYPE_FILM = 0,
	TC_TYPE_EBU,
	TC_TYPE_DF,
	TC_TYPE_SMPTE,
	TC_TYPE_UNKNOWN,
	TC_TYPE_INVALID = 255
};

struct TLtcDisabledOutputs {
	bool bDisplay;
	bool bMax7219;
	bool bMidi;
	bool bArtNet;
	bool bTCNet;
	bool bLtc;
	bool bNtp;
};

#define TC_CODE_MAX_LENGTH	11
#define TC_TYPE_MAX_LENGTH	11

class Ltc {
public:
	static const char *GetType(TTimecodeTypes tTimeCodeType);
	static TTimecodeTypes GetType(uint8_t nFps);

private:
};

#endif /* LTC_H_ */
