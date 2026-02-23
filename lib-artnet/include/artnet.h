/**
 * @file artnet.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARTNET_H_
#define ARTNET_H_

#include <cstdint>

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

namespace artnet
{
namespace defaults
{
inline constexpr uint32_t kNetSwitch = 0;
inline constexpr uint32_t kSubnetSwitch = 0;
inline constexpr uint32_t kSwitch = 1;
} // namespace defaults
#if !defined(ARTNET_VERSION)
inline constexpr uint32_t kVersion = 4;
#else
inline constexpr uint32_t kVersion = ARTNET_VERSION;
#endif
inline constexpr uint8_t kProtocolRevision = 14;
inline constexpr uint32_t kPorts = 4;
inline constexpr uint16_t kUdpPort = 0x1936;
inline constexpr uint32_t kDmxLength = 512;
inline constexpr uint32_t kShortNameLength = 18;
inline constexpr uint32_t kLongNameLength = 64;
inline constexpr uint32_t kReportLength = 64;
inline constexpr uint32_t kRdmUidWidth = 6;
inline constexpr uint32_t kMacSize = 6;
inline constexpr uint32_t kIpSize = 4;
inline constexpr uint32_t kEstaSize = 2;

inline constexpr char kNodeId[] = "Art-Net"; ///< Array of 8 characters, the final character is a null termination. Value = A r t - N e t 0x00
inline constexpr uint32_t kMergeTimeoutSeconds = 10;
inline constexpr uint32_t kNetworkDataLossTimeout = 10; ///< Seconds

enum class PortProtocol
{
    kArtnet, ///< Output both DMX512 and RDM packets from the Art-Net protocol (default).
    kSacn    ///< Output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
};

/**
 * Table 3 – NodeReport Codes
 * The NodeReport code defines generic error, advisory and status messages for both Nodes and Controllers.
 * The NodeReport is returned in ArtPollReply.
 */
enum class ReportCode : uint8_t
{
    kRcdebug,
    kRcpowerok,
    kRcpowerfail,
    kRcsocketwR1,
    kRcparsefail,
    kRcudpfail,
    kRcshnameok,
    kRclonameok,
    kRcdmxerror,
    kRcdmxudpfull,
    kRcdmxrxfull,
    kRcswitcherr,
    kRcconfigerr,
    kRcdmxshort,
    kRcfirmwarefail,
    kRcuserfail
};

enum class Status : uint8_t
{
    kOff,
    kStandby,
    kOn
};

enum class FailSafe : uint8_t
{
    kLast = 0x08,
    kOff = 0x09,
    kOn = 0x0a,
    kPlayback = 0x0b,
    kRecord = 0x0c
};

/**
 * Table 4 – Style Codes
 * The Style code defines the general functionality of a Controller.
 * The Style code is returned in ArtPollReply.
 */
enum class StyleCode : uint8_t
{
    kNode = 0x00,   ///< A DMX to / from Art-Net device
    kServer = 0x01, ///< A lighting console.
    kMedia = 0x02,  ///< A Media Server.
    kRoute = 0x03,  ///< A network routing device.
    kBackup = 0x04, ///< A backup device.
    kConfig = 0x05, ///< A configuration or diagnostic tool.
    kVisual = 0x06, ///< A visualizer.
};

/**
 * Table 5 – Priority Codes
 * Diagnostics Priority codes.
 * These are used in ArtPoll and ArtDiagData.
 */
enum class PriorityCodes : uint8_t
{
    kDiagLow = 0x10,
    kDiagMed = 0x40,
    kDiagHigh = 0x80,
    kDiagCritical = 0xE0,
    kDiagVolatile = 0xF0, ///< Volatile message. Messages of this type are displayed on a single line in the DMX-Workshop diagnostics display. All other types
                          ///< are displayed in a list box.
};

struct PortType
{
    static constexpr uint8_t kProtocolDmx = 0x00;    ///< DMX-512
    static constexpr uint8_t kProtocolMidi = 0x01;   ///< MIDI
    static constexpr uint8_t kProtocolAvab = 0x02;   ///< Avab
    static constexpr uint8_t kProtocolCmx = 0x03;    ///< Colortran CMX
    static constexpr uint8_t kProtocolAdb = 0x04;    ///< ABD 62.5
    static constexpr uint8_t kProtocolArtnet = 0x05; ///< ArtNet
    static constexpr uint8_t kInputArtnet = 0x40;    ///< Set if this channel can input onto the Art-Net network
    static constexpr uint8_t kOutputArtnet = 0x80;   ///< Set if this channel can output data from the Art-Net network
};

struct PortCommand
{
    static constexpr uint8_t kNone = 0x00;      ///< No action
    static constexpr uint8_t kCancel = 0x01;    ///< If Node is currently in merge mode, cancel merge mode upon receipt of next ArtDmx packet.
    static constexpr uint8_t kLedNormal = 0x02; ///< The front panel indicators of the Node operate normally.
    static constexpr uint8_t kLedMute = 0x03;   ///< The front panel indicators of the Node are disabled and switched off.
    static constexpr uint8_t kLedLocate =
        0x04; ///< Rapid flashing of the Node’s front panel indicators. It is intended as an outlet locator for large installations.
    static constexpr uint8_t kReset = 0x05; ///< Resets the Node’s Sip, Text, Test and data error flags.

    static constexpr uint8_t kFailHold = 0x08;   ///< Failsafe mode = hold last state
    static constexpr uint8_t kFailZero = 0x09;   ///< Failsafe mode = clear outputs
    static constexpr uint8_t kFailFull = 0x0a;   ///< Failsafe mode = outputs to full
    static constexpr uint8_t kFailScene = 0x0b;  ///< Failsafe mode = playback failsafe scene
    static constexpr uint8_t kFailRecord = 0x0c; ///< Failsafe mode = record current output as failsafe scene

    static constexpr uint8_t kMergeLtpO = 0x10; ///< Set DMX Port 0 to Merge in LTP mode.
    static constexpr uint8_t kMergeLtp1 = 0x11; ///< Set DMX Port 1 to Merge in LTP mode.
    static constexpr uint8_t kMergeLtp2 = 0x12; ///< Set DMX Port 2 to Merge in LTP mode.
    static constexpr uint8_t kMergeLtp3 = 0x13; ///< Set DMX Port 3 to Merge in LTP mode.

