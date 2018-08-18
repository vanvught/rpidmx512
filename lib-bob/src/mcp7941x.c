#if defined(HAVE_I2C)
/**
 * @file mcp7941x.c
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdlib.h>
#include <stdint.h>

#include "i2c.h"
#include "mcp7941x.h"

/** Time and Configuration Registers (TCR) **/
#define MCP7941X_RTCC_TCR_SECONDS				0x00
#define MCP7941X_RTCC_TCR_MINUTES				0x01
#define MCP7941X_RTCC_TCR_HOURS					0x02
#define MCP7941X_RTCC_TCR_DAY					0x03
#define MCP7941X_RTCC_TCR_DATE					0x04
#define MCP7941X_RTCC_TCR_MONTH					0x05
#define MCP7941X_RTCC_TCR_YEAR					0x06

#define MCP7941X_RTCC_BIT_ST					0x80
#define MCP7941X_RTCC_BIT_VBATEN				0x08

static uint8_t i2c_mcp7941x_slave_address __attribute__((aligned(4))) = (uint8_t)MCP7941X_DEFAULT_SLAVE_ADDRESS ;

#define BCD2DEC(val)	( ((val) & 0x0f) + ((val) >> 4) * 10 )
#define DEC2BCD(val)	( (((val) / 10) << 4) + (val) % 10 )

void inline static mcp7941x_setup(void) {
	i2c_set_address(i2c_mcp7941x_slave_address);
	i2c_set_baudrate(I2C_NORMAL_SPEED);
}

uint8_t mcp7941x_start(uint8_t slave_address) {

	i2c_begin();

	if (slave_address == (uint8_t) 0) {
		i2c_mcp7941x_slave_address = (uint8_t)MCP7941X_DEFAULT_SLAVE_ADDRESS;
	} else {
		i2c_mcp7941x_slave_address = slave_address;
	}

	mcp7941x_setup();

	if (!i2c_is_connected(i2c_mcp7941x_slave_address)) {
		return MCP7941X_ERROR;
	}

	return MCP7941X_OK;
}

void mcp7941x_get_date_time(struct rtc_time *t) {
	char reg[] = { (char) 0, (char) 0, (char) 0, (char) 0, (char) 0, (char) 0,	(char) 0 };

	mcp7941x_setup();

	(void) i2c_write(MCP7941X_RTCC_TCR_SECONDS);
	(void) i2c_read(reg, sizeof(reg)/sizeof(char));

	t->tm_sec = BCD2DEC((int)reg[MCP7941X_RTCC_TCR_SECONDS] & 0x7f);
	t->tm_min = BCD2DEC((int)reg[MCP7941X_RTCC_TCR_MINUTES] & 0x7f);
	t->tm_hour = BCD2DEC((int)reg[MCP7941X_RTCC_TCR_HOURS] & 0x3f);
	t->tm_wday = BCD2DEC((int)reg[MCP7941X_RTCC_TCR_DAY] & 0x07);
	t->tm_mday = BCD2DEC((int)reg[MCP7941X_RTCC_TCR_DATE] & 0x3f);
	t->tm_mon = BCD2DEC((int)reg[MCP7941X_RTCC_TCR_MONTH] & 0x1f);
	t->tm_year = BCD2DEC((int)reg[MCP7941X_RTCC_TCR_YEAR]);
}

void mcp7941x_set_date_time(const struct rtc_time *t) {
	char reg[] = { (char) 0, (char) 0, (char) 0, (char) 0, (char) 0, (char) 0,	(char) 0 };
	char data[8];

	reg[MCP7941X_RTCC_TCR_SECONDS] = (char) DEC2BCD(t->tm_sec & 0x7f);
	reg[MCP7941X_RTCC_TCR_MINUTES] = (char) DEC2BCD(t->tm_min & 0x7f);
	reg[MCP7941X_RTCC_TCR_HOURS] = (char) DEC2BCD(t->tm_hour & 0x1f);
	reg[MCP7941X_RTCC_TCR_DAY] = (char) DEC2BCD(t->tm_wday & 0x07);
	reg[MCP7941X_RTCC_TCR_DATE] = (char) DEC2BCD(t->tm_mday & 0x3f);
	reg[MCP7941X_RTCC_TCR_MONTH] = (char) DEC2BCD(t->tm_mon & 0x1f);
	reg[MCP7941X_RTCC_TCR_YEAR] = (char) DEC2BCD(t->tm_year);

	reg[MCP7941X_RTCC_TCR_SECONDS] |= MCP7941X_RTCC_BIT_ST;
	reg[MCP7941X_RTCC_TCR_DAY] |= MCP7941X_RTCC_BIT_VBATEN;

	data[0] = (char) MCP7941X_RTCC_TCR_SECONDS;
	data[1] = reg[0];
	data[2] = reg[1];
	data[3] = reg[2];
	data[4] = reg[3];
	data[5] = reg[4];
	data[6] = reg[5];
	data[7] = reg[6];

	mcp7941x_setup();

	(void) i2c_write_nb(data, sizeof(data)/sizeof(data[0]));
}
#endif
