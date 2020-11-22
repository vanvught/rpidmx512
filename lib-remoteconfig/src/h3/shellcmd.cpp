/**
 * @file shellcmd.cpp
 *
 */
/* Copyright (C) 2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "h3/shell.h"
#include "h3_uart0_debug.h"

#include "remoteconfig.h"
#include "network.h"

#include "hardware.h"
#include "firmwareversion.h"

#include "hwclock.h"

// Firmware specific BEGIN
#if defined (LTC_READER)
# include "ltc.h"
# include "h3/ltcgenerator.h"
# include "h3/systimereader.h"
#endif
// Firmware specific BEGIN

#include "debug.h"

#ifndef NDEBUG
# include "../debug/i2cdetect.h"
# include "ntpclient.h"
# include "gps.h"
extern "C" {
void h3_board_dump(void);
void h3_dump_memory_mapping(void);
void h3_ccu_pll_dump(void);
void arm_dump_memmap(void);
void arm_dump_page_table(void);
}
#endif

// TODO We can reshuffle namespaces here for removing duplicates?

namespace shell {

namespace set {
namespace arg {
static constexpr char IP[] = "ip";
static constexpr char HOSTNAME[] = "hostname";
static constexpr char LTC[] = "ltc";
}  // namespace arg
namespace length {
static constexpr auto IP = sizeof(arg::IP) - 1;
static constexpr auto HOSTNAME = sizeof(arg::HOSTNAME) - 1;
static constexpr auto LTC = sizeof(arg::LTC) - 1;
}  // namespace length
}  // namespace set

namespace dump {
namespace arg {
static constexpr char BOARD[] = "board";
static constexpr char MMAP[] = "mmap";
static constexpr char PLL[] = "pll";
static constexpr char LINKER[] = "linker";
}  // namespace arg
namespace length {
static constexpr auto BOARD = sizeof(arg::BOARD) - 1;
static constexpr auto MMAP = sizeof(arg::MMAP) - 1;
static constexpr auto PLL = sizeof(arg::PLL) - 1;
static constexpr auto LINKER = sizeof(arg::LINKER) - 1;
}  // namespace length
}  // namespace dump

namespace networktime {
namespace arg {
static constexpr char PRINT[] = "print";
}  // namespace arg
namespace length {
static constexpr auto PRINT = sizeof(arg::PRINT) - 1;
}  // namespace length
}  // namespace networktime

namespace hwclock {
namespace arg {
static constexpr char HCTOSYS[] = "hctosys";	// Set the System Clock from the Hardware Clock
static constexpr char SYSTOHC[] = "systohc";	// Set the Hardware Clock from the System Clock
static constexpr char PRINT[] = "print";
}  // namespace arg
namespace length {
static constexpr auto HCTOSYS = sizeof(arg::HCTOSYS) - 1;
static constexpr auto SYSTOHC = sizeof(arg::SYSTOHC) - 1;
static constexpr auto PRINT = sizeof(arg::PRINT) - 1;
}  // namespace length
}  // namespace hwclock

namespace gps {
namespace arg {
static constexpr char DATE[] = "date";
static constexpr char LOCALTIME[] = "localtime";
static constexpr char PRINT[] = "print";
}  // namespace arg
namespace length {
static constexpr auto DATE = sizeof(arg::DATE) - 1;
static constexpr auto LOCALTIME = sizeof(arg::LOCALTIME) - 1;
static constexpr auto PRINT = sizeof(arg::PRINT) - 1;
}  // namespace length
}  // namespace gps

namespace file {
static constexpr char EXT[] = ".txt";
namespace length {
static constexpr char EXT = sizeof(file::EXT) - 1;
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
static constexpr char INVALID[] = "Invalid command.\n";
static constexpr char INTERNAL[] = "Internal error.\n";
static constexpr char DHCP[] = "DHCP failed.\n";
static constexpr char TXT[] = ".txt not found\n";
static constexpr char PROPERTY[] = "Property not found.\n";
static constexpr char LTC[] = "This source does not support the set command.\n";
}  // namespace error
}  // namespace msg

}  // namespace shell

using namespace shell;

uint32_t Shell::hexadecimalToDecimal(const char *pHexValue, uint32_t nLength) {
	const auto *pSrc = pHexValue;
	uint32_t nValue = 0;

	while (nLength-- > 0) {
		const auto c = *pSrc;

		if (isxdigit(c) == 0) {
			break;
		}

		const uint8_t nNibble = c > '9' ? (c | 0x20) - 'a' + 10 : (c - '0');
		nValue = (nValue << 4) | nNibble;
		pSrc++;
	}

	return nValue;
}

void Shell::CmdReboot() {
	RemoteConfig::Get()->Reboot();
}

void Shell::CmdInfo() {
	uart0_printf("%s", FirmwareVersion::Get()->GetPrint());
	uart0_printf("Core Temperature: %.0f <%.0f>\n",Hardware::Get()->GetCoreTemperature(), Hardware::Get()->GetCoreTemperatureMax());
	uart0_printf("Uptime: %d\n", Hardware::Get()->GetUpTime());
	uart0_printf("Hostname: %s\n", Network::Get()->GetHostName());
	uart0_printf("IP " IPSTR "/%d %c\n", IP2STR(Network::Get()->GetIp()), Network::Get()->GetNetmaskCIDR(), Network::Get()->GetAddressingMode());
}

void Shell::CmdSet() {
	const auto nArgv0Length = m_nArgvLength[0];

	if ((nArgv0Length == set::length::IP) && (memcmp(m_Argv[0], set::arg::IP, set::length::IP) == 0)) {
		in_addr ip;
		if (inet_aton(m_Argv[1], &ip) != 0) {
			Network::Get()->SetIp(ip.s_addr);
		} else {
			uart0_puts(msg::usage::IP);
		}

		return;
	}

	if ((nArgv0Length == set::length::HOSTNAME) && (memcmp(m_Argv[0], set::arg::HOSTNAME, set::length::HOSTNAME) == 0)) {
		const auto nArgv1Length = m_nArgvLength[1];

		if ((nArgv1Length != 0) && (nArgv1Length <= TNetwork::NETWORK_HOSTNAME_SIZE)) {
			Network::Get()->SetHostName(m_Argv[1]);
		} else {
			uart0_puts(msg::usage::HOSTNAME);
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

 			const auto nRequestLenght = 4 + nArgv1Length;

 			DEBUG_PRINTF("Request: %.*s", nRequestLenght, request);

			switch (m_ltcSource) {
			case ltc::source::INTERNAL:
				LtcGenerator::Get()->HandleRequest(request, nRequestLenght);;
				break;	
			case ltc::source::SYSTIME:
				SystimeReader::Get()->HandleRequest(request, nRequestLenght);
				break;
			default:
				uart0_puts(msg::error::LTC);
				break;
			}	
		} else {
			//uart0_puts(msg::usage::LTC);
		}
		return;
	}
#endif
// Firmware specific END

	char buffer[1024];
	memcpy(buffer, m_Argv[0], nArgv0Length);
	memcpy(&buffer[nArgv0Length], file::EXT, file::length::EXT);
	buffer[nArgv0Length + file::length::EXT] = '\0';
	uint32_t nLength = nArgv0Length + file::length::EXT;

	if (RemoteConfig::GetIndex(buffer, nLength) < TXT_FILE_LAST) {
		DEBUG_PUTS(m_Argv[0]);

		if ((nLength = RemoteConfig::Get()->HandleGet(buffer, sizeof(buffer))) < (sizeof(buffer) - m_nArgvLength[1] - 1)) {
			memcpy(&buffer[nLength], m_Argv[1], m_nArgvLength[1]);
			RemoteConfig::Get()->HandleTxtFile(buffer, nLength + m_nArgvLength[1]);
			uart0_puts(msg::info::STORED);
			return;
		} else {
			uart0_puts(msg::error::INTERNAL);
			return;
		}

		return;
	}

	uart0_puts(msg::error::INVALID);
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
			uart0_puts(msg::error::TXT);
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
				uart0_puts(pValue);

				return;
			}

			for (; i < nLength; p++, i++) {
				if (*p == '\n') {
					break;
				}
			}
		}
	} else {
		uart0_puts(msg::error::INTERNAL);
		return;
	}

	uart0_puts(msg::error::PROPERTY);
	return;
}

void Shell::CmdDhcp() {
	if (Network::Get()->EnableDhcp()) {
		uart0_puts(msg::info::DHCP);
	} else {
		uart0_puts(msg::error::DHCP);
	}
}

void Shell::CmdDate() {
	time_t rawtime;
	time(&rawtime);

	uart0_puts(asctime(localtime(&rawtime)));
}

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

	uart0_puts(msg::error::INVALID);
}

/*
 * Debug commands
 */