    static constexpr uint8_t kDirectionTxO = 0x20; ///< Set Port 0 direction to Tx.
    static constexpr uint8_t kDirectionTx1 = 0x21; ///< Set Port 1 direction to Tx.
    static constexpr uint8_t kDirectionTx2 = 0x22; ///< Set Port 2 direction to Tx.
    static constexpr uint8_t kDirectionTx3 = 0x23; ///< Set Port 3 direction to Tx.

    static constexpr uint8_t kDirectionRxO = 0x30; ///< Set Port 0 direction to Rx.
    static constexpr uint8_t kDirectionRx1 = 0x31; ///< Set Port 1 direction to Rx.
    static constexpr uint8_t kDirectionRx2 = 0x32; ///< Set Port 2 direction to Rx.
    static constexpr uint8_t kDirectionRx3 = 0x33; ///< Set Port 3 direction to Rx.

    static constexpr uint8_t kMergeHtp0 = 0x50; ///< Set DMX Port 0 to Merge in HTP (default) mode.
    static constexpr uint8_t kMergeHtp1 = 0x51; ///< Set DMX Port 1 to Merge in HTP (default) mode.
    static constexpr uint8_t kMergeHtp2 = 0x52; ///< Set DMX Port 2 to Merge in HTP (default) mode.
    static constexpr uint8_t kMergeHtp3 = 0x53; ///< Set DMX Port 3 to Merge in HTP (default) mode.

    static constexpr uint8_t kArtnetSel0 = 0x60; ///< Set DMX Port 0 to output both DMX512 and RDM packets from the Art-Net protocol (default).
    static constexpr uint8_t kArtnetSel1 = 0x61; ///< Set DMX Port 1 to output both DMX512 and RDM packets from the Art-Net protocol (default).
    static constexpr uint8_t kArtnetSel2 = 0x62; ///< Set DMX Port 2 to output both DMX512 and RDM packets from the Art-Net protocol (default).
    static constexpr uint8_t kArtnetSel3 = 0x63; ///< Set DMX Port 3 to output both DMX512 and RDM packets from the Art-Net protocol (default).

    static constexpr uint8_t kAcnSel0 = 0x70; ///< Set DMX Port 0 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
    static constexpr uint8_t kAcnSel1 = 0x71; ///< Set DMX Port 1 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
    static constexpr uint8_t kAcnSel2 = 0x72; ///< Set DMX Port 2 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.
    static constexpr uint8_t kAcnSel3 = 0x73; ///< Set DMX Port 3 to output DMX512 data from the sACN protocol and RDM data from the Art-Net protocol.

    static constexpr uint8_t kClr0 = 0x90; ///< Clear DMX Output buffer for Port 0
    static constexpr uint8_t kClr1 = 0x91; ///< Clear DMX Output buffer for Port 1
    static constexpr uint8_t kClr2 = 0x92; ///< Clear DMX Output buffer for Port 2
    static constexpr uint8_t kClr3 = 0x93; ///< Clear DMX Output buffer for Port 3

    static constexpr uint8_t kStyleDelta0 = 0xa0; ///< Set output style to delta on output port 0
    static constexpr uint8_t kStyleDelta1 = 0xa1; ///< Set output style to delta on output port 1
    static constexpr uint8_t kStyleDelta2 = 0xa2; ///< Set output style to delta on output port 2
    static constexpr uint8_t kStyleDelta3 = 0xa3; ///< Set output style to delta on output port 3

    static constexpr uint8_t kStyleConstant0 = 0xb0; ///< Set output style to continuous on output port 0
    static constexpr uint8_t kStyleConstant1 = 0xb1; ///< Set output style to continuous on output port 1
    static constexpr uint8_t kStyleConstant2 = 0xb2; ///< Set output style to continuous on output port 2
    static constexpr uint8_t kStyleConstant3 = 0xb3; ///< Set output style to continuous on output port 3

    static constexpr uint8_t kRdmEnable0 = 0xc0; ///< Enable RDM on output port 0
    static constexpr uint8_t kRdmEnable1 = 0xc1; ///< Enable RDM on output port 1
    static constexpr uint8_t kRdmEnable2 = 0xc2; ///< Enable RDM on output port 2
    static constexpr uint8_t kRdmEnable3 = 0xc3; ///< Enable RDM on output port 3

    static constexpr uint8_t kRdmDisable0 = 0xd0; ///< Disable RDM on output port 0
    static constexpr uint8_t kRdmDisable1 = 0xd1; ///< Disable RDM on output port 1
    static constexpr uint8_t kRdmDisable2 = 0xd2; ///< Disable RDM on output port 2
    static constexpr uint8_t kRdmDisable3 = 0xd3; ///< Disable RDM on output port 3
};

struct TodControlCommand
{
    static constexpr uint8_t kAtcNone = 0x00;   ///< No action.
    static constexpr uint8_t kAtcFlush = 0x01;  ///< The port flushes its TOD and instigates full discovery.
    static constexpr uint8_t kAtcEnd = 0x02;    ///< The port ends current discovery but does not flush ToD.
    static constexpr uint8_t kAtcIncon = 0x03;  ///< The port enables incremental discovery.
    static constexpr uint8_t kAtcIncoff = 0x04; ///< The port disables incremental discovery.
};

struct Program
{
    static constexpr uint8_t kNoChange = 0x7F;
    static constexpr uint8_t kDefaults = 0x00;
    static constexpr uint8_t kChangeMask = 0x80;
};

struct Status1
{
    static constexpr uint8_t kIndicatorMask = (3U << 6);       ///< 0b11 bit 7-6, Indicator state.
    static constexpr uint8_t kIndicatorLocateMode = (1U << 6); ///< 0b01 Indicators in Locate Mode.
    static constexpr uint8_t kIndicatorMuteMode = (2U << 6);   ///< 0b10 Indicators in Mute Mode.
    static constexpr uint8_t kIndicatorNormalMode = (3U << 6); ///< 0b11 Indicators in Normal Mode.
    static constexpr uint8_t kPapMask = (3U << 4);             ///< 0b11 bit 5-4, Port Address Programming Authority
    static constexpr uint8_t kPapUnknown = (0 << 4);           ///< 0b00 Port Address Programming Authority unknown.
    static constexpr uint8_t kPapFrontPanel = (1U << 4);       ///< 0b01 All Port Address set by front panel controls.
    static constexpr uint8_t kPapNetwork = (2U << 4);          ///< 0b10 All or part of Port Address programmed by network or Web browser.
    static constexpr uint8_t kPapNotused = (3U << 4);          ///< 0b11 Not used.
    static constexpr uint8_t kNormalFirmwareBoot = (0 << 2);   ///< 0 = Normal firmware boot (from flash).
    static constexpr uint8_t kRomBoot = (1U << 2);             ///< 1 = Booted from ROM.
    static constexpr uint8_t kRdmCapable = (1U << 1);          ///< 1 = Capable of Remote Device Management
    static constexpr uint8_t kUbeaPresent = (1U << 0);         ///< 1 = UBEA present
};

