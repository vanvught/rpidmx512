/**
 * @file tlc59711.cpp
 *
 */
/* Copyright (C) 2018-2026 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if !defined(NDEBUG) || defined(__linux__)
#include <cstdio>
#endif
#include <cstring>
#include <cassert>
#include <cstdint>

#include "tlc59711.h"
#include "spi.h"

#define TLC59711_RGB_8BIT_VALUE(x) ((uint8_t)(x))
#define TLC59711_RGB_16BIT_VALUE(x) ((uint16_t)(x))

#define TLC59711_COMMAND 0x25U
#define TLC59711_COMMAND_SHIFT 26U

#define TLC59711_OUTTMG_DEFAULT 0U
#define TLC59711_OUTTMG_SHIFT 25U

#define TLC59711_EXTGCK_DEFAULT 0U
#define TLC59711_EXTGCK_SHIFT 24U

#define TLC59711_TMGRST_DEFAULT 1U
#define TLC59711_TMGRST_SHIFT 23U

#define TLC59711_DSPRPT_DEFAULT 1U
#define TLC59711_DSPRPT_SHIFT 22U

#define TLC59711_BLANK_DEFAULT 0U
#define TLC59711_BLANK_SHIFT 21U

#define TLC59711_GS_DEFAULT 0x7FU
#define TLC59711_GS_MASK 0x7FU
#define TLC59711_GS_RED_SHIFT 0U
#define TLC59711_GS_GREEN_SHIFT 7U
#define TLC59711_GS_BLUE_SHIFT 14U

TLC59711::TLC59711(uint8_t boards, uint32_t spi_speed_hz)
    : boards_(boards),
      spi_speed_hz_(spi_speed_hz == 0 ? TLC59711SpiSpeed::kDefault : spi_speed_hz)

{
    spi::Begin();

    if (spi_speed_hz_ > TLC59711SpiSpeed::kMax) {
        spi_speed_hz_ = TLC59711SpiSpeed::kMax;
    }

    if (boards == 0) {
        boards = 1;
    }

    buffer_size_ = boards * TLC59711Channels::kU16Bit;

    buffer_ = new uint16_t[buffer_size_];
    assert(buffer_ != nullptr);

    buffer_blackout_ = new uint16_t[buffer_size_];
    assert(buffer_blackout_ != nullptr);

    for (uint32_t i = 0; i < buffer_size_; i++) {
        buffer_[i] = 0;
    }

    first32_ |= (TLC59711_COMMAND << TLC59711_COMMAND_SHIFT);

    SetOnOffTiming(TLC59711_OUTTMG_DEFAULT);
    SetExternalClock(TLC59711_EXTGCK_DEFAULT);
    SetDisplayTimingReset(TLC59711_TMGRST_DEFAULT);
    SetDisplayRepeat(TLC59711_DSPRPT_DEFAULT);
    SetBlank(TLC59711_BLANK_DEFAULT);
    SetGbcRed(TLC59711_GS_DEFAULT);
    SetGbcGreen(TLC59711_GS_DEFAULT);
    SetGbcBlue(TLC59711_GS_DEFAULT);

    memcpy(buffer_blackout_, buffer_, buffer_size_ * 2);
}

TLC59711::~TLC59711() {
    delete[] buffer_;
    buffer_ = nullptr;
}

bool TLC59711::Get(uint32_t channel, uint16_t& value) {
    const uint32_t kBoardIndex = channel / TLC59711Channels::kOut;

    if (kBoardIndex < boards_) {
        const uint32_t kIndex = 2 + (kBoardIndex * TLC59711Channels::kU16Bit) + ((12 * kBoardIndex) + 11 - channel);
        value = __builtin_bswap16(buffer_[kIndex]);
        return true;
    }

    return false;
}

void TLC59711::Set(uint32_t channel, uint16_t value) {
    const uint32_t kBoardIndex = channel / TLC59711Channels::kOut;

    if (kBoardIndex < boards_) {
        const uint32_t kIndex = 2 + (kBoardIndex * TLC59711Channels::kU16Bit) + ((12 * kBoardIndex) + 11 - channel);
        buffer_[kIndex] = __builtin_bswap16(value);
    }
#ifndef NDEBUG
    else {
        printf("\t\tm_nBoards=%d, nBoardIndex=%d, channel=%d\n", static_cast<int>(boards_), static_cast<int>(nBoardIndex), static_cast<int>(channel));
    }
#endif
}

bool TLC59711::GetRgb(uint8_t out, uint16_t& red, uint16_t& green, uint16_t& blue) {
    const uint32_t kBoardIndex = out / 4;

    if (kBoardIndex < boards_) {
        uint32_t index = 2 + (kBoardIndex * TLC59711Channels::kU16Bit) + (((4 * kBoardIndex) + 3 - out) * 3);
        blue = __builtin_bswap16(buffer_[index++]);
        green = __builtin_bswap16(buffer_[index++]);
        red = __builtin_bswap16(buffer_[index]);
        return true;
    }

    return false;
}

void TLC59711::Set(uint32_t channel, uint8_t value) {
    const auto kBoardIndex = channel / TLC59711Channels::kOut;

    if (kBoardIndex < boards_) {
        const uint32_t kIndex = 2 + (kBoardIndex * TLC59711Channels::kU16Bit) + ((12 * kBoardIndex) + 11 - channel);
        buffer_[kIndex] = static_cast<uint16_t>((value << 8) | value);
    }
#ifndef NDEBUG
    else {
        printf("\t\tm_nBoards=%d, nBoardIndex=%d, channel=%d\n", static_cast<int>(boards_), static_cast<int>(nBoardIndex), static_cast<int>(channel));
    }
#endif
}

void TLC59711::SetRgb(uint8_t out, uint16_t red, uint16_t green, uint16_t blue) {
    const uint32_t kBoardIndex = out / 4;

    if (kBoardIndex < boards_) {
        uint32_t index = 2 + (kBoardIndex * TLC59711Channels::kU16Bit) + (((4 * kBoardIndex) + 3 - out) * 3);
        buffer_[index++] = __builtin_bswap16(blue);
        buffer_[index++] = __builtin_bswap16(green);
        buffer_[index] = __builtin_bswap16(red);
    }
#ifndef NDEBUG
    else {
        printf("boards_=%d, nBoardIndex=%d, nOut=%d\n", static_cast<int>(boards_), static_cast<int>(nBoardIndex), static_cast<int>(nOut));
    }
#endif
}

void TLC59711::SetRgb(uint8_t out, uint8_t red, uint8_t green, uint8_t blue) {
    const uint32_t kBoardIndex = out / 4;

    if (kBoardIndex < boards_) {
        uint32_t index = 2 + (kBoardIndex * TLC59711Channels::kU16Bit) + (((4 * kBoardIndex) + 3 - out) * 3);
        buffer_[index++] = static_cast<uint16_t>((blue << 8) | blue);
        buffer_[index++] = static_cast<uint16_t>((green << 8) | green);
        buffer_[index] = static_cast<uint16_t>((red << 8) | red);
    }
#ifndef NDEBUG
    else {
        printf("boards_=%d, nBoardIndex=%d, nOut=%d\n", static_cast<int>(boards_), static_cast<int>(nBoardIndex), static_cast<int>(nOut));
    }
#endif
}

int TLC59711::GetBlank() const {
    return (first32_ & (1U << TLC59711_BLANK_SHIFT)) == (1U << TLC59711_BLANK_SHIFT);
}

void TLC59711::SetBlank(bool blank) {
    first32_ &= ~(1U << TLC59711_BLANK_SHIFT);

    if (blank) {
        first32_ |= (1U << TLC59711_BLANK_SHIFT);
    }

    UpdateFirst32();
}

int TLC59711::GetDisplayRepeat() const {
    return (first32_ & (1U << TLC59711_DSPRPT_SHIFT)) == (1U << TLC59711_DSPRPT_SHIFT);
}

void TLC59711::SetDisplayRepeat(bool repeat) {
    first32_ &= ~(1U << TLC59711_DSPRPT_SHIFT);

    if (repeat) {
        first32_ |= (1U << TLC59711_DSPRPT_SHIFT);
    }

    UpdateFirst32();
}

int TLC59711::GetDisplayTimingReset() const {
    return (first32_ & (1U << TLC59711_TMGRST_SHIFT)) == (1U << TLC59711_TMGRST_SHIFT);
}

void TLC59711::SetDisplayTimingReset(bool reset) {
    first32_ &= ~(1U << TLC59711_TMGRST_SHIFT);

    if (reset) {
        first32_ |= (1U << TLC59711_TMGRST_SHIFT);
    }

    UpdateFirst32();
}

int TLC59711::GetExternalClock() const {
    return (first32_ & (1U << TLC59711_EXTGCK_SHIFT)) == (1U << TLC59711_EXTGCK_SHIFT);
}

void TLC59711::SetExternalClock(bool external_clock) {
    first32_ &= ~(1U << TLC59711_EXTGCK_SHIFT);

    if (external_clock) {
        first32_ |= (1U << TLC59711_EXTGCK_SHIFT);
    }

    UpdateFirst32();
}

int TLC59711::GetOnOffTiming() const {
    return (first32_ & (1U << TLC59711_OUTTMG_SHIFT)) == (1U << TLC59711_OUTTMG_SHIFT);
}

void TLC59711::SetOnOffTiming(bool on_off_timing) {
    first32_ &= ~(1U << TLC59711_OUTTMG_SHIFT);

    if (on_off_timing) {
        first32_ |= (1U << TLC59711_OUTTMG_SHIFT);
    }

    UpdateFirst32();
}

uint8_t TLC59711::GetGbcRed() const {
    return (first32_ >> TLC59711_GS_RED_SHIFT) & TLC59711_GS_MASK;
}

void TLC59711::SetGbcRed(uint8_t value) {
    first32_ &= ~(TLC59711_GS_MASK << TLC59711_GS_RED_SHIFT);
    first32_ |= ((value & TLC59711_GS_MASK) << TLC59711_GS_RED_SHIFT);

    UpdateFirst32();
}

uint8_t TLC59711::GetGbcGreen() const {
    return (first32_ >> TLC59711_GS_GREEN_SHIFT) & TLC59711_GS_MASK;
}

void TLC59711::SetGbcGreen(uint8_t value) {
    first32_ &= ~(TLC59711_GS_MASK << TLC59711_GS_GREEN_SHIFT);
    first32_ |= ((value & TLC59711_GS_MASK) << TLC59711_GS_GREEN_SHIFT);

    UpdateFirst32();
}

uint8_t TLC59711::GetGbcBlue() const {
    return (first32_ >> TLC59711_GS_BLUE_SHIFT) & TLC59711_GS_MASK;
}

void TLC59711::SetGbcBlue(uint8_t value) {
    first32_ &= ~(TLC59711_GS_MASK << TLC59711_GS_BLUE_SHIFT);
    first32_ |= ((value & TLC59711_GS_MASK) << TLC59711_GS_BLUE_SHIFT);

    UpdateFirst32();
}

void TLC59711::UpdateFirst32() {
    for (uint32_t i = 0; i < boards_; i++) {
        const auto kIndex = TLC59711Channels::kU16Bit * i;
        buffer_[kIndex] = __builtin_bswap16(static_cast<uint16_t>((first32_ >> 16)));
        buffer_[kIndex + 1] = __builtin_bswap16(static_cast<uint16_t>(first32_));
    }
}

void TLC59711::Dump() {
#ifndef NDEBUG
    printf("Command:0x%.2X\n", first32_ >> TLC59711_COMMAND_SHIFT);
    printf("\tOUTTMG:%d (default=%d)\n", GetOnOffTiming(), TLC59711_OUTTMG_DEFAULT);
    printf("\tEXTGCK:%d (default=%d)\n", GetExternalClock(), TLC59711_EXTGCK_DEFAULT);
    printf("\tTMGRST:%d (default=%d)\n", GetDisplayTimingReset(), TLC59711_TMGRST_DEFAULT);
    printf("\tDSPRPT:%d (default=%d)\n", GetDisplayRepeat(), TLC59711_DSPRPT_DEFAULT);
    printf("\tBLANK:%d  (default=%d)\n", GetBlank(), TLC59711_BLANK_DEFAULT);
    printf("\nGlobal Brightness\n");
    printf("\tRed:0x%.2X (default=0x%.2X)\n", GetGbcRed(), TLC59711_GS_DEFAULT);
    printf("\tGreen:0x%.2X (default=0x%.2X)\n", GetGbcGreen(), TLC59711_GS_DEFAULT);
    printf("\tBlue:0x%.2X (default=0x%.2X)\n", GetGbcBlue(), TLC59711_GS_DEFAULT);
    printf("\nBoards:%d\n", static_cast<int>(boards_));

    uint8_t nOut = 0;

    for (uint32_t i = 0; i < boards_; i++) {
        for (uint32_t j = 0; j < TLC59711Channels::RGB; j++) {
            uint16_t red = 0, green = 0, blue = 0;
            if (GetRgb(nOut, red, green, blue)) {
                printf("\tOut:%-2d, Red=0x%.4X, Green=0x%.4X, Blue=0x%.4X\n", nOut, red, green, blue);
            }
            nOut++;
        }
    }

    puts("");

    for (uint32_t i = 0; i < boards_ * TLC59711Channels::OUT; i++) {
        uint16_t nValue = 0;
        if (Get(i, nValue)) {
            printf("\tChannel:%-3d, Value=0x%.4X\n", static_cast<int>(i), nValue);
        }
    }

    puts("");
#endif
}

void TLC59711::Update() {
    assert(buffer_ != nullptr);

    spi::ChipSelect(spi::kCsNone);
    spi::SetSpeedHz(spi_speed_hz_);
    spi::SetDataMode(spi::kMode0);
    spi::Writenb(reinterpret_cast<char*>(buffer_), buffer_size_ * 2);
}

void TLC59711::Blackout() {
    assert(buffer_blackout_ != nullptr);

    spi::ChipSelect(spi::kCsNone);
    spi::SetSpeedHz(spi_speed_hz_);
    spi::SetDataMode(spi::kMode0);
    spi::Writenb(reinterpret_cast<char*>(buffer_blackout_), buffer_size_ * 2);
}
