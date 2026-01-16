/**
 * @file  hwclockrtc.cpp
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

/**
 * MCP7941X: It has specific control bits like the Start Timer (ST) and battery enable (VBATEN).
 */

/**
 * DS3231: Known for its accuracy.
 */

/**
 * PCF8563: The time integrity is checked using the SECONDS_VL bit.
 *  Alarm: Minute, Hour, Day, Weekday
 */

#include <cassert>
#include <time.h>

#include "hwclock.h"
#include "hal.h"
#include "hal_i2c.h"

 #include "firmware/debug/debug_debug.h"

#define BCD2DEC(val)	( ((val) & 0x0f) + ((val) >> 4) * 10 )
#define DEC2BCD(val)	static_cast<char>( (((val) / 10) << 4) + (val) % 10 )

namespace rtc {
namespace reg {
static constexpr uint8_t SECONDS			= 0x00;
static constexpr uint8_t kMinutes			= 0x01;
static constexpr uint8_t HOURS				= 0x02;
static constexpr uint8_t kWday				= 0x03;
static constexpr uint8_t kMday				= 0x04;
static constexpr uint8_t kMonth				= 0x05;
static constexpr uint8_t kYear				= 0x06;
}  // namespace reg
namespace mcp7941x {
namespace reg {
static constexpr uint8_t CONTROL			= 0x07;
}  // namespace reg
namespace bit {
static constexpr uint8_t kSt 				= 0x80;
static constexpr uint8_t kVbaten 			= 0x08;
static constexpr uint8_t kAlM0En 			= 0x10;
static constexpr uint8_t kAlmxIf 			= (1U << 3);
static constexpr uint8_t kAlmxC0 			= (1U << 4);
static constexpr uint8_t kAlmxC1 			= (1U << 5);
static constexpr uint8_t kAlmxC2 			= (1U << 6);
#ifndef NDEBUG
static constexpr uint8_t kAlmxPol			= (1U << 7);
#endif
static constexpr uint8_t kMskAlmxMatch		= (kAlmxC0 | kAlmxC1 | kAlmxC2);
}  // namespace bit
}  // namespace mcp7941x
namespace ds3231 {
namespace reg {
static constexpr uint8_t kAlarM1Seconds		= 0x07;
static constexpr uint8_t kControl 			= 0x0e;
}  // namespace reg
namespace bit {
static constexpr uint8_t kA1Ie				= (1U << 0);
static constexpr uint8_t kA2Ie				= (1U << 1);
static constexpr uint8_t kA1F				= (1U << 0);
static constexpr uint8_t kA2F				= (1U << 1);
}  // namespace bit
}  // namespace ds3231
namespace pcf8563 {
namespace reg {
static constexpr uint8_t kControlStatuS1	= 0x00;
static constexpr uint8_t kControlStatuS2	= 0x01;
static constexpr uint8_t kSeconds			= 0x02;
//static constexpr uint8_t MINUTES			= 0x03;
//static constexpr uint8_t HOURS			= 0x04;
static constexpr uint8_t kMday				= 0x05;
static constexpr uint8_t kWday				= 0x06;
//static constexpr uint8_t MONTH			= 0x07;
static constexpr uint8_t kYear				= 0x08;
static constexpr uint8_t kAlarm				= 0x09;
}  // namespace reg
namespace bit {
static constexpr uint8_t kSecondsVl			= (1U << 7);
static constexpr uint8_t kStatuS2Aie		= (1U << 1);	///< alarm interrupt enabled
static constexpr uint8_t kStatuS2Af			= (1U << 3);	///< read: alarm flag active
}  // namespace bit
}  // namespace pcf8563
namespace i2caddress {
static constexpr uint8_t kPcF8563			= 0x51;
static constexpr uint8_t kMcP7941X			= 0x6F;
static constexpr uint8_t kDS3231				= 0x68;
}  // namespace i2caddress
}  // namespace rtc

using namespace rtc;