struct Status2
{
    static constexpr uint8_t kWebBrowserNoSupport = (0 << 0); ///< bit 0 = 0 Node does not support web browser
    static constexpr uint8_t kWebBrowserSupport = (1U << 0);  ///< bit 0 = 1 Node supports web browser configuration
    static constexpr uint8_t kIpManualy = (0 << 1);           ///< bit 1 = 0 Node's IP address is manually configured
    static constexpr uint8_t kIpDhcp = (1U << 1);             ///< bit 1 = 1 Node's IP address is DHCP configured
    static constexpr uint8_t kDhcpNotCapable = (0 << 2);      ///< bit 2 = 0 Node is not DHCP capable
    static constexpr uint8_t kDhcpCapable = (1U << 2);        ///< bit 2 = 1 Node is DHCP capable
    static constexpr uint8_t kPortAddress8Bit = (0 << 3);     ///< bit 3 = 0 Node supports 8 bit Port-Address (Art-Net II).
    static constexpr uint8_t kPortAddress15Bit = (1U << 3);   ///< bit 3 = 1 Node supports 15 bit Port-Address (Art-Net 3 or 4).
    static constexpr uint8_t kSacnNoSwitch = (0 << 4);        ///< bit 4 = 0 Node is not able to switch Art-Net/sACN
    static constexpr uint8_t kSacnAbleToSwitch = (1U << 4);   ///< bit 4 = 1 Node is able to switch Art-Net/sACN
    static constexpr uint8_t kOutputStyleNoSwitch = (0 << 6); ///< bit 6 = 0 Node is not able to switch output style by ArtCommand
    static constexpr uint8_t kOutputStyleSwitch = (1U << 6);  ///< bit 6 = 1 Node is able to switch output style by ArtCommand
    static constexpr uint8_t kRdmNoSwitch = (0 << 7);         ///< bit 7 = 0 Node is not able to enable or disable RDM by ArtCommand
    static constexpr uint8_t kRdmSwitch = (1U << 7);          ///< bit 7 = 1 Node is able to enable or disable RDM by ArtCommand
};

struct Status3
{
    static constexpr uint8_t kNetworklossMask = (3U << 6);             ///< bit 76
    static constexpr uint8_t kNetworklossLastState = (0 << 6);         ///< bit 76 = 00 If network data is lost, it will hold last state
    static constexpr uint8_t kNetworklossOffState = (1U << 6);         ///< bit 76 = 01 If network data is lost, it will set all outputs to off state
    static constexpr uint8_t kNetworklossOnState = (2U << 6);          ///< bit 76 = 10 If network data is lost, it will set all outputs to full on
    static constexpr uint8_t kNetworklossPlayback = (3U << 6);         ///< bit 76 = 11 If network data is lost, it will playback the fail-over scene
    static constexpr uint8_t kFailsafeNoControl = (0 << 5);            ///< bit 5 = 0 Node is not able to control failsafe mode by ArtCommand
    static constexpr uint8_t kFailsafeControl = (1U << 5);             ///< bit 5 = 1 Node is able to control failsafe mode by ArtCommand
    static constexpr uint8_t kSupportsLlrp = (1U << 4);                ///< bit 4 = 1 Node supports LLRP (Low Level Recovery Protocol
    static constexpr uint8_t kOutputNoSwitch = (0 << 3);               ///< bit 3 = 0 Outputs cannot be switched to an input
    static constexpr uint8_t kOutputSwitch = (1U << 3);                ///< bit 3 = 1 Outputs can be switched to an input
    static constexpr uint8_t kSupportsRdmnet = (1U << 2);              ///< bit 2 = 1 Node supports RDMnet
    static constexpr uint8_t kSupportsBackgroundqueue = (1U << 1);     ///< bit 1 = 1 BackgroundQueue is supported
    static constexpr uint8_t kSupportsBackgrounddiscovery = (1U << 0); ///< bit 0 = 1 Programmable background discovery is supported.
};

struct Flags
{
    ///< bit 1 = 0 Node only sends ArtPollReply when polled
    static constexpr uint8_t kSendArtpOnChange = (1U << 1);    ///< bit 1 = 1 Node sends ArtPollReply when it needs to
                                                               ///< bit 2 = 0 Do not send me diagnostic messages
    static constexpr uint8_t kSendDiagMessages = (1U << 2);    ///< bit 2 = 1 Send me diagnostics messages.
                                                               ///< bit 3 = 0 Diagnostics messages are broadcast. (if bit 2).
    static constexpr uint8_t kSendDiagUnicast = (1U << 3);     ///< bit 3 = 1 Diagnostics messages are unicast. (if bit 2).
                                                               ///< bit 5 = 0 Ignore TargetPortAddress
    static constexpr uint8_t kUseTargetPortAddress = (1U < 5); ///< bit 5 = 1 Only reply if device has a Port-Address that is
                                                               ///<		   inclusively in the range TargetPortAddressBottom to
                                                               ///<		   TargetPortAddressTop.
};

struct GoodOutput
{
    static constexpr uint8_t kDataIsBeingTransmitted = (1U << 7); ///< bit 7 data is transmitting
    static constexpr uint8_t kIncludesDmxTestPackets = (1U << 6); ///< bit 6 data includes test packets
    static constexpr uint8_t kIncludesDmxSip = (1U << 5);         ///< bit 5 data includes SIP's
    static constexpr uint8_t kIncludesDmxTextPackets = (1U << 4); ///< bit 4 data includes text
    static constexpr uint8_t kOutputIsMerging = (1U << 3);        ///< bit 3 output is merging data.
    static constexpr uint8_t kDmxShortDetected = (1U << 2);       ///< bit 2 set if DMX output short detected on power up
    static constexpr uint8_t kMergeModeLtp = (1U << 1);           ///< bit 1 set if DMX output merge mode is LTP
    static constexpr uint8_t kOutputIsSacn = (1U << 0);           ///< bit 0 set if outputting sACN.
    static constexpr uint8_t kOutputNone = 0;
};

