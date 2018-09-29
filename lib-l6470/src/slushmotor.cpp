/**
 * @file slushengine.cpp
 *
 */
/*
 * Based on https://github.com/Roboteurs/slushengine/tree/master/Slush
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

#include <assert.h>
#if defined (__linux__)
 #include <stdio.h>
#endif

#include "bcm2835.h"

#if defined(__linux__)
#elif defined(__circle__)
#else
 #include "bcm2835_gpio.h"
 #include "bcm2835_spi.h"
#endif

#include "slushmotor.h"
#include "slushboard.h"

#include "l6470constants.h"

SlushMotor::SlushMotor(int nMotor, bool bUseSPI): m_bIsBusy(false), m_bIsConnected(false) {
	assert(nMotor <= 3);

	m_nMotorNumber = nMotor;
	m_bUseSpiBusy = bUseSPI;

	switch (nMotor) {
	case 0:
		m_nSpiChipSelect = SLUSH_MTR0_CHIPSELECT;
		m_nBusyPin = SLUSH_MTR0_BUSY;
		break;
	case 1:
		m_nSpiChipSelect = SLUSH_MTR1_CHIPSELECT;
		m_nBusyPin = SLUSH_MTR1_BUSY;
		break;
	case 2:
		m_nSpiChipSelect = SLUSH_MTR2_CHIPSELECT;
		m_nBusyPin = SLUSH_MTR2_BUSY;
		break;
	case 3:
		m_nSpiChipSelect = SLUSH_MTR3_CHIPSELECT;
		m_nBusyPin = SLUSH_MTR3_BUSY;
		break;
	default:
		m_nSpiChipSelect = SLUSH_MTR0_CHIPSELECT;
		m_nBusyPin = SLUSH_MTR0_BUSY;
		break;
	}

	if (getParam(L6470_PARAM_CONFIG) == 0x2e88) {
#if defined (__linux__)
		printf("Motor Drive Connected on GPIO %d\n", m_nSpiChipSelect);
#endif
		setOverCurrent(2000);
		setMicroSteps(16);
		setCurrent(70, 90, 100, 100);

		getStatus();
		free();

		m_bIsConnected = true;
	} else {
#if defined (__linux__)
		fprintf(stderr, "communication issues; check SPI configuration and cables\n");
#endif
	}
}

SlushMotor::~SlushMotor(void) {
	free();
	m_bIsBusy = false;
	m_bIsConnected = false;
}

int SlushMotor::busyCheck(void) {
	if (m_bUseSpiBusy) {
		if (getParam(L6470_PARAM_STATUS) & L6470_STATUS_BUSY) {
			return 0;
		} else {
			return 1;
		}
	} else {
		if (!m_bIsBusy) {
			if (getParam(L6470_PARAM_STATUS) & L6470_STATUS_BUSY) {
				return 0;
			} else {
				m_bIsBusy = true;
				return 1;
			}
		}
		// By default, the BUSY pin is forced low when the device is performing a command
		if (bcm2835_gpio_lev(m_nBusyPin) == HIGH) {
			m_bIsBusy = false;
			return 0;
		} else {
			return 1;
		}
	}
}

uint8_t SlushMotor::SPIXfer(uint8_t data) {
	char dataPacket[1];

	dataPacket[0] = (char) data;

	bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE3);

	bcm2835_gpio_clr(m_nSpiChipSelect);
	bcm2835_spi_transfern(dataPacket, 1);
	bcm2835_gpio_set(m_nSpiChipSelect);

	return (uint8_t) dataPacket[0];
}

/*
 * Roboteurs Slushengine Phyton compatible methods
 */

int SlushMotor::isBusy(void) {
	return busyCheck();
}

void SlushMotor::setAsHome(void) {
	resetPos();
}

void SlushMotor::setOverCurrent(unsigned int nCurrentmA) {
	uint8_t OCValue = nCurrentmA / 375;

	if (OCValue > 0x0F) {
		OCValue = 0x0F;
	}

	setParam(L6470_PARAM_OCD_TH, OCValue);
}

void SlushMotor::softFree(void) {
	softHiZ();
}

void SlushMotor::free(void) {
	hardHiZ();
}

/*
 * Additional methods
 */
bool SlushMotor::IsConnected(void) const {
	return m_bIsConnected;
}

bool SlushMotor::GetUseSpiBusy(void) const {
	return m_bUseSpiBusy;
}

void SlushMotor::SetUseSpiBusy(bool bUseSpiBusy) {
	m_bUseSpiBusy = bUseSpiBusy;
}
