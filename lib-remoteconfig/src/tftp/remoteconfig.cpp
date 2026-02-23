/**
 * @file remoteconfig.cpp
 *
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(ENABLE_TFTP_SERVER)
#if defined(DEBUG_TFTP)
#undef NDEBUG
#endif

#include <cstdint>
#include <cassert>

#include "remoteconfig.h"

#include "tftp/tftpfileserver.h"
#include "flashcodeinstall.h"
#include "firmware.h"

#include "display.h"

 #include "firmware/debug/debug_debug.h"

static uint8_t s_tftp_buffer[FIRMWARE_MAX_SIZE];

void RemoteConfig::PlatformHandleTftpSet()
{
    DEBUG_ENTRY();

    if (enable_tftp_ && (tftp_file_server_ == nullptr))
    {
        tftp_file_server_ = new TFTPFileServer(s_tftp_buffer, FIRMWARE_MAX_SIZE);
        assert(m_pTFTPFileServer != nullptr);
        Display::Get()->TextStatus("TFTP On", console::Colours::kConsoleGreen);
    }
    else if (!enable_tftp_ && (tftp_file_server_ != nullptr))
    {
        const uint32_t kFileSize = tftp_file_server_->GetFileSize();
        DEBUG_PRINTF("kFileSize=%d, %d", kFileSize, tftp_file_server_->IsDone());

        bool bSucces = true;

        if (tftp_file_server_->IsDone())
        {
            bSucces = FlashCodeInstall::Get()->WriteFirmware(s_tftp_buffer, kFileSize);

            if (!bSucces)
            {
                Display::Get()->TextStatus("Error: TFTP", console::Colours::kConsoleRed);
            }
        }

        delete tftp_file_server_;
        tftp_file_server_ = nullptr;

        if (bSucces)
        { // Keep error message
            Display::Get()->TextStatus("TFTP Off", console::Colours::kConsoleGreen);
        }
    }

    DEBUG_EXIT();
}

void RemoteConfig::PlatformHandleTftpGet()
{
    DEBUG_ENTRY();

    DEBUG_EXIT();
}
#endif
