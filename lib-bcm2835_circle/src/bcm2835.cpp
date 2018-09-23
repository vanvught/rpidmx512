/**
 * @file bcm2835.cpp
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include <circle/gpiopin.h>
#include <circle/spimaster.h>
#include <circle/i2cmaster.h>
#include <circle/timer.h>
#include <circle/types.h>

#include "bcm2835.h"

#define GPIO_MAX_PINS	32

static CGPIOPin *_pGpioPins[GPIO_MAX_PINS];
static CSPIMaster *_pSPIMaster;
static CI2CMaster *_pI2CMaster;

bool _IsBeginCalled = false;

static unsigned _nSpiChipSelect;
static unsigned _nI2cChipSelect;

extern "C" {

int bcm2835_init(void) {
	assert(!_IsBeginCalled);

	if (_IsBeginCalled) {
		return -1;
	}

	for (unsigned i = 0; i < GPIO_MAX_PINS ; i++) {
		_pGpioPins[i] = 0;
	}

	_pSPIMaster = new CSPIMaster;
	assert(_pSPIMaster != 0);

	_pI2CMaster = new CI2CMaster(1, TRUE);
	assert(_pI2CMaster != 0);

	_IsBeginCalled = true;

	return 1;
}

static enum TGPIOMode convert_mode(enum TBcm2835GioFsel mode) {
	switch (mode) {
		case BCM2835_GPIO_FSEL_INPT:
			return GPIOModeInput;
			break;
		case BCM2835_GPIO_FSEL_OUTP:
			return GPIOModeOutput;
			break;
		case BCM2835_GPIO_FSEL_ALT0:
			return GPIOModeAlternateFunction0;
			break;
		case BCM2835_GPIO_FSEL_ALT1:
			return GPIOModeAlternateFunction1;
			break;
		case BCM2835_GPIO_FSEL_ALT2:
			return GPIOModeAlternateFunction2;
			break;
		case BCM2835_GPIO_FSEL_ALT3:
			return GPIOModeAlternateFunction3;
			break;
		case BCM2835_GPIO_FSEL_ALT4:
			return GPIOModeAlternateFunction4;
			break;
		case BCM2835_GPIO_FSEL_ALT5:
			return GPIOModeAlternateFunction5;
			break;
		default:
			break;
	}

	return GPIOModeUnknown;
}

/*
 * GPIO
 */

void bcm2835_gpio_fsel(uint8_t pin, uint8_t mode) {
	assert(pin < GPIO_MAX_PINS);

	if (_pGpioPins[pin] == 0) {
		_pGpioPins[pin] = new CGPIOPin(pin, convert_mode((enum TBcm2835GioFsel) mode));
	} else {
		_pGpioPins[pin]->SetMode(convert_mode((enum TBcm2835GioFsel) mode), FALSE);
	}
}

void bcm2835_gpio_set(uint8_t pin) {
	assert(pin < GPIO_MAX_PINS);
	assert(_pGpioPins[pin] != 0);

	_pGpioPins[pin]->Write(HIGH);
}

void bcm2835_gpio_clr(uint8_t pin) {
	assert(pin < GPIO_MAX_PINS);
	assert(_pGpioPins[pin] != 0);

	_pGpioPins[pin]->Write(LOW);

}

uint8_t bcm2835_gpio_lev(uint8_t pin) {
	assert(pin < GPIO_MAX_PINS);
	assert(_pGpioPins[pin] != 0);

	return (uint8_t) _pGpioPins[pin]->Read();
}

/*
 * SPI
 */

int bcm2835_spi_begin(void) {
	assert(_pSPIMaster != 0);

	if (_pSPIMaster->Initialize()) {
		return 1;
	}

	return 0;
}

void bcm2835_spi_end(void) {
	// nothing to do here
}

void bcm2835_spi_chipSelect(uint8_t cs) {
	_nSpiChipSelect = cs;
}

