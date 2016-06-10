/**
 * @file kernel.cpp
 *
 */
/*
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Copyright (C) 2014-2015  R. Stange <rsta2@o2online.de>
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <circle/net/socket.h>
#include <circle/net/ipaddress.h>
#include <circle/net/in.h>
#include <circle/string.h>
#include <Properties/propertiesfile.h>

#include "kernel.h"

#include "Properties/propertiesfile.h"

#include "oscws28xx.h"

#define PARTITION			"emmc1-1"
#define PROPERTIES_FILE		"network.txt"

#define OSC_SERVER_PORT		8000

static const char FromKernel[] = "kernel";

CKernel::CKernel(void) :
		m_Screen(m_Options.GetWidth(), m_Options.GetHeight()), m_Timer(
				&m_Interrupt), m_Logger(m_Options.GetLogLevel(), &m_Timer), m_DWHCI(
				&m_Interrupt, &m_Timer), m_EMMC(&m_Interrupt, &m_Timer,
				&m_ActLED), m_Properties(PROPERTIES_FILE, &m_FileSystem)
{
	m_ActLED.Blink(5);	// show we are alive
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize(void)
{
	boolean bOK = TRUE;

	if (bOK)
	{
		bOK = m_Screen.Initialize();
	}

	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice(m_Options.GetLogDevice(), FALSE);
		if (pTarget == 0)
		{
			pTarget = &m_Screen;
		}

		bOK = m_Logger.Initialize(pTarget);
	}

	if (bOK)
	{
		bOK = m_Interrupt.Initialize();
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize();
	}

	if (bOK)
	{
		bOK = m_DWHCI.Initialize();
	}

	if (bOK)
	{
		bOK = m_EMMC.Initialize();
	}

	if (bOK)
	{
		CDevice *pPartition = m_DeviceNameService.GetDevice(PARTITION, TRUE);
		if (pPartition != 0)
		{
			if (!m_FileSystem.Mount(pPartition))
			{
				m_Logger.Write(FromKernel, LogError, "Cannot mount partition: %s", PARTITION);
				m_Logger.Write(FromKernel, LogWarning, "Not able to read properties file, using defaults");
			}
			else
			{
				bOK = Configure ();	// At this point we can load the properties file
				// It is only failing for static ip configuration errors. For all others we use defaults.
			}
		}
		else
		{
			m_Logger.Write(FromKernel, LogError, "Partition not found: %s", PARTITION);
			m_Logger.Write(FromKernel, LogWarning, "Not able to read properties file, using defaults");
		}
	}

	if (bOK)
	{
		bOK = m_Net.Initialize();
	}
	else
	{
		m_Logger.Write(FromKernel, LogError, "Invalid network configuration");
	}

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

	CString IPString;
	m_Net.GetConfig ()->GetIPAddress ()->Format (&IPString);
	m_Logger.Write (FromKernel, LogNotice, "OSC Server running at %s:%u", (const char *) IPString, OSC_SERVER_PORT);

	new COSCWS28xx (&m_Net, &m_Interrupt, &m_Screen, &m_FileSystem, OSC_SERVER_PORT);

	for (unsigned nCount = 0; 1; nCount++)
	{
		m_Scheduler.Yield ();

		m_Screen.Rotor (0, nCount);
	}

	return ShutdownHalt;
}

boolean CKernel::Configure (void)
{
	if (!m_Properties.Load())
	{
		m_Logger.Write(FromKernel, LogWarning, "Error loading properties from %s (line %u)", PROPERTIES_FILE, m_Properties.GetErrorLine());
		m_Logger.Write(FromKernel, LogWarning, "Continuing with DHCP configuration");
	}
	else if (m_Properties.GetNumber("use_dhcp", 0) == 0)
	{
		const u8 *pAddress = m_Properties.GetIPAddress("ip_address");
		if (pAddress != 0)
		{
			m_Net.GetConfig()->SetIPAddress(pAddress);
		}
		else
		{
			return FALSE;
		}

		pAddress = m_Properties.GetIPAddress("net_mask");
		if (pAddress != 0)
		{
			m_Net.GetConfig()->SetNetMask(pAddress);
		}
		else
		{
			return FALSE;
		}

		pAddress = m_Properties.GetIPAddress("default_gateway");
		if (pAddress != 0)
		{
			m_Net.GetConfig()->SetDefaultGateway(pAddress);
		}
		else
		{
			return FALSE;
		}

		pAddress = m_Properties.GetIPAddress("name_server");
		if (pAddress != 0)
		{
			m_Net.GetConfig()->SetDNSServer(pAddress);
		}
		else
		{
			return FALSE;
		}
	}

	return TRUE;
}