struct GoodOutputB
{
    static constexpr uint8_t kRdmDisabled = (1U << 7);         ///< bit 7 = 1 RDM is disabled.
    static constexpr uint8_t kRdmEnabled = (0 << 7);           ///< bit 7 = 0 RDM is enabled.
    static constexpr uint8_t kStyleConstant = (1U << 6);       ///< bit 6 = 1 Output style is continuous.
    static constexpr uint8_t kStyleDelta = (0 << 6);           ///< bit 6 = 0 Output style is delta.
    static constexpr uint8_t kDiscoveryNotRunning = (1U << 5); ///< bit 5 = 1 Discovery is currently not running.
    static constexpr uint8_t kDiscoveryIsRunning = (0 << 5);   ///< bit 5 = 0 Discovery is currently running.
    static constexpr uint8_t kDiscoveryDisabled = (1U << 4);   ///< bit 4 = 1 Background discovery is disabled.
    static constexpr uint8_t kDiscoveryEnabled = (0 << 4);     ///< bit 4 = 0 Background Discovery is enabled.
};

struct GoodInput
{
    static constexpr uint8_t kDataRecieved = (1U << 7);        ///< bit 7 is data received
    static constexpr uint8_t kIncludesTestPackets = (1U << 6); ///< bit 6 is data includes test packets
    static constexpr uint8_t kIncludesSip = (1U << 5);         ///< bit 5 is data includes SIP's
    static constexpr uint8_t kIncludesText = (1U << 4);        ///< bit 4 is data includes text
    static constexpr uint8_t kDisabled = (1U << 3);            ///< bit 3 set is input is disabled
    static constexpr uint8_t kErrors = (1U << 2);              ///< bit 2 is receive errors
    static constexpr uint8_t kInputIsSacn = (1U << 0);         ///<
};

inline const char* GetProtocolMode(artnet::PortProtocol protocol, bool to_upper = false)
{
    if (to_upper)
    {
        return (protocol == artnet::PortProtocol::kArtnet) ? "Art-Net" : "sACN";
    }
    return (protocol == artnet::PortProtocol::kArtnet) ? "artnet" : "sacn";
}

inline const char* GetProtocolMode(uint32_t protocol, bool to_upper = false)
{
    return GetProtocolMode(static_cast<artnet::PortProtocol>(protocol), to_upper);
}

inline artnet::PortProtocol GetProtocolMode(const char* protocol_mode)
{
    if (protocol_mode != nullptr)
    {
        if (((protocol_mode[0] | 0x20) == 's') && ((protocol_mode[1] | 0x20) == 'a') && ((protocol_mode[2] | 0x20) == 'c') &&
            ((protocol_mode[3] | 0x20) == 'n'))
        {
            return artnet::PortProtocol::kSacn;
        }
    }
    return artnet::PortProtocol::kArtnet;
}

inline uint16_t MakePortAddress(uint32_t net_switch, uint32_t sub_switch, uint32_t sw)
{
    // PortAddress Bit 15 = 0
    uint16_t port_address = (net_switch & 0x7F) << 8;                // Net : Bits 14-8
    port_address |= static_cast<uint16_t>((sub_switch & 0x0F) << 4); // Sub-Net : Bits 7-4
    port_address |= static_cast<uint16_t>(sw & 0x0F);                // Universe : Bits 3-0

    return port_address;
}

/**
 * Table 1 - OpCodes
 * The supported legal OpCode values used in Art-Net packets
 */
enum class OpCodes : uint16_t
{
    kOpPoll = 0x2000,        ///< This is an ArtPoll packet, no other data is contained in this UDP packet.
    kOpPollreply = 0x2100,   ///< This is an ArtPollReply Packet. It contains device status information.
    kOpDiagdata = 0x2300,    ///< Diagnostics and data logging packet.
    kOpDmx = 0x5000,         ///< This is an ArtDmx data packet. It contains zero start code DMX512 information for a single Universe.
    kOpSync = 0x5200,        ///< This is an ArtSync data packet. It is used to force synchronous transfer of ArtDmx packets to a node’s output.
    kOpAddress = 0x6000,     ///< This is an ArtAddress packet. It contains remote programming information for a Node.
    kOpInput = 0x7000,       ///< This is an ArtInput packet. It contains enable – disable data for DMX inputs.
    kOpTodrequest = 0x8000,  ///< This is an ArtTodRequest packet. It is used to request a Table of Devices (ToD) for RDM discovery.
    kOpToddata = 0x8100,     ///< This is an ArtTodData packet. It is used to send a Table of Devices (ToD) for RDM discovery.
    kOpTodcontrol = 0x8200,  ///< This is an ArtTodControl packet. It is used to send RDM discovery control messages
    kOpRdm = 0x8300,         ///< This is an ArtRdm packet. It is used to send all non discovery RDM messages.
    kOpRdmsub = 0x8400,      ///< This is an ArtRdmSub packet. It is used to send compressed, RDM Sub-Device data.
    kOpTimecode = 0x9700,    ///< This is an ArtTimeCode packet. It is used to transport time code over the network.
    kOpTimesync = 0x9800,    ///< Used to synchronize real time date and clock
    kOpTrigger = 0x9900,     ///< Used to send trigger macros
    kOpDirectory = 0x9A00,   ///< Requests a node's file list
    kOpIpprog = 0xF800,      ///< This is an ArtIpProg packet. It is used to re-programm the IP, Mask and Port address of the Node.
    kOpIpprogreply = 0xF900, ///< This is an ArtIpProgReply packet. It is returned by the node to acknowledge receipt of an ArtIpProg packet.
    kOpNotDefined = 0x0000   ///< OP_NOT_DEFINED
};

struct ArtPoll
{
    uint8_t Id[8];     ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t OpCode;   ///< The OpCode defines the class of data following ArtPoll within this UDP packet. Transmitted low byte first. See \ref TOpCodes for the
                       ///< OpCode listing.
    uint8_t ProtVerHi; ///< High byte of the Art-Net protocol revision number.
    uint8_t ProtVerLo; ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t Flags;     ///< Set behavior of Node
    uint8_t DiagPriority; ///< The lowest priority of diagnostics message that should be sent. See \ref TPriorityCodes
    uint8_t TargetPortAddressTopHi;
    uint8_t TargetPortAddressTopLo;
    uint8_t TargetPortAddressBottomHi;
    uint8_t TargetPortAddressBottomLo;
} PACKED;

