/**
 * @file oscserver.h
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef OSCSERVER_H_
#define OSCSERVER_H_

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "apps/mdns.h"
#include "hal_statusled.h"
#include "network.h"
#include "dmxnode.h"
#include "dmxnode_outputtype.h"
#include "configurationstore.h"
 #include "firmware/debug/debug_debug.h"

namespace osc::server
{
struct DefaultPort
{
    static constexpr uint16_t kIncoming = 8000;
    static constexpr uint16_t kOutgoing = 9000;
};
} // namespace osc::server

class OscServerHandler
{
   public:
    virtual ~OscServerHandler() {}
    virtual void Blackout() = 0;
    virtual void Update() = 0;
    virtual void Info(int32_t handle, uint32_t remote_ip, uint16_t port_outgoing) = 0;
};

class OscServer
{
   public:
    OscServer();

    OscServer(const OscServer&) = delete;
    OscServer& operator=(const OscServer&) = delete;

    ~OscServer() = default;

    void Start()
    {
        DEBUG_ENTRY();

        assert(handle_ == -1);
        handle_ = network::udp::Begin(port_incoming_, StaticCallbackFunction);
        assert(handle_ != -1);

        network::apps::mdns::ServiceRecordAdd(nullptr, network::apps::mdns::Services::kOsc, "type=server", port_incoming_);

        hal::statusled::SetMode(hal::statusled::Mode::NORMAL);

        DEBUG_EXIT();
    }

    void Stop()
    {
        DEBUG_ENTRY();

        if (dmxnode_output_type_ != nullptr)
        {
            dmxnode_output_type_->Stop(0);
        }

        network::apps::mdns::ServiceRecordDelete(network::apps::mdns::Services::kOsc);

        assert(handle_ != -1);
        network::udp::End(port_incoming_);
        handle_ = -1;

        DEBUG_EXIT();
    }

    void Print()
    {
        puts("OSC Server");
        printf(" Incoming Port        : %d\n", port_incoming_);
        printf(" Outgoing Port        : %d\n", port_outgoing_);
        printf(" DMX Path             : [%s][%s]\n", s_path, s_path_second);
        printf("  Blackout Path       : [%s]\n", s_path_blackout);
        printf(" Partial Transmission : %s\n", partial_transmission_ ? "Yes" : "No");
    }

    void Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port);

    void SetOutput(DmxNodeOutputType* dmx_node_output_type)
    {
        assert(dmx_node_output_type != nullptr);
        dmxnode_output_type_ = dmx_node_output_type;
    }

    void SetOscServerHandler(OscServerHandler* osc_server_handler)
    {
        assert(osc_server_handler != nullptr);
        handler_ = osc_server_handler;
    }

    void SetPortIncoming(uint16_t port_incoming)
    {
        if (port_incoming > 1023)
        {
            port_incoming_ = port_incoming;
        }
        else
        {
            port_incoming_ = osc::server::DefaultPort::kIncoming;
        }
    }

    uint16_t GetPortIncoming() const { return port_incoming_; }

    void SetPortOutgoing(uint16_t port_outgoing)
    {
        if (port_outgoing > 1023)
        {
            port_outgoing_ = port_outgoing;
        }
        else
        {
            port_outgoing_ = osc::server::DefaultPort::kOutgoing;
        }
    }

    uint16_t GetPortOutgoing() const { return port_outgoing_; }

    void SetPath(const char* path);
    const char* GetPath() { return s_path; }

    void SetPathInfo(const char* path_info);
    const char* GetPathInfo() { return s_path_info; }

    void SetPathBlackOut(const char* path_black_out);
    const char* GetPathBlackOut() { return s_path_blackout; }

    void SetPartialTransmission(bool partial_transmission = false) { partial_transmission_ = partial_transmission; }

    bool IsPartialTransmission() const { return partial_transmission_; }

    void SetEnableNoChangeUpdate(bool enable_no_change_update) { enable_no_change_update_ = enable_no_change_update; }
    bool GetEnableNoChangeUpdate() { return enable_no_change_update_; }

    static OscServer& Instance()
    {
        assert(s_this != nullptr); // Ensure that s_this is valid
        return *s_this;
    }

   private:
    int GetChannel(const char* p);
    bool IsDmxDataChanged(const uint8_t* data, uint16_t start_channel, uint32_t length);

    /**
     * @brief Static callback function for receiving UDP packets.
     *
     * @param pBuffer Pointer to the packet buffer.
     * @param nSize Size of the packet buffer.
     * @param from_ip IP address of the sender.
     * @param from_port Port number of the sender.
     */
    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(buffer, size, from_ip, from_port);
    }

   private:
    uint16_t port_incoming_{osc::server::DefaultPort::kIncoming};
    uint16_t port_outgoing_{osc::server::DefaultPort::kOutgoing};
    int32_t handle_{-1};
    uint32_t last_channel_{0};

    bool partial_transmission_{false};
    bool enable_no_change_update_{false};
    bool is_running_{false};
    char os_[32];

    OscServerHandler* handler_{nullptr};
    DmxNodeOutputType* dmxnode_output_type_{nullptr};

    const char* model_;
    const char* soc_;

    static inline char s_path[common::store::osc::server::kPathLength];
    static inline char s_path_second[common::store::osc::server::kPathLength];
    static inline char s_path_info[common::store::osc::server::kPathLength];
    static inline char s_path_blackout[common::store::osc::server::kPathLength];

    static inline uint8_t s_data[dmxnode::kUniverseSize];
    static inline uint8_t s_osc[dmxnode::kUniverseSize];

    static inline OscServer* s_this;
};

#endif  // OSCSERVER_H_
