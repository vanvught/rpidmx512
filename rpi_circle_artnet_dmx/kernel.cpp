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

#include <circle/interrupt.h>
#include <circle/string.h>
#include <circle/util.h>
#include <circle/machineinfo.h>
#include <circle/net/netsubsystem.h>
#include <circle/net/socket.h>
#include <circle/net/ipaddress.h>
#include <circle/net/in.h>

#include "Properties/propertiesfile.h"

#include "kernel.h"

#include "artnetnode.h"

#include "blinktask.h"
#include "dmxsend.h"
#include "spisend.h"

extern "C" {
extern void network_init(CNetSubSystem *);
}
#include "network.h"

#define PARTITION				"emmc1-1"

#define NETWORK_PROPERTIES_FILE	"network.txt"	///<
#define ARTNET_PROPERTIES_FILE	"artnet.txt"	///<
#define DMX_PROPERTIES_FILE		"params.txt"	///<
#define LEDS_PROPERTIES_FILE	"devices.txt"	///<

#define ARTNET_NODE_PORT		0x1936			///< The Port is always 0x1936

static const char FromKernel[] = "kernel";

static const char sLedTypes[7][8] = { "WS2801", "WS2811", "WS2812", "WS2812B", "WS2813", "SK6812", "SK6812W" };

enum TOuputType
{
	TOuputTypeDMX,
	TOuputTypeSPI
};

CKernel::CKernel(void) :
		m_Screen(m_Options.GetWidth(),
		m_Options.GetHeight()),
		m_Timer(&m_Interrupt),
		m_Logger(m_Options.GetLogLevel(), &m_Timer),
		m_DWHCI(&m_Interrupt, &m_Timer),
		m_EMMC(&m_Interrupt, &m_Timer, &m_ActLED),
		m_Properties(NETWORK_PROPERTIES_FILE, &m_FileSystem),
		m_DMX (&m_Interrupt),
		m_SPI (&m_Interrupt),
		m_HaveEMMC(false),
		m_OutputType(TOuputTypeDMX),
		m_BlinkTask(&m_ActLED, 0)
{
	m_ActLED.On();
}

CKernel::~CKernel(void)
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

	if (m_EMMC.Initialize())
	{
		CDevice *pPartition = m_DeviceNameService.GetDevice(PARTITION, TRUE);
		if (pPartition != 0)
		{
			if (!m_FileSystem.Mount(pPartition))
			{
				m_Logger.Write(FromKernel, LogError, "Cannot mount partition: %s", PARTITION);
				m_Logger.Write(FromKernel, LogWarning, "Not able to read properties files, using defaults");
			}
			else
			{
				bOK = Configure ();	// At this point we can load the properties file
				// It is only failing for static ip configuration errors. For all others we use defaults.
				m_HaveEMMC = true;
			}
		}
		else
		{
			m_Logger.Write(FromKernel, LogError, "Partition not found: %s", PARTITION);
			m_Logger.Write(FromKernel, LogWarning, "Not able to read properties files, using defaults");
		}
	}
	else
	{
		m_Logger.Write(FromKernel, LogError, "EMMC initialize failed");
		m_Logger.Write(FromKernel, LogWarning, "Not able to read properties files, using defaults");
	}

	if (bOK)
	{
		bOK = m_Net.Initialize();
	}
	else
	{
		m_Logger.Write(FromKernel, LogError, "Invalid network configuration");
	}

	bOK =  m_DMX.Initialize ();

	return bOK;
}