struct ArtPollReply
{
    uint8_t Id[8];        ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t OpCode;      ///< OpPollReply \ref TOpCodes
    uint8_t IPAddress[4]; ///< Array containing the Node’s IP address. First array entry is most significant byte of address.
    uint16_t Port;        ///< The Port is always 0x1936
    uint8_t VersInfoH;    ///< High byte of Node’s firmware revision number.
    uint8_t VersInfoL;    ///< Low byte of Node’s firmware revision number.
    uint8_t NetSwitch;    ///< Bits 14-8 of the 15 bit Port-Address are encoded into the bottom 7 bits of this field.
    uint8_t SubSwitch;    ///< Bits 7-4 of the 15 bit Port-Address are encoded into the bottom 4 bits of this field.
    uint8_t OemHi;        ///< The high byte of the Oem value
    uint8_t Oem;          ///< The low byte of the Oem value. The Oem word describes the equipment vendor and the feature set available.
    uint8_t Ubea; ///< This field contains the firmware version of the User Bios Extension Area (UBEA). If the UBEA is not programmed, this field contains zero.
    uint8_t Status1;    ///< General Status register
    uint8_t EstaMan[2]; ///< The ESTA manufacturer code. These codes are used to represent equipment manufacturer. They are assigned by ESTA.
    uint8_t ShortName[artnet::kShortNameLength]; ///< The array represents a null terminated short name for the Node.
    uint8_t LongName[artnet::kLongNameLength];   ///< The array represents a null terminated long name for the Node.
    uint8_t NodeReport[artnet::kReportLength]; ///< The array is a textual report of the Node’s operating status or operational errors. It is primarily intended
                                               ///< for ‘engineering’ data rather than ‘end user’ data.
    uint8_t
        NumPortsHi; ///< The high byte of the word describing the number of input or output ports. The high byte is for future expansion and is currently zero.
    uint8_t NumPortsLo;                 ///< The low byte of the word describing the number of input or output ports.
    uint8_t PortTypes[artnet::kPorts];  ///< This array defines the operation and protocol of each channel.
    uint8_t GoodInput[artnet::kPorts];  ///< This array defines input status of the node.
    uint8_t GoodOutput[artnet::kPorts]; ///< This array defines output status of the node.
    uint8_t SwIn[artnet::kPorts];       ///< Bits 3-0 of the 15 bit Port-Address for each of the 4 possible input ports are encoded into the low nibble.
    uint8_t SwOut[artnet::kPorts];      ///< Bits 3-0 of the 15 bit Port-Address for each of the 4 possible output ports are encoded into the low nibble.
    uint8_t AcnPriority;                ///< sACN Priority for sACN generated by this node
    uint8_t SwMacro;                    ///< If the Node supports macro key inputs, this byte represents the trigger values.
    uint8_t SwRemote;                   ///< If the Node supports remote trigger inputs, this byte represents the trigger values.
    uint8_t spare1;                     ///< Not used, set to zero
    uint8_t spare2;                     ///< Not used, set to zero
    uint8_t spare3;                     ///< Not used, set to zero
    uint8_t Style;                      ///< The Style code defines the equipment style of the device. See \ref TNodeStyleCode
    uint8_t MAC[artnet::kMacSize];      ///< MAC Address
    uint8_t BindIp[4];                  ///< If this unit is part of a larger or modular product, this is the IP of the root device.
    uint8_t bind_index; ///< Set to zero if no binding, otherwise this number represents the order of bound devices. A lower number means closer to root device.
                        ///< A value of 1 means root device.
    uint8_t Status2;    ///<
    uint8_t GoodOutputB[artnet::kPorts]; ///< This array defines output status of the node.
    uint8_t Status3;                     ///< General Status register
    uint8_t DefaultUidResponder[6];      ///< RDMnet & LLRP UID
    uint8_t UserHi;                      ///< Available for user specific data
    uint8_t UserLo;                      ///< Available for user specific data
    uint8_t RefreshRateHi;               ///< Hi byte of RefreshRate
    uint8_t RefreshRateLo;               ///< Lo Byte of RefreshRate.
                           ///< RefreshRate allows the device to specify the maximum refresh rate, expressed in Hz, at which it can process ArtDmx.
    ///< This is designed to allow refresh rates above DMX512 rates, for gateways that implement other protocols such as SPI.
    ///< A value of 0 to 44 represents the maximum DMX512 rate of 44Hz.
    uint8_t BackgroundQueuePolicy; ///< The BackgroundQueuePolicy defines the method by with the node retrieves STATUS_MESSAGE and QUEUED_MESSAGE pids from the
                                   ///< connected RDM devices.
    uint8_t Filler[10];            ///< Transmit as zero. For future expansion.
} PACKED;

/**
 * ArtDmx is the data packet used to transfer DMX512 data.
 */
struct ArtDmx
{
    uint8_t Id[8];                    ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t OpCode;                  ///< OpDmx \ref TOpCodes
    uint8_t ProtVerHi;                ///< High byte of the Art-Net protocol revision number.
    uint8_t ProtVerLo;                ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t Sequence;                 ///< The sequence number is used to ensure that ArtDmx packets are used in the correct order.
    uint8_t Physical;                 ///< The physical input port from which DMX512 data was input.
    uint16_t PortAddress;             ///< The 15 bit Port-Address to which this packet is destined.
    uint8_t LengthHi;                 ///< The length of the DMX512 data array. This value should be an even number in the range 2 – 512.
    uint8_t Length;                   ///< Low Byte of above.
    uint8_t data[artnet::kDmxLength]; ///< A variable length array of DMX512 lighting data.
} PACKED;

struct ArtDiagData
{
    uint8_t Id[8];     ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t OpCode;   ///< OpDiagData See \ref TOpCodes
    uint8_t ProtVerHi; ///< High byte of the Art-Net protocol revision number.
    uint8_t ProtVerLo; ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t filler1;   ///< Ignore by receiver, set to zero by sender
    uint8_t Priority;  ///< The priority of this diagnostic data. See \ref TPriorityCodes
    uint8_t filler2;   ///< Ignore by receiver, set to zero by sender
    uint8_t filler3;   ///< Ignore by receiver, set to zero by sender
    uint8_t LengthHi;  ///< The length of the text array below. High Byte.
    uint8_t LengthLo;  ///< Low byte
    uint8_t data[512]; ///< ASCII text array, null terminated. Max length is 512 bytes including the null terminator.
} PACKED;

struct ArtSync
{
    uint8_t Id[8];     ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t OpCode;   ///< OpSync \ref TOpCodes
    uint8_t ProtVerHi; ///< High byte of the Art-Net protocol revision number.
    uint8_t ProtVerLo; ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t aux1;      ///< Transmit as zero.
    uint8_t aux2;      ///< Transmit as zero.
} PACKED;

