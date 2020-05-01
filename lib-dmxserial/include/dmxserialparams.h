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

enum TDmxSerialParamsMask {
	DMXSERIAL_PARAMS_MASK_TYPE = (1 << 0),
	DMXSERIAL_PARAMS_MASK_BAUD = (1 << 1),
	DMXSERIAL_PARAMS_MASK_BITS = (1 << 2),
	DMXSERIAL_PARAMS_MASK_PARTITY = (1 << 3),
	DMXSERIAL_PARAMS_MASK_STOPBITS = (1 << 4),
	DMXSERIAL_PARAMS_MASK_SPI_SPEED_HZ = (1 << 5),
	DMXSERIAL_PARAMS_MASK_SPI_MODE = (1 << 6),
	DMXSERIAL_PARAMS_MASK_I2C_ADDRESS = (1 << 7),
	DMXSERIAL_PARAMS_MASK_I2C_SPEED_MODE = (1 << 8)
};

class DmxSerialParamsStore {
public:
	virtual ~DmxSerialParamsStore(void) {}

	virtual void Update(const struct TDmxSerialParams *pDmxSerialParams)=0;
	virtual void Copy(struct TDmxSerialParams *pDmxSerialParams)=0;
};

class DmxSerialParams {
public:
	DmxSerialParams(DmxSerialParamsStore *pDmxSerialParamsStore = 0);
	~DmxSerialParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TDmxSerialParams *pDmxSerialParams, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Dump(void);

	void Set(DmxSerial *pDmxSerial);

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) {
		return (m_tDmxSerialParams.nSetList & nMask) == nMask;
	}

private:
	DmxSerialParamsStore *m_pDmxSerialParamsStore;
    struct TDmxSerialParams	m_tDmxSerialParams;
};

#endif /* INCLUDE_DMXSERIALPARAMS_H_ */
