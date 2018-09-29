/**
 * @file sparkfundmx.cpp
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
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "bcm2835.h"
#if defined(__linux__)
 #define udelay bcm2835_delayMicroseconds
#elif defined(__circle__)
#else
 #include "bcm2835_gpio.h"
 #include "bcm2835_spi.h"
#endif

#include "sparkfundmx.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "l6470params.h"
#include "l6470dmxmodes.h"

#include "motorparams.h"
#include "modeparams.h"

#include "debug.h"

#ifndef MAX
 #define MAX(a,b)       (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
 #define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#define GPIO_BUSY_IN		RPI_V2_GPIO_P1_35
#define GPIO_RESET_OUT 		RPI_V2_GPIO_P1_38

#define DMX_MAX_CHANNELS	512

static const char SPARKFUN_PARAMS_POSITION[] ALIGNED = "sparkfun_position";
static const char SPARKFUN_PARAMS_SPI_CS[] ALIGNED = "sparkfun_spi_cs";
static const char SPARKFUN_PARAMS_RESET_PIN[] ALIGNED = "sparkfun_reset_pin";
static const char SPARKFUN_PARAMS_BUSY_PIN[] ALIGNED = "sparkfun_busy_pin";

static const char PARAMS_DMX_MODE[] ALIGNED = "dmx_mode";
static const char PARAMS_DMX_START_ADDRESS[] ALIGNED = "dmx_start_address";

void SparkFunDmx::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((SparkFunDmx *) p)->callbackFunction(s);
}

void SparkFunDmx::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t value8;
	uint16_t value16;

	if (Sscan::Uint8(pLine, PARAMS_DMX_MODE, &value8) == SSCAN_OK) {
		m_nDmxMode = value8;
	} else if (Sscan::Uint16(pLine, PARAMS_DMX_START_ADDRESS, &value16) == SSCAN_OK) {
		m_nDmxStartAddressMode = value16;
	} else if (sscan_uint8_t(pLine, SPARKFUN_PARAMS_POSITION, &value8) == SSCAN_OK) {
		m_nPosition = value8;
		m_bIsPositionSet = true;
	} else if (Sscan::Uint8(pLine, SPARKFUN_PARAMS_SPI_CS, &value8) == SSCAN_OK) {
		m_nSpiCs = value8;
		m_bIsSpiCsSet = true;
	} else if (Sscan::Uint8(pLine, SPARKFUN_PARAMS_RESET_PIN, &value8) == SSCAN_OK) {
		m_nResetPin = value8;
		m_bIsResetSet = true;
	} else if (Sscan::Uint8(pLine, SPARKFUN_PARAMS_BUSY_PIN, &value8) == SSCAN_OK) {
		m_nBusyPin = value8;
		m_bIsBusyPinSet = true;
	}
}

SparkFunDmx::SparkFunDmx(void): m_nDmxStartAddress(DMX_ADDRESS_INVALID), m_nDmxFootprint(0) {
	DEBUG_ENTRY;

	m_nPosition = 0;
	m_nSpiCs = 0;
	m_nResetPin = 0;
	m_nBusyPin = 0;

	m_bIsPositionSet = false;
	m_bIsSpiCsSet = false;
	m_bIsResetSet = false;
	m_bIsBusyPinSet = false;

	m_nDmxMode = L6470DMXMODE_UNDEFINED;
	m_nDmxStartAddressMode = 0;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		m_pAutoDriver[i] = 0;
		m_pMotorParams[i] = 0;
		m_pModeParams[i] = 0;
		m_pL6470DmxModes[i] = 0;
	}

#if defined(__linux__) || defined(__circle__)
	if (bcm2835_init() == 0) {
		printf("Not able to init the bmc2835 library\n");
		assert(0);
	}
#endif

	DEBUG_EXIT;
}

SparkFunDmx::~SparkFunDmx(void) {
	DEBUG_ENTRY;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pAutoDriver[i] != 0) {
			delete m_pAutoDriver[i];
			m_pAutoDriver[i] = 0;
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
	}

	DEBUG_EXIT;
}

void SparkFunDmx::Start(uint8_t nPort) {
	DEBUG_ENTRY;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->Start();
		}
	}

	DEBUG_EXIT;
}

void SparkFunDmx::Stop(uint8_t nPort) {
	DEBUG_ENTRY;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->Stop();
		}
	}

	DEBUG_EXIT;
}

void SparkFunDmx::ReadConfigFiles(void) {
	DEBUG_ENTRY;

	m_bIsSpiCsSet = false;
	m_bIsResetSet = false;
	m_bIsBusyPinSet = false;

	ReadConfigFile configfile(SparkFunDmx::staticCallbackFunction, this);

	if (configfile.Read("sparkfun.txt")) {
#ifndef NDEBUG
		printf("\'sparkfun.txt\' (global settings):\n");

		if (m_bIsSpiCsSet) {
			printf("\tSPI CS : %d\n", m_nSpiCs);
		}

		if (m_bIsResetSet) {
			printf("\tReset pin: %d\n", m_nResetPin);
		}

		if (m_bIsBusyPinSet) {
			printf("\tBusy pin: %d\n", m_nBusyPin);
		}
#endif
	}

	if (!m_bIsResetSet) {
		m_nResetPin = GPIO_RESET_OUT;
		m_bIsResetSet = true;
#ifndef NDEBUG
		printf("\tReset pin: %d\n", m_nResetPin);
#endif
	}

	if (m_bIsBusyPinSet) {
		bcm2835_gpio_fsel(m_nBusyPin, BCM2835_GPIO_FSEL_INPT);
	}

	bcm2835_gpio_fsel(m_nResetPin, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_set(m_nResetPin);

	bcm2835_gpio_clr(m_nResetPin);
	udelay(10000);
	bcm2835_gpio_set(m_nResetPin);
	udelay(10000);

	bcm2835_spi_begin();

	char fileName[] = "motor%.txt";

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {

		fileName[5] = (char) i + '0';

		m_bIsPositionSet = false;

		if (configfile.Read(fileName)) {
#ifndef NDEBUG
			printf("Motor %d:\n", i);
#endif
			if (m_bIsPositionSet && m_bIsSpiCsSet) {
#ifndef NDEBUG
				printf("\t%s=%d\n", SPARKFUN_PARAMS_POSITION, m_nPosition);
				printf("\t%s=%d\n", SPARKFUN_PARAMS_SPI_CS, m_nSpiCs);
				printf("\t%s=%d\n", SPARKFUN_PARAMS_RESET_PIN, m_nResetPin);
				if(m_bIsBusyPinSet) {
					printf("\t%s=%d\n", SPARKFUN_PARAMS_BUSY_PIN, m_nBusyPin);
				}
				printf("\t-----------------------------\n");
				printf("\t%s=%d (DMX footprint=%d)\n", PARAMS_DMX_MODE, m_nDmxMode, L6470DmxModes::GetDmxFootPrintMode(m_nDmxMode));
				printf("\t%s=%d\n", PARAMS_DMX_START_ADDRESS, m_nDmxStartAddressMode);
				printf("\t=============================\n");
#endif
				if ((m_nDmxStartAddressMode <= DMX_MAX_CHANNELS) && (L6470DmxModes::GetDmxFootPrintMode(m_nDmxMode) != 0)) {

					if (m_bIsBusyPinSet) {
						m_pAutoDriver[i] = new AutoDriver(m_nPosition, m_nSpiCs, m_nResetPin, m_nBusyPin);
					} else {
						m_pAutoDriver[i] = new AutoDriver(m_nPosition, m_nSpiCs, m_nResetPin);
					}

					assert(m_pAutoDriver[i] != 0);

					if (m_pAutoDriver[i] != 0) {
						if (m_pAutoDriver[i]->IsConnected()) {
							m_pAutoDriver[i]->setMotorNumber(i);
							m_pAutoDriver[i]->Dump();

							m_pMotorParams[i] = new MotorParams(fileName);
							assert(m_pMotorParams[i] != 0);
							m_pMotorParams[i]->Dump();
							m_pMotorParams[i]->Set(m_pAutoDriver[i]);

							L6470Params l6470Params(fileName);
							l6470Params.Dump();
							l6470Params.Set(m_pAutoDriver[i]);

							m_pAutoDriver[i]->Dump();

							m_pModeParams[i] = new ModeParams(fileName);
							assert(m_pModeParams[i] != 0);
							m_pModeParams[i]->Dump();

							m_pL6470DmxModes[i] = new L6470DmxModes((TL6470DmxModes) m_nDmxMode, m_nDmxStartAddressMode, m_pAutoDriver[i], m_pMotorParams[i], m_pModeParams[i]);
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
#ifndef NDEBUG
								printf("DMX Mode: %d, DMX Start Address: %d\n", m_pL6470DmxModes[i]->GetMode(), m_pL6470DmxModes[i]->GetDmxStartAddress());
								printf("DMX Start Address:%d, DMX Footprint:%d\n", (int) m_nDmxStartAddress, (int) m_nDmxFootprint);
#endif
							}
						} else {
							delete m_pAutoDriver[i];
							m_pAutoDriver[i] = 0;
							printf("Communication issues; check SPI configuration and cables\n");
						}
					} else {
						printf("Internal error!\n");
					}
				}
			} else {
				if(!m_bIsPositionSet) {
					printf("Missing %s=\n", SPARKFUN_PARAMS_POSITION);
				}
				if(!m_bIsSpiCsSet) {
					printf("Missing %s=\n", SPARKFUN_PARAMS_SPI_CS);
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

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->InitSwitch();
		}
	}

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pAutoDriver[i] != 0) {
			while (m_pAutoDriver[i]->busyCheck())
				;
		}
	}

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->InitPos();
		}
	}

#ifndef NDEBUG
	printf("Motors connected : %d\n", (int) AutoDriver::getNumBoards());
#endif
	DEBUG_EXIT;
}

void SparkFunDmx::SetData(uint8_t nPortId, const uint8_t *pData, uint16_t nLength) {
	DEBUG_ENTRY;

	assert(pData != 0);
	assert(nLength <= DMX_MAX_CHANNELS);

	bool bIsDmxDataChanged[SPARKFUN_DMX_MAX_MOTORS];

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
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

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (bIsDmxDataChanged[i]) {
			while (m_pL6470DmxModes[i]->BusyCheck())
				;
		}
	}

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (bIsDmxDataChanged[i]) {
			m_pL6470DmxModes[i]->DmxData(pData, nLength);
		}
	}

	DEBUG_EXIT;
}

bool SparkFunDmx::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	DEBUG_ENTRY;

	if (nDmxStartAddress == m_nDmxStartAddress) {
		return true;
	}

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			const uint16_t nCurrentDmxStartAddress = m_pL6470DmxModes[i]->GetDmxStartAddress();
			const uint16_t nNewDmxStartAddress =  (nCurrentDmxStartAddress - m_nDmxStartAddress) + nDmxStartAddress;
#ifndef NDEBUG
			printf("\tMotor=%d, Current DMX Start Address=%d, New DMX Start Address=%d\n", i, nCurrentDmxStartAddress, nNewDmxStartAddress);
#endif
			m_pL6470DmxModes[i]->SetDmxStartAddress(nNewDmxStartAddress);
		}
	}

	m_nDmxStartAddress = nDmxStartAddress;

	DEBUG_EXIT;
	return true;
}
