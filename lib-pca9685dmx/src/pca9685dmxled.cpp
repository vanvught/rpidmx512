/**
 * @file pca9685dmxled.cpp
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

#include "pca9685dmxled.h"
 #include "firmware/debug/debug_debug.h"
#include "pca9685.h"

PCA9685DmxLed::PCA9685DmxLed(const pca9685dmx::Configuration& configuration)
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
        if (channel_count_ > dmxnode::kUniverseSize / 2)
        {
            channel_count_ = dmxnode::kUniverseSize / 2;
        }
        dmx_footprint_ = 2 * channel_count_;
    }

    DEBUG_PRINTF("use8_bit_=%c, channel_count_=%u, dmx_footprint_=%u", use8_bit_ ? 'Y' : 'N', channel_count_, dmx_footprint_);

    board_instances_ = static_cast<uint8_t>((channel_count_ + (pca9685::PWM_CHANNELS - 1)) / pca9685::PWM_CHANNELS);

    DEBUG_PRINTF("board_instances_=%u", board_instances_);

    dmx_start_address_ = configuration.nDmxStartAddress;

    memset(dmx_data_, 0, sizeof(dmx_data_));

    pwm_led_ = new PCA9685PWMLed*[board_instances_];
    assert(pwmled_ != nullptr);

    for (uint32_t i = 0; i < board_instances_; i++)
    {
        pwm_led_[i] = new PCA9685PWMLed(static_cast<uint8_t>(configuration.address + i));
        assert(pwmled_[i] != nullptr);

        pwm_led_[i]->SetInvert(configuration.led.invert);
        pwm_led_[i]->SetOutDriver(configuration.led.output);
        pwm_led_[i]->SetFrequency(configuration.led.nLedPwmFrequency);
        pwm_led_[i]->SetFullOff(CHANNEL(16), true);
#ifndef NDEBUG
        printf("Instance %d [%X]\n", i, configuration.address + i);
        pwmled_[i]->Dump();
        puts("");
#endif
    }

    DEBUG_EXIT();
}

PCA9685DmxLed::~PCA9685DmxLed()
{
    DEBUG_ENTRY();

    for (uint32_t i = 0; i < board_instances_; i++)
    {
        delete pwm_led_[i];
        pwm_led_[i] = nullptr;
    }

    delete[] pwm_led_;
    pwm_led_ = nullptr;

    DEBUG_EXIT();
}

void PCA9685DmxLed::Stop([[maybe_unused]] uint32_t port_index)
{
    DEBUG_ENTRY();

    for (uint32_t j = 0; j < board_instances_; j++)
    {
        pwm_led_[j]->SetFullOff(CHANNEL(16), true);
    }

    DEBUG_EXIT();
}

void PCA9685DmxLed::SetDataImpl([[maybe_unused]] uint32_t port_index, const uint8_t* dmx_data, uint32_t length)
{
    assert(dmx_data != nullptr);
    assert(length <= dmxnode::kUniverseSize);

    auto dmx_start_address = dmx_start_address_;

    if (use8_bit_)
    {
        auto* current_data = const_cast<uint8_t*>(dmx_data) + dmx_start_address_ - 1;
        auto* previous_data = dmx_data_;

        for (uint32_t j = 0; j < board_instances_; j++)
        {
            for (uint32_t i = 0; i < pca9685::PWM_CHANNELS; i++)
            {
                if ((dmx_start_address >= (dmx_footprint_ + dmx_start_address_)) || (dmx_start_address > length))
                {
                    j = board_instances_;
                    break;
                }
                if (*current_data != *previous_data)
                {
                    *previous_data = *current_data;
                    const auto kValue = *current_data;
#ifndef NDEBUG
                    printf("pwmled_[%u]->SetDmx(CHANNEL(%u), %u)\n", j, i, static_cast<uint32_t>(kValue));
#endif
                    pwm_led_[j]->Set(i, kValue);
                }
                current_data++;
                previous_data++;
                dmx_start_address++;
            }
        }
    }
    else
    {
        auto* current_data = reinterpret_cast<const uint16_t*>(dmx_data + dmx_start_address_ - 1);
        auto* previous_data = reinterpret_cast<uint16_t*>(dmx_data_);

        for (uint32_t j = 0; j < board_instances_; j++)
        {
            for (uint32_t i = 0; i < pca9685::PWM_CHANNELS; i++)
            {
                if ((dmx_start_address >= (dmx_footprint_ + dmx_start_address_)) || (dmx_start_address > length))
                {
                    j = board_instances_;
                    break;
                }

                if (*current_data != *previous_data)
                {
                    *previous_data = *current_data;
                    const auto* data = reinterpret_cast<const uint8_t*>(current_data);
                    const auto kValue = static_cast<uint16_t>((static_cast<uint32_t>(data[0]) << 4) | static_cast<uint32_t>(data[1]));
#ifndef NDEBUG
                    printf("pwmled_[%u]->SetDmx(CHANNEL(%u), %u)\n", j, i, static_cast<uint32_t>(kValue));
#endif
                    pwm_led_[j]->Set(i, kValue);
                }
                current_data++;
                previous_data++;
                dmx_start_address += 2;
            }
        }
    }
}

bool PCA9685DmxLed::GetSlotInfo(uint16_t slot_offset, dmxnode::SlotInfo& slot_info)
{
    if (slot_offset > dmx_footprint_)
    {
        return false;
    }

    slot_info.type = 0;
    slot_info.category = 0;

    return true;
}

void PCA9685DmxLed::Print()
{
    printf("PWM Led %d-bit\n", use8_bit_ ? 8 : 16);
    printf(" Board instances: %u\n", board_instances_);
    printf(" DMX Start address: %u\n", dmx_start_address_);
    printf(" Channel count: %u\n", channel_count_);
    printf(" Output logic state %sinverted\n", pwm_led_[0]->GetInvert() == pca9685::Invert::kOutputNotInverted ? "" : "not ");
    printf(" The outputs are configured with %s structure\n", pwm_led_[0]->GetOutDriver() == pca9685::Output::kDriverOpendrain ? "an open-drain" : "a totem pole");
}
