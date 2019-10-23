/**
 * @file sparkfundmxparams.h
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
}__attribute__((packed));

enum TSparkFunDmxParamsMask {
	SPARKFUN_DMX_PARAMS_MASK_POSITION = (1 << 0),
	SPARKFUN_DMX_PARAMS_MASK_SPI_CS = (1 << 1),
	SPARKFUN_DMX_PARAMS_MASK_RESET_PIN = (1 << 2),
	SPARKFUN_DMX_PARAMS_MASK_BUSY_PIN = (1 << 3)
};

class SparkFunDmxParamsStore {
public:
	virtual ~SparkFunDmxParamsStore(void);

	virtual void Update(const struct TSparkFunDmxParams *ptSparkFunDmxParams)=0;
	virtual void Copy(struct TSparkFunDmxParams *ptSparkFunDmxParams)=0;

	virtual void Update(uint8_t nMotorIndex, const struct TSparkFunDmxParams *ptSparkFunDmxParams)=0;
	virtual void Copy(uint8_t nMotorIndex, struct TSparkFunDmxParams *ptSparkFunDmxParams)=0;
};

class SparkFunDmxParams {
public:
	SparkFunDmxParams(SparkFunDmxParamsStore *pSparkFunDmxParamsStore=0);
	~SparkFunDmxParams(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TSparkFunDmxParams *ptSparkFunDmxParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	bool Load(uint8_t nMotorIndex);

	void SetGlobal(SparkFunDmx *pSparkFunDmx);
	void SetLocal(SparkFunDmx *pSparkFunDmx);

	void Dump(uint8_t nMotorIndex = (1 + SPARKFUN_DMX_MAX_MOTORS));
public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
	bool isMaskSet(uint16_t nMask) const;

private:
	SparkFunDmxParamsStore *m_pSparkFunDmxParamsStore;
    struct TSparkFunDmxParams m_tSparkFunDmxParams;
    char m_aFileName[16];
};

#endif /* SPARKFUNDMXPARAMS_H_ */
