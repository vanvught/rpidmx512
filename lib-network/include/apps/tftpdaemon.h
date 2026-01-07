/**
 * @file tftpdaemon.h
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

#ifndef APPS_TFTPDAEMON_H_
#define APPS_TFTPDAEMON_H_

#include <cstdint>
#include <cstddef>

namespace tftp
{
enum class Mode
{
    kBinary,
    kAscii
};
} // namespace tftp

class TFTPDaemon
{
   public:
    TFTPDaemon();
    virtual ~TFTPDaemon();

    void Input(const uint8_t*, uint32_t, uint32_t, uint16_t);

    virtual bool FileOpen(const char* file_name, tftp::Mode mode) = 0;
    virtual bool FileCreate(const char* file_name, tftp::Mode mode) = 0;
    virtual bool FileClose() = 0;
    virtual size_t FileRead(void* buffer, size_t count, unsigned block_number) = 0;
    virtual size_t FileWrite(const void* buffer, size_t count, unsigned block_number) = 0;

    virtual void Exit() = 0;

   private:
    void Init();
    void HandleRequest();
    void HandleRecvAck();
    void HandleRecvData();
    void SendError(uint16_t error_code, const char* error_message);
    void DoRead();
    void DoWriteAck();

   private:
    enum class State
    {
        kInit,
        kWaitingRq,
        kRrqSendPacket,
        kRrqRecvAck,
        kWrqSendAck,
        kWrqRecvPacket
    };
    State state_{State::kInit};
    int32_t index_{-1};
    uint8_t* buffer_{nullptr};
    uint32_t from_ip_{0};
    uint32_t length_{0};
    uint32_t data_length_{0};
    uint32_t packet_length_{0};
    uint16_t from_port_{0};
    uint16_t block_number_{0};
    bool is_last_block_{false};

    static TFTPDaemon* Get() { return s_this; }

   private:
    void static StaticCallbackFunction(const uint8_t* buffer, uint32_t size, uint32_t from_ip, uint16_t from_port)
    {
        s_this->Input(buffer, size, from_ip, from_port);
    }
    static inline TFTPDaemon* s_this;
};

#endif  // APPS_TFTPDAEMON_H_
