/**
 * @file tlc59711.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef TLC59711_H_
#define TLC59711_H_

#define TLC59711_SPI_SPEED_DEFAULT	5000000
#define TLC59711_SPI_SPEED_MAX		10000000

#define TLC59711_16BIT_CHANNELS	14
#define TLC59711_OUT_CHANNELS	12
#define TLC59711_RGB_CHANNELS	4

#define TLC59711_RGB_8BIT_VALUE(x)	((uint8_t)(x))
#define TLC59711_RGB_16BIT_VALUE(x)	((uint16_t)(x))

#include <stdint.h>

class TLC59711 {
public:
	TLC59711(uint8_t nBoards = 1, uint32_t nSpiSpeedHz = TLC59711_SPI_SPEED_DEFAULT);
	~TLC59711(void);

	int GetBlank(void) const;
	void SetBlank(bool pBlank = false);

	int GetDisplayRepeat(void) const;
	void SetDisplayRepeat(bool pDisplayRepeat = true);

	int GetDisplayTimingReset(void) const;
	void SetDisplayTimingReset(bool pDisplayTimingReset = true);

	int GetExternalClock(void) const;
	void SetExternalClock(bool pExternalClock = false);

	int GetOnOffTiming(void) const;
	void SetOnOffTiming(bool pOnOffTiming = false);

	uint8_t GetGbcRed(void) const;
	void SetGbcRed(uint8_t nValue = 0x7F);

	uint8_t GetGbcGreen(void) const;
	void SetGbcGreen(uint8_t nValue = 0x7F);

	uint8_t GetGbcBlue(void) const;
	void SetGbcBlue(uint8_t nValue = 0x7F);

	bool Get(uint8_t nChannel, uint16_t &nValue);
	void Set(uint8_t nChannel, uint16_t nValue);

	void Set(uint8_t nChannel, uint8_t nValue);

	bool GetRgb(uint8_t nOut, uint16_t &nRed, uint16_t &nGreen, uint16_t &nBlue);
	void SetRgb(uint8_t nOut, uint16_t nRed, uint16_t nGreen, uint16_t nBlue);

	void SetRgb(uint8_t nOut, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);

	void Update(void);

#if defined (__circle__)
	// returns TRUE while DMA operation is active
	bool IsUpdating (void) const;
#else
	inline 	bool IsUpdating (void) const {
		return false;
	}
#endif

	void Dump(void);

private:
	void UpdateFirst32(void);

private:
	uint8_t m_nBoards;
	uint32_t m_nSpiSpeedHz;
	uint16_t m_nClockDivider;
	uint32_t m_nFirst32;
	uint16_t *m_pBuffer;
	uint32_t m_nBufSize;
};

#endif /* TLC59711_H_ */
