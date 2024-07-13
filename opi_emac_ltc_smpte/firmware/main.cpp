/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "mdns.h"

#include "display.h"

#include "ltcparams.h"
#include "ltcdisplayparams.h"
#include "ltcdisplayrgb.h"
#include "ltcdisplaymax7219.h"
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

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#if defined (NODE_RDMNET_LLRP_ONLY)
# include "rdmnetllrponly.h"
# include "rdm_e120.h"
# include "factorydefaults.h"
#endif

// System Time
#include "ntpclient.h"
#include "gpstimeclient.h"
#include "gpsparams.h"

#include "firmwareversion.h"
#include "software_version.h"

#if defined(ENABLE_SHELL)
# include "shell/shell.h"
#endif

namespace artnetnode {
namespace configstore {
uint32_t DMXPORT_OFFSET = 0;
}  // namespace configstore
}  // namespace artnetnode

void Hardware::RebootHandler() {
//	switch (m_tSource) {
//	case ltc::source::TCNET:
//		TCNet::Get()->Stop();
//		break;
//	default:
//		break;
//	}

	if (!ltc::g_DisabledOutputs.bMax7219) {
		LtcDisplayMax7219::Get()->Init(2); // TODO WriteChar
	}

	if ((!ltc::g_DisabledOutputs.bWS28xx) || (!ltc::g_DisabledOutputs.bRgbPanel)) {
		LtcDisplayRgb::Get()->WriteChar('-');
	}

	if (!RemoteConfig::Get()->IsReboot()) {
		Display::Get()->SetSleep(false);

		while (ConfigStore::Get()->Flash())
			;

		Network::Get()->Shutdown();

		Display::Get()->Cls();
		Display::Get()->TextStatus("Rebooting ...");
	}
}

extern "C" {
void h3_cpu_off(uint32_t);
}

int main() {
	Hardware hw;
	Display display(4);
	ConfigStore configStore;
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, CONSOLE_YELLOW);
	Network nw;
	display.TextStatus(NetworkConst::MSG_NETWORK_STARTED, CONSOLE_GREEN);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	FlashCodeInstall spiFlashInstall;

	fw.Print("LTC SMPTE");
	nw.Print();

#if defined(ENABLE_SHELL)
	Shell shell;
#endif

	display.ClearLine(1);
	display.ClearLine(2);

	MDNS mdns;

	NtpClient ntpClient;
	ntpClient.Start();
	ntpClient.Print();

	LtcParams ltcParams;

	struct ltc::TimeCode tStartTimeCode;
	struct ltc::TimeCode tStopTimeCode;

	ltcParams.Load();
	ltcParams.Set(&tStartTimeCode, &tStopTimeCode);

	LtcReader ltcReader;
	MidiReader midiReader;
	ArtNetReader artnetReader;
	TCNetReader tcnetReader;
	RtpMidiReader rtpMidiReader;
	SystimeReader sysTimeReader(ltcParams.GetFps());
	LtcEtcReader ltcEtcReader;

	ltc::Source ltcSource = ltcParams.GetSource();

	LtcDisplayParams ltcDisplayParams;

	ltcDisplayParams.Load();
	display.SetContrast(ltcDisplayParams.GetOledIntensity());

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
		}
	}

	/**
	 * From here work with source selection
	 */

#if defined(ENABLE_SHELL)
	shell.SetSource(ltcSource);
