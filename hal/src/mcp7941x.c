/**
 * @file mcp7941x.c
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#include <bcm2835.h>
#include <mcp7941x.h>

char i2c_mcp7941x_slave_address = MCP7941X_DEFAULT_SLAVE_ADDRESS;

#define BCD2DEC(val)	( ((val) & 0x0f) + ((val) >> 4) * 10 )
#define DEC2BCD(val)	( (((val) / 10) << 4) + (val) % 10 )

void inline static mcp7941x_setup(void) {
	bcm2835_i2c_setSlaveAddress(i2c_mcp7941x_slave_address);
	bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
}

int mcp7941x_start (char slave_address) {

	if (bcm2835_init() != 1)
		return MCP7941X_ERROR;

	bcm2835_i2c_begin();

	if (slave_address <= 0)
		i2c_mcp7941x_slave_address = MCP7941X_DEFAULT_SLAVE_ADDRESS;
	else
		i2c_mcp7941x_slave_address = slave_address;

	return MCP7941X_OK;
}

void mcp7941x_end (void) {
	bcm2835_i2c_end();
	bcm2835_close();
}

void mcp7941x_get_date_time(struct rtc_time *t) {
	static char cmd[]  = {MCP7941X_RTCC_TCR_SECONDS};
	char reg[] =  {0,0,0,0,0,0,0};

	mcp7941x_setup();

	bcm2835_i2c_write(cmd, sizeof(cmd)/sizeof(char));
	bcm2835_i2c_read(reg, sizeof(reg)/sizeof(char));

	t->tm_sec  = BCD2DEC(reg[MCP7941X_RTCC_TCR_SECONDS] & 0x7f);
	t->tm_min  = BCD2DEC(reg[MCP7941X_RTCC_TCR_MINUTES] & 0x7f);
	t->tm_hour = BCD2DEC(reg[MCP7941X_RTCC_TCR_HOURS] & 0x3f);
	t->tm_wday = BCD2DEC(reg[MCP7941X_RTCC_TCR_DAY] & 0x07);
	t->tm_mday = BCD2DEC(reg[MCP7941X_RTCC_TCR_DATE] & 0x3f);
	t->tm_mon  = BCD2DEC(reg[MCP7941X_RTCC_TCR_MONTH] & 0x1f);
	t->tm_year = BCD2DEC(reg[MCP7941X_RTCC_TCR_YEAR]);
}

void mcp7941x_set_date_time(struct rtc_time *t) {
	static char cmd[]  = {MCP7941X_RTCC_TCR_SECONDS};
	char reg[] =  {0,0,0,0,0,0,0};

	reg[MCP7941X_RTCC_TCR_SECONDS] = DEC2BCD(t->tm_sec & 0x7f);
	reg[MCP7941X_RTCC_TCR_MINUTES] = DEC2BCD(t->tm_min & 0x7f);
	reg[MCP7941X_RTCC_TCR_HOURS]   = DEC2BCD(t->tm_hour & 0x1f);
	reg[MCP7941X_RTCC_TCR_DAY]     = DEC2BCD(t->tm_wday & 0x07);
	reg[MCP7941X_RTCC_TCR_DATE]    = DEC2BCD(t->tm_mday & 0x3f);
	reg[MCP7941X_RTCC_TCR_MONTH]   = DEC2BCD(t->tm_mon & 0x1f);
	reg[MCP7941X_RTCC_TCR_YEAR]    = DEC2BCD(t->tm_year);

	reg[MCP7941X_RTCC_TCR_SECONDS] |=  MCP7941X_RTCC_BIT_ST;
	reg[MCP7941X_RTCC_TCR_DAY] |= MCP7941X_RTCC_BIT_VBATEN;

	char data[8];
	data[0] = cmd[1];
	data[1] = reg[0];
	data[2] = reg[1];
	data[3] = reg[2];
	data[4] = reg[3];
	data[5] = reg[4];
	data[6] = reg[5];
	data[7] = reg[6];
	
	mcp7941x_setup();
	
	bcm2835_i2c_write(data, sizeof(data)/sizeof(char));
}
