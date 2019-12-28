/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "hardware.h"
#include "networkh3emac.h"
#include "ledblink.h"

#include "console.h"

#include "ltcparams.h"
#include "ltcdisplayparams.h"
#include "ltc7segment.h"

#include "artnetnode.h"
#include "artnetparams.h"
#include "timesync.h"

#include "artnetconst.h"
#include "networkconst.h"

#include "ipprog.h"
#include "midi.h"
#include "rtpmidi.h"
#include "midiparams.h"

#include "mdnsservices.h"

#include "tcnet.h"
#include "tcnetparams.h"
#include "tcnettimecode.h"
#include "tcnetdisplay.h"

#include "ntpserver.h"
#include "ntpclient.h"

#include "oscserver.h"

#include "display.h"
#include "displaymax7219.h"
#include "displayws28xx.h"

#include "networkhandleroled.h"

#include "sourceselect.h"
#include "sourceselectconst.h"

#include "ntpserver.h"

#include "h3/ltc.h"

#include "spiflashinstall.h"

#include "spiflashstore.h"
#include "storeltc.h"
#include "storeltcdisplay.h"
#include "storetcnet.h"
#include "storeremoteconfig.h"

#include "remoteconfig.h"
#include "remoteconfigparams.h"

#include "firmwareversion.h"

#include "software_version.h"

