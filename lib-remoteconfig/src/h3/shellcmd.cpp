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

#include <h3/shell.h>
#include "remoteconfig.h"

#include "hardware.h"
#include "firmwareversion.h"

#ifndef NDEBUG
# include "../debug/i2cdetect.h"
#endif

#include "debug.h"

void Shell::CmdReboot() {
	DEBUG_ENTRY
	RemoteConfig::Get()->Reboot();
	DEBUG_EXIT
}

void Shell::CmdInfo() {
	DEBUG_ENTRY
	FirmwareVersion::Get()->Print("");
	printf("Core Temperature: %.2f\n",Hardware::Get()->GetCoreTemperature());
	printf("Core Temperature Max: %.2f\n",Hardware::Get()->GetCoreTemperatureMax());
	printf("Uptime: %d\n", Hardware::Get()->GetUpTime());
	printf("Hostname: %s\n", Network::Get()->GetHostName());
	printf("IP: %d.%d.%d.%d\n", IP2STR(Network::Get()->GetIp()));
	DEBUG_EXIT
}

#ifndef NDEBUG
void Shell::CmdI2cDetect() {
	DEBUG_ENTRY
	I2cDetect i2cdetect;
	DEBUG_EXIT
}
#endif
