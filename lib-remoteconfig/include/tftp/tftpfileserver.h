/**
 * @file tftpfileserver.h
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

#ifndef TFTP_TFTPFILESERVER_H_
#define TFTP_TFTPFILESERVER_H_

#include <cstddef>
#include <cstdint>

#include "apps/tftpdaemon.h"

#if defined(GD32)
#include "gd32.h"
#endif

namespace tftpfileserver
{
bool is_valid(const void* buffer);
} // namespace tftpfileserver

class TFTPFileServer final : public TFTPDaemon
{
   public:
    TFTPFileServer(uint8_t* buffer, uint32_t size);
    ~TFTPFileServer() override {}

    bool FileOpen(const char* file_name, tftp::Mode mode) override;
    bool FileCreate(const char* file_name, tftp::Mode mode) override;
    bool FileClose() override;
    size_t FileRead(void* buffer, size_t count, unsigned block_number) override;
    size_t FileWrite(const void* buffer, size_t count, unsigned block_number) override;
    void Exit() override;

    uint32_t GetFileSize() const { return m_nFileSize; }

    bool IsDone() const { return m_bDone; }

   private:
    uint8_t* buffer_;
    uint32_t m_nSize;
    uint32_t m_nFileSize{0};
    bool m_bDone{false};
};

#endif  // TFTP_TFTPFILESERVER_H_