#endif

	LtcOutputs ltcOutputs(ltcSource, ltcParams.IsShowSysTime());

	if (!ltc::g_DisabledOutputs.bMax7219) {
		DEBUG_PUTS("");
		ltcDdisplayMax7219.Init(ltcDisplayParams.GetMax7219Intensity());
		ltcDdisplayMax7219.Print();
	}

	if ((!ltc::g_DisabledOutputs.bWS28xx) || (!ltc::g_DisabledOutputs.bRgbPanel)) {
		ltcDisplayParams.Set(&ltcDisplayRgb);

		if (!ltc::g_DisabledOutputs.bRgbPanel) {
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

	if (ltc::g_DisabledOutputs.bRgbPanel) {
		for (uint32_t nCpuNumber = 1; nCpuNumber < 4; nCpuNumber++) {
			h3_cpu_off(nCpuNumber);
		}
	}

#if defined (NODE_RDMNET_LLRP_ONLY)
	RDMNetLLRPOnly rdmNetLLRPOnly("LTC SMPTE");

	rdmNetLLRPOnly.GetRDMNetDevice()->SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
	rdmNetLLRPOnly.GetRDMNetDevice()->SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
	rdmNetLLRPOnly.Init();
	rdmNetLLRPOnly.Print();
#endif

	/**
	 * Art-Net
	 */

	const auto bRunArtNet = ((ltcSource == ltc::Source::ARTNET) || (!ltc::g_DisabledOutputs.bArtNet));

	ArtNetNode node;

	if (bRunArtNet) {
		ArtNetParams artnetparams;
		artnetparams.Load();
		artnetparams.Set();

		node.SetShortName(0, "LTC SMPTE Node");

		if (ltcSource == ltc::Source::ARTNET) {
			node.SetTimeCodeHandler(&artnetReader);
		}

		node.SetTimeCodeIp(ltcParams.GetTimecodeIp());

		if (!ltcParams.IsTimeSyncDisabled()) {
			//TODO Send ArtTimeSync
		}
#if defined (NODE_RDMNET_LLRP_ONLY)
		node.SetRdmUID(rdmNetLLRPOnly.GetRDMNetDevice()->GetUID(), true);
#endif
		node.Start();
		node.Print();
	}

	/**
	 * TCNet
	 */

	const auto bRunTCNet = (ltcSource == ltc::Source::TCNET);

	TCNet tcnet;

	if (bRunTCNet) {
		TCNetParams tcnetparams;
		tcnetparams.Load();
		tcnetparams.Set(&tcnet);

		tcnet.SetTimeCodeHandler(&tcnetReader);
		tcnet.Start();
		tcnet.Print();
	}

	/**
	 * MIDI
	 */

	Midi midi;

	if ((ltcSource != ltc::Source::MIDI) && (!ltc::g_DisabledOutputs.bMidi)) {
		midi.Init(midi::Direction::OUTPUT);
	}

	if ((ltcSource == ltc::Source::MIDI) || (!ltc::g_DisabledOutputs.bMidi)) {
		midi.Print();
	}

	/**
	 * RTP-MIDI
	 */

	const auto bRunRtpMidi = ((ltcSource == ltc::Source::APPLEMIDI) || (!ltc::g_DisabledOutputs.bRtpMidi));

	RtpMidi rtpMidi;

	if (bRunRtpMidi) {
		if (ltcSource == ltc::Source::APPLEMIDI) {
			rtpMidi.SetHandler(&rtpMidiReader);
		}

		rtpMidi.Start();
		rtpMidi.Print();
	}

	/**
	 * ETC
	 */

	const auto bRunLtcEtc = (ltcSource == ltc::Source::ETC);

	LtcEtc ltcEtc;

	if (bRunLtcEtc || (!ltc::g_DisabledOutputs.bEtc)) {
		LtcEtcParams ltcEtcParams;

		ltcEtcParams.Load();
		ltcEtcParams.Set();

		if (ltcSource == ltc::Source::ETC) {
			ltcEtc.SetHandler(&ltcEtcReader);
		}

		ltcEtc.Start();
		ltcEtc.Print();
	}

	/**
	 * LTC Sender
	 */

	LtcSender ltcSender(ltcParams.GetVolume());

	if ((ltcSource != ltc::Source::LTC) && (!ltc::g_DisabledOutputs.bLtc)) {
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

		mdns.ServiceRecordAdd(nullptr, mdns::Services::OSC, "type=server", oscServer.GetPortIncoming());
	}

	/**
	 * The GPS Time client is running when enabled AND source = System-Time AND RGB Panel is disabled
	 * The NTP Client is stopped.
	 */

	GPSParams gpsParams;

	if (ltcSource == ltc::Source::SYSTIME) {
		gpsParams.Load();
	}

	const auto bRunGpsTimeClient = (gpsParams.IsEnabled() && (ltcSource == ltc::Source::SYSTIME) && ltc::g_DisabledOutputs.bRgbPanel);
	const auto bGpsStart = bRunGpsTimeClient && ltcParams.IsGpsStart();

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

		mdns.ServiceRecordAdd(nullptr, mdns::Services::NTP, "type=server");

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

	const auto bRunMidiSystemRealtime = (ltcSource != ltc::Source::MIDI) && (ltcSource != ltc::Source::APPLEMIDI) && ((!ltc::g_DisabledOutputs.bRtpMidi) || (!ltc::g_DisabledOutputs.bMidi));

	if (bRunMidiSystemRealtime) {
		ltcMidiSystemRealtime.Start();
	}

	/**
	 * Start the reader
	 */

	switch (ltcSource) {
	case ltc::Source::ARTNET:
		artnetReader.Start();
		break;
	case ltc::Source::MIDI:
		midiReader.Start();
		break;
	case ltc::Source::TCNET:
		tcnetReader.Start();
		break;
	case ltc::Source::INTERNAL:
		ltcGenerator.Start();
		ltcGenerator.Print();
		break;
	case ltc::Source::APPLEMIDI:
		rtpMidiReader.Start();
		break;
	case ltc::Source::ETC:
		ltcEtcReader.Start();
		break;
	case ltc::Source::SYSTIME:
		sysTimeReader.Start(ltcParams.IsAutoStart() && !bGpsStart);
		break;
	default:
		ltcReader.Start();
		break;
	}

	mdns.Print();

	RemoteConfig remoteConfig(remoteconfig::Node::LTC, remoteconfig::Output::TIMECODE, 1U + static_cast<uint32_t>(ltcSource));

	RemoteConfigParams remoteConfigParams;
	remoteConfigParams.Load();
	remoteConfigParams.Set(&remoteConfig);

	while (configStore.Flash())
		;

	printf("Source : %s\n", LtcSourceConst::NAME[static_cast<uint32_t>(ltcSource)]);

	ltc::source::show(ltcSource, bRunGpsTimeClient);

	if (!ltc::g_DisabledOutputs.bRgbPanel) {
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
					HwClock::Get()->Run(NtpClient::Get()->GetStatus() == ::ntp::Status::FAILED); // No need to check for STOPPED
				}
			} else {
				gpsTimeClient.Run();
				if (bGpsStart) {
					if (gpsTimeClient.GetStatus() == gps::Status::VALID) {
						sysTimeReader.ActionStart();
					} else {
						sysTimeReader.ActionStop();
					}
				}
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

		if (ltc::g_DisabledOutputs.bOled) {
			display.Run();
		}

		if ((!ltc::g_DisabledOutputs.bWS28xx) || (!ltc::g_DisabledOutputs.bRgbPanel)) {
			ltcDisplayRgb.Run();
		}

		if (ltc::g_DisabledOutputs.bRgbPanel) {
			hw.Run();
		}

		if (sourceSelect.IsConnected()) {
			sourceSelect.Run();
		}

#if defined (NODE_RDMNET_LLRP_ONLY)
		rdmNetLLRPOnly.Run();
#endif
		remoteConfig.Run();
		configStore.Flash();
#if defined(ENABLE_SHELL)
		shell.Run();
#endif
		mdns.Run();
	}
}
