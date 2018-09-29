/**
 * @file slushdmx.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>

#include "slushboard.h"
#include "slushmotor.h"

#include "lightset.h"

#include "l6470dmxmodes.h"

#define SLUSH_DMX_MAX_MOTORS			4
#define SLUSH_GPIO_DMX_DATA_DIRECTION	8	// RPI_V2_GPIO_P1_24 , UEXT SPI0 CE0

class SlushDmx: public LightSet {
public:
	SlushDmx(bool bUseSPI = false);
	~SlushDmx(void);

	void Start(uint8_t nPort);
	void Stop(uint8_t nPort);

	void SetData(uint8_t nPort, const uint8_t *, uint16_t);

public: // RDM
	bool SetDmxStartAddress(uint16_t nDmxStartAddress);
	inline uint16_t GetDmxStartAddress(void) {
		return m_nDmxStartAddress;
	}

	inline uint16_t GetDmxFootprint(void)  {
		return m_nDmxFootprint;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo);

public:
	bool GetUseSpiBusy(void) const;
	void SetUseSpiBusy(bool);

public:
	void ReadConfigFiles(void);

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);

private: // MCP23017 Port A , Port B
	void UpdateIOPorts(const uint8_t *, const uint16_t);

private: // MCP23017 Port A , Port B
	bool m_bSetPortA;
	bool m_bSetPortB;

	uint8_t m_nDataPortA;
	uint8_t m_nDataPortB;

	uint16_t m_nDmxStartAddressPortA;
	uint16_t m_nDmxFootprintPortA;
	char *m_pSlotInfoRawPortA;
	struct TLightSetSlotInfo *m_pSlotInfoPortA;

	uint16_t m_nDmxStartAddressPortB;
	uint16_t m_nDmxFootprintPortB;
	char *m_pSlotInfoRawPortB;
	struct TLightSetSlotInfo *m_pSlotInfoPortB;

private:
	SlushBoard *m_pBoard;
	bool m_bUseSpiBusy;

	SlushMotor	*m_pSlushMotor[SLUSH_DMX_MAX_MOTORS];
	MotorParams *m_pMotorParams[SLUSH_DMX_MAX_MOTORS];
	ModeParams *m_pModeParams[SLUSH_DMX_MAX_MOTORS];
	L6470DmxModes *m_pL6470DmxModes[SLUSH_DMX_MAX_MOTORS];
	struct TLightSetSlotInfo *m_pSlotInfo[SLUSH_DMX_MAX_MOTORS];

	uint8_t m_nDmxMode;
	uint16_t m_nDmxStartAddressMode;

	uint16_t m_nDmxStartAddress;
	uint16_t m_nDmxFootprint;

	char *m_pSlotInfoRaw;
};

#endif /* SLUSHDMX_H_ */