void HwClock::RtcProbe() {
	DEBUG_ENTRY();

	last_hc_to_sys_millis_ =hal::Millis();

	FUNC_PREFIX(I2cSetBaudrate(HAL_I2C::NORMAL_SPEED));

	uint8_t nValue;

#if !defined (CONFIG_RTC_DISABLE_MCP7941X)
	FUNC_PREFIX(I2cSetAddress(i2caddress::kMcP7941X));

	// The I2C bus is not stable at cold start? These dummy write/read helps.
	// This needs some more investigation for what is really happening here.
	FUNC_PREFIX(I2cReadReg(reg::kYear, nValue));

	if (FUNC_PREFIX(I2cWrite(nullptr, 0)) == 0) {
		DEBUG_PUTS("MCP7941X");

		is_connected_ = true;
		type_ = Type::kMcP7941X;
		address_ = i2caddress::kMcP7941X;

		FUNC_PREFIX(I2cReadReg(reg::SECONDS, nValue));

		if ((nValue & mcp7941x::bit::kSt) == 0) {
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

		DEBUG_EXIT();
		return;
	}
#endif

#if !defined (CONFIG_RTC_DISABLE_DS3231)
	FUNC_PREFIX(I2cSetAddress(i2caddress::kDS3231));

	// The I2C bus is not stable at cold start? These dummy write/read helps.
	// This needs some more investigation for what is really happening here.
	FUNC_PREFIX(I2cReadReg(reg::kYear, nValue));

	if (FUNC_PREFIX(I2cWrite(nullptr, 0)) == 0) {
		DEBUG_PUTS("DS3231");

		is_connected_ = true;
		type_ = Type::kDS3231;
		address_ = i2caddress::kDS3231;

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

		DEBUG_EXIT();
		return;
	}
#endif

#if !defined (CONFIG_RTC_DISABLE_PCF8563)
	FUNC_PREFIX(I2cSetAddress(i2caddress::kPcF8563));

	// The I2C bus is not stable at cold start? These dummy write/read helps.
	// This needs some more investigation for what is really happening here.
	FUNC_PREFIX(I2cReadReg(pcf8563::reg::kYear, nValue));

	if (FUNC_PREFIX(I2cWrite(nullptr, 0)) == 0) {
		DEBUG_PUTS("PCF8563");

		is_connected_ = true;
		type_ = Type::PCF8563;
		address_ = i2caddress::kPcF8563;

		FUNC_PREFIX(I2cWriteReg(pcf8563::reg::kControlStatuS1, 0));
		FUNC_PREFIX(I2cWriteReg(pcf8563::reg::kControlStatuS2, 0));

		FUNC_PREFIX(I2cReadReg(pcf8563::reg::kSeconds, nValue));

		// Register seconds has the VL bit
		// 0 - clock integrity is guaranteed
		// 1 - integrity of the clock information is not guaranteed
		if ((nValue & pcf8563::bit::kSecondsVl) == pcf8563::bit::kSecondsVl) {
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

		FUNC_PREFIX(I2cReadReg(pcf8563::reg::kSeconds, nValue));

		if ((nValue & pcf8563::bit::kSecondsVl) == pcf8563::bit::kSecondsVl) {
			DEBUG_PUTS("Clock is not running -> disconnected");
			is_connected_ = false;
		}

		DEBUG_EXIT();
		return;
	}
#endif

	DEBUG_EXIT();
}

bool HwClock::RtcSet(const struct tm *pTime) {
	DEBUG_ENTRY();
	assert(pTime != nullptr);

	if (!is_connected_) {
		DEBUG_EXIT();
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

	registers[reg::SECONDS] = DEC2BCD(pTime->tm_sec  & 0x7f);
	registers[reg::kMinutes] = DEC2BCD(pTime->tm_min  & 0x7f);
	registers[reg::HOURS]   = DEC2BCD(pTime->tm_hour & 0x1f);

#if !defined (CONFIG_RTC_DISABLE_PCF8563)
	if (type_ == Type::PCF8563) {
		registers[pcf8563::reg::kWday - pcf8563::reg::kSeconds] = DEC2BCD(pTime->tm_wday & 0x07);
		registers[pcf8563::reg::kMday - pcf8563::reg::kSeconds] = DEC2BCD(pTime->tm_mday & 0x3f);
	} else
#endif
	{
		registers[reg::kWday]    = DEC2BCD(pTime->tm_wday         & 0x07);
		registers[reg::kMday]    = DEC2BCD(pTime->tm_mday         & 0x3f);
	}

	registers[reg::kMonth]   = DEC2BCD((pTime->tm_mon + 1)    & 0x1f);
	registers[reg::kYear]    = DEC2BCD((pTime->tm_year - 100) & 0xff);

	if (type_ == Type::kMcP7941X) {
		registers[reg::SECONDS] |= mcp7941x::bit::kSt;
		registers[reg::kWday] |=  mcp7941x::bit::kVbaten;
	}

	if (type_ == Type::PCF8563) {
		data[0] = pcf8563::reg::kSeconds;
	} else {
		data[0] = reg::SECONDS;
	}

	FUNC_PREFIX(I2cSetAddress(address_));
	FUNC_PREFIX(I2cSetBaudrate(HAL_I2C::FULL_SPEED));
	FUNC_PREFIX(I2cWrite(data, sizeof(data) / sizeof(data[0])));

	DEBUG_EXIT();
	return true;
}

bool HwClock::RtcGet(struct tm *pTime) {
	DEBUG_ENTRY();
	assert(pTime != nullptr);

	if (!is_connected_) {
		DEBUG_EXIT();
		return false;
	}

	char registers[7];

	if (type_ == Type::PCF8563) {
		registers[0] = pcf8563::reg::kSeconds;
	} else {
		registers[0] = reg::SECONDS;
	}

	FUNC_PREFIX(I2cSetAddress(address_));
	FUNC_PREFIX(I2cSetBaudrate(HAL_I2C::FULL_SPEED));
	FUNC_PREFIX(I2cWrite(registers, 1));
	FUNC_PREFIX(I2cRead(registers, sizeof(registers) / sizeof(registers[0])));

	pTime->tm_sec  = BCD2DEC(registers[reg::kSeconds] & 0x7f);
	pTime->tm_min  = BCD2DEC(registers[reg::kMinutes] & 0x7f);
	pTime->tm_hour = BCD2DEC(registers[reg::kHours]   & 0x3f);

#if !defined (CONFIG_RTC_DISABLE_PCF8563)
	if (type_ == Type::PCF8563) {
		pTime->tm_wday = BCD2DEC(registers[pcf8563::reg::kWday - pcf8563::reg::kSeconds] & 0x07);
		pTime->tm_mday = BCD2DEC(registers[pcf8563::reg::kMday - pcf8563::reg::kSeconds] & 0x3f);
	} else
#endif
	{
		pTime->tm_wday = BCD2DEC(registers[reg::kWday] & 0x07);
		pTime->tm_mday = BCD2DEC(registers[reg::kMday] & 0x3f);
	}

	pTime->tm_mon  = BCD2DEC(registers[reg::kMonth]   & 0x1f) - 1;
	pTime->tm_year = BCD2DEC(registers[reg::kYear]) + 100;

	DEBUG_PRINTF("secs=%d, mins=%d, hours=%d, mday=%d, mon=%d, year=%d, wday=%d",
		pTime->tm_sec,
		pTime->tm_min,
		pTime->tm_hour,
		pTime->tm_mday,
		pTime->tm_mon,
		pTime->tm_year,
		pTime->tm_wday);

	DEBUG_EXIT();
	return true;
}

bool HwClock::RtcSetAlarm(const struct tm *pTime) {
	DEBUG_ENTRY();
	assert(pTime != nullptr);

	DEBUG_PRINTF("secs=%d, mins=%d, hours=%d, mday=%d, mon=%d, year=%d, wday=%d",
		pTime->tm_sec,
		pTime->tm_min,
		pTime->tm_hour,
		pTime->tm_mday,
		pTime->tm_mon,
		pTime->tm_year,
		pTime->tm_wday);

	switch (type_) {
#if !defined (CONFIG_RTC_DISABLE_MCP7941X)
	case Type::kMcP7941X: {
		const auto wday = static_cast<char>(MCP794xxAlarmWeekday(const_cast<struct tm *>(pTime)));

		// Read control and alarm 0 registers.
		char registers[11];
		auto data = &registers[1];
		data[0] = mcp7941x::reg::CONTROL;

		FUNC_PREFIX(I2cWrite(data, 1));
		FUNC_PREFIX(I2cRead(data, 10));

		// Set alarm 0, using 24-hour and day-of-month modes.
		data[3] = DEC2BCD(pTime->tm_sec);
		data[4] = DEC2BCD(pTime->tm_min);
		data[5] = DEC2BCD(pTime->tm_hour);
		data[6] = wday;
		data[7] = DEC2BCD(pTime->tm_mday);
		data[8] = DEC2BCD(pTime->tm_mon + 1);
		// Clear the alarm 0 interrupt flag.
		data[6] &= static_cast<char>(~mcp7941x::bit::kAlmxIf);
		// Set alarm match: second, minute, hour, day, date, month.
		data[6] |= mcp7941x::bit::kMskAlmxMatch;
		// Disable interrupt. We will not enable until completely programmed.
		data[0] &= static_cast<char>(~mcp7941x::bit::kAlM0En);

		registers[0] = mcp7941x::reg::kControl;
		FUNC_PREFIX(I2cWrite(registers, sizeof(registers) / sizeof(registers[0])));

		if (alarm_enabled_) {
			registers[0] = mcp7941x::reg::kControl;
			registers[1] |= mcp7941x::bit::kAlM0En;
			FUNC_PREFIX(I2cWrite(registers, 2));
		}

		DEBUG_EXIT();
		return true;
	}
	break;
#endif
#if !defined (CONFIG_RTC_DISABLE_DS3231)
	case Type::kDS3231: {
		char registers[10];
		auto data = &registers[1];
		data[0] = ds3231::reg::kAlarM1Seconds;

		FUNC_PREFIX(I2cSetAddress(address_));
		FUNC_PREFIX(I2cSetBaudrate(HAL_I2C::FULL_SPEED));
		FUNC_PREFIX(I2cWrite(data, 1));
		FUNC_PREFIX(I2cRead(data, 9));

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
		data[7] = control & static_cast<char>(~(ds3231::bit::kA1Ie | ds3231::bit::kA2Ie));
		data[8] = status & static_cast<char>(~(ds3231::bit::kA1F | ds3231::bit::kA2F));

		registers[0] = ds3231::reg::kAlarM1Seconds;
		FUNC_PREFIX(I2cWrite(registers, sizeof(registers) / sizeof(registers[0])));

		if (alarm_enabled_) {
			DEBUG_PUTS("Alarm is enabled");
			registers[0] = ds3231::reg::kControl;
			registers[1] |= ds3231::bit::kA1Ie;
			FUNC_PREFIX(I2cWrite(registers, 2));
		}

		DEBUG_EXIT();
		return true;
	}
		break;
#endif
#if !defined (CONFIG_RTC_DISABLE_PCF8563)
	case Type::kPcF8563: {
		char data[5];

		data[0] = pcf8563::reg::kAlarm;
		data[1] = DEC2BCD(pTime->tm_min);
		data[2] = DEC2BCD(pTime->tm_hour);
		data[3] = DEC2BCD(pTime->tm_mday);
		data[4] = pTime->tm_wday & 0x07;

		FUNC_PREFIX(I2cSetAddress(address_));
		FUNC_PREFIX(I2cSetBaudrate(HAL_I2C::FULL_SPEED));
		FUNC_PREFIX(I2cWrite(data, sizeof(data) / sizeof(data[0])));

		PCF8563SetAlarmMode();

		DEBUG_EXIT();
		return true;
	}
	break;
#endif
	default:
		break;
	}

	DEBUG_EXIT();
	return false;
}

bool HwClock::RtcGetAlarm(struct tm *pTime) {
	DEBUG_ENTRY();
	assert(pTime != nullptr);

	if (!RtcGet(pTime)) {
		DEBUG_EXIT();
		return false;
	}

	switch (type_) {
#if !defined (CONFIG_RTC_DISABLE_MCP7941X)
	case Type::kMcP7941X: {
		char registers[10];

		registers[0] = mcp7941x::reg::kControl;

		FUNC_PREFIX(I2cSetAddress(address_));
		FUNC_PREFIX(I2cSetBaudrate(HAL_I2C::FULL_SPEED));
		FUNC_PREFIX(I2cWrite(registers, 1));
		FUNC_PREFIX(I2cRead(registers, sizeof(registers) / sizeof(registers[0])));

		pTime->tm_sec = BCD2DEC(registers[3] & 0x7f);
		pTime->tm_min = BCD2DEC(registers[4] & 0x7f);
		pTime->tm_hour = BCD2DEC(registers[5] & 0x3f);
		pTime->tm_wday = BCD2DEC(registers[6] & 0x7) - 1;
		pTime->tm_mday = BCD2DEC(registers[7] & 0x3f);
		pTime->tm_mon = BCD2DEC(registers[8] & 0x1f) - 1;

		alarm_enabled_ = registers[0] & mcp7941x::bit::kAlM0En;

		DEBUG_PRINTF("sec=%d min=%d hour=%d wday=%d mday=%d mon=%d enabled=%d polarity=%d irq=%d match=%u",
				pTime->tm_sec,
				pTime->tm_min,
				pTime->tm_hour,
				pTime->tm_wday,
				pTime->tm_mday,
				pTime->tm_mon,
				alarm_enabled_,
				(registers[6] & mcp7941x::bit::kAlmxPol),
				(registers[6] & mcp7941x::bit::kAlmxIf),
				(registers[6] & mcp7941x::bit::kMskAlmxMatch) >> 4);

		DEBUG_EXIT();
		return true;
	}
	break;
#endif
#if !defined (CONFIG_RTC_DISABLE_DS3231)
	case Type::kDS3231: {
		char registers[10];

		registers[0] = ds3231::reg::kAlarM1Seconds;

		FUNC_PREFIX(I2cSetAddress(address_));
		FUNC_PREFIX(I2cSetBaudrate(HAL_I2C::FULL_SPEED));
		FUNC_PREFIX(I2cWrite(registers, 1));
		FUNC_PREFIX(I2cRead(registers, sizeof(registers) / sizeof(registers[0])));

		pTime->tm_sec = BCD2DEC(registers[0] & 0x7f);
		pTime->tm_min = BCD2DEC(registers[1] & 0x7f);
		pTime->tm_hour = BCD2DEC(registers[2] & 0x3f);
		pTime->tm_mday = BCD2DEC(registers[3] & 0x3f);

		alarm_enabled_ = (registers[7] & ds3231::bit::kA1Ie);
		alarm_pending_ = (registers[8] &ds3231::bit::kA1F);

		DEBUG_PRINTF("tm is sec=%d, min=%d, hour=%d, mday=%d, enabled=%d, pending=%d",
				pTime->tm_sec,
				pTime->tm_min,
				pTime->tm_hour,
				pTime->tm_mday,
				alarm_enabled_,
				alarm_pending_);

		DEBUG_EXIT();
		return true;
	}

		break;
#endif
#if !defined (CONFIG_RTC_DISABLE_PCF8563)
	case Type::kPcF8563: {
		char registers[4];

		registers[0] = pcf8563::reg::kAlarm;

		FUNC_PREFIX(I2cSetAddress(address_));
		FUNC_PREFIX(I2cSetBaudrate(HAL_I2C::FULL_SPEED));
		FUNC_PREFIX(I2cWrite(registers, 1));
		FUNC_PREFIX(I2cRead(registers, sizeof(registers) / sizeof(registers[0])));

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
				alarm_enabled_,
				alarm_pending_);

		DEBUG_EXIT();
		return true;
	}
	break;
#endif
	default:
		break;
	}

	DEBUG_EXIT();
	return false;
}

int HwClock::MCP794xxAlarmWeekday(struct tm *pTime) {
	DEBUG_ENTRY();
	assert(pTime != nullptr);
	assert(type_ == rtc::Type::kMcP7941X);

	struct tm tm_now;
	RtcGet(&tm_now);

	const auto days_now = mktime(&tm_now) / (24 * 60 * 60);
	const auto days_alarm = mktime(pTime) / (24 * 60 * 60);
	const auto i =  (tm_now.tm_wday + days_alarm - days_now) % 7 + 1;

	DEBUG_EXIT();
	return i;
}

void HwClock::PCF8563GetAlarmMode() {
	DEBUG_ENTRY();
	assert(type_ == rtc::Type::kPcF8563);

	uint8_t nValue;
	FUNC_PREFIX(I2cReadReg(pcf8563::reg::kControlStatuS2, nValue));

	alarm_enabled_ = nValue & pcf8563::bit::kStatuS2Aie;
	alarm_pending_ = nValue & pcf8563::bit::kStatuS2Af;

	DEBUG_EXIT();
}

void HwClock::PCF8563SetAlarmMode() {
	DEBUG_ENTRY();
	assert(type_ == rtc::Type::kPcF8563);

	char data[2];

	data[1] = pcf8563::reg::kControlStatuS2;

	FUNC_PREFIX(I2cWrite(&data[1], 1));
	FUNC_PREFIX(I2cRead(&data[1], 1));

	if (alarm_enabled_) {
		DEBUG_PUTS("Alarm is enabled");
		data[1] |= pcf8563::bit::kStatuS2Aie;
	} else {
		data[1] &= static_cast<char>(~pcf8563::bit::kStatuS2Aie);
	}

	data[0] = pcf8563::reg::kControlStatuS2;
	data[1] &= static_cast<char>(~pcf8563::bit::kStatuS2Af);

	FUNC_PREFIX(I2cWrite(data, 2));

	DEBUG_EXIT();
}
