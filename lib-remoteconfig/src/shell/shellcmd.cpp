/**
 * @file shellcmd.cpp
 *
 */
/* Copyright (C) 2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstring>
#include <cctype>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "shell/shell.h"

#include "remoteconfig.h"
#include "network.h"
#include "emac/phy.h"

#include "hardware.h"
#include "firmwareversion.h"

#include "hwclock.h"

// Firmware specific BEGIN
#if defined (LTC_READER)
# include "ltc.h"
# include "ltcgenerator.h"
# include "systimereader.h"
#endif
// Firmware specific BEGIN

#if defined (DEBUG_I2C)
# include "../lib-hal/debug/i2c/i2cdetect.h"
#endif

#if defined (ENABLE_NTP_CLIENT)
# include "net/apps/ntpclient.h"
#endif

#if defined (CONFIG_SHELL_GPS)
# include "gps.h"
#endif

#if (PHY_TYPE == RTL8201F)
# include "emac/phy/rtl8201f.h"
#endif

#if defined (GD32) // PHY_TYPE is defined here
# include "gd32.h"
#endif

#include "debug.h"

namespace shell {
// Centralize common arguments and lengths
namespace common_args {
static constexpr char PRINT[] = "print";
}  // namespace common_args

namespace common_lengths {
static constexpr auto PRINT = sizeof(common_args::PRINT) - 1;
}  // namespace common_lengths

namespace set {
namespace arg {
static constexpr char IP[] = "ip";
static constexpr char HOSTNAME[] = "hostname";
#if defined (LTC_READER)
static constexpr char LTC[] = "ltc";
#endif
}  // namespace arg
namespace length {
static constexpr auto IP = sizeof(arg::IP) - 1;
static constexpr auto HOSTNAME = sizeof(arg::HOSTNAME) - 1;
#if defined (LTC_READER)
static constexpr auto LTC = sizeof(arg::LTC) - 1;
#endif
}  // namespace length
}  // namespace set

namespace networktime {
namespace arg {
// Reuse common_args::PRINT
using common_args::PRINT;
}  // namespace arg
namespace length {
// Reuse common_lengths::PRINT
using common_lengths::PRINT;
}  // namespace length
}  // namespace networktime

#if !defined(DISABLE_RTC)
namespace hwclock {
namespace arg {
static constexpr char HCTOSYS[] = "hctosys";  // Set the System Clock from the Hardware Clock
static constexpr char SYSTOHC[] = "systohc";  // Set the Hardware Clock from the System Clock
// Reuse common_args::PRINT
using common_args::PRINT;
}  // namespace arg
namespace length {
static constexpr auto HCTOSYS = sizeof(arg::HCTOSYS) - 1;
static constexpr auto SYSTOHC = sizeof(arg::SYSTOHC) - 1;
// Reuse common_lengths::PRINT
using common_lengths::PRINT;
}  // namespace length
}  // namespace hwclock
#endif

#if defined (CONFIG_SHELL_GPS)
namespace gps {
namespace arg {
static constexpr char DATE[] = "date";
static constexpr char LOCALTIME[] = "localtime";
// Reuse common_args::PRINT
using common_args::PRINT;
}  // namespace arg
namespace length {
static constexpr auto DATE = sizeof(arg::DATE) - 1;
static constexpr auto LOCALTIME = sizeof(arg::LOCALTIME) - 1;
// Reuse common_lengths::PRINT
using common_lengths::PRINT;
}  // namespace length
}  // namespace gps
#endif

#if (PHY_TYPE == RTL8201F)
namespace rtl8201f {
namespace arg {
static constexpr char RXTIMING[] = "rxtiming";
static constexpr char TXTIMING[] = "txtiming";
static constexpr char TIMING[] = "timing";
// Reuse common_args::PRINT
using common_args::PRINT;
}  // namespace arg
namespace length {
static constexpr auto RXTIMING = sizeof(arg::RXTIMING) - 1;
static constexpr auto TXTIMING = sizeof(arg::TXTIMING) - 1;
static constexpr auto TIMING = sizeof(arg::TIMING) - 1;
// Reuse common_lengths::PRINT
using common_lengths::PRINT;
}  // namespace length
}  // namespace rtl8201f
#endif

namespace file {
static constexpr char EXT[] = ".txt";
namespace length {
static constexpr uint16_t EXT = sizeof(file::EXT) - 1;
}  // namespace length
}  // namespace file

namespace msg {
namespace usage {
static constexpr char IP[] = "Usage: set ip x.x.x.x\n";
static constexpr auto HOSTNAME = "Usage: set hostname name\n";
}  // namespace usage
namespace info {
static constexpr char DHCP[] = "DHCP enabled\n";
static constexpr char STORED[] = "Stored\n";
}  // namespace info
namespace error {
static constexpr char TXT[] = ".txt not found\n";
static constexpr char PROPERTY[] = "Property not found.\n";
#if defined (LTC_READER)
static constexpr char LTC[] = "This source does not support the set command.\n";
#endif
}  // namespace error
}  // namespace msg
}  // namespace shell

using namespace shell;

void Shell::CmdReboot() {
	RemoteConfig::Get()->Reboot();
}

void Shell::CmdInfo() {
	Printf("%s\n", FirmwareVersion::Get()->GetPrint());
	Printf("Core Temperature: %.0f <%.0f>\n",Hardware::Get()->GetCoreTemperature(), Hardware::Get()->GetCoreTemperatureMax());
	Printf("Uptime: %d\n", Hardware::Get()->GetUpTime());
	Printf("Hostname: %s\n", Network::Get()->GetHostName());
	Printf("IP " IPSTR "/%d %c\n", IP2STR(Network::Get()->GetIp()), Network::Get()->GetNetmaskCIDR(), Network::Get()->GetAddressingMode());
}

void Shell::CmdSet() {
	const auto nArgv0Length = m_nArgvLength[0];

	if ((nArgv0Length == set::length::IP) && (memcmp(m_Argv[0], set::arg::IP, set::length::IP) == 0)) {
		in_addr ip;
		if (inet_aton(m_Argv[1], &ip) != 0) {
			Network::Get()->SetIp(ip.s_addr);
		} else {
			Puts(msg::usage::IP);
		}

		return;
	}

	if ((nArgv0Length == set::length::HOSTNAME) && (memcmp(m_Argv[0], set::arg::HOSTNAME, set::length::HOSTNAME) == 0)) {
		const auto nArgv1Length = m_nArgvLength[1];

		if ((nArgv1Length != 0) && (nArgv1Length <= network::HOSTNAME_SIZE)) {
			Network::Get()->SetHostName(m_Argv[1]);
		} else {
			Puts(msg::usage::HOSTNAME);
		}

		return;
	}

// Firmware specific BEGIN
#if defined (LTC_READER)
	if ((nArgv0Length == set::length::LTC) && (memcmp(m_Argv[0], set::arg::LTC, set::length::LTC) == 0)) {
		const auto nArgv1Length = m_nArgvLength[1];
		char request[64];

		if ((nArgv1Length != 0) && (nArgv1Length <= (sizeof(request) - 4))) {
			request[0] = 'l';
 			request[1] = 't';
 			request[2] = 'c';
 			request[3] = '!';						
 			memcpy(&request[4], m_Argv[1], nArgv1Length);

 			const auto nRequestLenght = static_cast<uint16_t>(4 + nArgv1Length);

			switch (m_ltcSource) {
			case ltc::Source::INTERNAL:
				LtcGenerator::Get()->HandleRequest(request, nRequestLenght);
				break;	
			case ltc::Source::SYSTIME:
				SystimeReader::Get()->HandleRequest(request, nRequestLenght);
				break;
			default:
				Puts(msg::error::LTC);
				break;
			}	
		} else {
			//Puts(msg::usage::LTC);
		}
		return;
	}
#endif
// Firmware specific END

	char buffer[1024];
	memcpy(buffer, m_Argv[0], nArgv0Length);
	memcpy(&buffer[nArgv0Length], file::EXT, file::length::EXT);
	buffer[nArgv0Length + file::length::EXT] = '\0';
	auto nLength = static_cast<uint32_t>(nArgv0Length + file::length::EXT);

	if (RemoteConfig::Get()->GetIndex(buffer, nLength) >= 0) {
		DEBUG_PUTS(m_Argv[0]);

		if ((nLength = RemoteConfig::Get()->HandleGet(buffer, sizeof(buffer))) < (sizeof(buffer) - m_nArgvLength[1] - 1)) {
			memcpy(&buffer[nLength], m_Argv[1], m_nArgvLength[1]);
			RemoteConfig::Get()->HandleSet(buffer, nLength + m_nArgvLength[1]);
			Puts(msg::info::STORED);
			return;
		} else {
			Puts(msg::error::INTERNAL);
			return;
		}

		return;
	}

	Puts(msg::error::INVALID);
}

void Shell::CmdGet() {
	const auto nArgv0Length = m_nArgvLength[0];

	char buffer[1024];
	memcpy(buffer, m_Argv[0], nArgv0Length);
	memcpy(&buffer[nArgv0Length], file::EXT, file::length::EXT);
	buffer[nArgv0Length + file::length::EXT] = '\0';

	uint32_t nLength;

	if ((nLength = RemoteConfig::Get()->HandleGet(buffer, sizeof(buffer))) < (sizeof(buffer) - 1)) {

		if (*buffer == '?') { // "?get#ERROR#\n"
			Puts(msg::error::TXT);
			return;
		}

		buffer[nLength] = '\0';

		auto p = buffer;
		const auto nPropertyLength = m_nArgvLength[1];

		uint32_t i;

		for (i = 0; i < nLength; p++, i++) {
			if (*p == '#') {
				continue;
			}

			if (memcmp(p, m_Argv[1], nPropertyLength) == 0) {
				const char *pValue = p + nPropertyLength + 1;
				for (; i < nLength; p++, i++) {
					if (*p == '\n') {
						break;
					}
				}

				*++p = '\0';
				Puts(pValue);

				return;
			}

			for (; i < nLength; p++, i++) {
				if (*p == '\n') {
					break;
				}
			}
		}
	} else {
		Puts(msg::error::INTERNAL);
		return;
	}

	Puts(msg::error::PROPERTY);
	return;
}

void Shell::CmdDhcp() {
	Network::Get()->EnableDhcp();
	Puts(msg::info::DHCP);
}

void Shell::CmdDate() {
	time_t rawtime;
	time(&rawtime);

	Puts(asctime(localtime(&rawtime)));
}

void Shell::CmdPhy() {
	net::PhyStatus phyStatus;
	phy_customized_status(phyStatus);

	Printf("Link %s, %d, %s\n",
			phyStatus.link == net::Link::STATE_UP ? "Up" : "Down",
			phyStatus.speed == net::Speed::SPEED10 ? 10 : 100,
			phyStatus.duplex == net::Duplex::DUPLEX_HALF ? "HALF" : "FULL");
}

#ifndef NDEBUG
static uint32_t hexadecimal_to_decimal(const char *pHexValue, uint32_t nLength) {
	const auto *pSrc = pHexValue;
	uint32_t nValue = 0;

	while (nLength-- > 0) {
		const auto c = *pSrc;

		if (isxdigit(c) == 0) {
			break;
		}

		const auto nNibble = c > '9' ? static_cast<uint8_t>((c | 0x20) - 'a' + 10) : static_cast<uint8_t>(c - '0');
		nValue = (nValue << 4) | nNibble;
		pSrc++;
	}

	return nValue;
}
#endif

void Shell::CmdMem() {
#ifndef NDEBUG
	const auto pAddress = reinterpret_cast<const char *>(hexadecimal_to_decimal(m_Argv[0], m_nArgvLength[0]));
	const auto nSize = hexadecimal_to_decimal(m_Argv[1], m_nArgvLength[1]);

	debug_dump(pAddress, nSize);
#endif
}

#if !defined(DISABLE_RTC)
void Shell::CmdHwClock() {
	const auto nArgv0Length = m_nArgvLength[0];

	if ((nArgv0Length == hwclock::length::HCTOSYS) && (memcmp(m_Argv[0], hwclock::arg::HCTOSYS, hwclock::length::HCTOSYS) == 0)) {
		HwClock::Get()->HcToSys();
		return;
	}

	if ((nArgv0Length == hwclock::length::SYSTOHC) && (memcmp(m_Argv[0], hwclock::arg::SYSTOHC, hwclock::length::SYSTOHC) == 0)) {
		HwClock::Get()->SysToHc();
		return;
	}

	if ((nArgv0Length == hwclock::length::PRINT) && (memcmp(m_Argv[0], hwclock::arg::PRINT, hwclock::length::PRINT) == 0)) {
		HwClock::Get()->Print();
		return;
	}

	Puts(msg::error::INVALID);
}
#endif

#if defined (DEBUG_I2C)
void Shell::CmdI2cDetect() {
	I2cDetect i2cdetect;
}
#endif

#if defined (ENABLE_NTP_CLIENT)
void Shell::CmdNtp() {
	const auto nArgv0Length = m_nArgvLength[0];

	if ((nArgv0Length == networktime::length::PRINT) && (memcmp(m_Argv[0], networktime::arg::PRINT, networktime::length::PRINT) == 0)) {
		NtpClient::Get()->Print();
		return;
	}

	Puts(msg::error::INVALID);
}
#endif

#if defined (CONFIG_SHELL_GPS)
void Shell::CmdGps() {
	const auto nArgv0Length = m_nArgvLength[0];

	if ((nArgv0Length == shell::gps::length::DATE) && (memcmp(m_Argv[0], shell::gps::arg::DATE, shell::gps::length::DATE) == 0)) {
		const auto tm = GPS::Get()->GetDateTime();
		Printf("%.2d/%.2d/%.2d %.2d:%.2d:%.2d\n", tm->tm_mday, 1 + tm->tm_mon, 1900 + tm->tm_year, tm->tm_hour, tm->tm_min, tm->tm_sec);
		return;
	}

	if ((nArgv0Length == shell::gps::length::LOCALTIME) && (memcmp(m_Argv[0], shell::gps::arg::LOCALTIME, shell::gps::length::LOCALTIME) == 0)) {
		const auto t = GPS::Get()->GetLocalSeconds();
		Puts(asctime(localtime(&t)));
		return;
	}

	if ((nArgv0Length == shell::gps::length::PRINT) && (memcmp(m_Argv[0], shell::gps::arg::PRINT, shell::gps::length::PRINT) == 0)) {
		GPS::Get()->Print();
		return;
	}

	Puts(msg::error::INVALID);
}
#endif

#if (PHY_TYPE == RTL8201F)
void Shell::CmdPhyTypeRTL8201F() {
	const auto nArgv0Length = m_nArgvLength[0];

	if ((nArgv0Length == rtl8201f::length::RXTIMING) && (memcmp(m_Argv[0], rtl8201f::arg::RXTIMING, rtl8201f::length::RXTIMING) == 0)) {
		if (m_nArgvLength[1] == 1) {
			const auto c = m_Argv[1][0];
			if (isxdigit(c) != 0) {
				const uint32_t nRxTiming = c > '9' ? ((c | 0x20) - 'a' + 10) : (c - '0');
				net::phy::rtl8201f_set_rxtiming(nRxTiming);
				return;
			}
		}
	}

	if ((nArgv0Length == rtl8201f::length::TXTIMING) && (memcmp(m_Argv[0], rtl8201f::arg::TXTIMING, rtl8201f::length::TXTIMING) == 0)) {
		if (m_nArgvLength[1] == 1) {
			const auto c = m_Argv[1][0];
			if (isxdigit(c) != 0) {
				const uint32_t nTxTiming = c > '9' ? ((c | 0x20) - 'a' + 10) : (c - '0');
				net::phy::rtl8201f_set_txtiming(nTxTiming);
				return;
			}
		}
	}

	if ((nArgv0Length == rtl8201f::length::PRINT) && (memcmp(m_Argv[0], rtl8201f::arg::PRINT, rtl8201f::length::PRINT) == 0)) {
		if ((m_nArgvLength[1] == rtl8201f::length::TIMING) && (memcmp(m_Argv[1], rtl8201f::arg::TIMING, rtl8201f::length::TIMING) == 0)) {
			uint32_t nRxTiming;
			uint32_t nTxTiming;
			net::phy::rtl8201f_get_timings(nRxTiming, nTxTiming);
			Printf("RX: 0x%X, TX: 0x%X\n", nRxTiming, nTxTiming);
			return;
		}
	}

	Puts(msg::error::INVALID);
}
#endif
