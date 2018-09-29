/**
 * @file pca9685dmxparams.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__((aligned(4)))
#endif

#include "pca9685dmxparams.h"

#include "pca9685.h"

#include "readconfigfile.h"
#include "sscan.h"

#define DMX_START_ADDRESS_MASK	(1 << 0)
#define DMX_FOOTPRINT_MASK		(1 << 1)
#define DMX_SLOT_INFO_MASK		(1 << 2)
#define I2C_SLAVE_ADDRESS_MASK	(1 << 3)
#define BOARD_INSTANCES_MASK	(1 << 4)

static const char PARAMS_DMX_START_ADDRESS[] ALIGNED = "dmx_start_address";
static const char PARAMS_DMX_FOOTPRINT[] ALIGNED = "dmx_footprint";
static const char PARAMS_DMX_SLOT_INFO[] ALIGNED = "dmx_slot_info";
static const char PARAMS_I2C_SLAVE_ADDRESS[] ALIGNED = "i2c_slave_address";
static const char PARAMS_BOARD_INSTANCES[] ALIGNED = "board_instances";

#define PARAMS_DMX_START_ADDRESS_DEFAULT	1
#define PARAMS_DMX_FOOTPRINT_DEFAULT		PCA9685_PWM_CHANNELS
#define PARAMS_BOARD_INSTANCES_DEFAULT		1
#define PARAMS_BOARD_INSTANCES_MAX			32

#define DMX_SLOT_INFO_LENGTH				128

PCA9685DmxParams::PCA9685DmxParams(const char *pFileName): m_bSetList(0) {
	assert(pFileName != 0);

	m_nI2cAddress = PCA9685_I2C_ADDRESS_DEFAULT;

	m_nDmxStartAddress = PARAMS_DMX_START_ADDRESS_DEFAULT;
	m_nDmxFootprint = PARAMS_DMX_FOOTPRINT_DEFAULT;
	m_nBoardInstances = PARAMS_BOARD_INSTANCES_DEFAULT;

	m_pDmxSlotInfoRaw = new char[DMX_SLOT_INFO_LENGTH];

	assert(m_pDmxSlotInfoRaw != 0);
	for (unsigned i = 0; i < DMX_SLOT_INFO_LENGTH; i++) {
		m_pDmxSlotInfoRaw[i] = 0;
	}

	ReadConfigFile configfile(PCA9685DmxParams::staticCallbackFunction, this);
	configfile.Read(pFileName);
}

PCA9685DmxParams::~PCA9685DmxParams(void) {
	delete[] m_pDmxSlotInfoRaw;
	m_pDmxSlotInfoRaw = 0;
}

void PCA9685DmxParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((PCA9685DmxParams *) p)->callbackFunction(s);
}

void PCA9685DmxParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t value8;
	uint16_t value16;
	uint8_t len;

	if (Sscan::Uint16(pLine, PARAMS_DMX_START_ADDRESS, &value16) == SSCAN_OK) {
		if ((value16 != 0) && (value16 <= 512)) {
			m_nDmxStartAddress = value16;
			m_bSetList |= DMX_START_ADDRESS_MASK;
		}
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_DMX_FOOTPRINT, &value16) == SSCAN_OK) {
		if ((value16 != 0) && (value16 <= (PCA9685_PWM_CHANNELS * PARAMS_BOARD_INSTANCES_MAX))) {
			m_nDmxFootprint = value16;
			m_bSetList |= DMX_FOOTPRINT_MASK;
		}
		return;
	}

	if (Sscan::I2cAddress(pLine, PARAMS_I2C_SLAVE_ADDRESS, &value8) == SSCAN_OK) {
		if ((value8 >= PCA9685_I2C_ADDRESS_DEFAULT) && (value8 != PCA9685_I2C_ADDRESS_FIXED)) {
			m_nI2cAddress = value8;
			m_bSetList |= I2C_SLAVE_ADDRESS_MASK;
		return;
		}
	}

	if (Sscan::Uint8(pLine, PARAMS_BOARD_INSTANCES, &value8) == SSCAN_OK) {
		if ((value8 != 0) && (value8 <= PARAMS_BOARD_INSTANCES_MAX)) {
			m_nBoardInstances = value8;
			m_bSetList |= BOARD_INSTANCES_MASK;
		}
		return;
	}

	len = DMX_SLOT_INFO_LENGTH;
	if (Sscan::Char(pLine, PARAMS_DMX_SLOT_INFO, m_pDmxSlotInfoRaw, &len) == SSCAN_OK) {
		if (len >= 7) { // 00:0000 at least one value set
			m_bSetList |= DMX_SLOT_INFO_MASK;
		}
	}
}

uint8_t PCA9685DmxParams::GetI2cAddress(bool &pIsSet) const {
	pIsSet = isMaskSet(I2C_SLAVE_ADDRESS_MASK);
	return m_nI2cAddress;
}

uint16_t PCA9685DmxParams::GetDmxStartAddress(bool &pIsSet) const {
	pIsSet = isMaskSet(DMX_START_ADDRESS_MASK);
	return m_nDmxStartAddress;
}

uint16_t PCA9685DmxParams::GetDmxFootprint(bool &pIsSet) const {
	pIsSet = isMaskSet(DMX_FOOTPRINT_MASK);
	return m_nDmxFootprint;
}

uint8_t PCA9685DmxParams::GetBoardInstances(bool &pIsSet) const {
	pIsSet = isMaskSet(BOARD_INSTANCES_MASK);
	return m_nBoardInstances;
}

const char* PCA9685DmxParams::GetDmxSlotInfoRaw(bool &pIsSet) const {
	pIsSet = isMaskSet(DMX_SLOT_INFO_MASK);
	return m_pDmxSlotInfoRaw;
}

void PCA9685DmxParams::Dump(void) {
#ifndef NDEBUG
	if (m_bSetList == 0) {
		return;
	}

	if(isMaskSet(DMX_START_ADDRESS_MASK)) {
		printf(" %s=%d\n", PARAMS_DMX_START_ADDRESS, m_nDmxStartAddress);
	}

	if(isMaskSet(DMX_FOOTPRINT_MASK)) {
		printf(" %s=%d\n", PARAMS_DMX_FOOTPRINT, m_nDmxFootprint);
	}

	if(isMaskSet(I2C_SLAVE_ADDRESS_MASK)) {
		printf(" %s=0x%2x\n", PARAMS_I2C_SLAVE_ADDRESS, m_nI2cAddress);
	}

	if(isMaskSet(BOARD_INSTANCES_MASK)) {
		printf(" %s=%d\n", PARAMS_BOARD_INSTANCES, m_nBoardInstances);
	}

	if(isMaskSet(DMX_SLOT_INFO_MASK)) {
		printf(" %s=%s\n", PARAMS_DMX_SLOT_INFO, m_pDmxSlotInfoRaw);
	}
#endif
}

bool PCA9685DmxParams::isMaskSet(uint32_t mask) const {
	return (m_bSetList & mask) == mask;
}

bool PCA9685DmxParams::GetSetList(void) const {
	return m_bSetList != 0;
}
