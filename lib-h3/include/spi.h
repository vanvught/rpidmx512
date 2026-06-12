/**
 * @file spi.h
 *
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

#if defined(CONFIG_SPI_OPTIMIZE_O2) || defined(CONFIG_SPI_OPTIMIZE_O3)
#pragma GCC push_options
#if defined(CONFIG_SPIOPTIMIZE_O2)
#pragma GCC optimize("O2")
#else
#pragma GCC optimize("O3")
#endif
#endif

#include <cstdint>

#include "h3_spi.h"

namespace spi {
inline constexpr uint8_t kBitOrderLsbfirst = H3_SPI_BIT_ORDER_LSBFIRST; ///< LSB First
inline constexpr uint8_t kBitOrderMsbfirst = H3_SPI_BIT_ORDER_MSBFIRST; ///< MSB First

inline constexpr uint8_t kMode0 = H3_SPI_MODE0; ///< CPOL = 0, CPHA = 0
inline constexpr uint8_t kMode1 = H3_SPI_MODE1; ///< CPOL = 0, CPHA = 1
inline constexpr uint8_t kMode2 = H3_SPI_MODE2; ///< CPOL = 1, CPHA = 0
inline constexpr uint8_t kMode3 = H3_SPI_MODE3; ///< CPOL = 1, CPHA = 1

inline constexpr uint8_t kCs = H3_SPI_CS0;         ///< Chip Select
inline constexpr uint8_t kCsNone = H3_SPI_CS_NONE; ///< No CS, control it yourself

inline void Begin() {
    H3SpiBegin();
}

inline void End() {
    H3SpiEnd();
}

inline void SetSpeedHz(uint32_t speed_hz) {
    H3SpiSetSpeedHz(speed_hz);
}

inline void SetDataMode(uint8_t mode) {
    H3SpiSetDataMode(mode);
}
inline void ChipSelect(uint8_t chip_select) {
    H3SpiChipSelect(chip_select);
}

inline void Transfernb(const char* tx_buffer, char* rx_buffer, uint32_t length) {
    H3SpiTransfernb(tx_buffer, rx_buffer, length);
}

inline void Transfern(char* tx_buffer, uint32_t length) {
    H3SpiTransfern(tx_buffer, length);
}

inline void Write(uint16_t data) {
    H3SpiWrite(data);
}

inline void Writenb(const char* data, uint32_t length) {
    H3SpiWritenb(data, length);
}
} // namespace spi

class Spi {
   public:
    Spi(uint8_t chip_select, uint32_t speed_hz, uint8_t mode = spi::kMode0) : speed_hz_(speed_hz), chip_select_(chip_select), mode_(mode & 0x3) { H3SpiBegin(); }

    void Write(const char* data, uint32_t length, bool do_setup) {
        if (do_setup) {
            Setup();
        }
        H3SpiWritenb(data, length);
    }

    void Write(uint16_t data, bool do_setup = true) {
        if (do_setup) {
            Setup();
        }
        H3SpiWrite(data);
    }

    void WriteRead(char* data, uint32_t length, bool do_setup) {
        if (do_setup) {
            Setup();
        }
        H3SpiTransfern(data, length);
    }

   private:
    void Setup() {
        H3SpiChipSelect(chip_select_);
        H3SpiSetDataMode(mode_);
        H3SpiSetSpeedHz(speed_hz_);
    }

   private:
    uint32_t speed_hz_;
    uint8_t chip_select_;
    uint8_t mode_;
};

#if defined(CONFIG_SPI_OPTIMIZE_O2) || defined(CONFIG_SPI_OPTIMIZE_O3)
#pragma GCC pop_options
#endif

#endif // SPI_H_
