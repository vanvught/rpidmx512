/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#pragma GCC push_options
#pragma GCC optimize("O2")
#pragma GCC optimize("no-tree-loop-distribute-patterns")

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cassert>

#include "h3/hal.h"
#include "h3/hal_watchdog.h"
#include "network.h"
#include "net/apps/mdns.h"
#include "display.h"
#include "json/ltcparams.h"
#include "json/ltcdisplayparams.h"
#include "ltcdisplayrgb.h"
#include "ltcdisplaymax7219.h"
#include "ltcetc.h"
#include "json/ltcetcparams.h"
#include "dmxnodenode.h"
#include "midi.h"
#include "net/rtpmidi.h"
#include "tcnet.h"
#include "json/tcnetparams.h"
#include "ntpserver.h"
#include "mcpbuttons.h"
#include "ltcoscserver.h"
#include "ltcsourceconst.h"
#include "ltcsource.h"
#include "ltcsender.h"
#include "arm/artnetreader.h"
#include "arm/ltcreader.h"
#include "arm/midireader.h"
#include "arm/tcnetreader.h"
#include "arm/ltcgenerator.h"
#include "arm/rtpmidireader.h"
#include "arm/systimereader.h"
#include "arm/ltcetcreader.h"
#include "arm/ltcoutputs.h"
#include "arm/ltcmidisystemrealtime.h"
#include "flashcodeinstall.h"
#include "configstore.h"
#include "remoteconfig.h"
#if defined(NODE_RDMNET_LLRP_ONLY)
#include "rdmnetdevice.h"
#include "rdm_e120.h"
#endif
#include "net/apps/ntpclient.h"
#include "gpstimeclient.h"
#include "json/gpsparams.h"
#include "firmwareversion.h"
#include "software_version.h"
#include "net/protocol/ntp.h"
#include "common/utils/utils_flags.h"
#include "common/utils/utils_enum.h"
#include "configurationstore.h"
#include "pixeltype.h"
#include "hwclock.h"

static ltc::Source ltc_source;

namespace ntpclient
{
void DisplayStatus(::ntp::Status status)
{
    if (ltc_source != ltc::Source::SYSTIME)
    {
        return;
    }

    switch (status)
    {
        case ::ntp::Status::STOPPED:
            Display::Get()->TextStatus("No NTP Client");
            break;
        case ::ntp::Status::IDLE:
            LtcOutputs::Get()->ResetTimeCodeTypePrevious();
            Display::Get()->TextStatus("NTP Client");
            break;
        case ::ntp::Status::LOCKED:
            Display::Get()->TextStatus("NTP Client LOCKED");
            break;
        case ::ntp::Status::FAILED:
            Display::Get()->TextStatus("Error: NTP");
            break;
        default:
            break;
    }
}
} // namespace ntpclient

namespace hal
{
void RebootHandler()
{
    switch (ltc_source)
    {
        case ::ltc::Source::TCNET:
            TCNet::Get()->Stop();
            break;
        default:
            break;
    }

    //	if (!ltc::g_DisabledOutputs.bMax7219) {
    if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219))
    {
        LtcDisplayMax7219::Get()->Init(); // TODO (a) WriteChar
    }
#if !defined(CONFIG_LTC_DISABLE_WS28XX)
    //	if ((!ltc::g_DisabledOutputs.bWS28xx) || (!ltc::g_DisabledOutputs.bRgbPanel)) {
    if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX) || ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
    {
        LtcDisplayRgb::Get()->WriteChar('-');
    }
#endif
    if (!RemoteConfig::Get()->IsReboot())
    {
        Display::Get()->SetSleep(false);
        Display::Get()->Cls();
        Display::Get()->TextStatus("Rebooting ...");
    }
}
} // namespace hal

#if !defined(CONFIG_LTC_DISABLE_RGB_PANEL)
extern "C"
{
    void h3_cpu_off(uint32_t);
}
#endif

void static StaticCallbackFunction([[maybe_unused]] const struct artnet::TimeCode* pTimeCode) {}

using common::store::gps::Flags;

