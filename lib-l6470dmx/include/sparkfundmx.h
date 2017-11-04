/**
 * @file sparkfundmx.h
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

#ifndef SPARKFUNDMX_H_
#define SPARKFUNDMX_H_

#include <stdint.h>
#include <stdbool.h>

#include "lightset.h"
#include "motorparams.h"
#include "l6470dmxmodes.h"
#include "autodriver.h"

#define SPARKFUN_DMX_MAX_MOTORS	4

class SparkFunDmx: public LightSet {
public:
	SparkFunDmx(void);
	~SparkFunDmx(void);

	void Start(void);
	void Stop(void);

	void SetData(uint8_t, const uint8_t *, uint16_t);

public:
	void ReadConfigFiles(void);

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);

private:
	AutoDriver *m_pAutoDriver[SPARKFUN_DMX_MAX_MOTORS];
	MotorParams *m_pMotorParams[SPARKFUN_DMX_MAX_MOTORS];
	L6470DmxModes *m_pL6470DmxModes[SPARKFUN_DMX_MAX_MOTORS];

	uint8_t m_nPosition;
	uint8_t m_nSpiCs;
	uint8_t m_nResetPin;
	uint8_t m_nBusyPin;

	uint8_t m_nDmxMode;
	uint16_t m_nDmxStartAddress;

	bool is_position_set;
	bool is_spi_cs_set;
	bool is_reset_set;
	bool is_busy_pin_set;
};

#endif /* SPARKFUNDMX_H_ */
