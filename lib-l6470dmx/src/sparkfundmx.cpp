/**
 * @file sparkfundmx.cpp
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
#include <stdbool.h>
#include <assert.h>

#include "sparkfundmx.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "l6470params.h"
#include "l6470dmxmodes.h"

#include "motorparams.h"

#include "debug.h"

#include "bcm2835.h"
#if defined(__linux__)
#define udelay bcm2835_delayMicroseconds
#else
#include "bcm2835_gpio.h"
#include "bcm2835_spi.h"
#endif

#define GPIO_BUSY_IN		RPI_V2_GPIO_P1_35
#define GPIO_RESET_OUT 		RPI_V2_GPIO_P1_38

#define DMX_MAX_CHANNELS	512

static const char SPARKFUN_PARAMS_POSITION[] = "sparkfun_position";
static const char SPARKFUN_PARAMS_SPI_CS[] = "sparkfun_spi_cs";
static const char SPARKFUN_PARAMS_RESET_PIN[] = "sparkfun_reset_pin";
static const char SPARKFUN_PARAMS_BUSY_PIN[] = "sparkfun_busy_pin";

static const char PARAMS_DMX_MODE[] = "dmx_mode";
static const char PARAMS_DMX_START_ADDRESS[] = "dmx_start_address";

void SparkFunDmx::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((SparkFunDmx *) p)->callbackFunction(s);
}

void SparkFunDmx::callbackFunction(const char *pLine) {
	uint8_t value8;
	uint16_t value16;

	if (Sscan::Uint8(pLine, PARAMS_DMX_MODE, &value8) == SSCAN_OK) {
		m_nDmxMode = value8;
	} else if (Sscan::Uint16(pLine, PARAMS_DMX_START_ADDRESS, &value16) == SSCAN_OK) {
		m_nDmxStartAddress = value16;
	} else if (sscan_uint8_t(pLine, SPARKFUN_PARAMS_POSITION, &value8) == SSCAN_OK) {
		m_nPosition = value8;
		is_position_set = true;
	} else if (Sscan::Uint8(pLine, SPARKFUN_PARAMS_SPI_CS, &value8) == SSCAN_OK) {
		m_nSpiCs = value8;
		is_spi_cs_set = true;
	} else if (Sscan::Uint8(pLine, SPARKFUN_PARAMS_RESET_PIN, &value8) == SSCAN_OK) {
		m_nResetPin = value8;
		is_reset_set = true;
	} else if (Sscan::Uint8(pLine, SPARKFUN_PARAMS_BUSY_PIN, &value8) == SSCAN_OK) {
		m_nBusyPin = value8;
		is_busy_pin_set = true;
	}
}

SparkFunDmx::SparkFunDmx(void) {
	DEBUG_ENTRY;

	m_nPosition = 0;
	m_nSpiCs = 0;
	m_nResetPin = 0;
	m_nBusyPin = 0;

	is_position_set = false;
	is_spi_cs_set = false;
	is_reset_set = false;
	is_busy_pin_set = false;

	m_nDmxMode = L6470DMXMODE_UNDEFINED;
	m_nDmxStartAddress = 0;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		m_pAutoDriver[i] = 0;
		m_pMotorParams[i] = 0;
		m_pL6470DmxModes[i] = 0;
	}

#if defined(__linux__)
	if (bcm2835_init() == 0) {
		fprintf(stderr, "Not able to init the bmc2835 library\n");
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
		if (m_pL6470DmxModes[i] != 0) {
			delete m_pL6470DmxModes[i];
			m_pL6470DmxModes[i] = 0;
		}
	}

	DEBUG_EXIT;
}

void SparkFunDmx::Start(void) {
	DEBUG_ENTRY;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->Start();
		}
	}

	DEBUG_EXIT;
}

void SparkFunDmx::Stop(void) {
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

	is_spi_cs_set = false;
	is_reset_set = false;
	is_busy_pin_set = false;

	ReadConfigFile configfile(SparkFunDmx::staticCallbackFunction, this);

	if (configfile.Read("sparkfun.txt")) {
		puts("\'sparkfun.txt\' (global settings):");

		if (is_spi_cs_set) {
			printf("\tSPI CS : %d\n", m_nSpiCs);
		}

		if (is_reset_set) {
			printf("\tReset pin: %d\n", m_nResetPin);
		}

		if (is_busy_pin_set) {
			printf("\tBusy pin: %d\n", m_nBusyPin);
		}
	}

	if (!is_reset_set) {
		m_nResetPin = GPIO_RESET_OUT;
		is_reset_set = true;
		printf("\tReset pin: %d\n", m_nResetPin);
	}

	if (is_busy_pin_set) {
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

		is_position_set = false;

		if (configfile.Read(fileName)) {
			printf("Motor %d:\n", i);

			if (is_position_set && is_spi_cs_set) {
				printf("\t%s=%d\n", SPARKFUN_PARAMS_POSITION, m_nPosition);
				printf("\t%s=%d\n", SPARKFUN_PARAMS_SPI_CS, m_nSpiCs);
				printf("\t%s=%d\n", SPARKFUN_PARAMS_RESET_PIN, m_nResetPin);
				if(is_busy_pin_set) {
					printf("\t%s=%d\n", SPARKFUN_PARAMS_BUSY_PIN, m_nBusyPin);
				}
				puts("\t-----------------------------");
				printf("\t%s=%d (DMX footprint=%d)\n", PARAMS_DMX_MODE, m_nDmxMode, L6470DmxModes::GetDmxFootPrintMode(m_nDmxMode));
				printf("\t%s=%d\n", PARAMS_DMX_START_ADDRESS, m_nDmxStartAddress);
				puts("\t=============================");

				if ((m_nDmxStartAddress <= DMX_MAX_CHANNELS) && (L6470DmxModes::GetDmxFootPrintMode(m_nDmxMode) != 0)) {

					if (is_busy_pin_set) {
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

							m_pL6470DmxModes[i] = new L6470DmxModes((TL6470DmxModes) m_nDmxMode, m_nDmxStartAddress, m_pAutoDriver[i], m_pMotorParams[i]);
							assert(m_pL6470DmxModes[i] != 0);

							if (m_pL6470DmxModes[i] != 0) {
								printf("DMX Mode: %d, DMX Start Address: %d\n", m_pL6470DmxModes[i]->GetMode(), m_pL6470DmxModes[i]->GetDmxStartAddress());
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
				if(!is_position_set) {
					printf("Missing %s=\n", SPARKFUN_PARAMS_POSITION);
				}
				if(!is_spi_cs_set) {
					printf("Missing %s=\n", SPARKFUN_PARAMS_SPI_CS);
				}
			}
			printf("Motor %d: --------- end ---------\n", i);
		} else {
			printf("Configuration file : %s not found\n", fileName);
		}
	}

	printf("Motors connected : %d\n", (int) AutoDriver::getNumBoards());

	DEBUG_EXIT;
}

void SparkFunDmx::SetData(const uint8_t nPortId, const uint8_t *pData, const uint16_t nLength) {
	DEBUG_ENTRY;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->DmxData(pData, nLength);
		}
	}

	DEBUG_EXIT;
}