/**
 * ArtAddress packet definition
 *
 * Fields 5 to 13 contain the data that will be programmed into the node.
 */
struct ArtAddress
{
    uint8_t Id[8];     ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t OpCode;   ///< OpAddress \ref TOpCodes
    uint8_t ProtVerHi; ///< High byte of the Art-Net protocol revision number.
    uint8_t ProtVerLo; ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t
        NetSwitch; ///< This value is ignored unless bit 7 is high. Send 0x00 to reset this value to the physical switch setting. Use value 0x7f for no change.
    uint8_t bind_index; ///< The BindIndex defines the bound node which originated this packet and is used to uniquely identify the bound node when identical IP
                        ///< addresses are in use.
    uint8_t ShortName[artnet::kShortNameLength]; ///< The Node will ignore this value if the string is null.
    uint8_t LongName[artnet::kLongNameLength];   ///< The Node will ignore this value if the string is null.
    uint8_t SwIn[artnet::kPorts];  ///< This value is ignored unless bit 7 is high. Send 0x00 to reset this value to the physical switch setting. Use value 0x7f
                                   ///< for no change.
    uint8_t SwOut[artnet::kPorts]; ///< This value is ignored unless bit 7 is high. Send 0x00 to reset this value to the physical switch setting. Use value 0x7f
                                   ///< for no change.
    uint8_t
        SubSwitch; ///< This value is ignored unless bit 7 is high. Send 0x00 to reset this value to the physical switch setting. Use value 0x7f for no change.
    uint8_t SwVideo; ///< Reserved
    uint8_t Command; ///< Node configuration commands \ref TArtnetPortCommand
} PACKED;

/**
 * ArtInput packet definition
 *
 * A Controller or monitoring device on the network can
 * enable or disable individual DMX512 inputs on any of the network nodes.
 *
 * All nodes power on with all inputs enabled.
 */

struct ArtInput
{
    uint8_t Id[8];      ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t OpCode;    ///< OpAddress \ref TOpCodes
    uint8_t ProtVerHi;  ///< High byte of the Art-Net protocol revision number.
    uint8_t ProtVerLo;  ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t filler1;    ///< Pad length to match ArtPoll.
    uint8_t bind_index; ///< The BindIndex defines the bound node which originated this packet and is used to uniquely identify the bound node when identical IP
                        ///< addresses are in use.
    uint8_t
        NumPortsHi; ///< The high byte of the word describing the number of input or output ports. The high byte is for future expansion and is currently zero.
    uint8_t NumPortsLo;            ///< The low byte of the word describing the number of input or output ports.
    uint8_t Input[artnet::kPorts]; ///< This array defines input disable status of each channel.
} PACKED;

/**
 * ArtTimeCode packet definition
 *
 * ArtTimeCode allows time code to be transported over the network.
 * The data format is compatible with both longitudinal time code and MIDI time code.
 * The four key types of Film, EBU, Drop Frame and SMPTE are also encoded.
 *
 */
struct ArtTimeCode
{
    uint8_t Id[8];     ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t OpCode;   ///< OpAddress \ref TOpCodes
    uint8_t ProtVerHi; ///< High byte of the Art-Net protocol revision number.
    uint8_t ProtVerLo; ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t filler1;   ///< Ignore by receiver, set to zero by sender
    uint8_t filler2;   ///< Ignore by receiver, set to zero by sender
    uint8_t Frames;    ///< Frames time. 0 – 29 depending on mode.
    uint8_t Seconds;   ///< Seconds. 0 - 59.
    uint8_t Minutes;   ///< Minutes. 0 - 59.
    uint8_t Hours;     ///< Hours. 0 - 59.
    uint8_t Type;      ///< 0 = Film (24fps) , 1 = EBU (25fps), 2 = DF (29.97fps), 3 = SMPTE (30fps)
} PACKED;

struct ArtTimeSync
{
    uint8_t Id[8];      ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t OpCode;    ///< OpAddress \ref TOpCodes
    uint8_t ProtVerHi;  ///< High byte of the Art-Net protocol revision number.
    uint8_t ProtVerLo;  ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t filler1;    ///< Ignore by receiver, set to zero by sender
    uint8_t filler2;    ///< Ignore by receiver, set to zero by sender
    uint8_t prog;       ///<
    uint8_t tm_sec;     ///<
    uint8_t tm_min;     ///<
    uint8_t tm_hour;    ///<
    uint8_t tm_mday;    ///<
    uint8_t tm_mon;     ///<
    uint8_t tm_year_hi; ///<
    uint8_t tm_year_lo; ///<
    uint8_t tm_wday;    ///<
    uint8_t tm_isdst;   ///<
} PACKED;

/**
 * ArtTodRequest packet definition
 *
 * This packet is used to request the Table of RDM Devices (TOD).
 * A Node receiving this packet must not interpret it as forcing full discovery.
 * Full discovery is only initiated at power on or when an ArtTodControl.AtcFlush is received.
 * The response is ArtTodData.
 */
struct ArtTodRequest
{
    uint8_t Id[8];       ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t OpCode;     ///< OpAddress \ref TOpCodes
    uint8_t ProtVerHi;   ///< High byte of the Art-Net protocol revision number.
    uint8_t ProtVerLo;   ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t filler1;     ///< Pad length to match ArtPoll.
    uint8_t filler2;     ///< Pad length to match ArtPoll.
    uint8_t spare1;      ///< Transmit as zero, receivers don’t test.
    uint8_t spare2;      ///< Transmit as zero, receivers don’t test.
    uint8_t spare3;      ///< Transmit as zero, receivers don’t test.
    uint8_t spare4;      ///< Transmit as zero, receivers don’t test.
    uint8_t spare5;      ///< Transmit as zero, receivers don’t test.
    uint8_t spare6;      ///< Transmit as zero, receivers don’t test.
    uint8_t spare7;      ///< Transmit as zero, receivers don’t test.
    uint8_t Net;         ///< The top 7 bits of the 15 bit Port-Address of Nodes that must respond to this packet.
    uint8_t Command;     ///< 0x00 TodFull Send the entire TOD.
    uint8_t AddCount;    ///< The array size of the Address field. Max value is 32.
    uint8_t Address[32]; ///< This array defines the low byte of the Port-Address of the Output Gateway nodes that must respond to this packet.
} PACKED;

/**
 * The ArtTodControl packet is used to send RDM control parameters over Art-Net.
 * The response is ArtTodData.
 */
