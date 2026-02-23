/**
 * @file pca9685dmxservo.h
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

#ifndef PCA9685DMXSERVO_H_
#define PCA9685DMXSERVO_H_

#include <cstdint>

#include "pca9685dmx.h"
#include "pca9685dmxstore.h"
#include "pca9685servo.h"
#include "pca9685dmxset.h"

#include "dmxnode.h"

class PCA9685DmxServo : public PCA9685DmxSet
{
   public:
    explicit PCA9685DmxServo(const pca9685dmx::Configuration& configuration);
    ~PCA9685DmxServo() override;

    void Start([[maybe_unused]] uint32_t port_index) override {};
    void Stop([[maybe_unused]] uint32_t port_index) override {};

    void SetDataImpl(uint32_t port_index, const uint8_t* dmx_data, uint32_t length) override;

    void Sync([[maybe_unused]] uint32_t port_index) override {};
    void Sync() override {};

    bool SetDmxStartAddress(uint16_t dmx_start_address) override
    {
        assert((dmx_start_address != 0) && (dmx_start_address <= dmxnode::kUniverseSize));

        if ((dmx_start_address != 0) && (dmx_start_address <= dmxnode::kUniverseSize))
        {
            dmx_start_address_ = dmx_start_address;
            dmxpwm_store::SaveDmxStartAddress(dmx_start_address_);
            return true;
        }

        return false;
    }

    uint16_t GetDmxStartAddress() override { return dmx_start_address_; }

    uint16_t GetDmxFootprint() override { return dmx_footprint_; }

    void Print() override;

   private:
    uint16_t board_instances_;
    uint16_t dmx_footprint_;
    uint16_t dmx_start_address_;
    uint16_t channel_count_;
    bool use8_bit_;
    uint8_t dmx_data_[dmxnode::kUniverseSize];
    PCA9685Servo** servo_;
};

#endif  // PCA9685DMXSERVO_H_
