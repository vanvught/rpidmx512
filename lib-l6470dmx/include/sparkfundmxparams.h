/**
 * @file sparkfundmxparams.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SPARKFUNDMXPARAMS_H_
#define SPARKFUNDMXPARAMS_H_

#include <stdint.h>

#include "sparkfundmx.h"

struct TSparkFunDmxParams {
	uint32_t nSetList;
	uint8_t nPosition;
	uint8_t nSpiCs;
	uint8_t nResetPin;
	uint8_t nBusyPin;
} __attribute__((packed));

struct SparkFunDmxParamsMask {
	static constexpr auto POSITION = (1U << 0);
	static constexpr auto SPI_CS = (1U << 1);
	static constexpr auto RESET_PIN = (1U << 2);
	static constexpr auto BUSY_PIN = (1U << 3);
};

class SparkFunDmxParamsStore {
public:
	virtual ~SparkFunDmxParamsStore() {
	}

	virtual void Update(const struct TSparkFunDmxParams *ptSparkFunDmxParams)=0;
	virtual void Copy(struct TSparkFunDmxParams *ptSparkFunDmxParams)=0;

	virtual void Update(uint8_t nMotorIndex, const struct TSparkFunDmxParams *ptSparkFunDmxParams)=0;
	virtual void Copy(uint8_t nMotorIndex, struct TSparkFunDmxParams *ptSparkFunDmxParams)=0;
};

class SparkFunDmxParams {
public:
	SparkFunDmxParams(SparkFunDmxParamsStore *pSparkFunDmxParamsStore = nullptr);

	bool Load();
	bool Load(uint8_t nMotorIndex);
	void Load(const char *pBuffer, uint32_t nLength);
	void Load(uint8_t nMotorIndex, const char *pBuffer, uint32_t nLength);

	void Builder(const struct TSparkFunDmxParams *ptSparkFunDmxParams, char *pBuffer, uint32_t nLength, uint32_t &nSize, uint8_t nMotorIndex = 0xFF);
	void Save(char *pBuffer, uint32_t nLength, uint32_t &nSize, uint8_t nMotorIndex = 0xFF);

	void SetGlobal(SparkFunDmx *pSparkFunDmx);
	void SetLocal(SparkFunDmx *pSparkFunDmx);

	void Dump(uint8_t nMotorIndex = 0xFF);
public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_tSparkFunDmxParams.nSetList & nMask) == nMask;
    }

private:
	SparkFunDmxParamsStore *m_pSparkFunDmxParamsStore;
    struct TSparkFunDmxParams m_tSparkFunDmxParams;
    char m_aFileName[16];
};

#endif /* SPARKFUNDMXPARAMS_H_ */