void bcm2835_spi_transfernb(char* tbuf, char* rbuf, uint32_t len) {
	assert(_pSPIMaster != 0);

	(void) _pSPIMaster->WriteRead(_nSpiChipSelect, (const void *)tbuf, (void *)rbuf, (unsigned) len);
}

void bcm2835_spi_transfern(char* buf, uint32_t len) {
	bcm2835_spi_transfernb(buf, buf, len);
}

void bcm2835_spi_setDataMode(uint8_t mode) {
	assert(_pSPIMaster != 0);

	unsigned CPOL = 0;
	unsigned CPHA = 0;

	switch ((enum TBcm2835SPIMode) mode) {
		case BCM2835_SPI_MODE0:
			CPOL = 0;
			CPHA = 0;
			break;
		case BCM2835_SPI_MODE1:
			CPOL = 0;
			CPHA = 1;
			break;
		case BCM2835_SPI_MODE2:
			CPOL = 1;
			CPHA = 0;
			break;
		case BCM2835_SPI_MODE3:
			CPOL = 1;
			CPHA = 1;
			break;
		default:
			break;
	}

	_pSPIMaster->SetMode(CPOL, CPHA);
}

void bcm2835_spi_setClockDivider(uint16_t divider) {
	assert(_pSPIMaster != 0);

	_pSPIMaster->SetClock(BCM2835_CORE_CLK_HZ / divider);
}

void bcm2835_spi_set_speed_hz(uint32_t speed_hz) {
	assert(_pSPIMaster != 0);

	_pSPIMaster->SetClock(speed_hz);
}

/*
 * I2C
 */

int bcm2835_i2c_begin(void) {
	assert(_pI2CMaster != 0);

	if(_pI2CMaster->Initialize()) {
		return 1;
	}

	return 0;
}

void bcm2835_i2c_end(void) {
	// Nothing to do here
}

void bcm2835_i2c_setClockDivider(uint16_t divider) {
	assert(_pI2CMaster != 0);

	_pI2CMaster->SetClock(BCM2835_CORE_CLK_HZ / divider);
}

void bcm2835_i2c_set_baudrate(uint32_t baudrate) {
	assert(_pI2CMaster != 0);

	_pI2CMaster->SetClock(baudrate);
}

void bcm2835_i2c_setSlaveAddress(uint8_t addr) {
	_nI2cChipSelect = (uint8_t) addr;
}

uint8_t bcm2835_i2c_write(const char *buf, uint32_t len) {
	assert(buf != 0);
	assert(_pI2CMaster != 0);

	const int nResult = _pI2CMaster->Write(_nI2cChipSelect, buf, len);

	if (nResult > 0) {
		if ((uint32_t) nResult == len) {
			return BCM2835_I2C_REASON_OK;
		} else {
			return BCM2835_I2C_REASON_ERROR_DATA;
		}
	}

	if (nResult == -I2C_MASTER_ERROR_NACK) {
		return BCM2835_I2C_REASON_ERROR_NACK;
	}

	return BCM2835_I2C_REASON_ERROR_CLKT;
}

uint8_t bcm2835_i2c_read(char* buf, uint32_t len) {
	assert(buf != 0);
	assert(_pI2CMaster != 0);

	const int nResult = _pI2CMaster->Read(_nI2cChipSelect, buf, len);

	if (nResult > 0) {
		if ((uint32_t) nResult == len) {
			return BCM2835_I2C_REASON_OK;
		} else {
			return BCM2835_I2C_REASON_ERROR_DATA;
		}
	}

	if (nResult == -I2C_MASTER_ERROR_NACK) {
		return BCM2835_I2C_REASON_ERROR_NACK;
	}

	return BCM2835_I2C_REASON_ERROR_CLKT;
}


/*
 * Timer
 */

void bcm2835_delayMicroseconds(uint32_t micros) {
	CTimer::Get ()->usDelay (micros);
}

}
