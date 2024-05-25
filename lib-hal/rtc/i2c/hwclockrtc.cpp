/**
 * @file  hwclockrtc.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <time.h>

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
static constexpr uint8_t HOURS 	 = 0x02;
static constexpr uint8_t DAY 	 = 0x03;
static constexpr uint8_t DATE    = 0x04;
static constexpr uint8_t MONTH   = 0x05;
static constexpr uint8_t YEAR    = 0x06;
}  // namespace reg
namespace mcp7941x {
namespace reg {
static constexpr uint8_t CONTROL = 0x07;
}  // namespace reg
namespace bit {
static constexpr uint8_t ST 			= 0x80;
static constexpr uint8_t VBATEN 		= 0x08;
static constexpr uint8_t ALM0_EN 		= 0x10;
static constexpr uint8_t ALMX_IF 		= (1U << 3);
static constexpr uint8_t ALMX_C0 		= (1U << 4);
static constexpr uint8_t ALMX_C1 		= (1U << 5);
static constexpr uint8_t ALMX_C2 		= (1U << 6);
#ifndef NDEBUG
static constexpr uint8_t ALMX_POL 		= (1U << 7);
#endif
static constexpr uint8_t MSK_ALMX_MATCH = (ALMX_C0 | ALMX_C1 | ALMX_C2);
}  // namespace bit
}  // namespace mcp7941x
namespace ds3231 {
namespace reg {
static constexpr uint8_t ALARM1_SECONDS = 0x07;
static constexpr uint8_t CONTROL 		= 0x0e;
//static constexpr uint8_t STATUS 		= 0x0f;
}  // namespace reg
namespace bit {
static constexpr uint8_t A1IE = (1U << 0);
static constexpr uint8_t A2IE = (1U << 1);
static constexpr uint8_t A1F  = (1U << 0);
static constexpr uint8_t A2F  = (1U << 1);
}  // namespace bit
}  // namespace ds3231
namespace pcf8563 {
namespace reg {
static constexpr uint8_t CONTROL_STATUS1 = 0x00;
static constexpr uint8_t CONTROL_STATUS2 = 0x01;
static constexpr uint8_t SECONDS 		 = 0x02;
//static constexpr uint8_t MINUTES 		 = 0x03;
//static constexpr uint8_t HOURS 		 = 0x04;
//static constexpr uint8_t DAY 			 = 0x05;
//static constexpr uint8_t WEEKDAY 		 = 0x06;
//static constexpr uint8_t MONTH 		 = 0x07;
static constexpr uint8_t YEAR 			 = 0x08;
static constexpr uint8_t ALARM 			 = 0x09;
}  // namespace reg
namespace bit {
static constexpr uint8_t SEC_VL = (1U << 7);
static constexpr uint8_t AIE	= (1U << 1);
static constexpr uint8_t AF 	= (1U << 3);
static constexpr uint8_t ST2_N 	= (7U << 5);
}  // namespace bit
}  // namespace pcf8563
namespace i2caddress {
static constexpr uint8_t PCF8563  = 0x51;
static constexpr uint8_t MCP7941X = 0x6F;
static constexpr uint8_t DS3231   = 0x68;
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

			struct tm RtcTime;

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
	FUNC_PREFIX(i2c_set_address(i2caddress::DS3231));

	registers[0] = reg::YEAR;

	FUNC_PREFIX(i2c_write(registers, 1));
	FUNC_PREFIX(i2c_read(registers, sizeof(registers) / sizeof(registers[0])));

	if (FUNC_PREFIX(i2c_write(nullptr, 0)) == 0) {
		DEBUG_PUTS("DS3231");

		m_bIsConnected = true;
		m_Type = Type::DS3231;
		m_nAddress = i2caddress::DS3231;

		struct tm tm;
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

			struct tm RtcTime;

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

bool HwClock::RtcSet(const struct tm *pTime) {
	assert(pTime != nullptr);

	if (!m_bIsConnected) {
		return false;
	}

	DEBUG_PRINTF("secs=%d, mins=%d, hours=%d, mday=%d, mon=%d, year=%d, wday=%d",
		pTime->tm_sec,
		pTime->tm_min,
		pTime->tm_hour,
		pTime->tm_mday,
		pTime->tm_mon,
		pTime->tm_year,
		pTime->tm_wday);

	char data[8];
	auto registers = &data[1];

	registers[reg::SECONDS] = DEC2BCD(pTime->tm_sec          & 0x7f);
	registers[reg::MINUTES] = DEC2BCD(pTime->tm_min          & 0x7f);
	registers[reg::HOURS]   = DEC2BCD(pTime->tm_hour         & 0x1f);
	registers[reg::DAY]     = DEC2BCD(pTime->tm_wday         & 0x07);
	registers[reg::DATE]    = DEC2BCD(pTime->tm_mday         & 0x3f);
	registers[reg::MONTH]   = DEC2BCD((pTime->tm_mon + 1)    & 0x1f);
	registers[reg::YEAR]    = DEC2BCD((pTime->tm_year - 100) & 0xff);

	if (m_Type == Type::MCP7941X) {
		registers[reg::SECONDS] |= mcp7941x::bit::ST;
		registers[reg::DAY] |=  mcp7941x::bit::VBATEN;
	}

	if (m_Type == Type::PCF8563) {
		data[0] = pcf8563::reg::SECONDS;
	} else {
		data[0] = reg::SECONDS;
	}

	FUNC_PREFIX(i2c_set_address(m_nAddress));
	FUNC_PREFIX(i2c_set_baudrate(hal::i2c::FULL_SPEED));
	FUNC_PREFIX(i2c_write(data, sizeof(data) / sizeof(data[0])));

	return true;
}

bool HwClock::RtcGet(struct tm *pTime) {
	assert(pTime != nullptr);

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

	pTime->tm_sec  = BCD2DEC(registers[reg::SECONDS] & 0x7f);
	pTime->tm_min  = BCD2DEC(registers[reg::MINUTES] & 0x7f);
	pTime->tm_hour = BCD2DEC(registers[reg::HOURS]   & 0x3f);
	pTime->tm_wday = BCD2DEC(registers[reg::DAY]     & 0x07);
	pTime->tm_mday = BCD2DEC(registers[reg::DATE]    & 0x3f);
	pTime->tm_mon  = BCD2DEC(registers[reg::MONTH]   & 0x1f) - 1;
	pTime->tm_year = BCD2DEC(registers[reg::YEAR]) + 100;

	DEBUG_PRINTF("secs=%d, mins=%d, hours=%d, mday=%d, mon=%d, year=%d, wday=%d",
		pTime->tm_sec,
		pTime->tm_min,
		pTime->tm_hour,
		pTime->tm_mday,
		pTime->tm_mon,
		pTime->tm_year,
		pTime->tm_wday);

	return true;
}

bool HwClock::RtcSetAlarm(const struct tm *pTime) {
	DEBUG_ENTRY
	assert(pTime != nullptr);

	DEBUG_PRINTF("secs=%d, mins=%d, hours=%d, mday=%d, mon=%d, year=%d, wday=%d",
		pTime->tm_sec,
		pTime->tm_min,
		pTime->tm_hour,
		pTime->tm_mday,
		pTime->tm_mon,
		pTime->tm_year,
		pTime->tm_wday);

	switch (m_Type) {
#if !defined (CONFIG_RTC_DISABLE_MCP7941X)
	case Type::MCP7941X: {
		const auto wday = static_cast<char>(MCP794xxAlarmWeekday(const_cast<struct tm *>(pTime)));

		// Read control and alarm 0 registers.
		char registers[11];
		auto data = &registers[1];
		data[0] = mcp7941x::reg::CONTROL;

		FUNC_PREFIX(i2c_write(data, 1));
		FUNC_PREFIX(i2c_read(data, 10));

		// Set alarm 0, using 24-hour and day-of-month modes.
		data[3] = DEC2BCD(pTime->tm_sec);
		data[4] = DEC2BCD(pTime->tm_min);
		data[5] = DEC2BCD(pTime->tm_hour);
		data[6] = wday;
		data[7] = DEC2BCD(pTime->tm_mday);
		data[8] = DEC2BCD(pTime->tm_mon + 1);
		// Clear the alarm 0 interrupt flag.
		data[6] &= static_cast<char>(~mcp7941x::bit::ALMX_IF);
		// Set alarm match: second, minute, hour, day, date, month.
		data[6] |= mcp7941x::bit::MSK_ALMX_MATCH;
		// Disable interrupt. We will not enable until completely programmed.
		data[0] &= static_cast<char>(~mcp7941x::bit::ALM0_EN);

		registers[0] = mcp7941x::reg::CONTROL;
		FUNC_PREFIX(i2c_write(registers, sizeof(registers) / sizeof(registers[0])));

		if (m_bRtcAlarmEnabled) {
			registers[0] = mcp7941x::reg::CONTROL;
			registers[1] |= mcp7941x::bit::ALM0_EN;
			FUNC_PREFIX(i2c_write(registers, 2));
		}

		DEBUG_EXIT
		return true;
	}
	break;
#endif
#if !defined (CONFIG_RTC_DISABLE_DS3231)
	case Type::DS3231: {
		char registers[10];
		auto data = &registers[1];
		data[0] = ds3231::reg::ALARM1_SECONDS;

		FUNC_PREFIX(i2c_set_address(m_nAddress));
		FUNC_PREFIX(i2c_set_baudrate(hal::i2c::FULL_SPEED));
		FUNC_PREFIX(i2c_write(data, 1));
		FUNC_PREFIX(i2c_read(data, 9));

		const auto control = data[7];
		const auto status = data[8];

		// set ALARM1, using 24 hour and day-of-month modes
		data[0] = DEC2BCD(pTime->tm_sec);
		data[1] = DEC2BCD(pTime->tm_min);
		data[2] = DEC2BCD(pTime->tm_hour);
		data[3] = DEC2BCD(pTime->tm_mday);
		// set ALARM2 to non-garbage
		data[4] = 0;
		data[5] = 0;
		data[6] = 0;
		// disable alarms
		data[7] = control & static_cast<char>(~(ds3231::bit::A1IE | ds3231::bit::A2IE));
		data[8] = status & static_cast<char>(~(ds3231::bit::A1F | ds3231::bit::A2F));

		registers[0] = ds3231::reg::ALARM1_SECONDS;
		FUNC_PREFIX(i2c_write(registers, sizeof(registers) / sizeof(registers[0])));

		if (m_bRtcAlarmEnabled) {
			registers[0] = ds3231::reg::CONTROL;
			registers[1] |= ds3231::bit::A1IE;
			FUNC_PREFIX(i2c_write(registers, 2));
		}

		DEBUG_EXIT
		return true;
	}
		break;
#endif
#if !defined (CONFIG_RTC_DISABLE_PCF8563)
	case Type::PCF8563: {
		char data[5];

		data[0] = pcf8563::reg::ALARM;
		data[1] = DEC2BCD(pTime->tm_min);
		data[2] = DEC2BCD(pTime->tm_hour);
		data[3] = DEC2BCD(pTime->tm_mday);
		data[4] = pTime->tm_wday & 0x07;

		FUNC_PREFIX(i2c_set_address(m_nAddress));
		FUNC_PREFIX(i2c_set_baudrate(hal::i2c::FULL_SPEED));
		FUNC_PREFIX(i2c_write(data, sizeof(data) / sizeof(data[0])));

		PCF8563SetAlarmMode();

		DEBUG_EXIT
		return true;
	}
	break;
#endif
	default:
		break;
	}

	DEBUG_EXIT
	return false;
}

bool HwClock::RtcGetAlarm(struct tm *pTime) {
	DEBUG_ENTRY
	assert(pTime != nullptr);

	if (!RtcGet(pTime)) {
		DEBUG_EXIT
		return false;
	}

	switch (m_Type) {
#if !defined (CONFIG_RTC_DISABLE_MCP7941X)
	case Type::MCP7941X: {
		char registers[10];

		registers[0] = mcp7941x::reg::CONTROL;

		FUNC_PREFIX(i2c_set_address(m_nAddress));
		FUNC_PREFIX(i2c_set_baudrate(hal::i2c::FULL_SPEED));
		FUNC_PREFIX(i2c_write(registers, 1));
		FUNC_PREFIX(i2c_read(registers, sizeof(registers) / sizeof(registers[0])));

		pTime->tm_sec = BCD2DEC(registers[3] & 0x7f);
		pTime->tm_min = BCD2DEC(registers[4] & 0x7f);
		pTime->tm_hour = BCD2DEC(registers[5] & 0x3f);
		pTime->tm_wday = BCD2DEC(registers[6] & 0x7) - 1;
		pTime->tm_mday = BCD2DEC(registers[7] & 0x3f);
		pTime->tm_mon = BCD2DEC(registers[8] & 0x1f) - 1;

		m_bRtcAlarmEnabled = registers[0] & mcp7941x::bit::ALM0_EN;

		DEBUG_PRINTF("sec=%d min=%d hour=%d wday=%d mday=%d mon=%d enabled=%d polarity=%d irq=%d match=%u",
				pTime->tm_sec,
				pTime->tm_min,
				pTime->tm_hour,
				pTime->tm_wday,
				pTime->tm_mday,
				pTime->tm_mon,
				m_bRtcAlarmEnabled,
				(registers[6] & mcp7941x::bit::ALMX_POL),
				(registers[6] & mcp7941x::bit::ALMX_IF),
				(registers[6] & mcp7941x::bit::MSK_ALMX_MATCH) >> 4);

		DEBUG_EXIT
		return true;
	}
	break;
#endif
#if !defined (CONFIG_RTC_DISABLE_DS3231)
	case Type::DS3231: {
		char registers[10];

		registers[0] = ds3231::reg::ALARM1_SECONDS;

		FUNC_PREFIX(i2c_set_address(m_nAddress));
		FUNC_PREFIX(i2c_set_baudrate(hal::i2c::FULL_SPEED));
		FUNC_PREFIX(i2c_write(registers, 1));
		FUNC_PREFIX(i2c_read(registers, sizeof(registers) / sizeof(registers[0])));

		pTime->tm_sec = BCD2DEC(registers[0] & 0x7f);
		pTime->tm_min = BCD2DEC(registers[1] & 0x7f);
		pTime->tm_hour = BCD2DEC(registers[2] & 0x3f);
		pTime->tm_mday = BCD2DEC(registers[3] & 0x3f);

		m_bRtcAlarmEnabled = (registers[7] & ds3231::bit::A1IE);
		m_bRtcAlarmPending = (registers[8] &ds3231::bit::A1F);

		DEBUG_PRINTF("tm is sec=%d, min=%d, hour=%d, mday=%d, enabled=%d, pending=%d",
				pTime->tm_sec,
				pTime->tm_min,
				pTime->tm_hour,
				pTime->tm_mday,
				m_bRtcAlarmEnabled,
				m_bRtcAlarmPending);

		DEBUG_EXIT
		return true;
	}

		break;
#endif
#if !defined (CONFIG_RTC_DISABLE_PCF8563)
	case Type::PCF8563: {
		char registers[4];

		registers[0] = pcf8563::reg::ALARM;

		FUNC_PREFIX(i2c_set_address(m_nAddress));
		FUNC_PREFIX(i2c_set_baudrate(hal::i2c::FULL_SPEED));
		FUNC_PREFIX(i2c_write(registers, 1));
		FUNC_PREFIX(i2c_read(registers, sizeof(registers) / sizeof(registers[0])));

		DEBUG_PRINTF("raw data is min=%02x, hr=%02x, mday=%02x, wday=%02x",
				registers[0],
				registers[1],
				registers[2],
				registers[3]);

		pTime->tm_sec = 0;
		pTime->tm_min = BCD2DEC(registers[0] & 0x7F);
		pTime->tm_hour = BCD2DEC(registers[1] & 0x3F);
		pTime->tm_mday = BCD2DEC(registers[2] & 0x3F);
		pTime->tm_wday = BCD2DEC(registers[3] & 0x7);

		PCF8563GetAlarmMode();

		DEBUG_PRINTF("tm is mins=%d, hours=%d, mday=%d, wday=%d, enabled=%d, pending=%d",
				pTime->tm_min,
				pTime->tm_hour,
				pTime->tm_mday,
				pTime->tm_wday,
				m_bRtcAlarmEnabled,
				m_bRtcAlarmPending);

		DEBUG_EXIT
		return true;
	}
	break;
#endif
	default:
		break;
	}

	DEBUG_EXIT
	return false;
}

int HwClock::MCP794xxAlarmWeekday(struct tm *pTime) {
	DEBUG_ENTRY
	assert(pTime != nullptr);
	assert(m_Type == rtc::Type::MCP7941X);

	struct tm tm_now;
	RtcGet(&tm_now);

	const auto days_now = mktime(&tm_now) / (24 * 60 * 60);
	const auto days_alarm = mktime(pTime) / (24 * 60 * 60);
	const auto i =  (tm_now.tm_wday + days_alarm - days_now) % 7 + 1;

	DEBUG_EXIT
	return i;
}

void HwClock::PCF8563GetAlarmMode() {
	assert(m_Type == rtc::Type::PCF8563);

	char data[1];

	data[0] = pcf8563::reg::CONTROL_STATUS2;

	FUNC_PREFIX(i2c_write(data, 1));
	FUNC_PREFIX(i2c_read(data, sizeof(data) / sizeof(data[0])));

	m_bRtcAlarmEnabled = data[0] & pcf8563::bit::AIE;
	m_bRtcAlarmPending = data[0] & pcf8563::bit::AF;
}

void HwClock::PCF8563SetAlarmMode() {
	assert(m_Type == rtc::Type::PCF8563);

	char data[2];

	data[1] = pcf8563::reg::CONTROL_STATUS2;

	FUNC_PREFIX(i2c_write(&data[1], 1));
	FUNC_PREFIX(i2c_read(&data[1], 1));

	if (m_bRtcAlarmEnabled) {
		data[1] |= pcf8563::bit::AIE;
	} else {
		data[1] &= static_cast<char>(~pcf8563::bit::AIE);
	}

	data[1] &= static_cast<char>(~pcf8563::bit::AF | pcf8563::bit::ST2_N);

	data[0] = pcf8563::reg::CONTROL_STATUS2;

	FUNC_PREFIX(i2c_write(data, 2));
}
