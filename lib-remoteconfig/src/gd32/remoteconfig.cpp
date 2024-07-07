/**
 * @file remoteconfig.cpp
 *
 */
/* Copyright (C) 2022-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "remoteconfig.h"
#include "display.h"

#include "gd32.h"

#include "debug.h"

void RemoteConfig::PlatformHandleTftpSet() {
	DEBUG_ENTRY

	if (m_bEnableTFTP) {
		bkp_data_write(BKP_DATA_1, 0xA5A5);
		Display::Get()->TextStatus("TFTP On ", CONSOLE_GREEN);
	} else {
		bkp_data_write(BKP_DATA_1, 0x0);
		Display::Get()->TextStatus("TFTP Off", CONSOLE_GREEN);
	}

	DEBUG_EXIT
}

void RemoteConfig::PlatformHandleTftpGet() {
	DEBUG_ENTRY

	m_bEnableTFTP = (bkp_data_read(BKP_DATA_1) == 0xA5A5);

	DEBUG_PRINTF("m_bEnableTFTP=%d", m_bEnableTFTP);
	DEBUG_EXIT
}