int main() // NOLINT
{
    hal::Init();
    Display display(4);
    ConfigStore config_store;
    network::Init();
    FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
    FlashCodeInstall spiflash_install;

    fw.Print("LTC SMPTE");

    display.ClearLine(1);
    display.ClearLine(2);

    json::LtcParams ltc_params;
    ltc_params.Load();

    struct ltc::TimeCode tStartTimeCode;
    struct ltc::TimeCode tStopTimeCode;

    ltc_params.Set(&tStartTimeCode, &tStopTimeCode);

    const auto kLtcFlags = ConfigStore::Instance().LtcGet(&common::store::Ltc::flags);
    const auto kFps = ConfigStore::Instance().LtcGet(&common::store::Ltc::fps);
    const auto kUtcOffset = ConfigStore::Instance().LtcGet(&common::store::Ltc::utc_offset);
    const auto kSource = ConfigStore::Instance().LtcGet(&common::store::Ltc::source);
    const auto kRgbLedType = ConfigStore::Instance().LtcGet(&common::store::Ltc::rgb_led_type);
    const auto kDisplayFlags = ConfigStore::Instance().LtcDisplayGet(&common::store::LtcDisplay::flags);
    const auto kIsRotaryFullStep = common::IsFlagSet(kDisplayFlags, common::store::ltc::display::Flags::Flag::kRotaryFullStep);
    const auto kIsAltFunction = common::IsFlagSet(kLtcFlags, common::store::ltc::Flags::Flag::kIsAltFuntion);
    const auto kSkipSeconds = ConfigStore::Instance().LtcGet(&common::store::Ltc::skip_seconds);
    const auto kShowSystime = common::IsFlagSet(kLtcFlags, common::store::ltc::Flags::Flag::kShowSystime);
    const auto kTimecodeIp = ConfigStore::Instance().LtcGet(&common::store::Ltc::time_code_ip);
    const auto kTimeSyncDisabled = common::IsFlagSet(kLtcFlags, common::store::ltc::Flags::Flag::kTimeSyncDisabled);
    const auto kVolume = ConfigStore::Instance().LtcGet(&common::store::Ltc::volume);
    const auto kOscEnabled = common::IsFlagSet(kLtcFlags, common::store::ltc::Flags::Flag::kOscEnabled);
    const auto kOscPort = ConfigStore::Instance().LtcGet(&common::store::Ltc::osc_port);

    ltc_source = common::FromValue<ltc::Source>(kSource);

    LtcReader ltc_reader;
    MidiReader midiReader;
    ArtNetReader artnetReader;
    TCNetReader tcnetReader;
    RtpMidiReader rtpMidiReader;
    SystimeReader sysTimeReader(kFps, kUtcOffset);
    LtcEtcReader ltcEtcReader;

    LtcDisplayMax7219 display_max7219;

    json::LtcDisplayParams ltcdisplay_params;
    ltcdisplay_params.Load();
    ltcdisplay_params.Set();

#if !defined(CONFIG_LTC_DISABLE_WS28XX)
    const auto kLtcDisplayWs28xxType =
        common::FromValue<ltc::display::rgb::WS28xxType>(ConfigStore::Instance().LtcDisplayGet(&common::store::LtcDisplay::ws28xx_type));
    const auto kIsRgbPanelEnabled(kRgbLedType == json::LtcParams::RgbLedType::kRgbpanel);

    LtcDisplayRgb display_rgb(kIsRgbPanelEnabled ? ltc::display::rgb::Type::kRgbpanel : ltc::display::rgb::Type::kWS28Xx, kLtcDisplayWs28xxType);
#endif
    /**
     * Select the source using buttons/rotary
     */

    McpButtons source_select(ltc_source, kIsAltFunction, kSkipSeconds, !kIsRotaryFullStep);

    const auto kIsAutoStart = ((ltc_source == ltc::Source::SYSTIME) && common::IsFlagSet(kLtcFlags, common::store::ltc::Flags::Flag::kAutoStart));

    if (source_select.Check() && !kIsAutoStart)
    {
        while (source_select.Wait(ltc_source, tStartTimeCode, tStopTimeCode))
        {
            network::Run();
        }
    }

    /**
     * From here work with source selection
     */

    LtcOutputs ltc_outputs(ltc_source, kShowSystime);

    if (ltc::Destination::IsEnabled(ltc::Destination::Output::MAX7219))
    {
        display_max7219.Init();
        display_max7219.Print();
    }

#if !defined(CONFIG_LTC_DISABLE_WS28XX)
    if (ltc::Destination::IsEnabled(ltc::Destination::Output::WS28XX) || ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
    {
        if (ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
        {
            display_rgb.Init();

            char info_message[common::store::ltc::display::kMaxInfoMessage];
            ConfigStore::Instance().LtcDisplayCopyArray(info_message, &common::store::LtcDisplay::info_message);

            char info_nt[common::store::ltc::display::kMaxInfoMessage + 1] = {};
            memcpy(info_nt, info_message, sizeof(info_message));
            info_nt[sizeof(info_message)] = '\0';

            display_rgb.ShowInfo(info_nt);
        }
        else
        {
            const auto kWs28xxType = ConfigStore::Instance().LtcDisplayGet(&common::store::LtcDisplay::ws28xx_type);
            display_rgb.Init(common::FromValue<pixel::Type>(kWs28xxType));
        }

        display_rgb.Print();
    }
#endif

#if !defined(CONFIG_LTC_DISABLE_RGB_PANEL)
    if (ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
    {
        for (uint32_t nCpuNumber = 1; nCpuNumber < 4; nCpuNumber++)
        {
            h3_cpu_off(nCpuNumber);
        }
    }
#endif

#if defined(NODE_RDMNET_LLRP_ONLY)
    auto& rdm_device = RdmDevice::Get();
    rdm_device.SetProductCategory(E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION);
    rdm_device.SetProductDetail(E120_PRODUCT_DETAIL_ETHERNET_NODE);
    rdm_device.Init();
    rdm_device.Print();

    RDMNetDevice llrp_only_device;
#endif

    /**
     * Art-Net
     */

    const auto kRunArtNet = ((ltc_source == ltc::Source::ARTNET) || ltc::Destination::IsEnabled(ltc::Destination::Output::ARTNET));

    DmxNodeNode dmxnode_node;

    dmxnode_node.SetArtTimeCodeCallbackFunction(StaticCallbackFunction);

    if (kRunArtNet)
    {
        dmxnode_node.SetShortName(0, "LTC SMPTE Node");
        dmxnode_node.SetUniverse(0, 1);
        dmxnode_node.SetDirection(0, dmxnode::PortDirection::kOutput);
        dmxnode_node.SetShortName(0, "Not used");

        if (ltc_source == ltc::Source::ARTNET)
        {
            dmxnode_node.SetArtTimeCodeCallbackFunction(ArtNetReader::StaticCallbackFunction);
        }

        dmxnode_node.SetTimeCodeIp(kTimecodeIp);

        if (!kTimeSyncDisabled)
        {
            // TODO (a) Send ArtTimeSync
        }
#if defined(NODE_RDMNET_LLRP_ONLY)
        dmxnode_node.SetRdmUID(rdm_net_llrp_only.GetRDMNetDevice()->GetUID(), true);
#endif
        dmxnode_node.Start();
        dmxnode_node.Print();
    }

    /**
     * TCNet
     */

    const auto kRunTcNet = (ltc_source == ltc::Source::TCNET);

    TCNet tcnet;

    if (kRunTcNet)
    {
        json::TcNetParams tcnetparams;
        tcnetparams.Load();
        tcnetparams.Set();

        tcnet.SetArtTimeCodeCallbackFunction(TCNetReader::StaticCallbackFunctionHandler);
        tcnet.Start();
        tcnet.Print();
    }

    /**
     * MIDI
     */

    Midi midi;

    if ((ltc_source != ltc::Source::MIDI) && ltc::Destination::IsEnabled(ltc::Destination::Output::MIDI))
    {
        midi.Init(midi::Direction::OUTPUT);
    }

    if ((ltc_source == ltc::Source::MIDI) || ltc::Destination::IsEnabled(ltc::Destination::Output::MIDI))
    {
        midi.Print();
    }

    /**
     * RTP-MIDI
     */

    RtpMidi rtp_midi;

    if ((ltc_source == ltc::Source::APPLEMIDI) || ltc::Destination::IsEnabled(ltc::Destination::Output::RTPMIDI))
    {
        if (ltc_source == ltc::Source::APPLEMIDI)
        {
            rtp_midi.SetHandler(&rtpMidiReader);
        }

        rtp_midi.Start();
        rtp_midi.Print();
    }

    /**
     * ETC
     */

    LtcEtc ltc_etc;

    if ((ltc_source == ltc::Source::ETC) || ltc::Destination::IsEnabled(ltc::Destination::Output::ETC))
    {
        json::LtcEtcParams ltc_etc_params;

        ltc_etc_params.Load();
        ltc_etc_params.Set();

        if (ltc_source == ltc::Source::ETC)
        {
            ltc_etc.SetHandler(&ltcEtcReader);
        }

        ltc_etc.Start();
        ltc_etc.Print();
    }

    /**
     * LTC Sender
     */

    LtcSender ltc_sender(kVolume);

    if ((ltc_source != ltc::Source::LTC) && ltc::Destination::IsEnabled(ltc::Destination::Output::LTC))
    {
        ltc_sender.Start();
    }

    /**
     * The OSC Server is running when enabled AND source = TCNet OR Internal OR System-Time
     */

    const auto kRunOSCServer = ((ltc_source == ltc::Source::TCNET || ltc_source == ltc::Source::INTERNAL || ltc_source == ltc::Source::SYSTIME) && kOscEnabled);

    LtcOscServer oscserver;

    if (kRunOSCServer)
    {
        oscserver.SetPortIncoming(kOscPort);
        oscserver.Start();
        oscserver.Print();

        mdns::ServiceRecordAdd(nullptr, mdns::Services::OSC, "type=server", oscserver.GetPortIncoming());
    }

    /**
     * The GPS Time client is running when enabled AND source = System-Time AND RGB Panel is disabled
     * The NTP Client is stopped.
     */

    const auto kGpsFlags = ConfigStore::Instance().GpsGet(&common::store::Gps::flags);
    const auto kGpsUtcOffset = ConfigStore::Instance().GpsGet(&common::store::Gps::utc_offset);
    const auto kGpsModule = common::FromValue<gps::Module>(ConfigStore::Instance().GpsGet(&common::store::Gps::module));
    const auto kIsGpsEnabled = common::IsFlagSet(kGpsFlags, Flags::Flag::kEnable);

    const auto kRunGpsTimeClient = (kIsGpsEnabled && (ltc_source == ltc::Source::SYSTIME) && ltc::Destination::is_disabled(ltc::Destination::Output::RGBPANEL));
    const auto kGpsStart = kRunGpsTimeClient && common::IsFlagSet(kLtcFlags, common::store::ltc::Flags::Flag::kGpsStart);

    GPSTimeClient gpstime_client(kGpsUtcOffset, kGpsModule);

    json::GpsParams gps_params;
    gps_params.Load();
    gps_params.Set();

    if (kRunGpsTimeClient)
    {
        ntpclient::Stop(true);
        gpstime_client.Start();
        gpstime_client.Print();
    }

    /**
     * When the NTP Server is enabled then the NTP Client is not running (stopped).
     */

    const auto kRunNtpServer = common::IsFlagSet(kLtcFlags, common::store::ltc::Flags::Flag::kNtpEnable);
    const auto kNtpYear = ConfigStore::Instance().LtcGet(&common::store::Ltc::ntp_year);
    const auto kNtpMonth = ConfigStore::Instance().LtcGet(&common::store::Ltc::ntp_month);
    const auto kNtpDay = ConfigStore::Instance().LtcGet(&common::store::Ltc::ntp_day);

    NtpServer ntp_server(kNtpYear, kNtpMonth, kNtpDay);

    if (kRunNtpServer)
    {
        ntpclient::Stop(true);

        ntp_server.SetTimeCode(&tStartTimeCode);
        ntp_server.Start();
        ntp_server.Print();

        mdns::ServiceRecordAdd(nullptr, mdns::Services::NTP, "type=server");
    }

    /**
     * LTC Generator
     */

    const auto kSkipFree = common::IsFlagSet(kLtcFlags, common::store::ltc::Flags::Flag::kSkipFree);
    const auto kIgnoreStart = common::IsFlagSet(kLtcFlags, common::store::ltc::Flags::Flag::kIgnoreStart);
    const auto kIgnoreStop = common::IsFlagSet(kLtcFlags, common::store::ltc::Flags::Flag::kIgnoreStop);

    LtcGenerator ltc_generator(&tStartTimeCode, &tStopTimeCode, kSkipFree, kIgnoreStart, kIgnoreStop);

    /**
     * MIDI output System Real Time
     */

    LtcMidiSystemRealtime ltc_midi_system_realtime;

    /**
     * The UDP request handler is running when source is NOT MIDI AND source is NOT RTP-MIDI
     * AND when MIDI output is NOT disabled OR the RTP-MIDI is NOT disabled.
     */

    const auto kRunMidiSystemRealtime =
        (ltc_source != ltc::Source::MIDI) && (ltc_source != ltc::Source::APPLEMIDI) &&
        (ltc::Destination::IsEnabled(ltc::Destination::Output::RTPMIDI) || ltc::Destination::IsEnabled(ltc::Destination::Output::MIDI));

    if (kRunMidiSystemRealtime)
    {
        ltc_midi_system_realtime.Start();
    }

    /**
     * Start the reader
     */

    switch (ltc_source)
    {
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
            ltc_generator.Start();
            ltc_generator.Print();
            break;
        case ltc::Source::APPLEMIDI:
            rtpMidiReader.Start();
            break;
        case ltc::Source::ETC:
            ltcEtcReader.Start();
            break;
        case ltc::Source::SYSTIME:
            sysTimeReader.Start(kIsAutoStart && !kGpsStart);
            break;
        default:
            ltc_reader.Start();
            break;
    }

    RemoteConfig remote_config(remoteconfig::Output::TIMECODE, 1U + static_cast<uint32_t>(ltc_source));

    printf("Source : %s\n", LtcSourceConst::NAME[static_cast<uint32_t>(ltc_source)]);

    ltc::source::Show(ltc_source, kRunGpsTimeClient);

#if !defined(CONFIG_LTC_DISABLE_RGB_PANEL)
    if (ltc::Destination::IsEnabled(ltc::Destination::Output::RGBPANEL))
    {
        display_rgb.ShowSource(ltc_source);
    }
#endif

    ltc_outputs.Print();

    hal::WatchdogInit();

    for (;;)
    {
        hal::WatchdogFeed();
        network::Run();
        // Run the reader
        // Handles MIDI Quarter Frame output messages
        switch (ltc_source)
        {
            case ltc::Source::LTC:
                ltc_reader.Run();
                break;
            case ltc::Source::ARTNET:
                artnetReader.Run();
                break;
            case ltc::Source::MIDI:
                midiReader.Run();
                break;
            case ltc::Source::TCNET:
                tcnetReader.Run();
                break;
            case ltc::Source::INTERNAL:
                ltc_generator.Run();
                break;
            case ltc::Source::APPLEMIDI:
                rtpMidiReader.Run();
                break;
            case ltc::Source::ETC:
                ltcEtcReader.Run();
                break;
            case ltc::Source::SYSTIME:
                sysTimeReader.Run();
                if (!kRunGpsTimeClient)
                {
                    if (kRunNtpServer)
                    {
                        HwClock::Get()->Run(true);
                    }
                    else
                    {
                        HwClock::Get()->Run(ntpclient::GetStatus() == ::ntp::Status::FAILED); // No need to check for STOPPED
                    }
                }
                else
                {
                    gpstime_client.Run();
                    if (kGpsStart)
                    {
                        if (gpstime_client.GetStatus() == gps::Status::kValid)
                        {
                            sysTimeReader.ActionStart();
                        }
                        else
                        {
                            sysTimeReader.ActionStop();
                        }
                    }
                }
                break;
            default:
                break;
        }

        if (kRunArtNet)
        {
            dmxnode_node.Run();
        }

        if (kRunTcNet)
        {
            tcnet.Run();
        }

        if (ltc::Destination::IsEnabled(ltc::Destination::Output::DISPLAY_OLED))
        {
            display.Run();
        }

        if (source_select.IsConnected())
        {
            source_select.Run();
        }

        hal::Run();
    }
}
