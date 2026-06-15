/**
 * @file spi.h
 * @brief Linux
 */
/* Copyright (C) 2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SPI_H_
#define SPI_H_

#include <cstdint>

namespace spi {
inline constexpr uint8_t kBitOrderMsbfirst = 1; ///< MSB First

inline constexpr uint8_t kMode0 = 0; ///< CPOL = 0, CPHA = 0
inline constexpr uint8_t kMode1 = 1; ///< CPOL = 0, CPHA = 1
inline constexpr uint8_t kMode2 = 2; ///< CPOL = 1, CPHA = 0
inline constexpr uint8_t kMode3 = 3; ///< CPOL = 1, CPHA = 1

inline constexpr uint8_t kCs = 0;     ///< Chip Select
inline constexpr uint8_t kCsNone = 1; ///< No CS, control it yourself

void Begin();
void SetSpeedHz(uint32_t speed_hz);
void SetDataMode(uint8_t mode);
void ChipSelect(uint8_t chip_select);
void Transfern(char* tx_buffer, uint32_t length);
void Write(uint16_t data);
void Writenb(const char* data, uint32_t length);
} // namespace spi

class Spi {
   public:
    Spi(uint8_t chip_select, uint32_t speed_hz, uint8_t mode = spi::kMode0) : speed_hz_(speed_hz), chip_select_(chip_select), mode_(mode & 0x3) { spi::Begin(); }

    void Write(const char* data, uint32_t length, bool do_setup) {
        if (do_setup) {
            Setup();
        }
        spi::Writenb(data, length);
    }

    void Write(uint16_t data, bool do_setup = true) {
        if (do_setup) {
            Setup();
        }
        spi::Write(data);
    }

    void WriteRead(char* data, uint32_t length, bool do_setup) {
        if (do_setup) {
            Setup();
        }
        spi::Transfern(data, length);
    }

   private:
    void Setup() {
        spi::ChipSelect(chip_select_);
        spi::SetDataMode(mode_);
        spi::SetSpeedHz(speed_hz_);
    }

   private:
    uint32_t speed_hz_;
    uint8_t chip_select_;
    uint8_t mode_;
};

#endif // SPI_H_