#ifndef NDEBUG
void Shell::CmdI2cDetect() {
	I2cDetect i2cdetect;
}

void Shell::CmdDump() {
	const auto nArgv0Length = m_nArgvLength[0];

	if ((nArgv0Length == dump::length::BOARD) && (memcmp(m_Argv[0], dump::arg::BOARD, dump::length::BOARD) == 0)) {
		h3_board_dump();
		return;
	}

	if ((nArgv0Length == dump::length::MMAP) && (memcmp(m_Argv[0], dump::arg::MMAP, dump::length::MMAP) == 0)) {
		h3_dump_memory_mapping();
		return;
	}

	if ((nArgv0Length == dump::length::PLL) && (memcmp(m_Argv[0], dump::arg::PLL, dump::length::PLL) == 0)) {
		h3_ccu_pll_dump();
		return;
	}

	if ((nArgv0Length == dump::length::LINKER) && (memcmp(m_Argv[0], dump::arg::LINKER, dump::length::LINKER) == 0)) {
		arm_dump_memmap();
		return;
	}

	uart0_puts(msg::error::INVALID);
}

void Shell::CmdMem() {
	const auto pAddress = reinterpret_cast<const char *>(hexadecimalToDecimal(m_Argv[0], m_nArgvLength[0]));
	const auto nSize = hexadecimalToDecimal(m_Argv[1], m_nArgvLength[1]);

	debug_dump(pAddress, nSize);
}

