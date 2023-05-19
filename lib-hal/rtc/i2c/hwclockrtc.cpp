/**
 * @file  hwclockrtc.cpp
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cassert>

#include "hwclock.h"
#include "hardware.h"
#include "hal_i2c.h"

#include "debug.h"

#define BCD2DEC(val)	( ((val) & 0x0f) + ((val) >> 4) * 10 )
#define DEC2BCD(val)	static_cast<char>( (((val) / 10) << 4) + (val) % 10 )

namespace rtc {
namespace reg {
static constexpr uint8_t SECONDS = 0x00;
static constexpr uint8_t MINUTES = 0x01;
static constexpr uint8_t HOURS = 0x02;
static constexpr uint8_t DAY = 0x03;
static constexpr uint8_t DATE = 0x04;
static constexpr uint8_t MONTH = 0x05;
static constexpr uint8_t YEAR = 0x06;
}  // namespace reg
namespace mcp7941x {
namespace bit {
static constexpr uint8_t ST = 0x80;
static constexpr uint8_t VBATEN = 0x08;
}  // namespace bit
}  // namespace mcp7941x
namespace pcf8563 {
namespace reg {
static constexpr uint8_t CONTROL_STATUS1 = 0x00;
static constexpr uint8_t CONTROL_STATUS2 = 0x01;
static constexpr uint8_t SECONDS = 0x02;
static constexpr uint8_t MINUTES = 0x03;
static constexpr uint8_t HOURS = 0x04;
static constexpr uint8_t DAY = 0x05;
static constexpr uint8_t WEEKDAY = 0x06;
static constexpr uint8_t MONTH = 0x07;
static constexpr uint8_t YEAR = 0x08;
}  // namespace reg
namespace bit {
static constexpr uint8_t SEC_VL	= (1U << 7);
}  // namespace bit
}  // namespace pcf8563
namespace i2caddress {
static constexpr uint8_t PCF8563 = 0x51;
static constexpr uint8_t MCP7941X = 0x6F;
static constexpr uint8_t DS3231 = 0x68;
}  // namespace i2caddress
}  // namespace rtc

using namespace rtc;

static void write_register(uint8_t nRegister, uint8_t nValue) {
	char buffer[2];

	buffer[0] = static_cast<char>(nRegister);
	buffer[1] = static_cast<char>(nValue);

	FUNC_PREFIX(i2c_write(buffer, 2));
}

void HwClock::RtcProbe() {
	DEBUG_ENTRY

	m_nLastHcToSysMillis = Hardware::Get()->Millis();

	FUNC_PREFIX(i2c_set_baudrate(hal::i2c::NORMAL_SPEED));

	char registers[1];

#if !defined (CONFIG_RTC_DISABLE_MCP7941X)
	/**
	 * MCP7941X
	 */

	FUNC_PREFIX(i2c_set_address(i2caddress::MCP7941X));

	registers[0] = reg::YEAR;

	// The I2C bus is not stable at cold start? These dummy write/read helps.
	// This needs some more investigation for what is really happening here.
	FUNC_PREFIX(i2c_write(registers, 1));
	FUNC_PREFIX(i2c_read(registers, sizeof(registers) / sizeof(registers[0])));

	if (FUNC_PREFIX(i2c_write(nullptr, 0)) == 0) {
		DEBUG_PUTS("MCP7941X");

		m_bIsConnected = true;
		m_Type = Type::MCP7941X;
		m_nAddress = i2caddress::MCP7941X;

		registers[0] = reg::SECONDS;

		FUNC_PREFIX(i2c_write(registers, 1));
		FUNC_PREFIX(i2c_read(registers, sizeof(registers) / sizeof(registers[0])));

		if ((registers[0] & mcp7941x::bit::ST) == 0) {
			DEBUG_PUTS("Start the on-board oscillator");

			struct rtc_time RtcTime;

			RtcTime.tm_hour = 0;
			RtcTime.tm_min = 0;
			RtcTime.tm_sec = 0;
			RtcTime.tm_mday = _TIME_STAMP_DAY_;
			RtcTime.tm_mon = _TIME_STAMP_MONTH_ - 1;
			RtcTime.tm_year = _TIME_STAMP_YEAR_ - 1900;

			RtcSet(&RtcTime);
		}

		DEBUG_EXIT
		return;
	}
#endif

#if !defined (CONFIG_RTC_DISABLE_DS3231)
	/**
	 * DS3231
	 */

	FUNC_PREFIX(i2c_set_address(i2caddress::DS3231));

	registers[0] = reg::YEAR;

	FUNC_PREFIX(i2c_write(registers, 1));
	FUNC_PREFIX(i2c_read(registers, sizeof(registers) / sizeof(registers[0])));

	if (FUNC_PREFIX(i2c_write(nullptr, 0)) == 0) {
		DEBUG_PUTS("DS3231");

		m_bIsConnected = true;
		m_Type = Type::DS3231;
		m_nAddress = i2caddress::DS3231;

		struct rtc_time tm;
		RtcGet(&tm);

		if ((tm.tm_hour > 24) || (tm.tm_year > _TIME_STAMP_YEAR_ - 1900)) {
			tm.tm_hour = 0;
			tm.tm_min = 0;
			tm.tm_sec = 0;
			tm.tm_mday = _TIME_STAMP_DAY_;
			tm.tm_mon = _TIME_STAMP_MONTH_ - 1;
			tm.tm_year = _TIME_STAMP_YEAR_ - 1900;

			RtcSet(&tm);
		}

		DEBUG_EXIT
		return;
	}
