/**
 * @file ltcdisplayrgbpanel.cpp
 */
/*
 * Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cassert>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cstdio>

#include "ltcdisplayrgbpanel.h"
#include "ltc.h"
#include "rgbpanel.h"
 #include "firmware/debug/debug_debug.h"

static constexpr char kTypes[5][8 + 1] = {
    "Film 24 ", //
    "EBU 25  ", //
    "DF 29.97", //
    "SMPTE 30", //
    "----- --"  //
};

static constexpr char kSources[static_cast<uint32_t>(ltc::Source::UNDEFINED)][8 + 1] = {
    "LTC",      //
    "Art-Net",  //
    "Midi",     //
    "TCNet",    //
    "Internal", //
    "RtpMidi",  //
    "Systime",  //
    "ETC"       //
};

LtcDisplayRgbPanel::LtcDisplayRgbPanel()
{
    DEBUG_ENTRY();

    rgbpanel_ = new RgbPanel(64, 32);
    assert(rgbpanel_ != nullptr);

    DEBUG_EXIT();
}

LtcDisplayRgbPanel::~LtcDisplayRgbPanel()
{
    DEBUG_ENTRY();

    delete rgbpanel_;
    rgbpanel_ = nullptr;

    DEBUG_EXIT();
}

void LtcDisplayRgbPanel::Init()
{
    DEBUG_ENTRY();

    for (uint32_t i = 0; i < 4; i++)
    {
        memset(line_[i], ' ', 8);
        line_colours_[i].red = 0x00;
        line_colours_[i].green = 0x00;
        line_colours_[i].blue = 0x00;
    }

    rgbpanel_->Start();

    DEBUG_EXIT();
}

void LtcDisplayRgbPanel::Print()
{
    printf("RGB Panel\n");
}

void LtcDisplayRgbPanel::Show(const char* timecode, struct ltc::display::rgb::Colours& colours, struct ltc::display::rgb::Colours& colours_colons)
{
    rgbpanel_->SetColonsOff();
    rgbpanel_->SetColon(timecode[ltc::timecode::index::COLON_1], 1, 0, colours_colons.red, colours_colons.green, colours_colons.blue);
    rgbpanel_->SetColon(timecode[ltc::timecode::index::COLON_2], 3, 0, colours_colons.red, colours_colons.green, colours_colons.blue);
    rgbpanel_->SetColon(timecode[ltc::timecode::index::COLON_3], 5, 0, colours_colons.red, colours_colons.green, colours_colons.blue);

    const char kLine[8] = {timecode[0], timecode[1], timecode[3], timecode[4], timecode[6], timecode[7], timecode[9], timecode[10]};

    memcpy(line_[0], kLine, 8);

    line_colours_[0].red = colours.red;
    line_colours_[0].green = colours.green;
    line_colours_[0].blue = colours.blue;

    for (uint32_t i = 0; i < 4; i++)
    {
        rgbpanel_->TextLine(static_cast<uint8_t>(1 + i), line_[i], 8, line_colours_[i].red, line_colours_[i].green, line_colours_[i].blue);
    }

    rgbpanel_->Show();
}

void LtcDisplayRgbPanel::ShowSysTime(const char* systemtime, struct ltc::display::rgb::Colours& colours, struct ltc::display::rgb::Colours& colours_colons)
{
    rgbpanel_->SetColonsOff();
    rgbpanel_->SetColon(systemtime[ltc::systemtime::index::COLON_1], 2, 0, colours_colons.red, colours_colons.green, colours_colons.blue);
    rgbpanel_->SetColon(systemtime[ltc::systemtime::index::COLON_2], 4, 0, colours_colons.red, colours_colons.green, colours_colons.blue);

    const char kLine[] = {' ', systemtime[0], systemtime[1], systemtime[3], systemtime[4], systemtime[6], systemtime[7], ' '};

    memcpy(line_[0], kLine, 8);

    line_colours_[0].red = colours.red;
    line_colours_[0].green = colours.green;
    line_colours_[0].blue = colours.blue;

    rgbpanel_->TextLine(1, line_[0], 8, line_colours_[0].red, line_colours_[0].green, line_colours_[0].blue);
    rgbpanel_->ClearLine(2);
    rgbpanel_->TextLine(3, line_[2], 8, line_colours_[2].red, line_colours_[2].green, line_colours_[2].blue);
    rgbpanel_->TextLine(4, line_[3], 8, line_colours_[3].red, line_colours_[3].green, line_colours_[3].blue);

    rgbpanel_->Show();
}

void LtcDisplayRgbPanel::ShowMessage(const char* message, struct ltc::display::rgb::Colours& colours)
{
    rgbpanel_->SetColonsOff();
    rgbpanel_->TextLine(1, message, ltc::display::rgb::kMaxMessageSize, colours.red, colours.green, colours.blue);
    rgbpanel_->Show();
}

void LtcDisplayRgbPanel::ShowFPS(ltc::Type type, struct ltc::display::rgb::Colours& colours)
{
    memcpy(line_[1], kTypes[static_cast<uint32_t>(type)], 8);

    line_colours_[1].red = colours.red;
    line_colours_[1].green = colours.green;
    line_colours_[1].blue = colours.blue;
}

void LtcDisplayRgbPanel::ShowSource(ltc::Source source, struct ltc::display::rgb::Colours& colours)
{
    memcpy(line_[3], kSources[static_cast<uint32_t>(source)], 8);

    line_colours_[3].red = colours.red;
    line_colours_[3].green = colours.green;
    line_colours_[3].blue = colours.blue;
}

void LtcDisplayRgbPanel::ShowInfo(const char* info, uint32_t length, struct ltc::display::rgb::Colours& colours)
{
    length = std::min(static_cast<uint32_t>(8), length);
    uint32_t i;
    for (i = 0; i < length; i++)
    {
        line_[2][i] = info[i];
    }
    for (; i < 8; i++)
    {
        line_[2][i] = ' ';
    }

    line_colours_[2].red = colours.red;
    line_colours_[2].green = colours.green;
    line_colours_[2].blue = colours.blue;

    rgbpanel_->TextLine(3, line_[2], 8, colours.red, colours.green, colours.blue);
    rgbpanel_->TextLine(1, line_[0], 8, line_colours_[0].red, line_colours_[0].green, line_colours_[0].blue);
    rgbpanel_->Show();
}

void LtcDisplayRgbPanel::WriteChar([[maybe_unused]] uint8_t ch, [[maybe_unused]] uint8_t pos, [[maybe_unused]] struct ltc::display::rgb::Colours& colours)
{
    DEBUG_ENTRY();
    // TODO(avv): Implement WriteChar
    DEBUG_EXIT();
}
