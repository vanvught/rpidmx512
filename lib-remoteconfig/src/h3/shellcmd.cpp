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
	uart0_printf("IP: %d.%d.%d.%d\n", IP2STR(Network::Get()->GetIp()));
	DEBUG_EXIT
}

void Shell::CmdSet() {
	DEBUG_ENTRY
#ifndef NDEBUG
	uart0_printf("m_Argv[0..1]: %s %s\n", m_Argv[0], m_Argv[1]);
#endif

	// TOOD We know the m_Argv[] length in ValidateArg. Let's store it in member variable?

	const auto nArgv0Length = strlen(m_Argv[0]);

	if ((nArgv0Length == length::SET_IP) && (memcmp(m_Argv[0], cmd::SET_IP, length::SET_IP) == 0)) {
		in_addr group_ip;
		if (inet_aton(m_Argv[1], &group_ip)) {
			DEBUG_PRINTF("New IP: " IPSTR, IP2STR(group_ip.s_addr));
			Network::Get()->SetIp(group_ip.s_addr);
		} else {
			uart0_puts("Usage: set ip x.x.x.x\n");
		}

		return;
	}

	if ((nArgv0Length == length::SET_HOSTNAME) && (memcmp(m_Argv[0], cmd::SET_HOSTNAME, length::SET_HOSTNAME) == 0)) {
		const auto nArgv1Length = strlen(m_Argv[1]);	// 2nd arg is hostname string

		DEBUG_PRINTF("New hostname: %s", m_Argv[1]);

		if ((nArgv1Length != 0) && (nArgv1Length <= TNetwork::NETWORK_HOSTNAME_SIZE)) {
			Network::Get()->SetHostName(m_Argv[1]);
		} else {
			uart0_puts("Usage: set hostname name\n");	
		}

		return;
	}

	uint32_t nLength = nArgv0Length;
	if (RemoteConfig::GetIndex(m_Argv[0], nLength) < TXT_FILE_LAST) {
		DEBUG_PUTS(m_Argv[0]);
		// TODO
		return;
	}

	uart0_puts("Usage: set [ip][hostname][name\'.txt\'] [value]\n");
	DEBUG_EXIT
}


#ifndef NDEBUG
void Shell::CmdI2cDetect() {
	DEBUG_ENTRY
	I2cDetect i2cdetect;
	DEBUG_EXIT
}
#endif