#endif

#if !defined (CONFIG_RTC_DISABLE_PCF8563)
	/**
	 * PCF8563
	 */

	FUNC_PREFIX(i2c_set_address(i2caddress::PCF8563));

	registers[0] = pcf8563::reg::YEAR;

	FUNC_PREFIX(i2c_write(registers, 1));
	FUNC_PREFIX(i2c_read(registers, sizeof(registers) / sizeof(registers[0])));

	if (FUNC_PREFIX(i2c_write(nullptr, 0)) == 0) {
		DEBUG_PUTS("PCF8563");

		m_bIsConnected = true;
		m_Type = Type::PCF8563;
		m_nAddress = i2caddress::PCF8563;

		write_register(pcf8563::reg::CONTROL_STATUS1, 0);
		write_register(pcf8563::reg::CONTROL_STATUS2, 0);

		registers[0] = pcf8563::reg::SECONDS;

		FUNC_PREFIX(i2c_write(registers, 1));
		FUNC_PREFIX(i2c_read(registers,1));

		if (registers[0] & pcf8563::bit::SEC_VL) {
			DEBUG_PUTS("Integrity of the clock information is not guaranteed");

			struct rtc_time RtcTime;

			RtcTime.tm_hour = 0;
			RtcTime.tm_min = 0;
			RtcTime.tm_sec = 0;
			RtcTime.tm_mday = _TIME_STAMP_DAY_;
			RtcTime.tm_mon = _TIME_STAMP_MONTH_ - 1;
			RtcTime.tm_year = _TIME_STAMP_YEAR_ - 1900;

			RtcSet(&RtcTime);
		}

		registers[0] = pcf8563::reg::SECONDS;

		FUNC_PREFIX(i2c_write(registers, 1));
		FUNC_PREFIX(i2c_read(registers,1));

		if (registers[0] & pcf8563::bit::SEC_VL) {
			DEBUG_PUTS("Clock is not running -> disconnected");
			m_bIsConnected = false;
		}

		DEBUG_EXIT
		return;
	}
#endif

	DEBUG_EXIT
}

bool HwClock::RtcSet(const struct rtc_time *pRtcTime) {
	assert(pRtcTime != nullptr);

	if (!m_bIsConnected) {
		return false;
	}

	char registers[7];

	registers[reg::SECONDS] = DEC2BCD(pRtcTime->tm_sec          & 0x7f);
	registers[reg::MINUTES] = DEC2BCD(pRtcTime->tm_min          & 0x7f);
	registers[reg::HOURS]   = DEC2BCD(pRtcTime->tm_hour         & 0x1f);
	registers[reg::DAY]     = DEC2BCD(pRtcTime->tm_wday         & 0x07);
	registers[reg::DATE]    = DEC2BCD(pRtcTime->tm_mday         & 0x3f);
	registers[reg::MONTH]   = DEC2BCD((pRtcTime->tm_mon + 1)    & 0x1f);
	registers[reg::YEAR]    = DEC2BCD((pRtcTime->tm_year - 100) & 0xff);

	if (m_Type == Type::MCP7941X) {
		registers[reg::SECONDS] |= mcp7941x::bit::ST;
		registers[reg::DAY] |=  mcp7941x::bit::VBATEN;
	}

	char data[8];

	if (m_Type == Type::PCF8563) {
		data[0] = pcf8563::reg::SECONDS;
	} else {
		data[0] = reg::SECONDS;
	}

	data[1] = registers[0];
	data[2] = registers[1];
	data[3] = registers[2];
	data[4] = registers[3];
	data[5] = registers[4];
	data[6] = registers[5];
	data[7] = registers[6];

	FUNC_PREFIX(i2c_set_address(m_nAddress));
	FUNC_PREFIX(i2c_set_baudrate(hal::i2c::FULL_SPEED));

	FUNC_PREFIX(i2c_write(data, sizeof(data) / sizeof(data[0])));

	return true;
}

bool HwClock::RtcGet(struct rtc_time *pRtcTime) {
	assert(pRtcTime != nullptr);

	if (!m_bIsConnected) {
		return false;
	}

	char registers[7];

	if (m_Type == Type::PCF8563) {
		registers[0] = pcf8563::reg::SECONDS;
	} else {
		registers[0] = reg::SECONDS;
	}

	FUNC_PREFIX(i2c_set_address(m_nAddress));
	FUNC_PREFIX(i2c_set_baudrate(hal::i2c::FULL_SPEED));

	FUNC_PREFIX(i2c_write(registers, 1));
	FUNC_PREFIX(i2c_read(registers, sizeof(registers) / sizeof(registers[0])));

	pRtcTime->tm_sec  = BCD2DEC(registers[reg::SECONDS] & 0x7f);
	pRtcTime->tm_min  = BCD2DEC(registers[reg::MINUTES] & 0x7f);
	pRtcTime->tm_hour = BCD2DEC(registers[reg::HOURS]   & 0x3f);
	pRtcTime->tm_wday = BCD2DEC(registers[reg::DAY]     & 0x07);
	pRtcTime->tm_mday = BCD2DEC(registers[reg::DATE]    & 0x3f);
	pRtcTime->tm_mon  = BCD2DEC(registers[reg::MONTH]   & 0x1f) - 1;
	pRtcTime->tm_year = BCD2DEC(registers[reg::YEAR]) + 100;

	return true;
}
