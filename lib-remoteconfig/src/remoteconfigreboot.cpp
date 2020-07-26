/**
 * @file remoteconfigstatic.cpp
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

#include <stdio.h>
#include <cassert>

#include "remoteconfig.h"

#include "hardware.h"
#include "network.h"
#include "display.h"

#include "spiflashstore.h"

#include "debug.h"

void RemoteConfig::HandleReboot() {
	DEBUG_ENTRY

	m_bIsReboot = true;

	Display::Get()->SetSleep(false);

	while (SpiFlashStore::Get()->Flash())
		;

	Network::Get()->Shutdown();

	printf("Rebooting ...\n");

	Display::Get()->Cls();
	Display::Get()->TextStatus("Rebooting ...", Display7SegmentMessage::INFO_REBOOTING);

	Hardware::Get()->Reboot();

	DEBUG_EXIT
}