void Shell::CmdNtp() {
	const auto nArgv0Length = m_nArgvLength[0];

	if ((nArgv0Length == networktime::length::PRINT) && (memcmp(m_Argv[0], networktime::arg::PRINT, networktime::length::PRINT) == 0)) {
		NtpClient::Get()->Print();
		return;
	}

	uart0_puts(msg::error::INVALID);
}

void Shell::CmdGps() {
	const auto nArgv0Length = m_nArgvLength[0];

	if ((nArgv0Length == shell::gps::length::DATE) && (memcmp(m_Argv[0], shell::gps::arg::DATE, shell::gps::length::DATE) == 0)) {
		const auto tm = GPS::Get()->GetDateTime();
		uart0_printf("%.2d/%.2d/%.2d %.2d:%.2d:%.2d\n", tm->tm_mday, 1 + tm->tm_mon, 1900 + tm->tm_year, tm->tm_hour, tm->tm_min, tm->tm_sec);
		return;
	}

	if ((nArgv0Length == shell::gps::length::LOCALTIME) && (memcmp(m_Argv[0], shell::gps::arg::LOCALTIME, shell::gps::length::LOCALTIME) == 0)) {
		const auto t = GPS::Get()->GetLocalSeconds();
		uart0_puts(asctime(localtime(&t)));
		return;
	}

	if ((nArgv0Length == shell::gps::length::PRINT) && (memcmp(m_Argv[0], shell::gps::arg::PRINT, shell::gps::length::PRINT) == 0)) {
		GPS::Get()->Print();
		return;
	}

	uart0_puts(msg::error::INVALID);
}
#endif
