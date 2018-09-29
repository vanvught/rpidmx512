/**
 * @file slushdmx.h
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
#include <stdio.h>
#include <assert.h>

#include "slushdmx.h"
#include "slushboard.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "l6470params.h"
#include "l6470dmxmodes.h"

#include "motorparams.h"
#include "modeparams.h"

#include "parse.h"

#include "debug.h"

#ifndef MAX
 #define MAX(a,b)	(((a) > (b)) ? (a) : (b))
 #define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#define IO_PINS_IOPORT				8
#define DMX_MAX_CHANNELS			512
#define DMX_SLOT_INFO_RAW_LENGTH	128

static const char PARAMS_SLUSH_USE_SPI[] ALIGNED = "use_spi_busy";

static const char PARAMS_SLUSH_DMX_START_ADDRESS_PORT_A[] ALIGNED = "dmx_start_address_port_a";
static const char PARAMS_SLUSH_DMX_FOOTPRINT_PORT_A[] ALIGNED = "dmx_footprint_port_a";
static const char PARAMS_DMX_SLOT_INFO_PORT_A[] ALIGNED = "dmx_slot_info_port_a";

static const char PARAMS_SLUSH_DMX_START_ADDRESS_PORT_B[] ALIGNED = "dmx_start_address_port_b";
static const char PARAMS_SLUSH_DMX_FOOTPRINT_PORT_B[] ALIGNED = "dmx_footprint_port_b";
static const char PARAMS_DMX_SLOT_INFO_PORT_B[] ALIGNED = "dmx_slot_info_port_b";

static const char PARAMS_DMX_MODE[] ALIGNED = "dmx_mode";
static const char PARAMS_DMX_START_ADDRESS[] ALIGNED = "dmx_start_address";
static const char PARAMS_DMX_SLOT_INFO[] ALIGNED = "dmx_slot_info";

void SlushDmx::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((SlushDmx *) p)->callbackFunction(s);
}

void SlushDmx::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t value;
	uint16_t value16;
	uint8_t len;

	if (Sscan::Uint8(pLine, PARAMS_SLUSH_USE_SPI, &value) == SSCAN_OK) {
		if (value != 0) {
			m_bUseSpiBusy = true;
			return;
		}
	}

	if (Sscan::Uint16(pLine, PARAMS_SLUSH_DMX_START_ADDRESS_PORT_A, &value16) == SSCAN_OK) {
		if (value16 <= DMX_MAX_CHANNELS) {
			m_nDmxStartAddressPortA = value16;
		}
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_SLUSH_DMX_START_ADDRESS_PORT_B, &value16) == SSCAN_OK) {
		if (value16 <= DMX_MAX_CHANNELS) {
			m_nDmxStartAddressPortB = value16;
		}
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_SLUSH_DMX_FOOTPRINT_PORT_A, &value16) == SSCAN_OK) {
		if ((value16 > 0) && (value16 <= IO_PINS_IOPORT)) {
			m_nDmxFootprintPortA = value16;
		}
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_SLUSH_DMX_FOOTPRINT_PORT_B, &value16) == SSCAN_OK) {
		if ((value16 > 0) && (value16 <= IO_PINS_IOPORT)) {
			m_nDmxFootprintPortB = value16;
		}
		return;
	}

	if (Sscan::Uint8(pLine, PARAMS_DMX_MODE, &m_nDmxMode) == SSCAN_OK) {
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_DMX_START_ADDRESS, &m_nDmxStartAddressMode) == SSCAN_OK) {
		return;
	}

	len = DMX_SLOT_INFO_RAW_LENGTH;
	if (Sscan::Char(pLine, PARAMS_DMX_SLOT_INFO_PORT_A, m_pSlotInfoRawPortA, &len) == SSCAN_OK) {
		if (len < 7) { // 00:0000 at least one value set
			m_pSlotInfoRawPortA[0] = '\0';
		}
		return;
	}

	len = DMX_SLOT_INFO_RAW_LENGTH;
	if (Sscan::Char(pLine, PARAMS_DMX_SLOT_INFO_PORT_B, m_pSlotInfoRawPortB, &len) == SSCAN_OK) {
		if (len < 7) { // 00:0000 at least one value set
			m_pSlotInfoRawPortB[0] = '\0';
		}
	}

	len = DMX_SLOT_INFO_RAW_LENGTH;
	if (Sscan::Char(pLine, PARAMS_DMX_SLOT_INFO, m_pSlotInfoRaw, &len) == SSCAN_OK) {
		if (len < 7) { // 00:0000 at least one value set
			m_pSlotInfoRaw[0] = '\0';
		}
	}
}

SlushDmx::SlushDmx(bool bUseSPI): m_bSetPortA(false), m_bSetPortB(false), m_nDmxStartAddress(DMX_ADDRESS_INVALID), m_nDmxFootprint(0) { // Invalidate DMX Start Address and DMX Footprint
	DEBUG_ENTRY;

	m_bUseSpiBusy = bUseSPI;

	m_pBoard = new SlushBoard;
	assert(m_pBoard != 0);

	m_nDmxMode = L6470DMXMODE_UNDEFINED;
	m_nDmxStartAddressMode = 0;

	m_nDmxStartAddressPortA = 0;
	m_nDmxFootprintPortA = IO_PINS_IOPORT;
	m_pSlotInfoRawPortA = new char[DMX_SLOT_INFO_RAW_LENGTH];
	m_pSlotInfoPortA = 0;
	m_nDataPortA = 0;

	m_nDmxStartAddressPortB = 0;
	m_nDmxFootprintPortB = IO_PINS_IOPORT;
	m_pSlotInfoRawPortB = new char[DMX_SLOT_INFO_RAW_LENGTH];
	m_pSlotInfoPortB = 0;
	m_nDataPortB = 0;

	for (unsigned i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		m_pSlushMotor[i] = 0;
		m_pMotorParams[i] = 0;
		m_pModeParams[i] = 0;
		m_pL6470DmxModes[i] = 0;
		m_pSlotInfo[i] = 0;
	}

	m_pSlotInfoRaw = new char[DMX_SLOT_INFO_RAW_LENGTH];

	for (unsigned i = 0; i < DMX_SLOT_INFO_RAW_LENGTH; i++) {
		m_pSlotInfoRawPortA[i] = 0;
		m_pSlotInfoRawPortB[i] = 0;
		m_pSlotInfoRaw[i] = 0;
	}

	DEBUG_EXIT;
}

SlushDmx::~SlushDmx(void) {
	DEBUG_ENTRY;

	if (m_pSlotInfoRawPortA != 0) {
		delete[] m_pSlotInfoRawPortA;
		m_pSlotInfoRawPortA = 0;
	}

	if (m_pSlotInfoPortA != 0) {
		delete[] m_pSlotInfoPortA;
		m_pSlotInfoPortA = 0;
	}

	if (m_pSlotInfoRawPortB != 0) {
		delete[] m_pSlotInfoRawPortB;
		m_pSlotInfoRawPortB = 0;
	}

	if (m_pSlotInfoPortB != 0) {
		delete[] m_pSlotInfoPortB;
		m_pSlotInfoPortB = 0;
	}

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {

		if (m_pSlushMotor[i] != 0) {
			delete m_pSlushMotor[i];
			m_pSlushMotor[i] = 0;
		}

		if (m_pMotorParams[i] != 0) {
			delete m_pMotorParams[i];
			m_pMotorParams[i] = 0;
		}

		if (m_pModeParams[i] != 0) {
			delete m_pModeParams[i];
			m_pModeParams[i] = 0;
		}

		if (m_pL6470DmxModes[i] != 0) {
			delete m_pL6470DmxModes[i];
			m_pL6470DmxModes[i] = 0;
		}

		if (m_pSlotInfo[i] != 0) {
			delete[] m_pSlotInfo[i];
			m_pSlotInfo[i] = 0;
		}
	}

	if (m_pSlotInfoRaw != 0) {
		delete[] m_pSlotInfoRaw;
		m_pSlotInfoRaw = 0;
	}

	delete m_pBoard;
	m_pBoard = 0;

	DEBUG_EXIT;
}

void SlushDmx::Start(uint8_t nPort) {
	DEBUG_ENTRY;

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->Start();
		}
	}

	DEBUG_EXIT;
}

void SlushDmx::Stop(uint8_t nPort) {
	DEBUG_ENTRY;

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->Stop();
		}
	}

	DEBUG_EXIT;
}

void SlushDmx::ReadConfigFiles(void) {
	DEBUG_ENTRY;

	uint8_t nMotorsConnected = 0;

	ReadConfigFile configfile(SlushDmx::staticCallbackFunction, this);

	m_nDmxStartAddressPortA = 0;
	m_nDmxStartAddressPortB = 0;

	if (configfile.Read("slush.txt")) {

		// MCP23017 Port A
		if (m_nDmxStartAddressPortA > 0) {
			for (int pin = 0; pin < m_nDmxFootprintPortA; pin++) {
				m_pBoard->IOFSel(SLUSH_IO_PORTA, (TSlushIOPins) pin, SLUSH_IO_FSEL_OUTP);
			}
			m_bSetPortA = true;

			m_nDmxStartAddress = m_nDmxStartAddressPortA;
			m_nDmxFootprint = m_nDmxFootprintPortA;

			m_pSlotInfoPortA = new struct TLightSetSlotInfo[m_nDmxFootprintPortA];
			assert(m_pSlotInfoPortA != 0);

			char *pSlotInfoRaw = m_pSlotInfoRawPortA;

			for (unsigned i = 0; i < m_nDmxFootprintPortA; i++) {
				bool isSet = false;

				if (pSlotInfoRaw != 0) {
					pSlotInfoRaw = Parse::DmxSlotInfo(pSlotInfoRaw, isSet, m_pSlotInfoPortA[i].nType, m_pSlotInfoPortA[i].nCategory);
				}

				if (!isSet) {
					m_pSlotInfoPortA[i].nType = 0x00; // ST_PRIMARY
					m_pSlotInfoPortA[i].nCategory = 0xFFFF; // SD_UNDEFINED
				}
			}

#ifndef NDEBUG
			printf("DMX Start Address Output PortA = %d, Footprint PortA = %d\n", m_nDmxStartAddressPortA, m_nDmxFootprintPortA);
			printf("DMX Start Address:%d, DMX Footprint:%d\n", (int) m_nDmxStartAddress, (int) m_nDmxFootprint);
#endif
		}

		// MCP23017 Port B
		if (m_nDmxStartAddressPortB > 0) {
			for (int pin = 0; pin < m_nDmxFootprintPortB; pin++) {
				m_pBoard->IOFSel(SLUSH_IO_PORTB, (TSlushIOPins) pin, SLUSH_IO_FSEL_OUTP);
			}
			m_bSetPortB = true;

			if (m_nDmxStartAddress == DMX_ADDRESS_INVALID) {
				m_nDmxStartAddress = m_nDmxStartAddressPortB;
				m_nDmxFootprint = m_nDmxFootprintPortB;
			} else {
				const uint16_t nDmxChannelLastCurrent = m_nDmxStartAddress + m_nDmxFootprint;
				m_nDmxStartAddress = MIN(m_nDmxStartAddress, m_nDmxStartAddressPortB);

				const uint16_t nDmxChannelLastNew = m_nDmxStartAddressPortB + m_nDmxFootprintPortB;
				m_nDmxFootprint = MAX(nDmxChannelLastCurrent, nDmxChannelLastNew) - m_nDmxStartAddress;
			}

			m_pSlotInfoPortB = new struct TLightSetSlotInfo[m_nDmxFootprintPortB];
			assert(m_pSlotInfoPortB != 0);

			char *pSlotInfoRaw = m_pSlotInfoRawPortB;

			for (unsigned i = 0; i < m_nDmxFootprintPortB; i++) {
				bool isSet = false;

				if (pSlotInfoRaw != 0) {
					pSlotInfoRaw = Parse::DmxSlotInfo(pSlotInfoRaw, isSet, m_pSlotInfoPortB[i].nType, m_pSlotInfoPortB[i].nCategory);
				}

				if (!isSet) {
					m_pSlotInfoPortB[i].nType = 0x00; // ST_PRIMARY
					m_pSlotInfoPortB[i].nCategory = 0xFFFF; // SD_UNDEFINED
				}
			}

#ifndef NDEBUG
			printf("DMX Start Address Output PortB = %d, Footprint PortB = %d\n", m_nDmxStartAddressPortB, m_nDmxFootprintPortB);
			printf("DMX Start Address:%d, DMX Footprint:%d\n", (int) m_nDmxStartAddress, (int) m_nDmxFootprint);
#endif
		}
	}

	char fileName[] = "motor%.txt";

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {

		fileName[5] = (char) i + '0';

		m_pSlotInfoRaw[0] = 0;

		if (configfile.Read(fileName)) {
#ifndef NDEBUG
			printf("Motor %d:\n", i);
			printf("\t%s=%d (DMX footprint=%d)\n", PARAMS_DMX_MODE, m_nDmxMode, L6470DmxModes::GetDmxFootPrintMode(m_nDmxMode));
			printf("\t%s=%d\n", PARAMS_DMX_START_ADDRESS, m_nDmxStartAddressMode);
			printf("\t=============================\n");
#endif
			if ((m_nDmxStartAddressMode <= DMX_MAX_CHANNELS) && (L6470DmxModes::GetDmxFootPrintMode(m_nDmxMode) != 0)) {
				m_pSlushMotor[i] = new SlushMotor(i, m_bUseSpiBusy);
				assert(m_pSlushMotor[i] != 0);

				if (m_pSlushMotor[i] != 0) {
					if (m_pSlushMotor[i]->IsConnected()) {
						nMotorsConnected++;
						m_pSlushMotor[i]->Dump();

						m_pMotorParams[i] = new MotorParams(fileName);
						assert(m_pMotorParams[i] != 0);
						m_pMotorParams[i]->Dump();
						m_pMotorParams[i]->Set(m_pSlushMotor[i]);

						L6470Params l6470Params(fileName);
						l6470Params.Dump();
						l6470Params.Set(m_pSlushMotor[i]);

						m_pSlushMotor[i]->Dump();

						m_pModeParams[i] = new ModeParams(fileName);
						assert(m_pModeParams[i] != 0);
						m_pModeParams[i]->Dump();

						m_pL6470DmxModes[i] = new L6470DmxModes((TL6470DmxModes) m_nDmxMode, m_nDmxStartAddressMode, m_pSlushMotor[i], m_pMotorParams[i], m_pModeParams[i]);
						assert(m_pL6470DmxModes[i] != 0);

						if (m_pL6470DmxModes[i] != 0) {
							if (m_nDmxStartAddress == DMX_ADDRESS_INVALID) {
								m_nDmxStartAddress = m_pL6470DmxModes[i]->GetDmxStartAddress();
								m_nDmxFootprint = m_pL6470DmxModes[i]->GetDmxFootPrint();
							} else {
								const uint16_t nDmxChannelLastCurrent = m_nDmxStartAddress + m_nDmxFootprint;
								m_nDmxStartAddress = MIN(m_nDmxStartAddress, m_pL6470DmxModes[i]->GetDmxStartAddress());

								const uint16_t nDmxChannelLastNew = m_nDmxStartAddressMode + m_pL6470DmxModes[i]->GetDmxFootPrint();
								m_nDmxFootprint = MAX(nDmxChannelLastCurrent, nDmxChannelLastNew) - m_nDmxStartAddress;
							}

							m_pSlotInfo[i] = new struct TLightSetSlotInfo[m_pL6470DmxModes[i]->GetDmxFootPrint()];
							char *pSlotInfoRaw = m_pSlotInfoRaw;

							for (unsigned j = 0; j < m_pL6470DmxModes[i]->GetDmxFootPrint(); j++) {
								bool isSet = false;

								if (pSlotInfoRaw != 0) {
									pSlotInfoRaw = Parse::DmxSlotInfo(pSlotInfoRaw, isSet, m_pSlotInfo[i][j].nType, m_pSlotInfo[i][j].nCategory);
								}

								if (!isSet) {
									m_pSlotInfo[i][j].nType = 0x00; // ST_PRIMARY
									m_pSlotInfo[i][j].nCategory = 0xFFFF; // SD_UNDEFINED
								}
							}

#ifndef NDEBUG
							printf("DMX Mode: %d, DMX Start Address: %d\n", m_pL6470DmxModes[i]->GetMode(), m_pL6470DmxModes[i]->GetDmxStartAddress());
							printf("DMX Start Address:%d, DMX Footprint:%d\n", (int) m_nDmxStartAddress, (int) m_nDmxFootprint);
#endif
						}
#ifndef NDEBUG
						printf("Use SPI Busy: %s\n", m_pSlushMotor[i]->GetUseSpiBusy() ? "Yes" : "No");
#endif
					} else {
						delete m_pSlushMotor[i];
						m_pSlushMotor[i] = 0;
					}
				} else {
					printf("Internal error!\n");
				}
			}
#ifndef NDEBUG
			printf("Motor %d: --------- end ---------\n", i);
#endif
		} else {
#ifndef NDEBUG
			printf("Configuration file : %s not found\n", fileName);
#endif
		}
	}

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->InitSwitch();
		}
	}

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		if (m_pSlushMotor[i] != 0) {
			while (m_pL6470DmxModes[i]->BusyCheck())
				;
		}
	}

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->InitPos();
		}
	}

#ifndef NDEBUG
	printf("Motors connected : %d\n", (int) nMotorsConnected);
#endif
	DEBUG_EXIT;
}

void SlushDmx::SetData(uint8_t nPortId, const uint8_t *pData, uint16_t nLength) {
	DEBUG_ENTRY;

	assert(pData != 0);
	assert(nLength <= DMX_MAX_CHANNELS);

	bool bIsDmxDataChanged[SLUSH_DMX_MAX_MOTORS];

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			bIsDmxDataChanged[i] = m_pL6470DmxModes[i]->IsDmxDataChanged(pData, nLength);

			if(bIsDmxDataChanged[i]) {
				m_pL6470DmxModes[i]->HandleBusy();
			}
		} else {
			bIsDmxDataChanged[i] = false;
		}
#ifndef NDEBUG
		printf("bIsDmxDataChanged[%d]=%d\n", i, bIsDmxDataChanged[i]);
#endif
	}

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		if (bIsDmxDataChanged[i]) {
			while (m_pL6470DmxModes[i]->BusyCheck())
				;
		}
	}

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		if (bIsDmxDataChanged[i]) {
			m_pL6470DmxModes[i]->DmxData(pData, nLength);
		}
	}

	UpdateIOPorts(pData, nLength);

	DEBUG_EXIT;
}

void SlushDmx::UpdateIOPorts(const uint8_t *pData, uint16_t nLength) {
	DEBUG_ENTRY;

	assert(pData != 0);
	assert(nLength <= DMX_MAX_CHANNELS);

	uint8_t *p;
	uint8_t nPortData;
	uint16_t nDmxAddress;

	nDmxAddress = m_nDmxStartAddressPortA;

	if (m_bSetPortA && (nLength >= nDmxAddress)) {
		nPortData = 0;
		p = (uint8_t *) pData + nDmxAddress - 1;

		for (uint8_t i = 0; i < m_nDmxFootprintPortA; i++) {
			if (nDmxAddress++ > nLength) {
				break;
			}
			if ((*p & (uint8_t) 0x80) != 0) {	// 0-127 is off, 128-255 is on
				nPortData = nPortData | (uint8_t) (1 << i);
			}
			p++;
		}

		if (nPortData != m_nDataPortA) {
			m_nDataPortA = nPortData;
			m_pBoard->IOWrite(SLUSH_IO_PORTA, nPortData);
#ifndef NDEBUG
			printf("\tPort A: DMX data has changed! %.2X\n", nPortData);
#endif
		} else {
#ifndef NDEBUG
			printf("\tPort A: Nothing to do..\n");
#endif
		}
	}

	nDmxAddress = m_nDmxStartAddressPortB;

	if (m_bSetPortB && (nLength >= nDmxAddress)) {
		nPortData = 0;
		p = (uint8_t *) pData + nDmxAddress - 1;

		for (uint8_t i = 0; i < m_nDmxFootprintPortB; i++) {
			if (nDmxAddress++ > nLength) {
				break;
			}
			if ((*p & (uint8_t) 0x80) != 0) {	// 0-127 is off, 128-255 is on
				nPortData = nPortData | (uint8_t) (1 << i);
			}
			p++;
		}

		if (nPortData != m_nDataPortB) {
			m_nDataPortB = nPortData;
			m_pBoard->IOWrite(SLUSH_IO_PORTB, nPortData);
#ifndef NDEBUG
			printf("\tPort B: DMX data has changed! %.2X\n", nPortData);
#endif
		} else {
#ifndef NDEBUG
			printf("\tPort B: Nothing to do..\n");
#endif
		}
	}

	DEBUG_EXIT;
}

bool SlushDmx::GetUseSpiBusy(void) const {
	return m_bUseSpiBusy;
}

void SlushDmx::SetUseSpiBusy(bool bUseSpiBusy) {
	m_bUseSpiBusy = bUseSpiBusy;
}

bool SlushDmx::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	DEBUG_ENTRY;

	if (nDmxStartAddress == m_nDmxStartAddress) {
		return true;
	}

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			const uint16_t nCurrentDmxStartAddress = m_pL6470DmxModes[i]->GetDmxStartAddress();
			const uint16_t nNewDmxStartAddress =  (nCurrentDmxStartAddress - m_nDmxStartAddress) + nDmxStartAddress;
#ifndef NDEBUG
			printf("\tMotor=%d, Current: DMX Start Address=%d, New: DMX Start Address=%d\n", i, nCurrentDmxStartAddress, nNewDmxStartAddress);
#endif
			m_pL6470DmxModes[i]->SetDmxStartAddress(nNewDmxStartAddress);
		}
	}

#ifndef NDEBUG
	printf("Current: m_nDmxStartAddressPortA=%d, m_nDmxStartAddressPortB=%d\n", m_nDmxStartAddressPortA, m_nDmxStartAddressPortB);
#endif

	m_nDmxStartAddressPortA = (m_nDmxStartAddressPortA - m_nDmxStartAddress) +  nDmxStartAddress;
	m_nDmxStartAddressPortB = (m_nDmxStartAddressPortB - m_nDmxStartAddress) +  nDmxStartAddress;

#ifndef NDEBUG
	printf("New: m_nDmxStartAddressPortA=%d, m_nDmxStartAddressPortB=%d\n", m_nDmxStartAddressPortA, m_nDmxStartAddressPortB);
#endif

	m_nDmxStartAddress = nDmxStartAddress;

	DEBUG_EXIT
	return true;
}

bool SlushDmx::GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo& tSlotInfo) {
	DEBUG2_ENTRY;

	if (nSlotOffset >  m_nDmxFootprint) {
		DEBUG2_EXIT
		return false;
	}

	const uint16_t nDmxAddress = m_nDmxStartAddress + nSlotOffset;

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		if ((m_pL6470DmxModes[i] != 0) && (m_pSlotInfo[i] != 0)) {
			const int16_t nOffset = nDmxAddress - m_pL6470DmxModes[i]->GetDmxStartAddress();

			if ((nDmxAddress >= m_pL6470DmxModes[i]->GetDmxStartAddress()) && (nOffset < m_pL6470DmxModes[i]->GetDmxFootPrint())) {

				tSlotInfo.nType = m_pSlotInfo[i][nOffset].nType;
				tSlotInfo.nCategory = m_pSlotInfo[i][nOffset].nCategory;

				DEBUG2_EXIT
				return true;
			}
		}
	}

	int16_t nOffset = nDmxAddress - m_nDmxStartAddressPortA;

#ifndef NDEBUG
	printf("\t\tnDmxAddress=%d, nOffset=%d\n", nDmxAddress, (int) nOffset);

	printf("\t\tm_bSetPortA=%d, m_nDmxStartAddressPortA=%d, m_nDmxFootprintPortA=%d\n",m_bSetPortA,m_nDmxStartAddressPortA, m_nDmxFootprintPortA);
#endif

	if (m_bSetPortA && (nDmxAddress >= m_nDmxStartAddressPortA) && (nOffset < m_nDmxFootprintPortA)) {

		tSlotInfo.nType = m_pSlotInfoPortA[nOffset].nType;
		tSlotInfo.nCategory = m_pSlotInfoPortA[nOffset].nCategory;

		DEBUG2_EXIT
		return true;
	}

	nOffset = nDmxAddress - m_nDmxStartAddressPortB;

#ifndef NDEBUG
	printf("\t\tm_bSetPortB=%d, m_nDmxStartAddressPortB=%d, m_nDmxFootprintPortB=%d\n",m_bSetPortB,m_nDmxStartAddressPortB, m_nDmxFootprintPortB);
#endif

	if (m_bSetPortB && (nDmxAddress >= m_nDmxStartAddressPortB) && (nOffset < m_nDmxFootprintPortB)) {

		tSlotInfo.nType = m_pSlotInfoPortB[nOffset].nType;
		tSlotInfo.nCategory = m_pSlotInfoPortB[nOffset].nCategory;

		DEBUG2_EXIT
		return true;
	}

	DEBUG2_EXIT
	return false;
}
