/**
 * @file rdmsubdevicebw7fets.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SPI_RDMSUBDEVICEBW7FETS_H_
#define SPI_RDMSUBDEVICEBW7FETS_H_

#include <cstdint>

#include "rdmsubdevice.h"
#include "bwspi7fets.h"

class RDMSubDeviceBw7fets : public RDMSubDevice {
   public:
    explicit RDMSubDeviceBw7fets(uint16_t dmx_start_address = 1, char chip_select = 0, uint8_t device_address = bw::fets::address, uint32_t spi_speed_hz = bw::spi::speed::default_hz);

    bool Initialize() override {
        if (spi7fets_.IsConnected()) {
            spi7fets_.Output(0x00);
            return true;
        }
        return false;
    }

    void Start() override {}

    void Stop() override {
        spi7fets_.Output(0x00);
        data_ = 0;
    }

    void Data(const uint8_t* data, uint32_t length) override;

   private:
    void UpdateEvent(TRDMSubDeviceUpdateEvent event) override;

   private:
    BwSpi7fets spi7fets_;
    uint8_t data_{0};
};

#endif // SPI_RDMSUBDEVICEBW7FETS_H_
