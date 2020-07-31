/**
 * @file shellcmd.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

// FIXME Remove when finished
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "h3/shell.h"
#include "h3_uart0_debug.h"

#include "remoteconfig.h"
#include "network.h"

#include "hardware.h"
#include "firmwareversion.h"

#ifndef NDEBUG
# include "../debug/i2cdetect.h"
extern "C" {
void h3_board_dump(void);
void h3_dump_memory_mapping(void);
void h3_ccu_pll_dump(void);
void arm_dump_memmap(void);
}
#endif

#include "debug.h"

namespace shell {
namespace cmd {
static constexpr char SET_IP[] = "ip";
static constexpr char SET_HOSTNAME[] = "hostname";
}  // namespace cmd
namespace length {
static constexpr auto SET_IP = sizeof(cmd::SET_IP) - 1;
static constexpr auto SET_HOSTNAME = sizeof(cmd::SET_HOSTNAME) - 1;
}  // namespace length
namespace dump {
namespace cmd {
static constexpr char BOARD[] = "board";
static constexpr char MMAP[] = "mmap";
static constexpr char PLL[] = "pll";
static constexpr char LINKER[] = "linker";
}  // namespace cmd
namespace length {
static constexpr auto BOARD = sizeof(cmd::BOARD) - 1;
static constexpr auto MMAP = sizeof(cmd::MMAP) - 1;
static constexpr auto PLL = sizeof(cmd::PLL) - 1;
static constexpr auto LINKER = sizeof(cmd::LINKER) - 1;
}  // namespace length
}  // namespace dump
}  // namespace shell

using namespace shell;

void Shell::CmdReboot() {
	DEBUG_ENTRY
	RemoteConfig::Get()->Reboot();
	DEBUG_EXIT
}

void Shell::CmdInfo() {
	DEBUG_ENTRY
	uart0_printf("%s", FirmwareVersion::Get()->GetPrint());
	uart0_printf("Core Temperature: %.0f <%.0f>\n",Hardware::Get()->GetCoreTemperature(), Hardware::Get()->GetCoreTemperatureMax());
	uart0_printf("Uptime: %d\n", Hardware::Get()->GetUpTime());
	uart0_printf("Hostname: %s\n", Network::Get()->GetHostName());
	uart0_printf("IP " IPSTR "/%d %c\n", IP2STR(Network::Get()->GetIp()), Network::Get()->GetNetmaskCIDR(), Network::Get()->GetAddressingMode());
	DEBUG_EXIT
}

void Shell::CmdSet() {
	DEBUG_ENTRY
#ifndef NDEBUG
	uart0_printf("m_Argv[0..1]: %s %s\n", m_Argv[0], m_Argv[1]);
#endif

	if ((m_nArgv0Length == length::SET_IP) && (memcmp(m_Argv[0], cmd::SET_IP, length::SET_IP) == 0)) {
		in_addr group_ip;
		if (inet_aton(m_Argv[1], &group_ip)) {
			DEBUG_PRINTF("New IP: " IPSTR, IP2STR(group_ip.s_addr));
			Network::Get()->SetIp(group_ip.s_addr);
		} else {
			uart0_puts("Usage: set ip x.x.x.x\n");
		}

		return;
	}

	if ((m_nArgv0Length == length::SET_HOSTNAME) && (memcmp(m_Argv[0], cmd::SET_HOSTNAME, length::SET_HOSTNAME) == 0)) {
		const auto nArgv1Length = strlen(m_Argv[1]);	// 2nd arg is hostname string

		DEBUG_PRINTF("New hostname: %s", m_Argv[1]);

		if ((nArgv1Length != 0) && (nArgv1Length <= TNetwork::NETWORK_HOSTNAME_SIZE)) {
			Network::Get()->SetHostName(m_Argv[1]);
		} else {
			uart0_puts("Usage: set hostname name\n");	
		}

		return;
	}

	uint32_t nLength = m_nArgv0Length; 
	if (RemoteConfig::GetIndex(m_Argv[0], nLength) < TXT_FILE_LAST) {
		DEBUG_PUTS(m_Argv[0]);
		// TODO
		return;
	}

	uart0_puts("Usage: set [ip][hostname][name\'.txt\'] [value]\n");
	DEBUG_EXIT
}

void Shell::CmdGet() {
	DEBUG_ENTRY

	char buffer[1024];

	memcpy(buffer, m_Argv[0], m_nArgv0Length);
	uint32_t nLength;

	if ((nLength = RemoteConfig::Get()->HandleGet(buffer, sizeof(buffer))) < (sizeof(buffer) - 1)) {

		if (*buffer == '?') { // "?get#ERROR#\n"
			uart0_puts(".txt not found\n");
			return;
		}

		buffer[nLength] = '\0';

		char *p = buffer;
		// TOOD We know the m_Argv[] length in ValidateArg. Let's store it in member variable?
		const auto nPropertyLength = strlen(m_Argv[1]);

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

			// We could use returned value by memcmp?
			for (; i < nLength; p++, i++) {
				if (*p == '\n') {
					break;
				}
			}
		}
	} else {
		uart0_puts("Error\n");
	}

	uart0_puts("Property not found\n");
	return;

	DEBUG_EXIT
}

void Shell::CmdDhcp() {
	DEBUG_ENTRY

	if (Network::Get()->EnableDhcp()) {
		uart0_puts("DHCP is enabled\n");
	} else {
		uart0_puts("DHCP failed\n");
	}

	DEBUG_EXIT
}

/*
 * Debug commands
 */

#ifndef NDEBUG
void Shell::CmdI2cDetect() {
	DEBUG_ENTRY
	I2cDetect i2cdetect;
	DEBUG_EXIT
}

void Shell::CmdDump() {
	DEBUG_ENTRY

	if ((m_nArgv0Length == dump::length::BOARD) && (memcmp(m_Argv[0], dump::cmd::BOARD, dump::length::BOARD) == 0)) {
		h3_board_dump();
		return;
	}

	if ((m_nArgv0Length == dump::length::MMAP) && (memcmp(m_Argv[0], dump::cmd::MMAP, dump::length::MMAP) == 0)) {
		h3_dump_memory_mapping();
		return;
	}

	if ((m_nArgv0Length == dump::length::PLL) && (memcmp(m_Argv[0], dump::cmd::PLL, dump::length::PLL) == 0)) {
		h3_ccu_pll_dump();
		return;
	}

	if ((m_nArgv0Length == dump::length::LINKER) && (memcmp(m_Argv[0], dump::cmd::LINKER, dump::length::LINKER) == 0)) {
		arm_dump_memmap();
		return;
	}

	DEBUG_EXIT
}
#endif
