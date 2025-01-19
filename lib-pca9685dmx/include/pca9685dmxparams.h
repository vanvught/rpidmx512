/**
 * @file pca9685dmxparams.h
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PCA9685DMXPARAMS_H_
#define PCA9685DMXPARAMS_H_

#include <cstdint>

#include "pca9685.h"
#include "pca9685dmx.h"
#include "configstore.h"

namespace pca9685dmxparams {
struct Params {
    uint32_t nSetList;			///< 4	4
    uint8_t  nAddress;			///< 1	5
    uint16_t nChannelCount;		///< 2	7
	uint16_t nDmxStartAddress;	///< 2	9
	uint16_t nLedPwmFrequency;	///< 2	12
	uint16_t nServoLeftUs;		///< 2	14
	uint16_t nServoCenterUs;	///< 2	16
	uint16_t nServoRightUs;		///< 2	18
} __attribute__((packed));

static_assert(sizeof(struct Params) <= 32, "struct Params is too large");

struct Mask {
	static constexpr uint32_t ADDRESS			   = (1U << 0);
	static constexpr uint32_t MODE 				   = (1U << 1);
	static constexpr uint32_t CHANNEL_COUNT		   = (1U << 2);
	static constexpr uint32_t DMX_START_ADDRESS    = (1U << 3);
	static constexpr uint32_t USE_8BIT	       	   = (1U << 4);
	static constexpr uint32_t LED_PWM_FREQUENCY    = (1U << 5);
	static constexpr uint32_t LED_OUTPUT_INVERT    = (1U << 6);
	static constexpr uint32_t LED_OUTPUT_OPENDRAIN = (1U << 7);
	static constexpr uint32_t SERVO_LEFT_US 	   = (1U << 8);
	static constexpr uint32_t SERVO_CENTER_US 	   = (1U << 9);
	static constexpr uint32_t SERVO_RIGHT_US 	   = (1U << 10);
};
}  // namespace pca9685dmxparams

class PCA9685DmxParamsStore {
public:
	static void Update(const struct pca9685dmxparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::PCA9685, pParams, sizeof(struct pca9685dmxparams::Params));
	}

	static void Copy(struct pca9685dmxparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::PCA9685, pParams, sizeof(struct pca9685dmxparams::Params));
	}
};

class PCA9685DmxParams {
public:
	PCA9685DmxParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct pca9685dmxparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set(PCA9685Dmx *pPCA9685Dmx);

	static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *pLine);
    void SetBool(const uint8_t nValue, const uint32_t nMask);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    pca9685dmxparams::Params m_Params;
};

#endif /* PCA9685DMXPARAMS_H_ */
