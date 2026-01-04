/**
 * @file bwspilcd.h
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

#ifndef BWSPILCD_H_
#define BWSPILCD_H_

#include <cstdint>

#include "hal_micros.h"

#include "bw.h"

class BwSpiLcd : BwSpi
{
    void SpiWrite(const char* pData, uint32_t length)
    {
        do
        {
        } while ((hal::Micros() - m_nSpiWriteUs) < bw::lcd::spi::write_delay_us);

        HAL_SPI::Write(pData, length);

        m_nSpiWriteUs = hal::Micros();
    }

   public:
    explicit BwSpiLcd(uint8_t nChipSelect = 0, uint8_t address = bw::lcd::address) : BwSpi(nChipSelect, address, bw::lcd::id_string) {}

    void ReInit()
    {
        char cmd[3];

        cmd[0] = static_cast<char>(address_);
        cmd[1] = bw::port::write::reinit_lcd;
        cmd[2] = ' ';

        SpiWrite(cmd, sizeof(cmd));
    }

    void Cls()
    {
        char cmd[3];

        cmd[0] = static_cast<char>(address_);
        cmd[1] = bw::port::write::clear_screen;
        cmd[2] = ' ';

        SpiWrite(cmd, sizeof(cmd));
    }

    void SetCursor(uint8_t nLine, uint8_t nPosition)
    {
        char cmd[3];

        cmd[0] = static_cast<char>(address_);
        cmd[1] = bw::port::write::move_cursor;
        cmd[2] = static_cast<char>(((nLine & 0x03) << 5) | (nPosition & 0x1f));

        SpiWrite(cmd, sizeof(cmd));
    }

    void Text(const char* pText, uint8_t length)
    {
        char data[bw::lcd::max_characters + 2];

        data[0] = static_cast<char>(address_);
        data[1] = bw::port::write::display_data;

        if (length > bw::lcd::max_characters)
        {
            length = bw::lcd::max_characters;
        }

        for (uint32_t i = 0; i < length; i++)
        {
            data[i + 2] = pText[i];
        }

        length = static_cast<uint8_t>(length + 2);

        SpiWrite(data, length);
    }

    void TextLine(uint8_t nLine, const char* pText, uint8_t length)
    {
        SetCursor(nLine, 0);
        Text(pText, length);
    }

    bool IsConnected() { return m_IsConnected; }

   private:
    uint32_t m_nSpiWriteUs = hal::Micros();
};

#endif  // BWSPILCD_H_
