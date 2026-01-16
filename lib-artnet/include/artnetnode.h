/**
 * @file artnetnode.h
 *
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

#ifndef ARTNETNODE_H_
#define ARTNETNODE_H_

#if defined(DEBUG_ARTNETNODE)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <cassert>

#if !defined(ARTNET_VERSION)
#error ARTNET_VERSION is not defined
#endif

#if (ARTNET_VERSION >= 4) && defined(ARTNET_HAVE_DMXIN)
#if !defined(E131_HAVE_DMXIN)
#define E131_HAVE_DMXIN
#endif
#endif

#if defined(NODE_SHOWFILE) && defined(CONFIG_SHOWFILE_PROTOCOL_NODE_ARTNET)
#define ARTNET_SHOWFILE
#endif

#include "artnet.h"
#include "artnettimecode.h"
#include "artnettrigger.h"
#if defined(RDM_CONTROLLER)
#include "artnetrdmcontroller.h"
#endif
#if defined(RDM_RESPONDER)
#include "artnetrdmresponder.h"
#endif
#if (ARTNET_VERSION >= 4)
#include "e131bridge.h"
#endif
#include "dmxnode.h"
#include "dmxnode_outputtype.h"
#include "network.h"
#include "firmware/debug/debug_debug.h"

#ifndef ALIGNED
#define ALIGNED __attribute__((aligned(4)))
#endif

namespace artnetnode
{
inline constexpr uint32_t kPollReplyQueueSize = 4;

enum class PollReplyState : uint8_t
{
    kWaitingTimeout,
    kRunning
};

struct State
{
    struct
    {
        uint32_t diag_ip;
        uint32_t poll_ip;
        uint32_t poll_reply_count;
        uint32_t poll_reply_delay_millis;
        uint32_t dmx_ip;
        uint32_t sync_millis; ///< Latest ArtSync received time
        artnet::ArtPollQueue poll_reply_queue[kPollReplyQueueSize];
        uint8_t poll_reply_queue_index;
        uint8_t poll_reply_port_index;
        PollReplyState poll_reply_state;
    } art;
    artnet::ReportCode report_code;
    artnet::Status status;
    bool send_art_poll_reply_on_change;    ///< ArtPoll : Flags Bit 1 : 1 = Send ArtPollReply whenever Node conditions change.
    bool send_art_diag_data;               ///< ArtPoll : Flags Bit 2 : 1 = Send me diagnostics messages.
    bool is_multiple_controllers_req_diag; ///< ArtPoll : Multiple controllers requesting diagnostics
    bool is_synchronous_mode;              ///< ArtSync received
    bool is_merge_mode;
    bool is_changed;
    bool disable_merge_timeout;
    bool do_record;
    bool is_rdm_enabled;
    uint8_t receiving_dmx;
    uint8_t enabled_output_ports;
    uint8_t enabled_input_ports;
    uint8_t diag_priority; ///< ArtPoll : Field 6 : The lowest priority of diagnostics message that should be sent.
};

struct Node
{
    struct
    {
        uint16_t port_address; ///< The Port-Address is a 15 bit number composed of Net+Sub-Net+Universe.
        uint8_t sw;            ///< Bits 3-0 of the 15 bit Port-Address for a given port are encoded into the bottom 4 bits of this field.
        uint8_t sub_switch;    ///< Bits 7-4 of the 15 bit Port-Address are encoded into the bottom 4 bits of this field.
        uint8_t net_switch;    ///< Bits 14-8 of the 15 bit Port-Address are encoded into the bottom 7 bits of this field.
        dmxnode::PortDirection direction;
        artnet::PortProtocol protocol; ///< Art-Net 4
        bool local_merge;
    } port[dmxnode::kMaxPorts] ALIGNED;

    uint32_t ip_timecode;
    bool map_universe0; ///< Art-Net 4
};

struct Source
{
    uint32_t millis;   ///< The latest time of the data received from port
    uint32_t ip;       ///< The IP address for port
    uint16_t physical; ///< The physical input port from which DMX512 data was input.
};

struct OutputPort
{
    Source source_a ALIGNED;
    Source source_b ALIGNED;
    uint32_t rdm_destination_ip;
    uint8_t good_output;
    uint8_t good_output_b;
    bool is_transmitting;
    bool is_data_pending;
};

struct InputPort
{
    uint32_t destination_ip;
    uint32_t millis;
    uint8_t sequence_number;
    uint8_t good_input;
};

inline artnet::FailSafe ConvertFailsafe(dmxnode::FailSafe failsafe)
{
    if (failsafe > dmxnode::FailSafe::kPlayback)
    {
        return artnet::FailSafe::kLast;
    }

    return static_cast<artnet::FailSafe>(static_cast<uint32_t>(failsafe) + static_cast<uint32_t>(artnet::FailSafe::kLast));
}

inline dmxnode::FailSafe ConvertFailsafe(artnet::FailSafe failsafe)
{
    if (failsafe > artnet::FailSafe::kRecord)
    {
        return dmxnode::FailSafe::kHold;
    }

    return static_cast<dmxnode::FailSafe>(static_cast<uint32_t>(failsafe) - static_cast<uint32_t>(artnet::FailSafe::kLast));
}
} // namespace artnetnode

#if (ARTNET_VERSION >= 4)
class ArtNetNode : E131Bridge
{
#else
class ArtNetNode
{
#endif
   public:
    ArtNetNode();

    void SetOutput(DmxNodeOutputType* dmx_node_output_type);
    DmxNodeOutputType* GetOutput() const;

    void SetLongName(const char*);
    const char* GetLongName() const;
    void GetLongNameDefault(char*);

    void SetShortName(uint32_t port_index, const char*);
    const char* GetShortName(uint32_t port_index) const;

    void SetDisableMergeTimeout(bool disable);
    bool GetDisableMergeTimeout() const;

    void SetFailSafe(dmxnode::FailSafe failsafe);
    dmxnode::FailSafe GetFailSafe();

    void SetUniverse(uint32_t port_index, uint16_t universe);
    uint16_t GetUniverse(uint32_t port_index) const;

    void SetDirection(uint32_t port_index, dmxnode::PortDirection port_direction);
    dmxnode::PortDirection GetDirection(uint32_t port_index) const;

    void SetUniverse(uint32_t port_index, dmxnode::PortDirection port_direction, uint16_t universe);
    bool GetUniverse(uint32_t port_index, uint16_t& universe, dmxnode::PortDirection port_direction);

    void SetMergeMode(uint32_t port_index, dmxnode::MergeMode merge_mode);
    dmxnode::MergeMode GetMergeMode(uint32_t port_index) const;

#if defined(OUTPUT_HAVE_STYLESWITCH)
    void SetOutputStyle(uint32_t port_index, dmxnode::OutputStyle output_style);
    dmxnode::OutputStyle GetOutputStyle(uint32_t port_index) const;
#endif

    void GoodOutputBSet(uint32_t port_index, uint8_t b);
    void GoodOutputBClear(uint32_t port_index, uint8_t b);

    void SetRdm(bool do_enable);
    bool GetRdm() const { return state_.is_rdm_enabled; }

    void SetRdm(uint32_t port_index, bool enable);
    bool GetRdm(uint32_t port_index) const;

    void SendTod(uint32_t port_index);

    void Print();

    void Start();
    void Stop();
    void Run();

#if defined(ARTNET_SHOWFILE)
    void HandleShowFile(const artnet::ArtDmx* artdmx)
    {
        current_millis_ = hal::Millis();
        ip_address_from_ = network::GetPrimaryIp();
        receive_buffer_ = reinterpret_cast<uint8_t*>(const_cast<artnet::ArtDmx*>(artdmx));
        HandleDmx();
    }
#endif

    void SetRecordShowfile(bool do_record) { state_.do_record = do_record; }
    bool GetRecordShowfile() const { return state_.do_record; }

    uint8_t GetVersion() const { return artnet::kVersion; }

    uint32_t GetActiveInputPorts() const { return state_.enabled_input_ports; }
    uint32_t GetActiveOutputPorts() const { return state_.enabled_output_ports; }

    dmxnode::PortDirection GetPortDirection(uint32_t port_index) const;

    bool GetPortAddress(uint32_t port_index, uint16_t& address) const;
    bool GetPortAddress(uint32_t port_index, uint16_t& address, dmxnode::PortDirection port_direction) const;

    bool GetOutputPort(uint16_t universe, uint32_t& port_index);

    void RestartOutputPort(uint32_t port_index);

#if defined(RDM_CONTROLLER)
    void SetRdmController(ArtNetRdmController* art_net_rdm_controller, bool do_enable = true);
    uint32_t RdmCopyWorkingQueue(char* out_buffer, uint32_t out_buffer_size);
    uint32_t RdmGetUidCount(uint32_t port_index);
    uint32_t RdmCopyTod(uint32_t port_index, char* out_buffer, uint32_t out_buffer_size);
    bool RdmIsRunning(uint32_t port_index);
    bool RdmIsRunning(uint32_t port_index, bool& is_incremental);
    bool GetRdmDiscovery(uint32_t port_index);
#endif

#if defined(RDM_RESPONDER)
    void SetRdmResponder(ArtNetRdmResponder* pArtNetRdmResponder, const bool doEnable = true);
#endif

#if defined(ARTNET_HAVE_TIMECODE)
    void SendTimeCode(const struct artnet::TimeCode* timecode)
    {
        assert(timecode != nullptr);
        memcpy(&art_time_code_.Frames, timecode, sizeof(struct artnet::TimeCode));
        network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(&art_time_code_), sizeof(struct artnet::ArtTimeCode), node_.ip_timecode, artnet::kUdpPort);
    }

    void SetArtTimeCodeCallbackFunction(ArtTimeCodeCallbackFunctionPtr art_time_code_callback_function_ptr)
    {
        art_time_code_callback_function_ptr_ = art_time_code_callback_function_ptr;
    }

    void SetTimeCodeIp(uint32_t destination_ip) { node_.ip_timecode = destination_ip; }
#endif

#if defined(ARTNET_HAVE_TRIGGER)
    void SetArtTriggerCallbackFunctionPtr(ArtTriggerCallbackFunctionPtr art_trigger_callback_function_ptr)
    {
        art_trigger_callback_function_ptr_ = art_trigger_callback_function_ptr;
    }
#endif

    void SetDestinationIp(uint32_t port_index, uint32_t destination_ip)
    {
        if (port_index < dmxnode::kMaxPorts)
        {
            input_port_[port_index].destination_ip = destination_ip;
            DEBUG_PRINTF("destination_ip=" IPSTR, IP2STR(input_port_[port_index].destination_ip));
        }
    }

    uint32_t GetDestinationIp(uint32_t port_index) const
    {
        if (port_index < dmxnode::kMaxPorts)
        {
            return input_port_[port_index].destination_ip;
        }

        return 0;
    }

    /**
     * LLRP
     */
    void SetRdmUID(const uint8_t* uid, bool supports_llrp = false)
    {
        memcpy(art_poll_reply_.DefaultUidResponder, uid, sizeof(art_poll_reply_.DefaultUidResponder));

        if (supports_llrp)
        {
            art_poll_reply_.Status3 |= artnet::Status3::kSupportsLlrp;
        }
        else
        {
            art_poll_reply_.Status3 &= static_cast<uint8_t>(~artnet::Status3::kSupportsLlrp);
        }
    }

    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->InputUdp(buffer, size, from_ip, from_port);
    }

    static ArtNetNode* Get() { return s_this; }

