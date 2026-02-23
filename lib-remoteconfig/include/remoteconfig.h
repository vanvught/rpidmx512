/**
 * @file remoteconfig.h
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

#ifndef REMOTECONFIG_H_
#define REMOTECONFIG_H_

#include <cstdint>

#if defined(ENABLE_TFTP_SERVER)
#include "tftp/tftpfileserver.h"
#endif
#if defined(ENABLE_HTTPD)
#include "httpd/httpd.h"
#endif
#include "network.h"
#include "configstore.h"

namespace remoteconfig
{
namespace udp
{
static constexpr auto kBufferSize = 1420;
} // namespace udp

enum class Output
{
    DMX,
    RDM,
    MONITOR,
    PIXEL,
    TIMECODE,
    OSC,
    CONFIG,
    STEPPER,
    PLAYER,
    ARTNET,
    SERIAL,
    RGBPANEL,
    PWM,
    LAST
};

enum
{
    kDisplayNameLength = 24,
    kIdLength = (32 + common::store::remoteconfig::kDisplayNameLength + 2) // +2, comma and \n
};
} // namespace remoteconfig

class RemoteConfig
{
   public:
    explicit RemoteConfig(remoteconfig::Output output, uint32_t active_outputs = 0);
    ~RemoteConfig();

    const char* GetStringNode() const;
    const char* GetStringOutput() const;
    uint8_t GetOutputs() const { return s_list.active_outputs; }
    void SetDisplayName(const char* display_name);
    bool IsReboot() const { return is_reboot_; }
    void Reboot() { HandleReboot(); }
    void TftpExit();

    void Input(const uint8_t*, uint32_t, uint32_t, uint16_t);

    static RemoteConfig* Get() { return s_this; }

   private:
    void HandleRequest();
    void HandleReboot();
    void HandleFactory();
    void HandleList();
#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
    void HandleUptime();
#endif
    void HandleVersion();

    void HandleDisplaySet();
    void HandleDisplayGet();
    void HandleTftpSet();
    void HandleTftpGet();

    void PlatformHandleTftpSet();
    void PlatformHandleTftpGet();

   private:
    remoteconfig::Output output_;
    uint32_t active_outputs_;

    char* udp_buffer_{nullptr};
    int32_t handle_{-1};
    uint32_t ip_from_{0};
    uint32_t bytes_received_{0};

    struct Commands
    {
        void (RemoteConfig::*handler)();
        const char* cmd;
        const uint16_t kLength;
        const bool kGreaterThan;
    };

    static const Commands kGet[];
    static const Commands kSet[];

    struct List
    {
        uint8_t mac_address[network::iface::kMacSize];
        uint8_t output;
        uint8_t active_outputs;
    };

    bool is_reboot_{false};

#if defined(ENABLE_TFTP_SERVER)
    TFTPFileServer* tftp_file_server_{nullptr};
#endif
    bool enable_tftp_{false};

#if defined(ENABLE_HTTPD)
    HttpDaemon* http_daemon_{nullptr};
#endif

    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        RemoteConfig::Get()->Input(buffer, size, from_ip, from_port);
    }

    static inline List s_list;
    static inline RemoteConfig* s_this;
};

#endif  // REMOTECONFIG_H_
