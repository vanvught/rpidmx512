/*
 * spi.h
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

#include <cstdint>

#include "linux_spi.h"

namespace spi {
inline constexpr uint8_t kBitOrderMsbfirst = LINUX_SPI_BIT_ORDER_MSBFIRST; ///< MSB First

inline constexpr uint8_t kMode0 = LINUX_SPI_MODE0; ///< CPOL = 0, CPHA = 0
inline constexpr uint8_t kMode1 = LINUX_SPI_MODE1; ///< CPOL = 0, CPHA = 1
inline constexpr uint8_t kMode2 = LINUX_SPI_MODE2; ///< CPOL = 1, CPHA = 0
inline constexpr uint8_t kMode3 = LINUX_SPI_MODE3; ///< CPOL = 1, CPHA = 1

inline constexpr uint8_t kCs = LINUX_SPI_CS0;         ///< Chip Select
inline constexpr uint8_t kCsNone = LINUX_SPI_CS_NONE; ///< No CS, control it yourself

inline void Begin() {
    LinuxSpiBegin();
}

inline void SetSpeedHz(uint32_t speed_hz) {
    LinuxSpiSetSpeedHz(speed_hz);
}

inline void SetDataMode(uint8_t mode) {
    LinuxSpiSetDataMode(mode);
}
inline void ChipSelect(uint8_t chip_select) {
    LinuxSpiChipSelect(chip_select);
}

inline void Transfern(char* tx_buffer, uint32_t length) {
    LinuxSpiTransfern(tx_buffer, length);
}

inline void Write(uint16_t data) {
    LinuxSpiWrite(data);
}

inline void Writenb(const char* data, uint32_t length) {
    LinuxSpiWritenb(data, length);
}
} // namespace spi

#endif // SPI_H_