#if (ARTNET_VERSION >= 4)
   private:
    void SetUniverse4(uint32_t port_index);
    void SetDirection4(uint32_t port_index);
    void SetLedBlinkMode4(hal::statusled::Mode mode);
    void HandleAddress4(uint8_t command, uint32_t port_index);
    uint8_t GetGoodOutput4(uint32_t port_index);

   public:
    void SetPortProtocol4(uint32_t port_index, artnet::PortProtocol port_protocol);
    artnet::PortProtocol GetPortProtocol4(uint32_t port_index) const;

    void SetMapUniverse0(bool map_universe0 = false) { node_.map_universe0 = map_universe0; }
    bool IsMapUniverse0() const { return node_.map_universe0; }

    void SetPriority4(uint32_t priority);
    void SetPriority4(uint32_t port_index, uint8_t priority);
    uint8_t GetPriority4(uint32_t port_index) const;

    uint32_t GetActiveOutputPorts4() const { return E131Bridge::GetActiveOutputPorts(); }
    uint32_t GetActiveInputPorts4() const { return E131Bridge::GetActiveInputPorts(); }
#endif

   private:
    void SetFailSafe(artnet::FailSafe failsafe);
    void SetSwitch(uint32_t port_index, uint8_t sw);

    void SendDiag(const artnet::PriorityCodes kPriorityCode, const char* format, ...);

    void HandlePoll();
    void HandleDmx();
    void HandleSync();
    void HandleAddress();
    void HandleTimeCode();
    void HandleTimeSync();
    void HandleTodControl();
    void HandleTodData();
    void HandleTodRequest();
    void HandleRdm();
    void HandleRdmSub();
    void HandleIpProg();
    void HandleDmxIn();
    void HandleInput();
    void SetLocalMerging();
    void HandleRdmIn();
    void HandleTrigger();

    void SetPortAddress(uint32_t port_index);

    void UpdateMergeStatus(uint32_t port_index);
    void CheckMergeTimeouts(uint32_t port_index);

    void ProcessPollReply(uint32_t port_index, uint32_t& num_ports_input, uint32_t& num_ports_output);
    void SendPollReply(uint32_t port_index, uint32_t destination_ip, artnet::ArtPollQueue* poll_queue = nullptr);

    void SendTodRequest(uint32_t port_index);

    void SetNetworkDataLossCondition();

    void FailSafeRecord();
    void FailSafePlayback();

