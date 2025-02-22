/**
 * @file sparkfundmx.h
 *
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SPARKFUNDMX_H_
#define SPARKFUNDMX_H_

#include <cstdint>

#include "l6470dmxmodes.h"

#include "modeparams.h"
#include "motorparams.h"
#include "l6470params.h"

#include "autodriver.h"

#include "modestore.h"

#define SPARKFUN_DMX_MAX_MOTORS		8

struct TSparkFunStores {
	ModeParamsStore *pModeParamsStore;
	MotorParamsStore *pMotorParamsStore;
	L6470ParamsStore *pL6470ParamsStore;
};

class SparkFunDmx {
public:
	SparkFunDmx();
	~SparkFunDmx() ;

	void Start(const uint32_t nPortIndex) ;
	void Stop(const uint32_t nPortIndex) ;

	void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true) ;
	void Sync(const uint32_t nPortIndex) ;
	void Sync() ;

	void Print() ;

	uint32_t GetMotorsConnected() {
		return AutoDriver::getNumBoards();
	}

// RDM
	bool SetDmxStartAddress(uint16_t nDmxStartAddress) ;
	uint16_t GetDmxStartAddress()  {
		return m_nDmxStartAddress;
	}

	uint16_t GetDmxFootprint()  {
		return m_nDmxFootprint;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, dmxnode::SlotInfo &tSlotInfo) ;

//
	void SetGlobalSpiCs(uint8_t nSpiCs) {
		m_nGlobalSpiCs = nSpiCs;
		m_bIsGlobalSpiCsSet = true;
	}

	void SetGlobalResetPin(uint8_t nResetPin) {
		m_nGlobalResetPin = nResetPin;
		m_bIsGlobalResetSet = true;
	}

	void SetGlobalBusyPin(uint8_t nBusyPin) {
		m_nGlobalBusyPin = nBusyPin;
		m_bIsGlobalBusyPinSet = true;
	}

	void SetLocalPosition(uint8_t nPosition) {
		m_nLocalPosition = nPosition;
		m_bIsLocalPositionSet = true;
	}

	void SetLocalSpiCs(uint8_t nSpiCs) {
		m_nLocalSpiCs = nSpiCs;
		m_bIsLocalSpiCsSet = true;
	}

	void SetLocalResetPin(uint8_t nResetPin) {
		m_nLocalResetPin = nResetPin;
		m_bIsLocalResetSet = true;
	}

	void SetLocalBusyPin(uint8_t nBusyPin) {
		m_nLocalBusyPin = nBusyPin;
		m_bIsLocalBusyPinSet = true;
	}

	void ReadConfigFiles();

private:
	AutoDriver *m_pAutoDriver[SPARKFUN_DMX_MAX_MOTORS];
	MotorParams *m_pMotorParams[SPARKFUN_DMX_MAX_MOTORS];
	ModeParams *m_pModeParams[SPARKFUN_DMX_MAX_MOTORS];
	L6470DmxModes *m_pL6470DmxModes[SPARKFUN_DMX_MAX_MOTORS];
	dmxnode::SlotInfo *m_pSlotInfo[SPARKFUN_DMX_MAX_MOTORS];

	uint8_t m_nGlobalSpiCs;
	uint8_t m_nGlobalResetPin;
	uint8_t m_nGlobalBusyPin;

	bool m_bIsGlobalSpiCsSet;
	bool m_bIsGlobalResetSet;
	bool m_bIsGlobalBusyPinSet;

	uint8_t m_nLocalPosition;
	uint8_t m_nLocalSpiCs;
	uint8_t m_nLocalResetPin;
	uint8_t m_nLocalBusyPin;

	bool m_bIsLocalPositionSet;
	bool m_bIsLocalSpiCsSet;
	bool m_bIsLocalResetSet;
	bool m_bIsLocalBusyPinSet;

	uint16_t m_nDmxMode;
	uint16_t m_nDmxStartAddressMode;

	uint16_t m_nDmxStartAddress;
	uint16_t m_nDmxFootprint { 0 };
};

#endif /* SPARKFUNDMX_H_ */