extern "C" {

void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	Display display(0,4);
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);

	SpiFlashInstall spiFlashInstall;
	SpiFlashStore spiFlashStore;

	StoreLtc storeLtc;
	LtcParams ltcParams((LtcParamsStore *)&storeLtc);

	struct TLtcDisabledOutputs tLtcDisabledOutputs;
	struct TLtcTimeCode tStartTimeCode;
	struct TLtcTimeCode tStopTimeCode;

	if (ltcParams.Load()) {
		ltcParams.CopyDisabledOutputs(&tLtcDisabledOutputs);
		ltcParams.StartTimeCodeCopyTo(&tStartTimeCode);
		ltcParams.StopTimeCodeCopyTo(&tStopTimeCode);
		ltcParams.Dump();
	}

	Ltc7segment leds;

	fw.Print();

	hw.SetLed(HARDWARE_LED_ON);

	display.ClearLine(1);
	display.ClearLine(2);

	console_status(CONSOLE_YELLOW, NetworkConst::MSG_NETWORK_INIT);
	display.TextStatus(NetworkConst::MSG_NETWORK_INIT, DISPLAY_7SEGMENT_MSG_INFO_NETWORK_INIT);

	NetworkHandlerOled nwHandlerDisplay;
	nw.SetNetworkDisplay((NetworkDisplay *) &nwHandlerDisplay);

	nw.Init((NetworkParamsStore *)spiFlashStore.GetStoreNetwork());
	nw.SetNetworkStore((NetworkStore *)spiFlashStore.GetStoreNetwork());
	nw.Print();

	NtpClient ntpClient;
	ntpClient.Init();
	ntpClient.Print();

	Midi midi;
	ArtNetNode node;
	TCNet tcnet;
	LtcSender ltcSender;
	RtpMidi rtpMidi;
	OSCServer oscServer;

	LtcReader ltcReader(&tLtcDisabledOutputs);
	MidiReader midiReader(&tLtcDisabledOutputs);
	ArtNetReader artnetReader(&tLtcDisabledOutputs);
	TCNetReader tcnetReader(&tLtcDisabledOutputs);
	RtpMidiReader rtpMidiReader(&tLtcDisabledOutputs);
	SystimeReader sysTimeReader(&tLtcDisabledOutputs, ltcParams.GetFps());

	LtcGenerator ltcGenerator(&tStartTimeCode, &tStopTimeCode, &tLtcDisabledOutputs);
	NtpServer ntpServer(ltcParams.GetYear(), ltcParams.GetMonth(), ltcParams.GetDay());

	StoreTCNet storetcnet;
	TCNetParams tcnetparams((TCNetParamsStore *) &storetcnet);

	if (tcnetparams.Load()) {
		tcnetparams.Set(&tcnet);
		tcnetparams.Dump();
	}

	StoreLtcDisplay storeLtcDisplay;
	LtcDisplayParams ltcDisplayParams((LtcDisplayParamsStore *)&storeLtcDisplay);

	if (ltcDisplayParams.Load()) {
		ltcDisplayParams.Dump();
	}

	DisplayMax7219 displayMax7219(ltcDisplayParams.GetMax7219Type());

	if(!tLtcDisabledOutputs.bMax7219) {
		displayMax7219.Init(ltcDisplayParams.GetMax7219Intensity());
	}

	DisplayWS28xx displayWS28xx(ltcDisplayParams.GetLedType());

	if (!tLtcDisabledOutputs.bWS28xx){
		ltcDisplayParams.Set(&displayWS28xx);
		displayWS28xx.Init(ltcDisplayParams.GetGlobalBrightness());
	}

	display.ClearLine(3);
	display.Printf(3, IPSTR "/%d %c", IP2STR(nw.GetIp()), (int) nw.GetNetmaskCIDR(), nw.IsDhcpKnown() ? (nw.IsDhcpUsed() ? 'D' : 'S') : ' ');

	TLtcReaderSource source = ltcParams.GetSource();

	/**
	 * Select the source using buttons/rotary
	 */

	const bool IsAutoStart = ((source == LTC_READER_SOURCE_SYSTIME) && ltcParams.IsAutoStart());

	SourceSelect sourceSelect(source);

	if (sourceSelect.Check() && !IsAutoStart) {
		while (sourceSelect.Wait(source)) {
			nw.Run();
			lb.Run();
		}
	}

	/**
	 * From here work with source selection
	 */

	display.Status(DISPLAY_7SEGMENT_MSG_INFO_NONE);

	const bool bRunArtNet = ((source == LTC_READER_SOURCE_ARTNET) || (!tLtcDisabledOutputs.bArtNet));

	IpProg ipprog;
	TimeSync timeSync;

	if (bRunArtNet) {
		console_status(CONSOLE_YELLOW, ArtNetConst::MSG_NODE_PARAMS);
		display.TextStatus(ArtNetConst::MSG_NODE_PARAMS, DISPLAY_7SEGMENT_MSG_INFO_NODE_PARMAMS);

		ArtNetParams artnetparams((ArtNetParamsStore *)spiFlashStore.GetStoreArtNet());

		if (artnetparams.Load()) {
			artnetparams.Set(&node);
			artnetparams.Dump();
		}

		node.SetArtNetStore((ArtNetStore *)spiFlashStore.GetStoreArtNet());
		node.SetShortName("LTC SMPTE Node");
		node.SetIpProgHandler(&ipprog);

		if (!ltcParams.IsTimeSyncDisabled()) {
			node.SetTimeSyncHandler(&timeSync);
		}

		console_status(CONSOLE_YELLOW, ArtNetConst::MSG_NODE_START);
		display.TextStatus(ArtNetConst::MSG_NODE_START, DISPLAY_7SEGMENT_MSG_INFO_NODE_START);

		node.Print();
		node.Start();

		console_status(CONSOLE_GREEN, ArtNetConst::MSG_NODE_STARTED);
		display.TextStatus(ArtNetConst::MSG_NODE_STARTED, DISPLAY_7SEGMENT_MSG_INFO_NODE_STARTED);
	}

	LtcOutputs ltcOutputs(&tLtcDisabledOutputs, source, ltcParams.IsShowSysTime());

	if (source != LTC_READER_SOURCE_MIDI) {
		midi.Init(MIDI_DIRECTION_OUTPUT);
	}

	if ((source != LTC_READER_SOURCE_LTC) && (!tLtcDisabledOutputs.bLtc)) {
		ltcSender.Start();
	}

	/**
	 * Start the reader
	 */
	switch (source) {
	case LTC_READER_SOURCE_ARTNET:
		node.SetTimeCodeHandler(&artnetReader);
		artnetReader.Start();
		break;
	case LTC_READER_SOURCE_MIDI:
		midiReader.Start();
		break;
	case LTC_READER_SOURCE_TCNET:
		tcnet.SetTimeCodeHandler(&tcnetReader);
		tcnet.Start();
		tcnetReader.Start();
		break;
	case LTC_READER_SOURCE_INTERNAL:
		ltcGenerator.Start();
		break;
	case LTC_READER_SOURCE_APPLEMIDI:
		rtpMidi.SetHandler(&rtpMidiReader);
		rtpMidiReader.Start();
		break;
	case LTC_READER_SOURCE_SYSTIME:
		sysTimeReader.Start(ltcParams.IsAutoStart());
		break;
	default:
		ltcReader.Start();
		break;
	}

	const bool bEnableRtpMidi = ((source == LTC_READER_SOURCE_APPLEMIDI) || (!tLtcDisabledOutputs.bRtpMidi));

	if (bEnableRtpMidi) {
		rtpMidi.Start();
		rtpMidi.AddServiceRecord(0, MDNS_SERVICE_CONFIG, 0x2905);
	}

	/**
	 * The OSC Server is running when enabled AND source = TCNet OR Internal OR System-Time
	 */

	const bool bRunOSCServer = ((source == LTC_READER_SOURCE_TCNET || source == LTC_READER_SOURCE_INTERNAL || source == LTC_READER_SOURCE_SYSTIME) && ltcParams.IsOscEnabled());

	if (bRunOSCServer) {
		bool isSet;
		const uint16_t nPort = ltcParams.GetOscPort(isSet);

		if (isSet) {
			oscServer.SetPortIncoming(nPort);
		}

		oscServer.Start();
		oscServer.Print();
		rtpMidi.AddServiceRecord(0, MDNS_SERVICE_OSC, oscServer.GetPortIncoming(), "type=server");
	}

	const bool bEnableNtp = ltcParams.IsNtpEnabled() && (ntpClient.GetStatus() == NTP_CLIENT_STATUS_STOPPED);

	if (bEnableNtp) {
		ntpServer.SetTimeCode(&tStartTimeCode);
		ntpServer.Start();
		ntpServer.Print();
		rtpMidi.AddServiceRecord(0, MDNS_SERVICE_NTP, NTP_UDP_PORT, "type=server");
	}

	ltcGenerator.Print();
	tcnet.Print();
	midi.Print();
	rtpMidi.Print();

	if(!tLtcDisabledOutputs.bMax7219) {
		displayMax7219.Print();
	}

	if (!tLtcDisabledOutputs.bWS28xx){
		displayWS28xx.Print();
	}

	RemoteConfig remoteConfig(REMOTE_CONFIG_LTC, REMOTE_CONFIG_MODE_TIMECODE, 1 + source);

	StoreRemoteConfig storeRemoteConfig;
	RemoteConfigParams remoteConfigParams(&storeRemoteConfig);

	if(remoteConfigParams.Load()) {
		remoteConfigParams.Set(&remoteConfig);
		remoteConfigParams.Dump();
	}

	// OLED display
	display.ClearLine(4);

	console_puts("Source : ");
	display.SetCursorPos(0,3);

	puts(SourceSelectConst::SOURCE[source]);
	display.PutString(SourceSelectConst::SOURCE[source]);

	if (source == LTC_READER_SOURCE_TCNET) {
		TCNetDisplay::Show();
	}

	ltcOutputs.Print();

	printf("Display : %d (%d,%d)\n", display.GetDetectedType(), display.getCols(), display.getRows());

	hw.WatchdogInit();

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();

		if (source == LTC_READER_SOURCE_TCNET) {		//FIXME Remove when MASTER is implemented
			tcnet.Run();
		}

		// Run the reader
		switch (source) {
		case LTC_READER_SOURCE_LTC:
			ltcReader.Run();
			break;
		case LTC_READER_SOURCE_ARTNET:
			artnetReader.Run();		// Handles MIDI Quarter Frame output messages
			break;
		case LTC_READER_SOURCE_MIDI:
			midiReader.Run();
			break;
		case LTC_READER_SOURCE_TCNET:
			tcnetReader.Run();		// Handles MIDI Quarter Frame output messages
			break;
		case LTC_READER_SOURCE_INTERNAL:
			ltcGenerator.Run();		// Handles MIDI Quarter Frame output messages
			break;
		case LTC_READER_SOURCE_APPLEMIDI:
			rtpMidiReader.Run();	// Handles status led
			break;
		case LTC_READER_SOURCE_SYSTIME:
			sysTimeReader.Run();
			break;
		default:
			break;
		}

		if (bRunArtNet) {
			node.Run();
		}

		if (bEnableRtpMidi) {
			rtpMidi.Run();
		}

		if (bRunOSCServer) {
			oscServer.Run();
		}

		if (bEnableNtp) {
			ntpServer.Run();
		}

		if (tLtcDisabledOutputs.bDisplay) {
			display.Run();
		}

		if (!tLtcDisabledOutputs.bWS28xx){
			displayWS28xx.Run();
		}

		if (sourceSelect.IsConnected()) {
			sourceSelect.Run();
		}

		ntpClient.Run();

		remoteConfig.Run();
		spiFlashStore.Flash();
		lb.Run();
	}
}

}
