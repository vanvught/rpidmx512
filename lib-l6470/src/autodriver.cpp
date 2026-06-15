/**
 * @file autodriver.cpp
 *
 */
/*
 * Based on https://github.com/sparkfun/L6470-AutoDriver/tree/master/Libraries/Arduino
 */
/* Copyright (C) 2017-2026 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "spi.h"
#include "gpio.h"
#include "autodriver.h"
#include "l6470constants.h"
#include "firmware/debug/debug_debug.h"

static constexpr uint8_t kBusyPinNotUsed = 0xFF;

uint8_t AutoDriver::s_num_boards[2];

AutoDriver::AutoDriver(uint8_t position, uint8_t chip_select, uint8_t reset_pin, uint8_t busy_pin) : chip_select_(chip_select), reset_pin_(reset_pin), busy_pin_(busy_pin), position_(position), busy_(false) {
    DEBUG_ENTRY();
    DEBUG_PRINTF("position=%d, chip_select=%d\n", static_cast<int>(position), static_cast<int>(chip_select));

    s_num_boards[chip_select]++;

    DEBUG_PRINTF("m_nNumBoards[%d]=%d", static_cast<int>(chip_select), static_cast<int>(m_nNumBoards[chip_select]));
    DEBUG_EXIT();
}

AutoDriver::AutoDriver(uint8_t position, uint8_t chip_select, uint8_t reset_pin) : chip_select_(chip_select), reset_pin_(reset_pin), busy_pin_(kBusyPinNotUsed), position_(position), busy_(false) {
    DEBUG_ENTRY();
    DEBUG_PRINTF("position=%d, chip_select=%d\n", static_cast<int>(position), static_cast<int>(chip_select));

    s_num_boards[chip_select]++;

    DEBUG_PRINTF("m_nNumBoards[%d]=%d", static_cast<int>(chip_select), static_cast<int>(m_nNumBoards[chip_select]));
    DEBUG_EXIT();
}

AutoDriver::~AutoDriver() {
    hardHiZ();
    busy_ = false;
    s_num_boards[chip_select_]--;
}

int AutoDriver::busyCheck() {
    if (busy_pin_ == kBusyPinNotUsed) {
        if (getParam(L6470_PARAM_STATUS) & L6470_STATUS_BUSY) {
            return 0;
        } else {
            return 1;
        }
    } else {
        if (!busy_) {
            if (getParam(L6470_PARAM_STATUS) & L6470_STATUS_BUSY) {
                return 0;
            } else {
                busy_ = true;
                return 1;
            }
        }
        // By default, the BUSY pin is forced low when the device is performing a command
        if (gpio::Lev(busy_pin_) == HIGH) {
            busy_ = false;
            return 0;
        } else {
            return 1;
        }
    }
}

void AutoDriver::Print() {
    printf("SparkFun AutoDriver [%d]\n", motor_number_);
    printf(" Position=%d, ChipSelect=%d, ResetPin=%d, BusyPin=%d [%s]\n", position_, chip_select_, reset_pin_, busy_pin_, busy_pin_ == 0xFF ? "SPI" : "GPIO");
    printf(" MinSpeed=%3.0f, MaxSpeed=%3.0f, Acc=%4.0f, Dec=%4.0f\n", getMinSpeed(), getMaxSpeed(), getAcc(), getDec());
    printf(" AccKVAL=%d, DecKVAL=%d, RunKVAL=%d, HoldKVAL=%d\n", static_cast<int>(getAccKVAL()), static_cast<int>(getDecKVAL()), static_cast<int>(getRunKVAL()), static_cast<int>(getHoldKVAL()));
    printf(" MicroSteps=%u, SwitchMode=%d\n", static_cast<unsigned>(1) << getStepMode(), getSwitchMode());
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvla"
#pragma GCC diagnostic ignored "-Wstack-usage=" // FIXME Needed for compilation on GD32F (https://www.gd32-dmx.org)

uint8_t AutoDriver::SPIXfer(uint8_t data) {
    DEBUG_ENTRY();

    char data_packet[s_num_boards[chip_select_]];

    for (uint32_t i = 0; i < s_num_boards[chip_select_]; i++) {
        data_packet[i] = 0;
    }

    data_packet[position_] = static_cast<char>(data);

    spi::ChipSelect(chip_select_);
    spi::SetSpeedHz(2000000);
    spi::SetDataMode(spi::kMode3);
    spi::Transfern(data_packet, s_num_boards[chip_select_]);

    DEBUG_PRINTF("data=%x, dataPacket[%d]=%x", data, position_, dataPacket[position_]);
    DEBUG_EXIT();
    return static_cast<uint8_t>(data_packet[position_]);
}

#pragma GCC diagnostic pop

uint16_t AutoDriver::GetNumBoards() {
    uint16_t n = 0;

    for (uint32_t i = 0; i < (sizeof(s_num_boards) / sizeof(s_num_boards[0])); i++) {
        n += s_num_boards[i];
    }

    return n;
}

uint8_t AutoDriver::GetNumBoards(uint8_t cs) {
    if (cs < (sizeof(s_num_boards) / sizeof(s_num_boards[0]))) {
        return s_num_boards[cs];
    } else {
        return 0;
    }
}

bool AutoDriver::IsConnected() {
    DEBUG_ENTRY();

    if (getParam(L6470_PARAM_CONFIG) == 0x2e88) {
        DEBUG_EXIT();
        return true;
    }

    DEBUG_EXIT();
    return false;
}
