#if !defined (__CYGWIN__)
/**
 * @file buttonsbw.cpp
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "input.h"

#include "buttonsbw.h"

#include "hal_i2c.h"

extern "C" {
extern uint32_t micros(void);
#if defined(__linux__)
extern void bcm2835_delayMicroseconds (const uint64_t);
#define udelay bcm2835_delayMicroseconds
#else
extern void udelay(uint32_t);
#endif
}

#define BW_UI_I2C_BYTE_WAIT_US			28
#define BW_UI_I2C_DELAY_WRITE_READ_US	90

#define BW_PORT_READ_BUTTON_SINCE_LAST	0x31
#define BW_PORT_READ_BUTTON_1			0x40
#define BW_PORT_READ_BUTTON_2			0x41
#define BW_PORT_READ_BUTTON_3			0x42
#define BW_PORT_READ_BUTTON_4			0x43
#define BW_PORT_READ_BUTTON_5			0x44
#define BW_PORT_READ_BUTTON_6			0x45

ButtonsBw::ButtonsBw(): m_I2C(BW_UI_DEFAULT_SLAVE_ADDRESS) {
}

ButtonsBw::~ButtonsBw() {
}

bool ButtonsBw::Start() {
	return m_I2C.IsConnected();
}

bool ButtonsBw::IsAvailable() {
	const char cmd[] = { BW_PORT_READ_BUTTON_SINCE_LAST, static_cast<char>(0xFF) };


	Write(cmd, sizeof(cmd) / sizeof(cmd[0]));
	udelay(BW_UI_I2C_DELAY_WRITE_READ_US);

	m_nButtons = m_I2C.Read();

	return m_nButtons == 0 ? false : true;
}

int ButtonsBw::GetChar() {
	switch (m_nButtons) {
		case (1 << 0):
			return INPUT_KEY_ESC;
			break;
		case (1 << 1):
			return INPUT_KEY_ENTER;
			break;
		case (1 << 2):
			return INPUT_KEY_UP;
			break;
		case (1 << 3):
			return INPUT_KEY_DOWN;
			break;
		case (1 << 4):
			return INPUT_KEY_RIGHT;
			break;
		case (1 << 5):
			return INPUT_KEY_LEFT;
			break;
		default:
			break;
	}

	return INPUT_KEY_NOT_DEFINED;
}

void ButtonsBw::Write(const char *buffer, uint32_t size) {
	const uint32_t elapsed = micros() - m_nWriteMicros;

	if (elapsed < BW_UI_I2C_BYTE_WAIT_US) {
		udelay(BW_UI_I2C_BYTE_WAIT_US - elapsed);
	}

	m_I2C.Write(buffer, size);

	m_nWriteMicros = micros();
}

#endif