struct ArtTodControl
{
    uint8_t Id[8];     ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t OpCode;   ///< OpAddress \ref TOpCodes
    uint8_t ProtVerHi; ///< High byte of the Art-Net protocol revision number.
    uint8_t ProtVerLo; ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t filler1;   ///< Pad length to match ArtPoll.
    uint8_t filler2;   ///< Pad length to match ArtPoll.
    uint8_t spare1;    ///< Transmit as zero, receivers don’t test.
    uint8_t spare2;    ///< Transmit as zero, receivers don’t test.
    uint8_t spare3;    ///< Transmit as zero, receivers don’t test.
    uint8_t spare4;    ///< Transmit as zero, receivers don’t test.
    uint8_t spare5;    ///< Transmit as zero, receivers don’t test.
    uint8_t spare6;    ///< Transmit as zero, receivers don’t test.
    uint8_t spare7;    ///< Transmit as zero, receivers don’t test.
    uint8_t Net;       ///< The top 7 bits of the 15 bit Port-Address of Nodes that must respond to this packet.
    uint8_t Command;   ///< 0x00 AtcNone No action. 0x01 AtcFlush The node flushes its TOD and instigates full discovery.
    uint8_t Address;   ///< The low byte of the 15 bit Port-Address of the DMX Port that should action this command.
} PACKED;

struct ArtTodData
{
    uint8_t Id[8];           ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t OpCode;         ///< OpAddress \ref TOpCodes
    uint8_t ProtVerHi;       ///< High byte of the Art-Net protocol revision number.
    uint8_t ProtVerLo;       ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t RdmVer;          ///< Art-Net Devices that only support RDM DRAFT V1.0 set field to 0x00. Devices that support RDM STANDARD V1.0 set field to 0x01.
    uint8_t Port;            ///< Physical Port. Range 1-4.
    uint8_t spare1;          ///< Transmit as zero, receivers don’t test.
    uint8_t spare2;          ///< Transmit as zero, receivers don’t test.
    uint8_t spare3;          ///< Transmit as zero, receivers don’t test.
    uint8_t spare4;          ///< Transmit as zero, receivers don’t test.
    uint8_t spare5;          ///< Transmit as zero, receivers don’t test.
    uint8_t spare6;          ///< Transmit as zero, receivers don’t test.
    uint8_t bind_index;      ///< The BindIndex defines the bound node which originated this packet. In combination with Port and Source IP address, it uniquely
                             ///< identifies the sender.
    uint8_t Net;             ///< The top 7 bits of the 15 bit Port-Address of Nodes that must respond to this packet.
    uint8_t CommandResponse; ///< 0x00 TodFull The packet contains the entire TOD or is the first packet in a sequence of packets that contains the entire TOD.
    uint8_t Address; ///< The low 8 bits of the Port-Address of the Output Gateway DMX Port that generated this packet. The high nibble is the Sub-Net switch.
                     ///< The low nibble corresponds to the Universe.
    uint8_t UidTotalHi; ///< The total number of RDM devices discovered by this Universe.
    uint8_t UidTotalLo;
    uint8_t BlockCount;  ///< The index number of this packet. When UidTotal exceeds 200, multiple ArtTodData packets are used.
    uint8_t UidCount;    ///< The number of UIDs encoded in this packet. This is the index of the following array.
    uint8_t Tod[200][6]; ///< 48 bit An array of RDM UID.
} PACKED;

/**
 * 	The ArtRdm packet is used to transport all non-discovery RDM messages over Art-Net.
 */
struct ArtRdm
{
    uint8_t Id[8];          ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t OpCode;        ///< OpAddress \ref TOpCodes
    uint8_t ProtVerHi;      ///< High byte of the Art-Net protocol revision number.
    uint8_t ProtVerLo;      ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t RdmVer;         ///< Art-Net Devices that only support RDM DRAFT V1.0 set field to 0x00. Devices that support RDM STANDARD V1.0 set field to 0x01.
    uint8_t filler2;        ///< Pad length to match ArtPoll.
    uint8_t spare1;         ///< Transmit as zero, receivers don’t test.
    uint8_t spare2;         ///< Transmit as zero, receivers don’t test.
    uint8_t spare3;         ///< Transmit as zero, receivers don’t test.
    uint8_t spare4;         ///< Transmit as zero, receivers don’t test.
    uint8_t spare5;         ///< Transmit as zero, receivers don’t test.
    uint8_t spare6;         ///< Transmit as zero, receivers don’t test.
    uint8_t spare7;         ///< Transmit as zero, receivers don’t test.
    uint8_t Net;            ///< The top 7 bits of the 15 bit Port-Address of Nodes that must respond to this packet.
    uint8_t Command;        ///< 0x00 ArProcess Process RDM Packet0x00 AtcNone No action. 0x01 AtcFlush The node flushes its TOD and instigates full discovery.
    uint8_t Address;        ///< The low 8 bits of the Port-Address that should action this command.
    uint8_t RdmPacket[256]; ///< The RDM data packet excluding the DMX StartCode with Checksum
} PACKED;

/**
 * The ArtRdmSub packet is used to transfer Get, Set, GetResponse and SetResponse data to and from multiple sub-devices within an RDM device.
 */
struct ArtRdmSub
{
    uint8_t id[8];           ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t op_code;        ///< OpAddress \ref TOpCodes
    uint8_t prot_ver_hi;     ///< High byte of the Art-Net protocol revision number.
    uint8_t prot_ver_lo;     ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t rdm_ver;         ///< Art-Net Devices that only support RDM DRAFT V1.0 set field to 0x00. Devices that support RDM STANDARD V1.0 set field to 0x01.
    uint8_t filler2;         ///< Transmit as zero, receivers don’t test.
    uint8_t uid[6];          ///< UID of target RDM device.
    uint8_t spare1;          ///< Transmit as zero, receivers don’t test.
    uint8_t command_class;   ///< As per RDM specification. This field defines whether this is a Get, Set, GetResponse, SetResponse.
    uint8_t parameter_id[2]; ///< As per RDM specification. This field defines the type of parameter contained in this packet. Big- endian.
    uint8_t sub_device[2];   ///< Defines the first device information contained in packet. This follows the RDM convention that 0 = root device and 1 = first
                             ///< subdevice. Big-endian.
    uint8_t sub_count[2];    ///< The number of sub devices packed into packet. Zero is illegal. Big-endian.
    uint8_t spare2;          ///< Transmit as zero, receivers don’t test.
    uint8_t spare3;          ///< Transmit as zero, receivers don’t test.
    uint8_t spare4;          ///< Transmit as zero, receivers don’t test.
    uint8_t spare5;          ///< Transmit as zero, receivers don’t test.
    uint8_t data[231];
} PACKED;

