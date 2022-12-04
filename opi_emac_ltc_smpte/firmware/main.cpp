/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cassert>

#include "hardware.h"
#include "network.h"
#include "ledblink.h"
#include "display.h"

#include "ltcparams.h"
#include "ltcdisplayparams.h"
#include "ltcdisplayrgb.h"
#include "ltcdisplaymax7219.h"
#include "ltc7segment.h"
#include "ltcmidisystemrealtime.h"
#include "ltcetc.h"
#include "ltcetcparams.h"

#include "artnetnode.h"
#include "artnetparams.h"
#include "artnetmsgconst.h"

#include "artnetconst.h"
#include "networkconst.h"

#include "midi.h"
#include "rtpmidi.h"
#include "midiparams.h"

#include "mdnsservices.h"
#if defined (ENABLE_HTTPD)
# include "httpd/httpd.h"
#endif

#include "tcnet.h"
#include "tcnetparams.h"
#include "tcnettimecode.h"
#include "tcnetdisplay.h"

#include "ntpserver.h"

#include "mcpbuttons.h"
#include "ltcoscserver.h"

#include "ltcsourceconst.h"
#include "ltcsource.h"

#include "artnetreader.h"
#include "ltcreader.h"
#include "ltcsender.h"
#include "midireader.h"
#include "tcnetreader.h"
#include "ltcgenerator.h"
#include "rtpmidireader.h"
#include "systimereader.h"
#include "ltcetcreader.h"
#include "ltcoutputs.h"

#include "flashcodeinstall.h"

#include "configstore.h"
#include "storenetwork.h"
#include "storeltc.h"
#include "storeltcdisplay.h"
#include "storeltcetc.h"
#include "storeartnet.h"
#include "storetcnet.h"
#include "storeremoteconfig.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

// LLRP Only Device
#include "rdmnetllrponly.h"
#include "rdm_e120.h"
// LLRP Handler
#include "factorydefaults.h"

// System Time
#include "ntpclient.h"
#include "gpstimeclient.h"
#include "gpsparams.h"
#include "storegps.h"

#include "firmwareversion.h"
#include "software_version.h"

#if defined(ENABLE_SHELL)
# include "shell/shell.h"
#endif


void Hardware::RebootHandler() {
//	switch (m_tSource) {
//	case ltc::source::TCNET:
//		TCNet::Get()->Stop();
//		break;
//	default:
//		break;
//	}

	HwClock::Get()->SysToHc();

	if (!g_ltc_ptLtcDisabledOutputs.bMax7219) {
		LtcDisplayMax7219::Get()->Init(2); // TODO WriteChar
	}

	if ((!g_ltc_ptLtcDisabledOutputs.bWS28xx) || (!g_ltc_ptLtcDisabledOutputs.bRgbPanel)) {
		LtcDisplayRgb::Get()->WriteChar('-');
	}

	if (!RemoteConfig::Get()->IsReboot()) {
		Display::Get()->SetSleep(false);

		while (ConfigStore::Get()->Flash())
			;

		Network::Get()->Shutdown();

		printf("Rebooting ...\n");

		Display::Get()->Cls();
		Display::Get()->TextStatus("Rebooting ...", Display7SegmentMessage::INFO_REBOOTING);
	}
}