boolean CKernel::Configure (void)
{
	if (!m_Properties.Load())
	{
		m_Logger.Write(FromKernel, LogWarning, "Error loading properties from %s", NETWORK_PROPERTIES_FILE);
		m_Logger.Write(FromKernel, LogWarning, "Continuing with DHCP configuration");
	}
	else if (m_Properties.GetNumber("use_dhcp", 1) == 0)
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

	CPropertiesFile DmxProperties(DMX_PROPERTIES_FILE, &m_FileSystem);
	if (!DmxProperties.Load())
	{
		m_Logger.Write(FromKernel, LogWarning, "Error loading properties from %s", DMX_PROPERTIES_FILE);
		m_Logger.Write(FromKernel, LogWarning, "Continuing with default DMX configuration");
	}
	else
	{
		m_DMX.SetBreakTime(DmxProperties.GetNumber("dmxsend_break_time", DMX_TRANSMIT_BREAK_TIME_MIN));
		m_DMX.SetMabTime(DmxProperties.GetNumber("dmxsend_mab_time", DMX_TRANSMIT_MAB_TIME_MIN));
		uint32_t period = (uint32_t) 0;

		uint8_t refresh_rate = (uint8_t) DmxProperties.GetNumber("dmxsend_refresh_rate", DMX_TRANSMIT_REFRESH_RATE_DEFAULT);
		if (refresh_rate != (uint8_t) 0)
		{
			period = (uint32_t) (1E6 / refresh_rate);
		}

		m_DMX.SetPeriodTime(period);
	}

	CPropertiesFile LedsProperties(LEDS_PROPERTIES_FILE, &m_FileSystem);
	if (!LedsProperties.Load())
	{
		m_Logger.Write(FromKernel, LogWarning, "Error loading properties from %s", LEDS_PROPERTIES_FILE);
		m_Logger.Write(FromKernel, LogWarning, "Continuing with DMX configuration");
	}
	else
	{
		const char *pType = LedsProperties.GetString("led_type");
		if (pType == 0)
		{
			m_Logger.Write(FromKernel, LogWarning, "No led_type configured");
			m_Logger.Write(FromKernel, LogNotice, "Using default type : %s", sLedTypes[m_SPI.GetLEDType()]);
		}
		else
		{
			if (strcmp(pType, sLedTypes[WS2801]) == 0)
			{
				m_SPI.SetLEDType(WS2801);
			}
			else if (strcmp(pType, sLedTypes[WS2811]) == 0)
			{
				m_SPI.SetLEDType(WS2811);
			}
			else if (strcmp(pType, sLedTypes[WS2812]) == 0)
			{
				m_SPI.SetLEDType(WS2812);
			}
			else if (strcmp(pType, sLedTypes[WS2812B]) == 0)
			{
				m_SPI.SetLEDType(WS2812B);
			}
			else if (strcmp(pType, sLedTypes[WS2813]) == 0)
			{
				m_SPI.SetLEDType(WS2813);
			}
			else if (strcmp(pType, sLedTypes[SK6812]) == 0)
			{
				m_SPI.SetLEDType(SK6812);
			}
			else if (strcmp(pType, sLedTypes[SK6812W]) == 0)
			{
				m_SPI.SetLEDType(SK6812W);
			}
			else
			{
				m_Logger.Write(FromKernel, LogWarning, "Wrong led_type configured (%s)", pType);
				m_Logger.Write(FromKernel, LogNotice, "Using default type : %s", sLedTypes[m_SPI.GetLEDType()]);
			}
		}

		unsigned nLEDCount = LedsProperties.GetNumber("led_count");
		if (nLEDCount == 0)
		{
			m_Logger.Write(FromKernel, LogWarning, "No or wrong led_count configured");
			m_Logger.Write(FromKernel, LogNotice, "Using default led count : %d", m_SPI.GetLEDCount());
		}
		else
		{
			m_SPI.SetLEDCount(nLEDCount);
		}

	}

	return TRUE;
}

