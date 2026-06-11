/**
 * @file bw.h
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

#ifndef BW_H_
#define BW_H_

#include <cstdint>
#include <string.h>
#include <algorithm>

#include "spi.h"

/*
 * http://www.bitwizard.nl/wiki/index.php/Default_addresses
 */

namespace bw {
namespace id_string {
inline constexpr size_t kLength = 0;
} // namespace id_string

namespace spi::speed {
inline constexpr uint32_t max_hz = 50000;     ///< 50 kHz
inline constexpr uint32_t default_hz = 50000; ///< 50 kHz
} // namespace spi::speed

namespace lcd {
inline constexpr uint8_t address = 0x82;
inline constexpr char id_string[] = "spi_lcd";
inline constexpr uint8_t max_characters = 16;
inline constexpr uint8_t max_lines = 2;
namespace spi {
inline constexpr uint32_t write_delay_us = 12;
} // namespace spi
} // namespace lcd

namespace dio {
inline constexpr uint8_t address = 0x84;
inline constexpr char id_string[] = "spi_dio";
} // namespace dio

namespace fets {
inline constexpr uint8_t address = 0x88;
inline constexpr char id_string[] = "spi_7fets";
} // namespace fets

namespace relay {
inline constexpr uint8_t address = 0x8E;
inline constexpr char id_string[] = "spi_relay";
} // namespace relay

namespace dimmer {
inline constexpr uint8_t address = 0x9E;
inline constexpr char id_string[] = "spi_dimmer";
} // namespace dimmer

namespace port {
namespace read {
inline constexpr uint8_t id_string = 0x01;
} // namespace read
namespace write {
inline constexpr uint8_t kSetAllOutputs = 0x10;
inline constexpr uint8_t kIoDirection = 0x30;

inline constexpr uint8_t kDisplayData = 0x00;
inline constexpr uint8_t kClearScreen = 0x10; ///< any data clears the screen
inline constexpr uint8_t kMoveCursor = 0x11;
inline constexpr uint8_t kReinitLcd = 0x14;

} // namespace write
} // namespace port

} // namespace bw

class BwSpi {
   public:
    BwSpi(uint8_t chip_select, uint8_t address, const char* string) : chip_select_(chip_select), address_(address) {
        spi::Begin();
        Setup();

        char buffer[bw::id_string::kLength + 2];

        buffer[0] = static_cast<char>(address_ | 1);
        buffer[1] = bw::port::read::id_string;

        spi::Transfern(buffer, sizeof(buffer));

        if (string != nullptr) {
            const auto kLength = std::min(bw::id_string::kLength, strlen(string));
            m_IsConnected = (strncmp(&buffer[2], string, kLength) == 0);
        }
    }

    void Write(const char* data, uint32_t length, bool do_setup = true) {
        if (do_setup) {
            Setup();
        }

        spi::Transfern(const_cast<char*>(data), length);
    }

   private:
    void Setup() {
        spi::ChipSelect(chip_select_);
        spi::SetDataMode(spi::kMode0);
        spi::SetSpeedHz(bw::spi::speed::default_hz);
    }

   protected:
    uint8_t chip_select_;
    uint8_t address_;
    bool m_IsConnected = false;
};

#endif // BW_H_
