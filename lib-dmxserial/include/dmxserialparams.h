/**
 * @file dmxserialparams.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DMXSERIALPARAMS_H_
#define DMXSERIALPARAMS_H_

#include <stdint.h>

#include "dmxserial.h"

struct TDmxSerialParams {
	uint32_t nSetList;
	uint8_t nType;
	uint32_t nBaud;
	uint8_t nBits;
	uint8_t nParity;
	uint8_t nStopBits;
	uint32_t nSpiSpeedHz;
	uint8_t nSpiMode;
	uint8_t nI2cAddress;
	uint8_t nI2cSpeedMode;
} __attribute__((packed));

static_assert(sizeof(struct TDmxSerialParams) <= 32, "struct TDmxSerialParams is too large");

struct DmxSerialParamsMask {
	static constexpr auto TYPE = (1U << 0);
	static constexpr auto BAUD = (1U << 1);
	static constexpr auto BITS = (1U << 2);
	static constexpr auto PARTITY = (1U << 3);
	static constexpr auto STOPBITS = (1U << 4);
	static constexpr auto SPI_SPEED_HZ = (1U << 5);
	static constexpr auto SPI_MODE = (1U << 6);
	static constexpr auto I2C_ADDRESS = (1U << 7);
	static constexpr auto I2C_SPEED_MODE = (1U << 8);
};

class DmxSerialParamsStore {
public:
	virtual ~DmxSerialParamsStore() {
	}

	virtual void Update(const struct TDmxSerialParams *pDmxSerialParams)=0;
	virtual void Copy(struct TDmxSerialParams *pDmxSerialParams)=0;
};

class DmxSerialParams {
public:
	DmxSerialParams(DmxSerialParamsStore *pDmxSerialParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TDmxSerialParams *pDmxSerialParams, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Dump();

	void Set();

    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const {
		return (m_tDmxSerialParams.nSetList & nMask) == nMask;
	}

private:
	DmxSerialParamsStore *m_pDmxSerialParamsStore;
	TDmxSerialParams m_tDmxSerialParams;
};

#endif /* INCLUDE_DMXSERIALPARAMS_H_ */
