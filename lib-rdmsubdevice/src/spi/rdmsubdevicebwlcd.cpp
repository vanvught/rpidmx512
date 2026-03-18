/**
 * @file rdmsubdevicebwlcd.cpp
 *
 */
/* Copyright (C) 2018-2026 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "spi/rdmsubdevicebwlcd.h"
#include "bwspilcd.h"
#include "common/utils/utils_hex.h"

static constexpr uint32_t kDmxFootprint = 4;
static RDMPersonality* rdm_personalities[] = {new RDMPersonality("LCD 4-slots H", kDmxFootprint), new RDMPersonality("LCD 4-slots D", kDmxFootprint),
                                              new RDMPersonality("LCD 4-slots %%", kDmxFootprint)};
static constexpr char kLine[bw::lcd::max_characters + 1] = "--- --- --- --- ";

RDMSubDeviceBwLcd::RDMSubDeviceBwLcd(uint16_t start_address, char chip_select, uint8_t spi_address, [[maybe_unused]] uint32_t speed)
    : RDMSubDevice("bw_spi_lcd", start_address), m_BwSpiLcd(chip_select, spi_address)
{
    SetDmxFootprint(kDmxFootprint);
    SetPersonalities(rdm_personalities, 3);

    memset(m_aText, ' ', sizeof(m_aText));
    memset(data_, 0, sizeof(data_));
}

bool RDMSubDeviceBwLcd::Initialize()
{
    if (m_BwSpiLcd.IsConnected())
    {
        memset(m_aText, ' ', sizeof(m_aText));
        m_BwSpiLcd.TextLine(0, m_aText, sizeof(m_aText));
        m_BwSpiLcd.TextLine(1, m_aText, sizeof(m_aText));
        return true;
    }
    return false;
}

void RDMSubDeviceBwLcd::Start()
{
    if (m_IsStarted)
    {
        return;
    }

    m_IsStarted = true;

    DisplayChannels();

    memset(m_aText, ' ', sizeof(m_aText));

    DisplayUpdatePersonality();

    m_BwSpiLcd.TextLine(1, m_aText, bw::lcd::max_characters);
}

void RDMSubDeviceBwLcd::Stop()
{
    if (!m_IsStarted)
    {
        return;
    }

    m_IsStarted = false;
    m_BwSpiLcd.TextLine(1, kLine, bw::lcd::max_characters - 1); // Leave H, D, % at the end
}

void RDMSubDeviceBwLcd::Data(const uint8_t* data, uint32_t length)
{
    const auto kDmxStartAddress = GetDmxStartAddress();
    auto is_data_changed = false;

    length = std::min(length, kDmxFootprint);
    length = std::min(length, static_cast<uint32_t>(513U - kDmxStartAddress));

    const auto* p = &data[kDmxStartAddress - 1];

    for (uint32_t i = 0; (i < sizeof(data_)) && (i < length); i++)
    {
        if (data_[i] != p[i])
        {
            is_data_changed = true;
        }
        data_[i] = p[i];
    }

    if (!is_data_changed)
    {
        return;
    }

    length_ = length;

    switch (GetPersonalityCurrent())
    {
        case 1:
            DataHex(p, length);
            break;
        case 2:
            DataDec(p, length);
            break;
        case 3:
            DataPct(p, length);
            break;
        default:
            break;
    }

    m_BwSpiLcd.TextLine(1, m_aText, bw::lcd::max_characters - 1);
}

void RDMSubDeviceBwLcd::DisplayChannels()
{
    char text[bw::lcd::max_characters];
    const uint16_t kStartAddress = GetDmxStartAddress();
    unsigned i;

    for (i = 0; (i < kDmxFootprint) && (kStartAddress + i) <= 512; i++)
    {
        unsigned offset = i * 4;
        text[offset] = ' ';
        text[offset + 1] = ' ';
        text[offset + 2] = ' ';
        text[offset + 3] = ' ';

        itoaBase10(static_cast<uint16_t>(kStartAddress + i), &text[offset]);
    }

    for (; i < kDmxFootprint; i++)
    {
        unsigned offset = i * 4;
        text[offset] = ' ';
        text[offset + 1] = ' ';
        text[offset + 2] = ' ';
        text[offset + 3] = ' ';
    }

    m_BwSpiLcd.TextLine(0, text, bw::lcd::max_characters);
}

void RDMSubDeviceBwLcd::DataHex(const uint8_t* data, uint32_t length)
{
    unsigned j;

    for (j = 0; j < length; j++)
    {
        unsigned offset = j * 4;
        const uint8_t kData = data[j];
        m_aText[offset] = ' ';
        m_aText[offset + 1] = common::hex::ToCharUppercase((kData & 0xF0) >> 4);
        m_aText[offset + 2] = common::hex::ToCharUppercase(kData & 0x0F);
    }

    for (; j < kDmxFootprint; j++)
    {
        unsigned offset = j * 4;
        m_aText[offset] = ' ';
        m_aText[offset + 1] = ' ';
        m_aText[offset + 2] = ' ';
    }
}

void RDMSubDeviceBwLcd::DataDec(const uint8_t* data, uint32_t length)
{
    unsigned j;

    for (j = 0; j < length; j++)
    {
        unsigned offset = j * 4;
        m_aText[offset] = ' ';
        m_aText[offset + 1] = ' ';
        itoaBase10(data[j], &m_aText[offset]);
    }

    for (; j < kDmxFootprint; j++)
    {
        unsigned offset = j * 4;
        m_aText[offset] = ' ';
        m_aText[offset + 1] = ' ';
        m_aText[offset + 2] = ' ';
    }
}

void RDMSubDeviceBwLcd::DataPct(const uint8_t* data, uint32_t length)
{
    unsigned j;

    for (j = 0; j < length; j++)
    {
        unsigned offset = j * 4;
        m_aText[offset] = ' ';
        m_aText[offset + 1] = ' ';
        const auto kPct = static_cast<uint16_t>((data[j] / 255) * 100);
        itoaBase10(kPct, &m_aText[offset]);
    }

    for (; j < kDmxFootprint; j++)
    {
        unsigned offset = j * 4;
        m_aText[offset] = ' ';
        m_aText[offset + 1] = ' ';
        m_aText[offset + 2] = ' ';
    }
}

void RDMSubDeviceBwLcd::itoaBase10(uint16_t arg, char buf[])
{
    char* n = buf + 2;

    if (arg == 0) *n = '0';

    while (arg != 0)
    {
        *n = static_cast<char>('0' + (arg % 10));
        n--;
        arg /= 10;
    }
}

void RDMSubDeviceBwLcd::UpdateEvent(TRDMSubDeviceUpdateEvent update_event)
{
    if (update_event == RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS)
    {
        DisplayChannels();

        for (uint32_t i = 0; i < bw::lcd::max_characters - 1; i++)
        { // Leave H, D, % at the end
            m_aText[i] = ' ';
        }

        m_BwSpiLcd.TextLine(1, m_aText, bw::lcd::max_characters - 1); // Leave H, D, % at the end
        return;
    }

    if (update_event == RDM_SUBDEVICE_UPDATE_EVENT_PERSONALITY)
    {
        DisplayUpdatePersonality();
        if (m_aText[2] != ' ')
        {
            switch (GetPersonalityCurrent())
            {
                case 1:
                    DataHex(data_, length_);
                    break;
                case 2:
                    DataDec(data_, length_);
                    break;
                case 3:
                    DataPct(data_, length_);
                    break;
                default:
                    break;
            }
        }

        m_BwSpiLcd.TextLine(1, m_aText, bw::lcd::max_characters);
    }
}

void RDMSubDeviceBwLcd::DisplayUpdatePersonality()
{
    const uint8_t kPersonalityCurrent = GetPersonalityCurrent();

    switch (kPersonalityCurrent)
    {
        case 1:
            m_aText[15] = 'H';
            break;
        case 2:
            m_aText[15] = 'D';
            break;
        case 3:
            m_aText[15] = '%';
            break;
        default:
            break;
    }
}
