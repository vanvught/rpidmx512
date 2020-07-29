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

#include "h3/shell.h"
#include "h3_uart0_debug.h"

#include "remoteconfig.h"
#include "network.h"

#include "hardware.h"
#include "firmwareversion.h"

#include "sscan.h"

#ifndef NDEBUG
# include "../debug/i2cdetect.h"
#endif

#include "debug.h"


static constexpr char sSetIP[] = "ip";
static constexpr auto SETIP_LENGTH = sizeof(sSetIP) - 1;

static constexpr char sSetHostname[] = "hostname";
static constexpr auto SETHOSTNAME_LENGTH = sizeof(sSetHostname) - 1;


using namespace shell;

void Shell::CmdReboot() {
	DEBUG_ENTRY
	RemoteConfig::Get()->Reboot();
	DEBUG_EXIT
}

void Shell::CmdInfo() {
	DEBUG_ENTRY
	uart0_printf("%s", FirmwareVersion::Get()->GetPrint());
	uart0_printf("Core Temperature: %.0f\n",Hardware::Get()->GetCoreTemperature());
	uart0_printf("Core Temperature Max: %.0f\n",Hardware::Get()->GetCoreTemperatureMax());
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
	size_t nArgLen = strlen(m_Argv[0]);
	
	if ((nArgLen == SETIP_LENGTH) && (memcmp(m_Argv[0], sSetIP, SETIP_LENGTH) == 0)) {					
		nArgLen = strlen(m_Argv[1]);	// set ip command, 2nd arg is IP address string	
		uint32_t nValue32;
		if (Sscan::IpAddress(m_Argv[1],nValue32) == Sscan::OK) {
			uart0_printf("New IP: %d.%d.%d.%d\n", IP2STR(nValue32));  // testing					
			Network::Get()->SetIp(nValue32);
		} else {
			uart0_puts("Usage: set ip x.x.x.x\n");
		}	
	} else if ((nArgLen == SETHOSTNAME_LENGTH) && (memcmp(m_Argv[0], sSetHostname, SETHOSTNAME_LENGTH) == 0)) {
		nArgLen = strlen(m_Argv[1]);	// hostname command, 2nd arg is hostname string		
		uart0_printf("New hostname: %s\n", m_Argv[1]);					
		if ((nArgLen) && (nArgLen <= TNetwork::NETWORK_HOSTNAME_SIZE)) {				
			Network::Get()->SetHostName(m_Argv[1]);	// FIXME do some validation ... check for special characters maybe?
		} else {
			uart0_puts("Usage: set hostname name\n");	
		}	  
	} else { // unknown set command
		uart0_puts("Usage: set [ip][hostname] [value]\n");
	}
	DEBUG_EXIT
}


#ifndef NDEBUG
void Shell::CmdI2cDetect() {
	DEBUG_ENTRY
	I2cDetect i2cdetect;
	DEBUG_EXIT
}
#endif
