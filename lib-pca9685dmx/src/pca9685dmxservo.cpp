/**
 * @file pca9685dmxservo.cpp
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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "pca9685dmxservo.h"
 #include "firmware/debug/debug_debug.h"

PCA9685DmxServo::PCA9685DmxServo(const pca9685dmx::Configuration& configuration)
{
    DEBUG_ENTRY();

    use8_bit_ = configuration.bUse8Bit;
    channel_count_ = configuration.nChannelCount;

    if (use8_bit_)
    {
        dmx_footprint_ = channel_count_;
    }
    else
    {
        dmx_footprint_ = 2 * channel_count_;
    }

    DEBUG_PRINTF("m_bUse8Bit=%c, m_nChannelCount=%u, dmx_footprint_=%u", m_bUse8Bit ? 'Y' : 'N', m_nChannelCount, dmx_footprint_);

    board_instances_ = static_cast<uint8_t>((channel_count_ + (pca9685::PWM_CHANNELS - 1)) / pca9685::PWM_CHANNELS);

    DEBUG_PRINTF("board_instances_=%u", board_instances_);

    dmx_start_address_ = configuration.nDmxStartAddress;

    memset(dmx_data_, 0, sizeof(dmx_data_));

    servo_ = new PCA9685Servo*[board_instances_];
    assert(m_pServo != nullptr);

    for (uint32_t i = 0; i < board_instances_; i++)
    {
        servo_[i] = new PCA9685Servo(static_cast<uint8_t>(configuration.address + i));
        assert(m_pServo[i] != nullptr);

        servo_[i]->SetLeftUs(configuration.servo.nLeftUs);
        servo_[i]->SetCenterUs(configuration.servo.nCenterUs);
        servo_[i]->SetRightUs(configuration.servo.nRightUs);
#ifndef NDEBUG
        printf("Instance %d [%X]\n", i, configuration.address + i);
        m_pServo[i]->Dump();
        puts("");
#endif
    }

    DEBUG_EXIT();
}

PCA9685DmxServo::~PCA9685DmxServo()
{
    DEBUG_ENTRY();

    for (uint32_t i = 0; i < board_instances_; i++)
    {
        delete servo_[i];
        servo_[i] = nullptr;
    }

    delete[] servo_;
    servo_ = nullptr;

    DEBUG_EXIT();
}

void PCA9685DmxServo::SetDataImpl([[maybe_unused]] uint32_t port_index, const uint8_t* pDmxData, uint32_t length)
{
    assert(pDmxData != nullptr);
    assert(length <= dmxnode::kUniverseSize);

    auto dmx_address = dmx_start_address_;

    if (use8_bit_)
    {
        auto* current_data = const_cast<uint8_t*>(pDmxData) + dmx_start_address_ - 1;
        auto* previous_data = dmx_data_;

        for (uint32_t j = 0; j < board_instances_; j++)
        {
            for (uint32_t i = 0; i < pca9685::PWM_CHANNELS; i++)
            {
                if ((dmx_address >= (dmx_footprint_ + dmx_start_address_)) || (dmx_address > length))
                {
                    j = board_instances_;
                    break;
                }
                if (*current_data != *previous_data)
                {
                    *previous_data = *current_data;
                    const auto value = *current_data;
#ifndef NDEBUG
                    printf("m_pServo[%u]->SetDmx(CHANNEL(%u), %u)\n", j, i, static_cast<uint32_t>(value));
#endif
                    servo_[j]->Set(i, value);
                }
                current_data++;
                previous_data++;
                dmx_address++;
            }
        }
    }
    else
    {
        auto* pCurrentData = reinterpret_cast<const uint16_t*>(pDmxData + dmx_start_address_ - 1);
        auto* pPreviousData = reinterpret_cast<uint16_t*>(dmx_data_);

        for (uint32_t j = 0; j < board_instances_; j++)
        {
            for (uint32_t i = 0; i < pca9685::PWM_CHANNELS; i++)
            {
                if ((dmx_address >= (dmx_footprint_ + dmx_start_address_)) || (dmx_address > length))
                {
                    j = board_instances_;
                    break;
                }

                if (*pCurrentData != *pPreviousData)
                {
                    *pPreviousData = *pCurrentData;
                    const auto* pData = reinterpret_cast<const uint8_t*>(pCurrentData);
                    const auto kValue = static_cast<uint16_t>((static_cast<uint32_t>(pData[0]) << 4) | static_cast<uint32_t>(pData[1]));
#ifndef NDEBUG
                    printf("m_pServo[%u]->SetDmx(CHANNEL(%u), %u)\n", j, i, static_cast<uint32_t>(value));
#endif
                    servo_[j]->Set(i, kValue);
                }
                pCurrentData++;
                pPreviousData++;
                dmx_address += 2;
            }
        }
    }
}

void PCA9685DmxServo::Print()
{
    printf("PCA9685 Servo %d-bit\n", use8_bit_ ? 8 : 16);
    printf(" Board instances: %u\n", board_instances_);
    printf(" Channel count: %u\n", channel_count_);
    printf(" DMX start address: %u [footprint: %u]\n", dmx_start_address_, dmx_footprint_);
}
