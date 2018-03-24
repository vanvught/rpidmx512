/**
 * @file kernel.cpp
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "kernel.h"

#include "hardwarecircle.h"
#include "networkcircle.h"
#include "ledblinkcircle.h"

#include "networkparams.h"

// DMX output
#include "dmxparams.h"
#include "circle/dmxsend.h"

// WS28xx output
#include "ws28xxstripeparams.h"
#include "ws28xxstripedmx.h"

#include "artnetnode.h"
#include "artnetparams.h"

#include "software_version.h"

#define PARTITION	"emmc1-1"
#define DRIVE		"SD:"

#define ARTNET_NODE_PORT		0x1936			///< The Port is always 0x1936

static const char FromKernel[] = "kernel";

CKernel::CKernel(void) :
		m_Screen(m_Options.GetWidth(),
		m_Options.GetHeight()),
		m_Timer(&m_Interrupt),
		m_Logger(m_Options.GetLogLevel(), &m_Timer),
		m_DWHCI(&m_Interrupt, &m_Timer),
		m_EMMC(&m_Interrupt, &m_Timer, &m_ActLED),
		m_DMX (&m_Interrupt),
		m_SPI (&m_Interrupt),
		m_BlinkTask()
{
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

	bOK = m_DMX.Initialize();

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

TShutdownMode CKernel::Run(void)
{
	HardwareCircle hw;
	NetworkCircle nw;
	ArtNetParams artnetparams;
	DMXParams dmxParams;
	WS28XXStripeParams deviceparams;
	uint8_t nHwTextLength;

	nw.Init(&m_Net);

	if (artnetparams.Load()) {
		artnetparams.Dump();
	}

	TOutputType output_type = artnetparams.GetOutputType();

	if (output_type == OUTPUT_TYPE_SPI) {
		if (deviceparams.Load()) {
			deviceparams.Dump();
			deviceparams.Set(&m_SPI);
		} else {
			m_Logger.Write(FromKernel, LogWarning, "Continuing with default LED configuration");
		}
	} else {
		output_type = OUTPUT_TYPE_DMX;
		if (dmxParams.Load()) {
			dmxParams.Dump();
			dmxParams.Set(&m_DMX);
		} else {
			m_Logger.Write(FromKernel, LogWarning, "Continuing with default DMX configuration");
		}
	}

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);

	ArtNetNode node;

	artnetparams.Set(&node);
	node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, artnetparams.GetUniverse());

	if (output_type == OUTPUT_TYPE_DMX) {
		node.SetOutput(&m_DMX);
		node.SetDirectUpdate(false);
	} else {
		node.SetOutput(&m_SPI);
		node.SetDirectUpdate(true);

		const uint16_t led_count = m_SPI.GetLEDCount();
		const uint8_t universe = artnetparams.GetUniverse();

		if (m_SPI.GetLEDType() == SK6812W) {
			if (led_count > 128) {
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, universe + 1);
			}
			if (led_count > 256) {
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, universe + 2);
			}
			if (led_count > 384) {
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, universe + 3);
			}
		} else {
			if (led_count > 170) {
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, universe + 1);
			}
			if (led_count > 340) {
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, universe + 2);
			}
			if (led_count > 510) {
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, universe + 3);
			}
		}
	}

	nw.Print();
	node.Print();

	if (output_type == OUTPUT_TYPE_DMX) {
		m_Logger.Write(FromKernel, LogNotice, "DMX Send parameters :");
		m_Logger.Write(FromKernel, LogNotice, " Break time   : %u", m_DMX.GetDmxBreakTime());
		m_Logger.Write(FromKernel, LogNotice, " MAB time     : %u", m_DMX.GetDmxMabTime());
		m_Logger.Write(FromKernel, LogNotice, " Refresh rate : %u", (unsigned) (1000000 / m_DMX.GetDmxPeriodTime()));
	} else {
		const TWS28XXType tType = m_SPI.GetLEDType();
		m_Logger.Write(FromKernel, LogNotice, "Led stripe parameters :");
		m_Logger.Write(FromKernel, LogNotice, " Type         : %s [%d]", WS28XXStripeParams::GetLedTypeString(tType), tType);
		m_Logger.Write(FromKernel, LogNotice, " Count        : %u", m_SPI.GetLEDCount());
	}

	node.Start();

	while (1) {
		node.HandlePacket();
		m_Scheduler.Yield();
	}

	return ShutdownHalt;
}
