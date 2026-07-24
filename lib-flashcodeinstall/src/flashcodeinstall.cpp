/**
 * @file flashcodeinstall.cpp
 */
/* Copyright (C) 2018-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <cassert>

#include "flashcodeinstall.h"
#include "firmware.h"
#include "display.h" // IWYU pragma: keep
#include "watchdog.h"

bool FlashCodeInstall::WriteFirmware(const uint8_t* buffer, uint32_t size) {
    FLASHCODE_INSTALL_DEBUG_ENTRY();

    assert(buffer != nullptr);
    assert(size != 0);

	
    FLASHCODE_INSTALL_DEBUG_PRINTF("(%p + %x)=%p, flash_size_=%u", reinterpret_cast<void*>(OFFSET_UIMAGE), static_cast<unsigned>(size), reinterpret_cast<void*>(OFFSET_UIMAGE + size), static_cast<unsigned>(flash_size_));

    if ((OFFSET_UIMAGE + size) > flash_size_) {
        printf("Error: (OFFSET_UIMAGE + size) %u > flash_size_ %u\n", static_cast<unsigned>(OFFSET_UIMAGE + size), static_cast<unsigned>(flash_size_));
        FLASHCODE_INSTALL_DEBUG_EXIT();
        return false;
    }

    const auto kWatchdog = watchdog::Watchdog();

    if (kWatchdog) {
        watchdog::Stop();
    }

    puts("Write firmware");

    const auto kSectorSize = FlashCode::GetSectorSize();
    const auto kEraseSize = (size + kSectorSize - 1) & ~(kSectorSize - 1);

    FLASHCODE_INSTALL_DEBUG_PRINTF("size=%x, kSectorSize=%x, kEraseSize=%x", static_cast<unsigned>(size), static_cast<unsigned>(kSectorSize), static_cast<unsigned>(kEraseSize));

    Display::Get()->TextStatus("Erase", ansi::Colours::Colour::kGreen);

    flashcode::Result result;

    while (!FlashCode::Erase(OFFSET_UIMAGE, kEraseSize, result)) {
    }

    if (flashcode::Result::kError == result) {
        puts("Error: flash erase");
        return false;
    }

    Display::Get()->TextStatus("Writing", ansi::Colours::Colour::kGreen);

    while (!FlashCode::Write(OFFSET_UIMAGE, size, buffer, result)) {
    }

    if (flashcode::Result::kError == result) {
        puts("Error: flash write");
        return false;
    }

    if (kWatchdog) {
        watchdog::Init();
    }

    Display::Get()->TextStatus("Done", ansi::Colours::Colour::kGreen);

    FLASHCODE_INSTALL_DEBUG_EXIT();
    return true;
}

bool FlashCodeInstall::Erase(uint32_t firmware_size) {
    FLASHCODE_INSTALL_DEBUG_ENTRY();
    FLASHCODE_INSTALL_DEBUG_PRINTF("firmware_size=%u", static_cast<unsigned>(firmware_size));

    //    if (chunk_state_ != ChunkState::kStart) {
    //        FLASHCODE_INSTALL_DEBUG_EXIT();
    //        return false;
    //    }

    firmware_size_ = firmware_size;
    write_count_ = 0;

    const auto kSectorSize = FlashCode::GetSectorSize();
    erase_size_ = ((firmware_size_ + kSectorSize - 1) / kSectorSize) * kSectorSize;

    FLASHCODE_INSTALL_DEBUG_PRINTF("firmware_size_=%u, kSectorSize=%u, erase_size_=%u", static_cast<unsigned>(firmware_size_), static_cast<unsigned>(kSectorSize), static_cast<unsigned>(erase_size_));

    flashcode::Result result;
    while (!FlashCode::Erase(OFFSET_UIMAGE, erase_size_, result)) {
        watchdog::Feed();
        Display::Get()->Progress();
    }

    putchar('\n');

    if (flashcode::Result::kOk == result) {
        chunk_state_ = ChunkState::kWrite;
        FLASHCODE_INSTALL_DEBUG_EXIT();
        return true;
    }

    FLASHCODE_INSTALL_DEBUG_EXIT();
    return false;
}

bool FlashCodeInstall::WriteChunk(const uint8_t* chunck, uint32_t chunk_size, uint32_t& written) {
    flashcode::Result result;
    while (!FlashCode::Write(OFFSET_UIMAGE + write_count_, chunk_size, chunck, result)) {
        watchdog::Feed();
    }

    write_count_ += chunk_size;
    written = write_count_;

    if (write_count_ > erase_size_) {
        return false;
    }

    return (flashcode::Result::kOk == result);
}

bool FlashCodeInstall::WriteChunkComplete(uint32_t& write_count) {
    FLASHCODE_INSTALL_DEBUG_ENTRY();

    write_count = write_count_;
    const auto kWriteCount = write_count_;
    write_count_ = 0;
    const auto kState = chunk_state_;
    chunk_state_ = ChunkState::kStart;

    if (kState != ChunkState::kWrite) {
        FLASHCODE_INSTALL_DEBUG_EXIT();
        return false;
    }

    if (firmware_size_ != kWriteCount) {
        FLASHCODE_INSTALL_DEBUG_EXIT();
        return false;
    }

    FLASHCODE_INSTALL_DEBUG_EXIT();
    return true;
}