struct ArtIpProg
{
    uint8_t id[8];    ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t op_code; ///< The OpCode defines the class of data following ArtPoll within this UDP packet. Transmitted low byte first. See \ref TOpCodes for the
                      ///< OpCode listing.
    uint8_t prot_ver_hi; ///< High byte of the Art-Net protocol revision number.
    uint8_t prot_ver_lo; ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t filler1;     ///< Pad length to match ArtPoll.
    uint8_t filler2;     ///< Pad length to match ArtPoll.
    uint8_t command;     ///< Defines the how this packet is processed.
    uint8_t filler;      ///< Set to zero. Pads data structure for word alignment.
    uint8_t prog_ip_hi;  ///< IP Address to be programmed into Node if enabled by Command Field
    uint8_t prog_ip2;
    uint8_t prog_ip1;
    uint8_t prog_ip_lo;
    uint8_t prog_sm_hi; ///< Subnet mask to be programmed into Node if enabled by Command Field
    uint8_t prog_sm2;
    uint8_t prog_sm1;
    uint8_t prog_sm_lo;
    uint8_t prog_port_hi; ///< PortAddress to be programmed into Node if enabled by Command Field
    uint8_t prog_port_lo;
    uint8_t prog_gw_hi; ///< Gateway to be programmed into Node if enabled by Command Field
    uint8_t prog_gw2;
    uint8_t prog_gw1;
    uint8_t prog_gwt_lo;
} PACKED;

struct ArtIpProgReply
{
    uint8_t id[8];    ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t op_code; ///< The OpCode defines the class of data following ArtPoll within this UDP packet. Transmitted low byte first. See \ref TOpCodes for the
                      ///< OpCode listing.
    uint8_t prot_ver_hi; ///< High byte of the Art-Net protocol revision number.
    uint8_t prot_ver_lo; ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t filler1;     ///< Pad length to match ArtPoll.
    uint8_t filler2;     ///< Pad length to match ArtPoll.
    uint8_t filler3;     ///< Pad length to match ArtIpProg.
    uint8_t filler4;     ///< Pad length to match ArtIpProg.
    uint8_t prog_ip_hi;  ///< IP Address of Node
    uint8_t prog_ip2;
    uint8_t prog_ip1;
    uint8_t prog_ip_lo;
    uint8_t prog_sm_hi; ///< Subnet mask of Node
    uint8_t prog_sm2;
    uint8_t prog_sm1;
    uint8_t prog_sm_lo;
    uint8_t prog_port_hi; ///< Port Address of Node
    uint8_t prog_port_lo;
    uint8_t status; ///< Bit 6 DHCP enabled.
    uint8_t spare2;
    uint8_t prog_gw_hi; ///< Gateway of Node
    uint8_t prog_gw2;
    uint8_t prog_gw1;
    uint8_t prog_gwt_lo;
    uint8_t spare7;
    uint8_t spare8;
} PACKED;

struct ArtTrigger
{
    uint8_t id[8];       ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t op_code;    ///< OpPollReply
    uint8_t prot_ver_hi; ///< High byte of the Art-Net protocol revision number.
    uint8_t prot_ver_lo; ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t filler1;
    uint8_t filler2;
    uint8_t oem_code_hi; ///< The manufacturer code (high byte) of nodes that shall accept this trigger.
    uint8_t oem_code_lo; ///< The manufacturer code (low byte) of nodes that shall accept this trigger.
    uint8_t key;         ///< The Trigger Key.
    uint8_t sub_key;     ///< The Trigger SubKey.
    uint8_t data[512];   ///< The interpretation of the payload is defined by the Key.
} PACKED;

struct ArtDirectory
{
    uint8_t id[8];       ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t op_code;    ///< OpPollReply \ref TOpCodes
    uint8_t prot_ver_hi; ///< High byte of the Art-Net protocol revision number.
    uint8_t prot_ver_lo; ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t filler1;
    uint8_t filler2;
    uint8_t command; ///< Defines the purpose of the packet
    uint8_t file_hi; ///< The most significant byte of the file number requested
    uint8_t file_lo; ///< The least significant byte of the file number requested
} PACKED;

struct ArtDirectoryReply
{
    uint8_t id[8];       ///< Array of 8 characters, the final character is a null termination. Value = ‘A’ ‘r’ ‘t’ ‘-‘ ‘N’ ‘e’ ‘t’ 0x00
    uint16_t op_code;    ///< OpPollReply \ref TOpCodes
    uint8_t prot_ver_hi; ///< High byte of the Art-Net protocol revision number.
    uint8_t prot_ver_lo; ///< Low byte of the Art-Net protocol revision number. Current value 14.
    uint8_t filler1;
    uint8_t filler2;
    uint8_t flags;           ///< Bit fields
    uint8_t file_hi;         ///< The most significant byte of the file number requested
    uint8_t file_lo;         ///< The least significant byte of the file number requested
    uint8_t name83[16];      ///< The file's name
    uint8_t description[64]; ///< Description text for the file;
    uint8_t length[8];       ///< File length in bytes
    uint8_t data[64];        ///< Application specific data
} PACKED;

union UArtPacket
{
    struct ArtPoll art_poll;
    struct ArtPollReply art_poll_reply;
    struct ArtDmx art_dmx;
    struct ArtDiagData art_diag_data;
    struct ArtSync art_sync;
    struct ArtAddress art_address;
    struct ArtInput art_input;
    struct ArtTimeCode art_time_code;
    struct ArtTimeSync art_time_sync;
    struct ArtTodRequest art_tod_request;
    struct ArtTodControl art_tod_control;
    struct ArtTodData art_tod_data;
    struct ArtRdm art_rdm;
    struct ArtIpProg art_ip_prog;
    struct ArtIpProgReply art_ip_prog_reply;
    struct ArtTrigger art_trigger;
    struct ArtDirectory art_directory;
    struct ArtDirectoryReply art_directory_reply;
};

struct ArtPollQueue
{
    uint32_t art_poll_millis;
    uint32_t art_poll_reply_ip_address;
    struct
    {
        uint16_t target_port_address_top;
        uint16_t target_port_address_bottom;
    } art_poll_reply;
};
} // namespace artnet

#endif // ARTNET_H_
