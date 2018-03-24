/**
 * @file kernel.cpp
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <circle/interrupt.h>
#include <circle/string.h>
#include <circle/util.h>
#include <circle/machineinfo.h>
#include <circle/net/netsubsystem.h>
#include <circle/net/ipaddress.h>

#include <fatfs/ff.h>
#include <ledblinkcircle.h>

#include "kernel.h"

#include "hardwarecircle.h"

#include "networkcircle.h"
#include "networkparams.h"

#include "oscparams.h"
#include "oscws28xx.h"

#include "software_version.h"

#define PARTITION	"emmc1-1"
#define DRIVE		"SD:"

static const char FromKernel[] = "kernel";

CKernel::CKernel(void) :
		m_Screen(m_Options.GetWidth(),
		m_Options.GetHeight()),
		m_Timer(&m_Interrupt),
		m_Logger(m_Options.GetLogLevel(), &m_Timer),
		m_DWHCI(&m_Interrupt, &m_Timer),
		m_EMMC(&m_Interrupt, &m_Timer, &m_ActLED),
		m_BlinkTask() {
	m_ActLED.On();
}

CKernel::~CKernel(void) {
}

boolean CKernel::Initialize(void) {
	boolean bOK = TRUE;

	if (bOK) {
		bOK = m_Screen.Initialize();
	}

	if (bOK) {
		CDevice *pTarget = m_DeviceNameService.GetDevice(m_Options.GetLogDevice(), FALSE);
		if (pTarget == 0) {
			pTarget = &m_Screen;
		}

		bOK = m_Logger.Initialize(pTarget);
	}

	if (bOK) {
		bOK = m_Interrupt.Initialize();
	}

	if (bOK) {
		bOK = m_Timer.Initialize();
	}

	if (bOK) {
		bOK = m_DWHCI.Initialize();
	}

	if (m_EMMC.Initialize()) {
		CDevice *pPartition = m_DeviceNameService.GetDevice(PARTITION, TRUE);
		if (pPartition != 0) {
			// Mount file system
			if (f_mount(&m_FileSystem, DRIVE, 1) != FR_OK) {
				m_Logger.Write(FromKernel, LogPanic, "Cannot mount drive: %s", DRIVE);
			} else {
				m_Logger.Write(FromKernel, LogNotice, "Drive mount: %s", DRIVE);
				bOK = Configure();// At this point we can load the properties file
			}
		} else {
			m_Logger.Write(FromKernel, LogError, "Partition not found: %s", PARTITION);
			m_Logger.Write(FromKernel, LogWarning, "Not able to read properties files, using defaults");
		}
	} else {
		m_Logger.Write(FromKernel, LogError, "EMMC initialize failed");
		m_Logger.Write(FromKernel, LogWarning, "Not able to read properties files, using defaults");
	}

	if (bOK) {
		bOK = m_Net.Initialize();
	} else {
		m_Logger.Write(FromKernel, LogError, "Invalid network configuration");
	}

	return bOK;
}

boolean CKernel::Configure(void) {
	NetworkParams networkParams;

	if (networkParams.Load()) {
		networkParams.Dump();
		if (!networkParams.isDhcpUsed()) {
			uint32_t ip;

			if ((ip = networkParams.GetIpAddress()) != 0) {
				m_Net.GetConfig()->SetIPAddress(ip);
			} else {
				return FALSE;
			}

			if ((ip = networkParams.GetNetMask()) != 0) {
				m_Net.GetConfig()->SetNetMask(ip);
			} else {
				return FALSE;
			}

			if ((ip = networkParams.GetDefaultGateway()) != 0) {
				m_Net.GetConfig()->SetDefaultGateway(ip);
			} else {
				return FALSE;
			}

			if ((ip = networkParams.GetNameServer()) != 0) {
				m_Net.GetConfig()->SetDNSServer(ip);
			} else {
				return FALSE;
			}
		}
	} else {
		m_Logger.Write(FromKernel, LogWarning, "Continuing with DHCP configuration");
	}

	return TRUE;
}

TShutdownMode CKernel::Run(void) {
	HardwareCircle hw;
	NetworkCircle nw;
	OSCParams oscparms;
	uint8_t nHwTextLength;

	if (oscparms.Load()) {
		oscparms.Dump();
	}

	const uint16_t nIncomingPort = oscparms.GetIncomingPort();
	const uint16_t nOutgoingPort = oscparms.GetOutgoingPort();

	nw.Init(&m_Net);
	nw.Begin(nIncomingPort);

	CString IPString;
	m_Net.GetConfig ()->GetIPAddress ()->Format (&IPString);

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);
	printf("Ethernet OSC Pixel controller, Incoming port: %d, Outgoing port: %d", nIncomingPort, nOutgoingPort);

	COSCWS28xx oscws28xx(&m_Interrupt, &m_Screen, nOutgoingPort);

	oscws28xx.Start();
	m_BlinkTask.SetFrequency(1);

	for (;;) {
		oscws28xx.Run();
		m_Scheduler.Yield();
	}

	return ShutdownHalt;
}
