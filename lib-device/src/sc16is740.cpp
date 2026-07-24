/**
 * @file sc16is740.cpp
 *
 */
/* Copyright (C) 2020-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "sc16is740.h"
#include "sc16is7x0.h"
#include "timing.h"
#include "i2c.h"
#include "firmware/debug/debug_printbits.h"
#include "firmware/debug/debug_debug.h"

SC16IS740::SC16IS740(uint8_t address, uint32_t on_board_crystal) : I2c(address, i2c::kFullSpeed), on_board_crystal_hz_(on_board_crystal) {
    is_connected_ = IsConnected();

    if (!is_connected_) {
        DEBUG_PRINTF("Not connected at address %.2x", GetAddress());
        return;
    }

    SetFormat(8, SerialParity::kNone, 1);
    SetBaud(SC16IS7X0_DEFAULT_BAUDRATE);

    const uint8_t kTestCharacter = 'A';
    WriteRegister(SC16IS7X0_SPR, kTestCharacter, true);

    if ((ReadRegister(SC16IS7X0_SPR, false) != kTestCharacter)) {
        DEBUG_PUTS("Test character failed");
        is_connected_ = false;
        return;
    }

    auto register_mcr = ReadRegister(SC16IS7X0_MCR, false);
    register_mcr |= MCR_ENABLE_TCR_TLR;
    WriteRegister(SC16IS7X0_MCR, register_mcr, false);

    auto register_efr = ReadRegister(SC16IS7X0_EFR, false);
    WriteRegister(SC16IS7X0_EFR, static_cast<uint8_t>(register_efr | EFR_ENABLE_ENHANCED_FUNCTIONS), false);

    WriteRegister(SC16IS7X0_TLR, static_cast<uint8_t>(0x10), false);

    WriteRegister(SC16IS7X0_EFR, register_efr, false);

    WriteRegister(SC16IS7X0_FCR, static_cast<uint8_t>(FCR_RX_FIFO_RST | FCR_TX_FIFO_RST), false);
    WriteRegister(SC16IS7X0_FCR, FCR_ENABLE_FIFO, false);
    WriteRegister(SC16IS7X0_IER, static_cast<uint8_t>(IER_ELSI | IER_ERHRI), false);

    DEBUG_PRINTF("TLR=%.2x", ReadRegister(SC16IS7X0_TLR, false));
    debug::PrintBits(ReadRegister(SC16IS7X0_TLR, false));

    DEBUG_PRINTF("IER=%.2x", ReadRegister(SC16IS7X0_IER, false));
    debug::PrintBits(ReadRegister(SC16IS7X0_IER, false));

    DEBUG_PRINTF("IIR=%.2x", ReadRegister(SC16IS7X0_IIR, false));
    debug::PrintBits(ReadRegister(SC16IS7X0_IIR, false));
}

void SC16IS740::SetFormat(uint32_t bits, SerialParity parity, uint32_t stop_bits) {
    uint8_t register_lcr = 0x00;

    switch (bits) {
        case 5:
            register_lcr |= LCR_BITS5;
            break;
        case 6:
            register_lcr |= LCR_BITS6;
            break;
        case 7:
            register_lcr |= LCR_BITS7;
            break;
        case 8:
            register_lcr |= LCR_BITS8;
            break;
        default:
            register_lcr |= LCR_BITS8;
    }

    switch (parity) {
        case SerialParity::kNone:
            register_lcr |= LCR_NONE;
            break;
        case SerialParity::kOdd:
            register_lcr |= LCR_ODD;
            break;
        case SerialParity::kEven:
            register_lcr |= LCR_EVEN;
            break;
        case SerialParity::kForceD1:
            register_lcr |= LCR_FORCED1;
            break;
        case SerialParity::kForceD0:
            register_lcr |= LCR_FORCED0;
            break;
        default:
            register_lcr |= LCR_NONE;
    }

    switch (stop_bits) {
        case 1:
            register_lcr |= LCR_BITS1;
            break;
        case 2:
            register_lcr |= LCR_BITS2;
            break;
        default:
            register_lcr |= LCR_BITS1;
    }

    WriteRegister(SC16IS7X0_LCR, register_lcr, true);

    DEBUG_PRINTF("LCR=%.2x:%.2x", ReadRegister(SC16IS7X0_LCR, false), register_lcr);
}

void SC16IS740::SetBaud(uint32_t baud) {
    uint32_t prescaler;

    if ((ReadRegister(SC16IS7X0_MCR, true) & MCR_PRESCALE_4) == MCR_PRESCALE_4) {
        prescaler = 4;
    } else {
        prescaler = 1;
    }

    const uint32_t kDivisor = ((on_board_crystal_hz_ / prescaler) / (baud * 16U));
    const auto kRegisterLcr = ReadRegister(SC16IS7X0_LCR, false);

    WriteRegister(SC16IS7X0_LCR, static_cast<uint8_t>(kRegisterLcr | LCR_ENABLE_DIV), false);
    WriteRegister(SC16IS7X0_DLL, static_cast<uint8_t>(kDivisor & 0xFF), false);
    WriteRegister(SC16IS7X0_DLH, static_cast<uint8_t>((kDivisor >> 8) & 0xFF), false);
    WriteRegister(SC16IS7X0_LCR, kRegisterLcr, false);

    DEBUG_PRINTF("prescaler=%u", static_cast<unsigned>(prescaler));
    DEBUG_PRINTF("kDivisor=%u", static_cast<unsigned>(kDivisor));
    DEBUG_PRINTF("on_board_crystal_hz_=%u", static_cast<unsigned>(on_board_crystal_hz_));

    DEBUG_PRINTF("LCR=%.2x:%.2x", ReadRegister(SC16IS7X0_LCR, false), kRegisterLcr);
}

void SC16IS740::WriteBytes(const uint8_t* bytes, uint32_t size) {
    if (!is_connected_) {
        return;
    }

    auto* p = const_cast<uint8_t*>(bytes);

    Setup();

    while (size > 0) {
        uint32_t available = ReadRegister(SC16IS7X0_TXLVL, false);

        while ((size > 0) && (available > 0)) {
            WriteRegister(SC16IS7X0_THR, *p, false);
            size--;
            available--;
            p++;
        }
    }
}

void SC16IS740::ReadBytes(uint8_t* bytes, uint32_t& size, uint32_t time_out) {
    if (!is_connected_) {
        size = 0;
        return;
    }

    auto* destination = bytes;
    uint32_t remaining = size;

    Setup();

    while (remaining > 0) {
        const uint32_t kMillis = timing::Millis();
        uint32_t available;

        while ((available = ReadRegister(SC16IS7X0_RXLVL, false)) == 0) {
            if ((timing::Millis() - time_out) > kMillis) {
                remaining = 0;
                break;
            }
        }

        while ((remaining > 0) && (available > 0)) {
            *destination++ = ReadRegister(SC16IS7X0_RHR, false);
            remaining--;
            available--;
        }
    }

    size = static_cast<uint16_t>(destination - bytes);
}

void SC16IS740::FlushRead(uint32_t time_out) {
    if (!is_connected_) {
        return;
    }

    bool is_remaining = true;

    Setup();

    while (is_remaining) {
        const auto kMillis = timing::Millis();
        uint32_t available;

        while ((available = ReadRegister(SC16IS7X0_RXLVL, false)) == 0) {
            if ((timing::Millis() - time_out) > kMillis) {
                is_remaining = false;
                break;
            }
        }

        while (available > 0) {
            ReadRegister(SC16IS7X0_RHR, false);
            available--;
        }
    }
}
