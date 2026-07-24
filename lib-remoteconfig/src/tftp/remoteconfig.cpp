#if defined(ENABLE_TFTP_SERVER)
/**
 * @file remoteconfig.cpp
 *
 */
/* Copyright (C) 2022-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cassert>

#include "remoteconfig.h"
#include "tftp/tftpfileserver.h"
#include "flashcodeinstall.h"
#include "firmware.h"
#include "display.h"
#include "firmware/debug/debug_debug.h"

static uint8_t s_tftp_buffer[FIRMWARE_MAX_SIZE];

void RemoteConfig::PlatformHandleTftpSet() {
    REMOTECONFIG_DEBUG_ENTRY();

    if (enable_tftp_ && (tftp_file_server_ == nullptr)) {
        tftp_file_server_ = new TFTPFileServer(s_tftp_buffer, FIRMWARE_MAX_SIZE);
        assert(m_pTFTPFileServer != nullptr);
        Display::Get()->TextStatus("TFTP On", ansi::Colours::Colour::kGreen);
    } else if (!enable_tftp_ && (tftp_file_server_ != nullptr)) {
        const uint32_t kFileSize = tftp_file_server_->GetFileSize();
        REMOTECONFIG_DEBUG_PRINTF("kFileSize=%u, %u", static_cast<unsigned>(kFileSize), static_cast<unsigned>(tftp_file_server_->IsDone()));

        auto succes = true;

        if (tftp_file_server_->IsDone()) {
            succes = FlashCodeInstall::Get()->WriteFirmware(s_tftp_buffer, kFileSize);

            if (!succes) {
                Display::Get()->TextStatus("Error: TFTP", ansi::Colours::Colour::kRed);
            }
        }

        delete tftp_file_server_;
        tftp_file_server_ = nullptr;

        if (succes) { // Keep error message
            Display::Get()->TextStatus("TFTP Off", ansi::Colours::Colour::kGreen);
        }
    }

    REMOTECONFIG_DEBUG_EXIT();
}

void RemoteConfig::PlatformHandleTftpGet() {
    REMOTECONFIG_DEBUG_ENTRY();

    REMOTECONFIG_DEBUG_EXIT();
}
#endif
