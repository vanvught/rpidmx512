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

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "slushdmx.h"
#include "slushboard.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "l6470params.h"
#include "l6470dmxmodes.h"

#include "motorparams.h"

#include "debug.h"

#define IO_PINS_IOPORT		8
#define DMX_MAX_CHANNELS	512

static const char PARAMS_SLUSH_DMX_START_ADDRESS_PORT_A[]= "dmx_start_address_port_a";
static const char PARAMS_SLUSH_DMX_START_ADDRESS_PORT_B[]= "dmx_start_address_port_b";

static const char PARAMS_DMX_MODE[] = "dmx_mode";
static const char PARAMS_DMX_START_ADDRESS[]= "dmx_start_address";

void SlushDmx::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((SlushDmx *) p)->callbackFunction(s);
}

void SlushDmx::callbackFunction(const char *pLine) {
	if (Sscan::Uint8(pLine, PARAMS_DMX_MODE, &m_nDmxMode) == SSCAN_OK) {
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_DMX_START_ADDRESS, &m_nDmxStartAddress) == SSCAN_OK) {
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_SLUSH_DMX_START_ADDRESS_PORT_A, &m_nDmxStartAddressPortA) == SSCAN_OK) {
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_SLUSH_DMX_START_ADDRESS_PORT_B, &m_nDmxStartAddressPortB) == SSCAN_OK) {
		return;
	}
}

SlushDmx::SlushDmx(void): m_bSetPortA(false), m_bSetPortB(false) {
	DEBUG_ENTRY;

	m_pBoard = new SlushBoard();
	assert(m_pBoard != 0);

	m_nDmxMode = L6470DMXMODE_UNDEFINED;
	m_nDmxStartAddress = 0;

	m_nDmxStartAddressPortA = 0;
	m_nDmxStartAddressPortB = 0;

	m_nDataPortA = 0;
	m_nDataPortB = 0;

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		m_pSlushMotor[i] = 0;
		m_pL6470DmxModes[i] = 0;
	}

	DEBUG_EXIT;
}

SlushDmx::~SlushDmx(void) {
	DEBUG_ENTRY;

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {

		if (m_pSlushMotor[i] != 0) {
			delete m_pSlushMotor[i];
			m_pSlushMotor[i] = 0;
		}

		if (m_pL6470DmxModes[i] != 0) {
			delete m_pL6470DmxModes[i];
			m_pL6470DmxModes[i] = 0;
		}
	}

	delete m_pBoard;
	m_pBoard = 0;

	DEBUG_EXIT;
}

void SlushDmx::Start(void) {
	DEBUG_ENTRY;

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->Start();
		}
	}

	DEBUG_EXIT;
}