extern "C" {
void h3_cpu_off(uint8_t);

void notmain(void) {
	Hardware hw;
	Network nw;
	LedBlink lb;
	Display display(4);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
#if defined(ENABLE_SHELL)
	Shell shell;
#endif

	FlashCodeInstall spiFlashInstall;
	ConfigStore configStore;

	Ltc7segment leds;

	fw.Print("LTC SMPTE");

	hw.SetLed(hardware::LedStatus::ON);

	display.ClearLine(1);
	display.ClearLine(2);

	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, Display7SegmentMessage::INFO_NETWORK_INIT, CONSOLE_YELLOW);

	StoreNetwork storeNetwork;
	nw.SetNetworkStore(&storeNetwork);
	nw.Init(&storeNetwork);
	nw.Print();

	NtpClient ntpClient;
	ntpClient.Start();
	ntpClient.Print();

	StoreLtc storeLtc;
	LtcParams ltcParams(&storeLtc);

	struct ltc::TimeCode tStartTimeCode;
	struct ltc::TimeCode tStopTimeCode;

	if (ltcParams.Load()) {
		ltcParams.Dump();
		ltcParams.Set(&tStartTimeCode, &tStopTimeCode);
	}

	LtcReader ltcReader;
	MidiReader midiReader;
	ArtNetReader artnetReader;
	TCNetReader tcnetReader;
	RtpMidiReader rtpMidiReader;
	SystimeReader sysTimeReader(ltcParams.GetFps());
	LtcEtcReader ltcEtcReader;

	ltc::Source ltcSource = ltcParams.GetSource();

	StoreLtcDisplay storeLtcDisplay;
	LtcDisplayParams ltcDisplayParams(&storeLtcDisplay);

	if (ltcDisplayParams.Load()) {
		ltcDisplayParams.Dump();
		display.SetContrast(ltcDisplayParams.GetOledIntensity());
	}

	LtcDisplayMax7219 ltcDdisplayMax7219(ltcDisplayParams.GetMax7219Type());
	LtcDisplayRgb ltcDisplayRgb(ltcParams.IsRgbPanelEnabled() ? ltcdisplayrgb::Type::RGBPANEL : ltcdisplayrgb::Type::WS28XX, ltcDisplayParams.GetWS28xxDisplayType());

	/**
	 * Select the source using buttons/rotary
	 */

	const auto IsAutoStart = ((ltcSource == ltc::Source::SYSTIME) && ltcParams.IsAutoStart());

	McpButtons sourceSelect(ltcSource, ltcParams.IsAltFunction(), ltcParams.GetSkipSeconds(), !ltcDisplayParams.IsRotaryFullStep());

	if (sourceSelect.Check() && !IsAutoStart) {
		while (sourceSelect.Wait(ltcSource, tStartTimeCode, tStopTimeCode)) {
			nw.Run();
			lb.Run();
		}
	}


	/**
	 * From here work with source selection
	 */

	Display::Get()->Status(Display7SegmentMessage::INFO_NONE);

	LtcOutputs ltcOutputs(ltcSource, ltcParams.IsShowSysTime());

	if (!g_ltc_ptLtcDisabledOutputs.bMax7219) {
		DEBUG_PUTS("");
		ltcDdisplayMax7219.Init(ltcDisplayParams.GetMax7219Intensity());
		ltcDdisplayMax7219.Print();
	}

	if ((!g_ltc_ptLtcDisabledOutputs.bWS28xx) || (!g_ltc_ptLtcDisabledOutputs.bRgbPanel)) {
		ltcDisplayParams.Set(&ltcDisplayRgb);

		if (!g_ltc_ptLtcDisabledOutputs.bRgbPanel) {
			ltcDisplayRgb.Init();

			char aInfoMessage[8 + 1];
			uint32_t nLength;
			const char *p = ltcDisplayParams.GetInfoMessage(nLength);
			assert(nLength == 8);
			memcpy(aInfoMessage, p, 8);
			aInfoMessage[8] = '\0';
			ltcDisplayRgb.ShowInfo(aInfoMessage);
		} else {
			ltcDisplayParams.Set(&ltcDisplayRgb);
			ltcDisplayRgb.Init(ltcDisplayParams.GetWS28xxLedType());
		}

		ltcDisplayRgb.Print();
	}

	if (g_ltc_ptLtcDisabledOutputs.bRgbPanel) {
		for (uint8_t nCpuNumber = 1; nCpuNumber < 4; nCpuNumber++) {
			h3_cpu_off(nCpuNumber);
		}
	}

	/**
	 * Art-Net
	 */

	const auto bRunArtNet = ((ltcSource == ltc::Source::ARTNET) || (!g_ltc_ptLtcDisabledOutputs.bArtNet));

	ArtNetNode node;
	StoreArtNet storeArtnet;

	if (bRunArtNet) {
		ArtNetParams artnetparams(&storeArtnet);

		if (artnetparams.Load()) {
			artnetparams.Dump();
			artnetparams.Set();
		}

		node.SetArtNetStore(StoreArtNet::Get());
		node.SetShortName("LTC SMPTE Node");

		node.SetTimeCodeIp(ltcParams.GetTimecodeIp());

		if (!ltcParams.IsTimeSyncDisabled()) {
			//TODO Send ArtTimeSync
		}

		node.Start();
		node.Print();
	}

	/**
	 * TCNet
	 */

	const auto bRunTCNet = (ltcSource == ltc::Source::TCNET);

	TCNet tcnet(TCNET_TYPE_SLAVE);
	StoreTCNet storetcnet;

	if (bRunTCNet) {
		TCNetParams tcnetparams(&storetcnet);

		if (tcnetparams.Load()) {
			tcnetparams.Set(&tcnet);
			tcnetparams.Dump();
		}

		tcnet.Start();
		tcnet.Print();
	}

	/**
	 * MIDI
	 */

	Midi midi;

	if ((ltcSource != ltc::Source::MIDI) && (!g_ltc_ptLtcDisabledOutputs.bMidi)) {
		midi.Init(midi::Direction::OUTPUT);
	}

	if ((ltcSource == ltc::Source::MIDI) || (!g_ltc_ptLtcDisabledOutputs.bMidi)) {
		midi.Print();
	}

	/**
	 * RTP-MIDI
	 */

	const auto bRunRtpMidi = ((ltcSource == ltc::Source::APPLEMIDI) || (!g_ltc_ptLtcDisabledOutputs.bRtpMidi));

	RtpMidi rtpMidi;

	if (bRunRtpMidi) {
		rtpMidi.Start();
		rtpMidi.AddServiceRecord(nullptr, MDNS_SERVICE_CONFIG, 0x2905);
	}

	/**
	 * ETC
	 */

	const auto bRunLtcEtc = (ltcSource == ltc::Source::ETC);

	LtcEtc ltcEtc;
	StoreLtcEtc storeLtcEtc;

	if (bRunLtcEtc || (!g_ltc_ptLtcDisabledOutputs.bEtc)) {
		LtcEtcParams ltcEtcParams(&storeLtcEtc);

		if (ltcEtcParams.Load()) {
			ltcEtcParams.Set();
			ltcEtcParams.Dump();
		}

		ltcEtc.Start();
		ltcEtc.Print();
	}

	/**
	 * LTC Sender
	 */

	LtcSender ltcSender(ltcParams.GetVolume());

	if ((ltcSource != ltc::Source::LTC) && (!g_ltc_ptLtcDisabledOutputs.bLtc)) {
		ltcSender.Start();
	}

	/**
	 * The OSC Server is running when enabled AND source = TCNet OR Internal OR System-Time
	 */

	const auto bRunOSCServer = ((ltcSource == ltc::Source::TCNET || ltcSource == ltc::Source::INTERNAL || ltcSource == ltc::Source::SYSTIME) && ltcParams.IsOscEnabled());

	LtcOscServer oscServer;

	if (bRunOSCServer) {
		bool isSet;
		const uint16_t nPort = ltcParams.GetOscPort(isSet);

		if (isSet) {
			oscServer.SetPortIncoming(nPort);
		}

		oscServer.Start();
		oscServer.Print();
		if (bRunRtpMidi) {
			rtpMidi.AddServiceRecord(nullptr, MDNS_SERVICE_OSC, oscServer.GetPortIncoming(), mdns::Protocol::UDP, "type=server");
		}
	}

	/**
	 * The GPS Time client is running when enabled AND source = System-Time AND RGB Panel is disabled
	 * The NTP Client is stopped.
	 */

	StoreGPS storeGPS;
	GPSParams gpsParams(&storeGPS);

	if (ltcSource == ltc::Source::SYSTIME) {
		if (gpsParams.Load()) {
			gpsParams.Dump();
		}
	}

	const auto bRunGpsTimeClient = (gpsParams.IsEnabled() && (ltcSource == ltc::Source::SYSTIME) && g_ltc_ptLtcDisabledOutputs.bRgbPanel);

	GPSTimeClient gpsTimeClient(gpsParams.GetUtcOffset(), gpsParams.GetModule());

	if (bRunGpsTimeClient) {
		ntpClient.Stop();

		gpsTimeClient.Start();
		gpsTimeClient.Print();
	}

	/**
	 * When the NTP Server is enabled then the NTP Client is not running (stopped).
	 */

	const auto bRunNtpServer = ltcParams.IsNtpEnabled();

	NtpServer ntpServer(ltcParams.GetYear(), ltcParams.GetMonth(), ltcParams.GetDay());

	if (bRunNtpServer) {
		ntpClient.Stop();

		ntpServer.SetTimeCode(&tStartTimeCode);
		ntpServer.Start();
		ntpServer.Print();
		if (bRunRtpMidi) {
			rtpMidi.AddServiceRecord(nullptr, MDNS_SERVICE_NTP, NTP_UDP_PORT, mdns::Protocol::UDP, "type=server");
		}
	}

	if (bRunRtpMidi) {
#if defined (ENABLE_HTTPD)
		rtpMidi.AddServiceRecord(nullptr, MDNS_SERVICE_HTTP, 80, mdns::Protocol::TCP, "node=LTC SMPTE");
#endif
		rtpMidi.Print();
	}

	/**
	 * LTC Generator
	 */

	LtcGenerator ltcGenerator(&tStartTimeCode, &tStopTimeCode, ltcParams.GetSkipFree());

	/**
	 * MIDI output System Real Time
	 */

	LtcMidiSystemRealtime ltcMidiSystemRealtime;

	/**
	 * The UDP request handler is running when source is NOT MIDI AND source is NOT RTP-MIDI
	 * AND when MIDI output is NOT disabled OR the RTP-MIDI is NOT disabled.
	 */

	const auto bRunMidiSystemRealtime = (ltcSource != ltc::Source::MIDI) && (ltcSource != ltc::Source::APPLEMIDI) && ((!g_ltc_ptLtcDisabledOutputs.bRtpMidi) || (!g_ltc_ptLtcDisabledOutputs.bMidi));

	if (bRunMidiSystemRealtime) {
		ltcMidiSystemRealtime.Start();
	}

	/**
	 * Start the reader
	 */

	switch (ltcSource) {
	case ltc::Source::ARTNET:
		node.SetTimeCodeHandler(&artnetReader);
		artnetReader.Start();
		break;
	case ltc::Source::MIDI:
		midiReader.Start();
		break;
	case ltc::Source::TCNET:
		tcnet.SetTimeCodeHandler(&tcnetReader);
		tcnetReader.Start();
		break;
	case ltc::Source::INTERNAL:
		ltcGenerator.Start();
		ltcGenerator.Print();
		break;
	case ltc::Source::APPLEMIDI:
		rtpMidi.SetHandler(&rtpMidiReader);
		rtpMidiReader.Start();
		break;
	case ltc::Source::ETC:
		ltcEtc.SetHandler(&ltcEtcReader);
		ltcEtcReader.Start();
		break;
	case ltc::Source::SYSTIME:
		sysTimeReader.Start(ltcParams.IsAutoStart());
		break;
	default:
		ltcReader.Start();
		break;
	}

	RDMNetLLRPOnly rdmNetLLRPOnly("LTC SMPTE");

	rdmNetLLRPOnly.GetRDMNetDevice()->SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
	rdmNetLLRPOnly.GetRDMNetDevice()->SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	rdmNetLLRPOnly.Init();
	rdmNetLLRPOnly.Start();
	rdmNetLLRPOnly.Print();

	node.SetRdmUID(rdmNetLLRPOnly.GetRDMNetDevice()->GetUID(), true);

#if defined (ENABLE_HTTPD)
	HttpDaemon httpDaemon;
	httpDaemon.Start();
#endif

	RemoteConfig remoteConfig(remoteconfig::Node::LTC, remoteconfig::Output::TIMECODE, 1U + static_cast<uint32_t>(ltcSource));
	RemoteConfigParams remoteConfigParams(new StoreRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Dump();
		remoteConfigParams.Set(&remoteConfig);
	}

	while (configStore.Flash())
		;

	printf("Source : %s\n", LtcSourceConst::NAME[static_cast<uint32_t>(ltcSource)]);

	ltc::source::show(ltcSource, bRunGpsTimeClient);

	if (!g_ltc_ptLtcDisabledOutputs.bRgbPanel) {
		ltcDisplayRgb.ShowSource(ltcSource);
	}

	ltcOutputs.Print();

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();

		// Run the reader
		switch (ltcSource) {
		case ltc::Source::LTC:
			ltcReader.Run();
			break;
		case ltc::Source::ARTNET:
			artnetReader.Run();		// Handles MIDI Quarter Frame output messages
			break;
		case ltc::Source::MIDI:
			midiReader.Run();
			break;
		case ltc::Source::TCNET:
			tcnetReader.Run();		// Handles MIDI Quarter Frame output messages
			break;
		case ltc::Source::INTERNAL:
			ltcGenerator.Run();		// Handles MIDI Quarter Frame output messages
			break;
		case ltc::Source::APPLEMIDI:
			rtpMidiReader.Run();	// Handles status led
			break;
		case ltc::Source::ETC:
			ltcEtcReader.Run();		// Handles MIDI Quarter Frame output messages
			break;
		case ltc::Source::SYSTIME:
			sysTimeReader.Run();
			if (!bRunGpsTimeClient) {
				if (bRunNtpServer) {
					HwClock::Get()->Run(true);
				} else {
					HwClock::Get()->Run(NtpClient::Get()->GetStatus() == ntpclient::Status::FAILED); // No need to check for STOPPED
				}
			} else {
				gpsTimeClient.Run();
			}
			break;
		default:
			break;
		}

		if (bRunArtNet) {
			node.Run();
		}

		if (bRunTCNet) {
			tcnet.Run();
		}

		if (bRunRtpMidi) {
			rtpMidi.Run();
		}

		if (bRunLtcEtc) {
			ltcEtc.Run();
		}

		if (bRunOSCServer) {
			oscServer.Run();
		}

		if (bRunNtpServer) {
			ntpServer.Run();
		} else {
			ntpClient.Run();	// We could check for GPS Time client running. But not really needed.
		}

		if (bRunMidiSystemRealtime) {
			ltcMidiSystemRealtime.Run();	// UDP requests
		}

		if (g_ltc_ptLtcDisabledOutputs.bOled) {
			display.Run();
		}

		if ((!g_ltc_ptLtcDisabledOutputs.bWS28xx) || (!g_ltc_ptLtcDisabledOutputs.bRgbPanel)) {
			ltcDisplayRgb.Run();
		}

		if (g_ltc_ptLtcDisabledOutputs.bRgbPanel) {
			lb.Run();
		}

		if (sourceSelect.IsConnected()) {
			sourceSelect.Run();
		}

		rdmNetLLRPOnly.Run();
		remoteConfig.Run();
		configStore.Flash();
#if defined(ENABLE_SHELL)
		shell.Run();
#endif
#if defined (ENABLE_HTTPD)
		httpDaemon.Run();
#endif
	}
}

}
