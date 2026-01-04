/**
 * @file e131bridge.h
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

#ifndef E131BRIDGE_H_
#define E131BRIDGE_H_

#include <cstdint>

#if defined(ARTNET_VERSION) && (ARTNET_VERSION >= 4)
#define E131_HAVE_ARTNET
#endif

#include "dmxnode.h"
#include "e131.h"
#include "e131sync.h"
#include "dmxnode_outputtype.h"
#include "softwaretimers.h"

#ifndef ALIGNED
#define ALIGNED __attribute__((aligned(4)))
#endif

namespace e131bridge
{
enum class Status : uint8_t
{
    kOff,
    kStandby,
    kOn
};

enum StateFlags : uint8_t
{
    kNetworkDataLoss = (1 << 0),
    kMergeMode = (1 << 1),
    kSynchronized = (1 << 2),
    kForcedSynchronized = (1 << 3),
    kChanged = (1 << 4),
    kDisableMergeTimeout = (1 << 5),
    kDisableSynchronize = (1 << 6)
};

struct State
{
    uint8_t enabled_input_ports;
    uint8_t enabled_output_ports;
    uint8_t priority;
    uint8_t receiving_dmx;
    dmxnode::FailSafe failsafe;
    e131bridge::Status status;
    uint16_t discovery_packet_length;
    uint16_t synchronization_address_source_a;
    uint16_t synchronization_address_source_b;
    uint32_t synchronization_time;
    bool is_network_data_loss;
    bool is_merge_mode;
    bool is_synchronized;
    bool is_forced_synchronized;
    bool is_changed;
    bool disable_merge_timeout;
    bool disable_synchronize;
};

struct Bridge
{
    struct Port
    {
        uint16_t universe;
        dmxnode::PortDirection direction;
        bool local_merge;
    } port[dmxnode::kMaxPorts] ALIGNED;
};

struct Source
{
    uint32_t millis;
    uint32_t ip;
    uint8_t cid[e117::kCidLength];
    uint8_t sequence_number_data;
};

struct OutputPort
{
    Source source_a ALIGNED;
    Source source_b ALIGNED;
    dmxnode::MergeMode merge_mode;
    dmxnode::OutputStyle output_style;
    bool is_merging;
    bool is_transmitting;
    bool is_data_pending;
};

struct InputPort
{
    uint32_t multicast_ip;
    uint32_t millis;
    uint8_t sequence_number;
    uint8_t priority;
    bool is_disabled;
};
} // namespace e131bridge

class E131Bridge
{
   public:
    E131Bridge();
    E131Bridge(const E131Bridge&) = delete;
    E131Bridge(E131Bridge&&) = delete;
    E131Bridge& operator=(const E131Bridge&) = delete;
    E131Bridge& operator=(E131Bridge&&) = delete;
    ~E131Bridge() = default;

    void SetOutput(DmxNodeOutputType* dmx_node_output_type);
    DmxNodeOutputType* GetOutput() const;

    void SetLongName(const char*);
    const char* GetLongName();
    void GetLongNameDefault(char*);

    void SetShortName(uint32_t port_index, const char*);
    const char* GetShortName(uint32_t port_index) const;

    void SetDisableMergeTimeout(bool disable) { state_.disable_merge_timeout = disable; }
    bool GetDisableMergeTimeout() const;

    void SetFailSafe(dmxnode::FailSafe failsafe) { state_.failsafe = failsafe; }
    dmxnode::FailSafe GetFailSafe() const;

    void SetUniverse(uint32_t port_index, uint16_t universe);
    uint16_t GetUniverse(uint32_t port_index) const;

    void SetDirection(uint32_t port_index, dmxnode::PortDirection port_direction);
    dmxnode::PortDirection GetDirection(uint32_t port_index) const;

    void SetUniverse(uint32_t port_index, dmxnode::PortDirection port_direction, uint16_t universe);
    bool GetUniverse(uint32_t port_index, uint16_t& universe, dmxnode::PortDirection port_direction) const;

    void SetMergeMode(uint32_t port_index, dmxnode::MergeMode merge_mode);
    dmxnode::MergeMode GetMergeMode(uint32_t port_index) const;

    dmxnode::PortDirection GetPortDirection(uint32_t port_index) const;

#if defined(OUTPUT_HAVE_STYLESWITCH)
    void SetOutputStyle(uint32_t port_index, dmxnode::OutputStyle output_style);
    dmxnode::OutputStyle GetOutputStyle(uint32_t port_index) const;
#endif

    void SetPriority(uint32_t port_index, uint8_t priority);
    uint8_t GetPriority(uint32_t port_index) const;

    bool GetOutputPort(uint16_t universe, uint32_t& port_index);

    uint32_t GetActiveOutputPorts() const;
    uint32_t GetActiveInputPorts() const;

    bool IsTransmitting(uint32_t port_index) const;
    bool IsMerging(uint32_t port_index) const;
    bool IsStatusChanged();

    void SetEnableDataIndicator(bool enable);
    bool GetEnableDataIndicator() const;

    void SetDisableSynchronize(bool disable_synchronize);
    bool GetDisableSynchronize() const;

    void SetE131Sync(E131SyncCallbackFunctionPtr e131_sync) { sync_callback_function_pointer_ = e131_sync; }

    void SetInputDisabled(uint32_t port_index, bool disable);
    bool GetInputDisabled(uint32_t port_index) const;

    void Clear(uint32_t port_index);

#if defined(E131_HAVE_DMXIN) || defined(NODE_SHOWFILE)
    void SetSourceName(const char* source_name);
    const char* GetSourceName() const;
    const uint8_t* GetCid() const;
#endif

#if defined(NODE_SHOWFILE) && defined(CONFIG_SHOWFILE_PROTOCOL_NODE_E131)
    void HandleShowFile(const e131::DataPacket* pE131DataPacket);
#endif

    void Print();

    void Start();
    void Stop();
    void Run();

    void static StaticCallbackFunctionUdp(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->InputUdp(buffer, size, from_ip, from_port);
    }

    static E131Bridge* Get() { return s_this; }

   private:
    void InputUdp(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port);

    void SetNetworkDataLossCondition(bool source_a = true, bool source_b = true);

    void SetSynchronizationAddress(bool source_a, bool source_b, uint16_t synchronization_address);

    void CheckMergeTimeouts(uint32_t port_index);
    bool IsPriorityTimeOut(uint32_t port_index) const;
    bool IsIpCidMatch(const e131bridge::Source* const kSource) const;
    void UpdateMergeStatus(uint32_t port_index);

    void HandleDmx();
    void HandleSynchronization();

    enum class JoinLeave
    {
        kJoin,
        kLeave
    };

    void JoinUniverse(uint32_t port_index, uint16_t universe);
    void LeaveUniverse(uint32_t port_index, uint16_t universe);

    void HandleDmxIn();
    void SetLocalMerging();
    void FillDataPacket();
    void FillDiscoveryPacket();
    void SendDiscoveryPacket();

    void static StaticCallbackFunctionSendDiscoveryPacket([[maybe_unused]] TimerHandle_t timer_handle) { s_this->SendDiscoveryPacket(); }

   private:
    int32_t handle_{-1};
    uint8_t* receive_buffer_{nullptr};
    uint32_t packet_millis_{0};
    uint32_t current_millis_{0};
    uint32_t ip_address_from_{0};
    char node_name_[dmxnode::kNodeNameLength];

    e131bridge::State state_;
    e131bridge::Bridge bridge_;
    e131bridge::OutputPort output_port_[dmxnode::kMaxPorts];
    e131bridge::InputPort input_port_[dmxnode::kMaxPorts];

    bool enable_data_indicator_{true};

    DmxNodeOutputType* dmxnode_output_type_{nullptr};
    E131SyncCallbackFunctionPtr sync_callback_function_pointer_{nullptr};

#if defined(E131_HAVE_DMXIN) || defined(NODE_SHOWFILE)
    char source_name_[e131::kSourceNameLength];
    uint8_t cid_[e117::kCidLength];
#endif

#if defined(E131_HAVE_DMXIN)
    e131::DataPacket e131_data_packet_;
    e131::DiscoveryPacket e131_discovery_packet_;
    uint32_t discovery_ip_address_{0};
    TimerHandle_t timer_handle_send_discovery_packet_{-1};
#endif

    static inline E131Bridge* s_this;
};

#include "e131bridge_inline_impl.h" // IWYU pragma: keep

#endif  // E131BRIDGE_H_