#if defined(RDM_CONTROLLER)
    bool RdmDiscoveryRun();
#endif

    void InputUdp(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port);

   private:
    int32_t handle_{-1};
    uint32_t ip_address_from_;
    uint32_t current_millis_{0};
    uint32_t packet_millis_{0};
    uint8_t* receive_buffer_{nullptr};

    DmxNodeOutputType* dmxnode_output_type_{nullptr};

    ArtTimeCodeCallbackFunctionPtr art_time_code_callback_function_ptr_{nullptr};
    ArtTriggerCallbackFunctionPtr art_trigger_callback_function_ptr_{nullptr};

    artnetnode::Node node_;
    artnetnode::State state_;
    artnetnode::OutputPort output_port_[dmxnode::kMaxPorts];
    artnetnode::InputPort input_port_[dmxnode::kMaxPorts];

    artnet::ArtPollReply art_poll_reply_;
#if defined(ARTNET_HAVE_DMXIN)
    artnet::ArtDmx art_dmx_;
#endif
#if defined(RDM_CONTROLLER) || defined(RDM_RESPONDER)
    union UArtTodPacket
    {
        artnet::ArtTodData art_tod_data;
        artnet::ArtTodRequest art_tod_request;
        artnet::ArtRdm art_rdm;
    };
    UArtTodPacket art_tod_packet_;
#if defined(RDM_CONTROLLER)
    ArtNetRdmController rdm_controller_;
#endif
#if defined(RDM_RESPONDER)
    ArtNetRdmResponder* rdm_responder_;
#endif
#endif
#if defined(ARTNET_HAVE_TIMECODE)
    artnet::ArtTimeCode art_time_code_;
#endif
#if defined(ARTNET_ENABLE_SENDDIAG)
    artnet::ArtDiagData diag_data_;
#endif

    static inline ArtNetNode* s_this;
};

#include "artnetnode_inline_impl.h" // IWYU pragma: keep
#if (ARTNET_VERSION >= 4)
#include "artnetnode4_inline_impl.h" // IWYU pragma: keep
#endif
#if defined(RDM_CONTROLLER)
#include "artnetnode_rdm_controller_inline_impl.h" // IWYU pragma: keep
#endif

#endif // ARTNETNODE_H_
