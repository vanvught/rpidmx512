/**
 * @file rtc.c
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stddef.h>
#include <time.h>

#if defined(H3)
# include "h3/hal_api.h"
# include "h3/hal_i2c.h"
#else
# include "rpi/hal_api.h"
# include "rpi/hal_i2c.h"
#endif

#include "rtc.h"

#include "debug.h"

#define RTC_REG_SECONDS					0x00
#define RTC_REG_MINUTES					0x01
#define RTC_REG_HOURS					0x02
#define RTC_REG_DAY						0x03
#define RTC_REG_DATE					0x04
#define RTC_REG_MONTH					0x05
#define RTC_REG_YEAR					0x06

/**
 * MCP7941X
 */
#define MCP7941X_DEFAULT_SLAVE_ADDRESS	0x6F
#define MCP7941X_RTCC_BIT_ST			0x80
#define MCP7941X_RTCC_BIT_VBATEN		0x08

/**
 * DS3231
 */
#define DS3231_DEFAULT_SLAVE_ADDRESS	0x68


/**
 * Defaults to MCP7941X
 */
static uint32_t s_i2c_address = MCP7941X_DEFAULT_SLAVE_ADDRESS;
static rtc_types_t s_type = RTC_MCP7941X;

#define BCD2DEC(val)	( ((val) & 0x0f) + ((val) >> 4) * 10 )
#define DEC2BCD(val)	( (((val) / 10) << 4) + (val) % 10 )

inline static void i2c_setup(void) {
	FUNC_PREFIX(i2c_set_address(s_i2c_address));
	FUNC_PREFIX(i2c_set_baudrate(400000));
}

inline static bool i2c_is_connected(uint8_t address) {
	uint8_t ret;
	char buf;

	FUNC_PREFIX(i2c_set_baudrate(100000));
	FUNC_PREFIX(i2c_set_address(address));

	if ((address >= 0x30 && address <= 0x37) || (address >= 0x50 && address <= 0x5F)) {
		ret = FUNC_PREFIX(i2c_read(&buf, 1));
	} else {
		/* This is known to corrupt the Atmel AT24RF08 EEPROM */
		ret = FUNC_PREFIX(i2c_write(NULL, 0));
	}

	return (ret == 0) ? true : false;
}

bool rtc_start(rtc_types_t type) {
	DEBUG_ENTRY

	switch (type) {
		case RTC_MCP7941X:
			/* default RTC */
			break;
		case RTC_DS3231:
			s_i2c_address = DS3231_DEFAULT_SLAVE_ADDRESS;
			s_type = RTC_DS3231;
			break;
		default:
			/* Probing is handled separately */
			break;
	}

	if (type == RTC_PROBE) {
		if (i2c_is_connected(MCP7941X_DEFAULT_SLAVE_ADDRESS)) {
			DEBUG_EXIT
			return true;
		} else if (i2c_is_connected(DS3231_DEFAULT_SLAVE_ADDRESS)) {
			s_i2c_address = DS3231_DEFAULT_SLAVE_ADDRESS;
			s_type = RTC_DS3231;
			DEBUG_EXIT
			return true;
		}

		DEBUG_EXIT
		return false;
	}

	if (i2c_is_connected(s_i2c_address)) {
		DEBUG_EXIT
		return true;
	}

	DEBUG_EXIT
	return false;
}

bool rtc_is_connected(void) {
	i2c_setup();
	return i2c_is_connected(s_i2c_address);
}


rtc_types_t rtc_get_type(void) {
	return s_type;
}

/*
 * Members of the tm structure:
 *
 * tm_wday The number of days since Sunday, in the range 0 to 6.
 * tm_mday The day of the month, in the range 1 to 31.
 * tm_mon  The number of months since January, in the range 0 to 11.
 * tm_year The number of years since 1900.
 */

/*
 * RTC
 *
 * Day 1-7
 * Date 01-31
 * Month 01-12
 * Year 00-99
 */

void rtc_get_date_time(struct tm *tm) {
	char reg[7];

	reg[0] = RTC_REG_SECONDS;

	i2c_setup();
	FUNC_PREFIX(i2c_write(reg, 1));
	FUNC_PREFIX(i2c_read(reg, sizeof(reg) / sizeof(reg[0])));

	tm->tm_sec = BCD2DEC(reg[RTC_REG_SECONDS] & 0x7f);
	tm->tm_min = BCD2DEC(reg[RTC_REG_MINUTES] & 0x7f);
	tm->tm_hour = BCD2DEC(reg[RTC_REG_HOURS] & 0x3f);
	tm->tm_wday = BCD2DEC(reg[RTC_REG_DAY] & 0x07);
	tm->tm_mday = BCD2DEC(reg[RTC_REG_DATE] & 0x3f);
	tm->tm_mon = BCD2DEC(reg[RTC_REG_MONTH] & 0x1f) - 1;
	tm->tm_year = BCD2DEC(reg[RTC_REG_YEAR]) + 100;
	tm->tm_isdst = 0;
}

void rtc_set_date_time(const struct tm *tm) {
	char reg[7];
	char data[8];

	reg[RTC_REG_SECONDS] = DEC2BCD(tm->tm_sec & 0x7f);
	reg[RTC_REG_MINUTES] = DEC2BCD(tm->tm_min & 0x7f);
	reg[RTC_REG_HOURS] = DEC2BCD(tm->tm_hour & 0x1f);
	reg[RTC_REG_DAY] = DEC2BCD(tm->tm_wday & 0x07);
	reg[RTC_REG_DATE] = DEC2BCD(tm->tm_mday & 0x3f);
	reg[RTC_REG_MONTH] = DEC2BCD((tm->tm_mon + 1) & 0x1f);
	reg[RTC_REG_YEAR] = DEC2BCD((tm->tm_year - 100)) ;

	if (s_type == RTC_MCP7941X) {
		reg[RTC_REG_SECONDS] |= MCP7941X_RTCC_BIT_ST;
		reg[RTC_REG_DAY] |= MCP7941X_RTCC_BIT_VBATEN;
	}

	data[0] = RTC_REG_SECONDS;
	data[1] = reg[0];
	data[2] = reg[1];
	data[3] = reg[2];
	data[4] = reg[3];
	data[5] = reg[4];
	data[6] = reg[5];
	data[7] = reg[6];

	i2c_setup();
	FUNC_PREFIX(i2c_write(data, sizeof(data) / sizeof(data[0])));
}