TShutdownMode CKernel::Run(void)
{
	m_Logger.Write(FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);
	m_Logger.Write(FromKernel, LogNotice, "%s %dMB (%s)", m_MachineInfo.GetMachineName(), m_MachineInfo.GetRAMSize(),  m_MachineInfo.GetSoCName());

	uint8_t NetSwitch = 0;
	uint8_t SubnetSwitch = 0;
	uint8_t UniverseSwitch = 0;

	if (m_HaveEMMC)
	{
		CPropertiesFile ArtnetProperties(ARTNET_PROPERTIES_FILE, &m_FileSystem);
		if (!ArtnetProperties.Load())
		{
			m_Logger.Write(FromKernel, LogWarning, "Error loading properties from %s", ARTNET_PROPERTIES_FILE);
			m_Logger.Write(FromKernel, LogWarning, "Continuing with default Art-Net node configuration");

		}
		else
		{
			NetSwitch = ArtnetProperties.GetNumber("net", 0);
			SubnetSwitch = ArtnetProperties.GetNumber("subnet", 0);
			UniverseSwitch = ArtnetProperties.GetNumber("universe", 0);
			// output device DMX (default) or SPI
			const char *pOutputType = ArtnetProperties.GetString("output");
			if (pOutputType == 0)
			{
				m_Logger.Write(FromKernel, LogWarning, "No output configured");
				m_Logger.Write(FromKernel, LogNotice, "Using default output : DMX");
			}
			else
			{
				if (strcmp(pOutputType, "dmx") == 0)
				{
					m_OutputType = TOuputTypeDMX;
				}
				else if (strcmp(pOutputType, "spi") == 0)
				{
					m_OutputType = TOuputTypeSPI;
				}
				else
				{
					m_Logger.Write(FromKernel, LogWarning, "Wrong output configured (%s)", pOutputType);
					m_Logger.Write(FromKernel, LogNotice, "Using default output : DMX");
				}
			}
		}
	}

	network_init(&m_Net);

	ArtNetNode node;

	node.SetLedBlink(&m_BlinkTask);

	if (m_OutputType == TOuputTypeDMX)
	{
		node.SetOutput(&m_DMX);
		node.SetDirectUpdate(false);
		node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, UniverseSwitch);
	}
	else if (m_OutputType == TOuputTypeSPI)
	{
		node.SetOutput(&m_SPI);
		node.SetDirectUpdate(false);
		node.SetUniverseSwitch(0, ARTNET_OUTPUT_PORT, UniverseSwitch);

		const unsigned LEDCount = m_SPI.GetLEDCount();

		if (m_SPI.GetLEDType() == SK6812W)
		{
			if (LEDCount > 128)
			{
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, UniverseSwitch + 1);
			}
			if (LEDCount > 256)
			{
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, UniverseSwitch + 2);
			}
			if (LEDCount > 384)
			{
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, UniverseSwitch + 3);
			}
		}
		else
		{
			if (LEDCount > 170)
			{
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(1, ARTNET_OUTPUT_PORT, UniverseSwitch + 1);
			}
			if (LEDCount > 340)
			{
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(2, ARTNET_OUTPUT_PORT, UniverseSwitch + 2);
			}
			if (LEDCount > 510)
			{
				node.SetDirectUpdate(true);
				node.SetUniverseSwitch(3, ARTNET_OUTPUT_PORT, UniverseSwitch + 3);
			}
		}
	}

	node.SetNetSwitch(NetSwitch);
	node.SetSubnetSwitch(SubnetSwitch);

	m_Logger.Write(FromKernel, LogNotice, "Node configuration :");
	const uint8_t *FirmwareVersion = node.GetSoftwareVersion();
	m_Logger.Write(FromKernel, LogNotice, " Firmware   : v%u.%u", FirmwareVersion[0], FirmwareVersion[1]);
	CString IPString;
	m_Net.GetConfig()->GetIPAddress()->Format(&IPString);
	m_Logger.Write(FromKernel, LogNotice, " Running at : %s:%u ", (const char *) IPString, ARTNET_NODE_PORT);
	m_Net.GetConfig()->GetBroadcastAddress()->Format(&IPString);
	m_Logger.Write(FromKernel, LogNotice, " Broadcast  : %s", (const char *) IPString);
	m_Logger.Write(FromKernel, LogNotice, " Short name : %s", node.GetShortName());
	m_Logger.Write(FromKernel, LogNotice, " Long name  : %s", node.GetLongName());
	m_Logger.Write(FromKernel, LogNotice, " Net        : %u", node.GetNetSwitch());
	m_Logger.Write(FromKernel, LogNotice, " Sub-Net    : %u", node.GetSubnetSwitch());
	m_Logger.Write(FromKernel, LogNotice, " Universe   : %u", node.GetUniverseSwitch(0));

	if (m_OutputType == TOuputTypeDMX)
	{
		m_Logger.Write(FromKernel, LogNotice, "DMX Send parameters :");
		m_Logger.Write(FromKernel, LogNotice, " Break time   : %u", m_DMX.GetBreakTime());
		m_Logger.Write(FromKernel, LogNotice, " MAB time     : %u", m_DMX.GetMabTime());
		m_Logger.Write(FromKernel, LogNotice, " Refresh rate : %u", (unsigned) (1E6 / m_DMX.GetPeriodTime()));
	}
	else if (m_OutputType == TOuputTypeSPI)
	{
		m_Logger.Write(FromKernel, LogNotice, "Led stripe parameters :");
		m_Logger.Write(FromKernel, LogNotice, " Type         : %s", sLedTypes[m_SPI.GetLEDType()]);
		m_Logger.Write(FromKernel, LogNotice, " Count        : %u", m_SPI.GetLEDCount());
	}

	node.Start();

	while(1)
	{
		node.HandlePacket();
		m_Scheduler.Yield();
	}

	return ShutdownHalt;
}
