/**
 * @file slushdmx.h
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SLUSHDMX_H_
#define SLUSHDMX_H_

#include <cstdint>

#include "slushboard.h"
#include "slushmotor.h"

#include "lightset.h"

#include "l6470dmxmodes.h"

#define SLUSH_DMX_MAX_MOTORS			4
#define SLUSH_GPIO_DMX_DATA_DIRECTION	8	// RPI_V2_GPIO_P1_24 , UEXT SPI0 CE0
#define IO_PINS_IOPORT					8

class SlushDmx: public LightSet {
public:
	SlushDmx(bool bUseSPI = false);
	~SlushDmx() override;

	void Start(const uint32_t nPortIndex) override;
	void Stop(const uint32_t nPortIndex) override;

	void SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true) override;
	void Sync(const uint32_t nPortIndex) override;
	void Sync(const bool doForce = false) override;

	uint32_t GetMotorsConnected() {
		return m_nMotorsConnected;
	}

public: // RDM
	bool SetDmxStartAddress(uint16_t nDmxStartAddress) override;
	uint16_t GetDmxStartAddress() override {
		return m_nDmxStartAddress;
	}

	uint16_t GetDmxFootprint() override {
		return m_nDmxFootprint;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, lightset::SlotInfo &tSlotInfo) override;

public:
	void SetUseSpiBusy(bool bUseSpiBusy) {
		m_bUseSpiBusy = bUseSpiBusy;
	}
	bool GetUseSpiBusy()  {
		return m_bUseSpiBusy;
	}

public:	// MCP23017 Port A , Port B
	void SetDmxStartAddressPortA(uint16_t nDmxStartAddress) {
		m_nDmxStartAddressPortA = nDmxStartAddress;
	}

	void SetDmxFootprintPortA(uint16_t nDmxFootprint) {
		m_nDmxFootprintPortA = nDmxFootprint;
	}

	void SetDmxStartAddressPortB(uint16_t nDmxStartAddress) {
		m_nDmxStartAddressPortB = nDmxStartAddress;
	}

	void SetDmxFootprintPortB(uint16_t nDmxFootprint) {
		m_nDmxFootprintPortB = nDmxFootprint;
	}

public:
	void ReadConfigFiles();

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);

private: // MCP23017 Port A , Port B
	void UpdateIOPorts(const uint8_t *pData, uint32_t nLength);

private: // MCP23017 Port A , Port B
	bool m_bSetPortA{false};
	bool m_bSetPortB{false};

	uint8_t m_nDataPortA;
	uint8_t m_nDataPortB;

	uint16_t m_nDmxStartAddressPortA;
	uint16_t m_nDmxFootprintPortA;
	char *m_pSlotInfoRawPortA;
	lightset::SlotInfo *m_pSlotInfoPortA;

	uint16_t m_nDmxStartAddressPortB;
	uint16_t m_nDmxFootprintPortB;
	char *m_pSlotInfoRawPortB;
	lightset::SlotInfo *m_pSlotInfoPortB;

private:
	SlushBoard *m_pBoard;
	bool m_bUseSpiBusy;
	uint32_t m_nMotorsConnected{0};

	SlushMotor	*m_pSlushMotor[SLUSH_DMX_MAX_MOTORS];
	MotorParams *m_pMotorParams[SLUSH_DMX_MAX_MOTORS];
	ModeParams *m_pModeParams[SLUSH_DMX_MAX_MOTORS];
	L6470DmxModes *m_pL6470DmxModes[SLUSH_DMX_MAX_MOTORS];
	lightset::SlotInfo *m_pSlotInfo[SLUSH_DMX_MAX_MOTORS];

	uint8_t m_nDmxMode;
	uint16_t m_nDmxStartAddressMode;

	uint16_t m_nDmxStartAddress;
	uint16_t m_nDmxFootprint{0};

	char *m_pSlotInfoRaw;
};

#endif /* SLUSHDMX_H_ */
