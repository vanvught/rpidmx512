/**
 * @file dmxserialparams.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>

#include "dmxserial.h"
#include "configstore.h"

namespace dmxserialparams {
struct Params {
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

static_assert(sizeof(struct Params) <= 32, "struct Params is too large");

struct Mask {
	static constexpr uint32_t TYPE = (1U << 0);
	static constexpr uint32_t BAUD = (1U << 1);
	static constexpr uint32_t BITS = (1U << 2);
	static constexpr uint32_t PARTITY = (1U << 3);
	static constexpr uint32_t STOPBITS = (1U << 4);
	static constexpr uint32_t SPI_SPEED_HZ = (1U << 5);
	static constexpr uint32_t SPI_MODE = (1U << 6);
	static constexpr uint32_t I2C_ADDRESS = (1U << 7);
	static constexpr uint32_t I2C_SPEED_MODE = (1U << 8);
};
}  // namespace dmxserialparams

class DmxSerialStore {
public:
	static void Update(const struct dmxserialparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::SERIAL, pParams, sizeof(struct dmxserialparams::Params));
	}

	static void Copy(struct dmxserialparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::SERIAL, pParams, sizeof(struct dmxserialparams::Params));
	}
};

class DmxSerialParams {
public:
	DmxSerialParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct dmxserialparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set();

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const {
		return (m_Params.nSetList & nMask) == nMask;
	}

private:
	dmxserialparams::Params m_Params;
};

#endif /* INCLUDE_DMXSERIALPARAMS_H_ */
