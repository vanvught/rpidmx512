/**
 * @file bwspi7fets.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef BWSPI7FETS_H_
#define BWSPI7FETS_H_

#include <cstdint>

#include "bw.h"

class BwSpi7fets : BwSpi
{
    void SetDirection(uint8_t nMask)
    {
        char cmd[3];

        cmd[0] = static_cast<char>(address_);
        cmd[1] = bw::port::write::io_direction;
        cmd[2] = static_cast<char>(nMask);

        HAL_SPI::Write(cmd, sizeof(cmd));
    }

   public:
    explicit BwSpi7fets(uint8_t nChipSelect = 0, uint8_t address = bw::fets::address) : BwSpi(nChipSelect, address, bw::fets::id_string) { SetDirection(0x7F); }

    void Output(uint8_t nPins)
    {
        char cmd[3];

        cmd[0] = static_cast<char>(address_);
        cmd[1] = bw::port::write::set_all_outputs;
        cmd[2] = static_cast<char>(nPins);

        HAL_SPI::Write(cmd, sizeof(cmd));
    }

    bool IsConnected() { return m_IsConnected; }
};

#endif  // BWSPI7FETS_H_
