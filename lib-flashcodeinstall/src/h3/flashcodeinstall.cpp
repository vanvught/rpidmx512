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

#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "flashcodeinstall.h"
#include "firmware.h"
#include "display.h"
#include "board.h"

static constexpr uint32_t kCompareBytes = 1024;
static constexpr uint32_t kFlashSizeMinimum = 0x200000;
static constexpr const char kFileUbootSpi[] = "uboot.spi";
static constexpr const char kFileuImage[] = "uImage";
static constexpr const char kWriting[] = "Writing";
static constexpr const char kCheckDifference[] = "Check difference";
static constexpr const char kNoDifference[] = "No difference";
static constexpr const char kDone[] = "Done";

FlashCodeInstall::FlashCodeInstall() {
    FLASHCODE_INSTALL_DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    Display::Get()->Cls();

    if (!FlashCode::IsDetected()) {
        Display::Get()->TextStatus("No SPI flash");
        FLASHCODE_INSTALL_DEBUG_PUTS("No SPI flash chip");
    } else {
        flash_size_ = FlashCode::GetSize();
        Display::Get()->Write(1, FlashCode::GetName());
    }

    if (board::GetBootDevice() == board::BootDevice::kMmc0) {
        FLASHCODE_INSTALL_DEBUG_PUTS("BOOT_DEVICE_MMC0");

        if (flash_size_ >= kFlashSizeMinimum) {
            have_flash_ = true;
            erase_size_ = FlashCode::GetSectorSize();

            file_buffer_ = new uint8_t[erase_size_];
            assert(file_buffer_ != nullptr);

            flash_buffer_ = new uint8_t[erase_size_];
            assert(flash_buffer_ != nullptr);

            Process(kFileUbootSpi, OFFSET_UBOOT_SPI);
            Process(kFileuImage, OFFSET_UIMAGE);
        }
    }

    FLASHCODE_INSTALL_DEBUG_EXIT();
}

FlashCodeInstall::~FlashCodeInstall() {
    FLASHCODE_INSTALL_DEBUG_ENTRY();

    delete[] file_buffer_;
    delete[] flash_buffer_;

    FLASHCODE_INSTALL_DEBUG_EXIT();
}

void FlashCodeInstall::Process(const char* file_name, uint32_t offset) {
    if (Open(file_name)) {
        Display::Get()->TextStatus(kCheckDifference);
        puts(kCheckDifference);

        if (Diff(offset)) {
            Display::Get()->TextStatus(kWriting);
            puts(kWriting);
            Write(offset);
        } else {
            Display::Get()->TextStatus(kNoDifference);
            puts(kNoDifference);
        }
        Close();
    }
}

bool FlashCodeInstall::Open(const char* file_name) {
    FLASHCODE_INSTALL_DEBUG_ENTRY();

    assert(file_name != nullptr);
    assert(file_ == nullptr);

    file_ = fopen(file_name, "r");

    if (file_ == nullptr) {
        printf("Could not open file: %s\n", file_name);
        FLASHCODE_INSTALL_DEBUG_EXIT();
        return false;
    }

    Display::Get()->ClearEndOfLine();
    Display::Get()->Write(2, file_name);
    puts(file_name);

    FLASHCODE_INSTALL_DEBUG_EXIT();
    return true;
}

void FlashCodeInstall::Close() {
    FLASHCODE_INSTALL_DEBUG_ENTRY();

    static_cast<void>(fclose(file_));
    file_ = nullptr;

    Display::Get()->TextStatus(kDone);
    puts(kDone);

    FLASHCODE_INSTALL_DEBUG_EXIT();
}

bool FlashCodeInstall::BuffersCompare(uint32_t size) {
    FLASHCODE_INSTALL_DEBUG_ENTRY();

    assert(size <= erase_size_);

    const auto* src32 = reinterpret_cast<uint32_t*>(file_buffer_);
    assert((reinterpret_cast<uint32_t>(src32) & 0x3) == 0);

    const auto* dst32 = reinterpret_cast<uint32_t*>(flash_buffer_);
    assert((reinterpret_cast<uint32_t>(dst32) & 0x3) == 0);

    while (size >= 4) {
        if (*src32++ != *dst32++) {
            FLASHCODE_INSTALL_DEBUG_EXIT();
            return false;
        }
        size -= 4;
    }

    const auto* src8 = reinterpret_cast<const uint8_t*>(src32);
    const auto* dst8 = reinterpret_cast<const uint8_t*>(dst32);

    while (size--) {
        if (*src8++ != *dst8++) {
            FLASHCODE_INSTALL_DEBUG_EXIT();
            return false;
        }
    }

    FLASHCODE_INSTALL_DEBUG_EXIT();
    return true;
}

bool FlashCodeInstall::Diff(uint32_t offset) {
    FLASHCODE_INSTALL_DEBUG_ENTRY();

    assert(file_ != nullptr);
    assert(offset < flash_size_);
    assert(file_buffer_ != nullptr);
    assert(flash_buffer_ != nullptr);

    if (fseek(file_, 0L, SEEK_SET) != 0) {
        FLASHCODE_INSTALL_DEBUG_EXIT();
        return false;
    }

    if (fread(file_buffer_, sizeof(uint8_t), kCompareBytes, file_) != kCompareBytes) {
        FLASHCODE_INSTALL_DEBUG_EXIT();
        return false;
    }

    flashcode::Result result;
    FlashCode::Read(offset, kCompareBytes, flash_buffer_, result);

    if (flashcode::Result::kError == result) {
        FLASHCODE_INSTALL_DEBUG_EXIT();
        return false;
    }

    if (!BuffersCompare(kCompareBytes)) {
        FLASHCODE_INSTALL_DEBUG_EXIT();
        return true;
    }

    FLASHCODE_INSTALL_DEBUG_EXIT();
    return false;
}

void FlashCodeInstall::Write(uint32_t offset) {
    FLASHCODE_INSTALL_DEBUG_ENTRY();

    assert(file_ != nullptr);
    assert(offset < flash_size_);
    assert(file_buffer_ != nullptr);

    auto success = false;

    uint32_t address = offset;
    size_t total_bytes = 0;

    static_cast<void>(fseek(file_, 0L, SEEK_SET));

    while (address < flash_size_) {
        const auto kBytes = fread(file_buffer_, sizeof(uint8_t), erase_size_, file_);
        total_bytes += kBytes;

        flashcode::Result result;
        FlashCode::Erase(address, erase_size_, result);

        if (flashcode::Result::kError == result) {
            puts("error: flash erase");
            break;
        }

        if (kBytes < erase_size_) {
            for (uint32_t i = kBytes; i < erase_size_; i++) {
                file_buffer_[i] = 0xFF;
            }
        }

        FlashCode::Write(address, erase_size_, file_buffer_, result);

        if (flashcode::Result::kError == result) {
            puts("error: flash write");
            break;
        }

        FlashCode::Read(address, kBytes, flash_buffer_, result);

        if (flashcode::Result::kError == result) {
            puts("error: flash read");
            break;
        }

        if (!BuffersCompare(kBytes)) {
            puts("error: flash verify");
            break;
        }

        if (kBytes != erase_size_) {
            if (ferror(file_) == 0) {
                success = true;
            }
            break; // Error or end of file
        }

        address += erase_size_;
    }

    if (success) {
        Display::Get()->ClearEndOfLine();
        Display::Get()->Printf(3, "%d", static_cast<int>(total_bytes));
        printf("%d bytes written\n", static_cast<int>(total_bytes));
    }

    FLASHCODE_INSTALL_DEBUG_EXIT();
}
