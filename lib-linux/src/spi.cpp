/**
 * @file spi.cpp
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

#include <cstdio>
#if defined(__linux__)
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#endif

#include "spi.h"

namespace spi {
#if defined(__linux__)
static constexpr char kDevice[] = "/dev/spidev0.0";
int file = -1;
#endif

void Begin() {
#if defined(__linux__)
    if (file != -1) {
        close(file);
        file = -1;
    }

    file = open(kDevice, O_RDWR | O_CLOEXEC);
    if (file < 0) {
        perror(kDevice);
        return;
    }

    uint8_t bits = 8;

    if (ioctl(file, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1) {
        perror("SPI_IOC_WR_BITS_PER_WORD");
        close(file);
        file = -1;
    }
#else
    puts("spi::Begin");
#endif
}

void SetSpeedHz(uint32_t speed_hz) {
#if defined(__linux__)
    if (file < 0) [[unlikely]] {
        return;
    }

    if (ioctl(file, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz) == -1) {
        perror("SetSpeedHz");
    }
#else
    printf("spi::SetSpeedHz=%u\n", speed_hz);
#endif
}

void SetDataMode([[maybe_unused]] uint8_t mode) {
#if defined(__linux__)
    if (file < 0) [[unlikely]] {
        return;
    }

    uint8_t ioctl_mode = SPI_MODE_0;
    if (ioctl(file, SPI_IOC_WR_MODE, &ioctl_mode) == -1) {
        perror("SPI_IOC_WR_MODE");
    }
#else
    printf("spi::SetDataMode=%u\n", mode);
#endif
}

void ChipSelect([[maybe_unused]] uint8_t chip_select) {
#if defined(__linux__)
#else
    printf("spi::ChipSelect=%u\n", chip_select);
#endif
}

void Transfern(char* tx_buffer, uint32_t length) {
#if defined(__linux__)
    if (file < 0) [[unlikely]] {
        return;
    }

    struct spi_ioc_transfer tr{};

    tr.tx_buf = reinterpret_cast<unsigned long>(tx_buffer);
    tr.rx_buf = reinterpret_cast<unsigned long>(nullptr);
    tr.len = length;

    if (ioctl(file, SPI_IOC_MESSAGE(1), &tr) < 1) {
        perror("Transfern");
    }
#else
    printf("spi::Transfern=%p:%u\n", reinterpret_cast<void*>(tx_buffer), length);
#endif
}

void Write(uint16_t data) {
#if defined(__linux__)
    const char kSpiData[2] = {static_cast<char>(data >> 8), static_cast<char>(data & 0xFF)};
    Writenb(kSpiData, 2);
#else
    printf("spi::Write=%u\n", data);
#endif
}

void Writenb(const char* data, uint32_t length) {
#if defined(__linux__)
    Transfern(const_cast<char*>(data), length);
#else
    printf("spi::Writenb=%p:%u\n", reinterpret_cast<const void*>(data), length);
#endif
}
} // namespace spi