void SlushDmx::Stop(void) {
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

	ReadConfigFile configfile(SlushDmx::staticCallbackFunction, this);

	m_nDmxStartAddressPortA = 0;
	m_nDmxStartAddressPortB = 0;

	if (configfile.Read("slush.txt")) {
		if ((m_nDmxStartAddressPortA >= 1) && (m_nDmxStartAddressPortA <= (DMX_MAX_CHANNELS - IO_PINS_IOPORT))) {
			for (int pin = 0; pin < IO_PINS_IOPORT; pin++) {
				m_pBoard->IOFSel(SLUSH_IO_PORTA, (TSlushIOPins) pin, SLUSH_IO_FSEL_OUTP);
			}
			m_bSetPortA = true;
			printf("DMX Start Address Output PortA = %d\n", m_nDmxStartAddressPortA);

		}
		if ((m_nDmxStartAddressPortB >= 1) && (m_nDmxStartAddressPortB <= (DMX_MAX_CHANNELS - IO_PINS_IOPORT))) {
			for (int pin = 0; pin < IO_PINS_IOPORT; pin++) {
				m_pBoard->IOFSel(SLUSH_IO_PORTB, (TSlushIOPins) pin, SLUSH_IO_FSEL_OUTP);
			}
			m_bSetPortB = true;
			printf("DMX Start Address Output PortB = %d\n", m_nDmxStartAddressPortB);
		}
	}

	char fileName[] = "motor%.txt";

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {

		fileName[5] = (char) i + '0';

		if (configfile.Read(fileName)) {
			printf("Motor %d:\n", i);
			printf("\t%s=%d (DMX footprint=%d)\n", PARAMS_DMX_MODE, m_nDmxMode, L6470DmxModes::GetDmxFootPrintMode(m_nDmxMode));
			printf("\t%s=%d\n", PARAMS_DMX_START_ADDRESS, m_nDmxStartAddress);
			puts("\t=============================");

			if ((m_nDmxStartAddress <= DMX_MAX_CHANNELS) && (L6470DmxModes::GetDmxFootPrintMode(m_nDmxMode) != 0)) {
				m_pSlushMotor[i] = new SlushMotor(i, false);
				assert(m_pSlushMotor[i] != 0);

				if (m_pSlushMotor[i] != 0) {
					m_pSlushMotor[i]->Dump();

					m_pMotorParams[i] = new MotorParams(fileName);
					assert(m_pMotorParams[i] != 0);
					m_pMotorParams[i]->Dump();
					m_pMotorParams[i]->Set(m_pSlushMotor[i]);

					L6470Params l6470Params(fileName);
					l6470Params.Dump();
					l6470Params.Set(m_pSlushMotor[i]);

					m_pSlushMotor[i]->Dump();

					m_pL6470DmxModes[i] = new L6470DmxModes((TL6470DmxModes) m_nDmxMode, m_nDmxStartAddress, m_pSlushMotor[i], m_pMotorParams[i]);
					assert(m_pL6470DmxModes[i] != 0);

					if (m_pL6470DmxModes[i] != 0) {
						printf("DMX Mode = %d, DMX Start Address = %d\n", m_pL6470DmxModes[i]->GetMode(), m_pL6470DmxModes[i]->GetDmxStartAddress());
					}

					printf("Motor %d --------- end ---------\n", i);
				}
			}
		} else {
			printf("Configuration file : %s not found\n", fileName);
		}
	}

	DEBUG_EXIT;
}

void SlushDmx::SetData(const uint8_t nPortId, const uint8_t *pData, const uint16_t nLength) {
	DEBUG_ENTRY;

	for (int i = 0; i < SLUSH_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->DmxData(pData, nLength);
		}
	}

	UpdateIOPorts(pData, nLength);

	DEBUG_EXIT;
}

void SlushDmx::UpdateIOPorts(const uint8_t *pData, const uint16_t nLength) {
	DEBUG_ENTRY;

	assert(pData != 0);

	uint8_t *p;
	uint8_t nPortData;

	if (m_bSetPortA && (nLength >= m_nDmxStartAddressPortA + IO_PINS_IOPORT)) {
		nPortData = 0;
		p = (uint8_t *) pData + m_nDmxStartAddressPortA - 1;

		for (uint8_t i = 0; i < IO_PINS_IOPORT; i++) {
			if ((*p & (uint8_t) 0x80) != 0) {	// 0-127 is off, 128-255 is on
				nPortData = nPortData | (uint8_t) (1 << i);
			}
			p++;
		}

		if (nPortData != m_nDataPortA) {
			m_nDataPortA = nPortData;
			m_pBoard->IOWrite(SLUSH_IO_PORTA, nPortData);
#ifndef NDEBUG
			printf("\tDMX data has changed! %.2X\n", nPortData);
#endif
		} else {
#ifndef NDEBUG
			puts("\tNothing to do..");
#endif
		}
	}

	if (m_bSetPortB && (nLength >= m_nDmxStartAddressPortB + IO_PINS_IOPORT)) {
		nPortData = 0;
		p = (uint8_t *) pData + m_nDmxStartAddressPortB - 1;

		for (uint8_t i = 0; i < IO_PINS_IOPORT; i++) {
			if ((*p & (uint8_t) 0x80) != 0) {	// 0-127 is off, 128-255 is on
				nPortData = nPortData | (uint8_t) (1 << i);
			}
			p++;
		}

		if (nPortData != m_nDataPortB) {
			m_nDataPortB = nPortData;
			m_pBoard->IOWrite(SLUSH_IO_PORTB, nPortData);
#ifndef NDEBUG
			printf("\tDMX data has changed! %.2X\n", nPortData);
#endif
		} else {
#ifndef NDEBUG
			puts("\tNothing to do..");
#endif
		}
	}

	DEBUG_EXIT;
}
