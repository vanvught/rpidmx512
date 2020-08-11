/**
 * @file tlc59711.h
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#ifndef TLC59711_H_
#define TLC59711_H_

struct TLC59711SpiSpeed {
	static constexpr uint32_t DEFAULT = 5000000;	// 5 MHz
	static constexpr uint32_t MAX = 10000000;		// 10 MHz
};

struct TLC59711Channels {
	static constexpr uint32_t U16BIT = 14;
	static constexpr uint32_t OUT = 12;
	static constexpr uint32_t RGB = 4;
};

class TLC59711 {
public:
	TLC59711(uint8_t nBoards = 1, uint32_t nSpiSpeedHz = TLC59711SpiSpeed::DEFAULT);
	~TLC59711();

	int GetBlank() const;
	void SetBlank(bool pBlank = false);

	int GetDisplayRepeat() const;
	void SetDisplayRepeat(bool pDisplayRepeat = true);

	int GetDisplayTimingReset() const;
	void SetDisplayTimingReset(bool pDisplayTimingReset = true);

	int GetExternalClock() const;
	void SetExternalClock(bool pExternalClock = false);

	int GetOnOffTiming() const;
	void SetOnOffTiming(bool pOnOffTiming = false);

	uint8_t GetGbcRed() const;
	void SetGbcRed(uint8_t nValue = 0x7F);

	uint8_t GetGbcGreen() const;
	void SetGbcGreen(uint8_t nValue = 0x7F);

	uint8_t GetGbcBlue() const;
	void SetGbcBlue(uint8_t nValue = 0x7F);

	bool Get(uint8_t nChannel, uint16_t &nValue);
	void Set(uint8_t nChannel, uint16_t nValue);

	void Set(uint8_t nChannel, uint8_t nValue);

	bool GetRgb(uint8_t nOut, uint16_t &nRed, uint16_t &nGreen, uint16_t &nBlue);
	void SetRgb(uint8_t nOut, uint16_t nRed, uint16_t nGreen, uint16_t nBlue);

	void SetRgb(uint8_t nOut, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);

	void Update();
	void Blackout();

	void Dump();

private:
	void UpdateFirst32();

private:
	uint8_t m_nBoards;
	uint32_t m_nSpiSpeedHz;
	uint32_t m_nFirst32;
	uint16_t *m_pBuffer;
	uint16_t *m_pBufferBlackout;
	uint32_t m_nBufSize;
};

#endif /* TLC59711_H_ */
