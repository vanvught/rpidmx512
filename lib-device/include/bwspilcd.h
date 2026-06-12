/**
 * @file bwspilcd.h
 *
 */
/* Copyright (C) 2020-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "timing.h"
#include "bw.h"

class BwSpiLcd : BwSpi {
   public:
    explicit BwSpiLcd(uint8_t chip_select = 0, uint8_t address = bw::lcd::address) : BwSpi(chip_select, address, bw::lcd::id_string) {}

    void ReInit() {
        char cmd[3];

        cmd[0] = static_cast<char>(address_);
        cmd[1] = bw::port::write::kReinitLcd;
        cmd[2] = ' ';

        SpiWrite(cmd, sizeof(cmd));
    }

    void Cls() {
        char cmd[3];

        cmd[0] = static_cast<char>(address_);
        cmd[1] = bw::port::write::kClearScreen;
        cmd[2] = ' ';

        SpiWrite(cmd, sizeof(cmd));
    }

    void SetCursor(uint8_t line, uint8_t position) {
        char cmd[3];

        cmd[0] = static_cast<char>(address_);
        cmd[1] = bw::port::write::kMoveCursor;
        cmd[2] = static_cast<char>(((line & 0x03) << 5) | (position & 0x1f));

        SpiWrite(cmd, sizeof(cmd));
    }

    void Text(const char* text, uint8_t length) {
        char data[bw::lcd::max_characters + 2];

        data[0] = static_cast<char>(address_);
        data[1] = bw::port::write::kDisplayData;

        if (length > bw::lcd::max_characters) {
            length = bw::lcd::max_characters;
        }

        for (uint32_t i = 0; i < length; i++) {
            data[i + 2] = text[i];
        }

        length = static_cast<uint8_t>(length + 2);

        SpiWrite(data, length);
    }

    void TextLine(uint8_t line, const char* text, uint8_t length) {
        SetCursor(line, 0);
        Text(text, length);
    }

    bool IsConnected() { return connected_; }

   private:
    void SpiWrite(const char* data, uint32_t length) {
        do {
        } while ((timing::Micros() - write_us_) < bw::lcd::spi::write_delay_us);

        Spi::Write(data, length, true);

        write_us_ = timing::Micros();
    }

   private:
    uint32_t write_us_ = timing::Micros();
};

#endif // BWSPILCD_H_
