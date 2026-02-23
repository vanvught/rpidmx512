/**
 * @file ltcdisplayrgb.cpp
 */
/*
 * Copyright (C) 2019-2020 by hippy mailto:dmxout@gmail.com
 * Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_LTCDISPLAYRGB)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <cassert>

#include "ltcdisplayrgb.h"
#include "ltc.h"
#include "hal_millis.h"
#include "network.h"
#include "ltcdisplaypixel7segment.h"
#include "ltcdisplaypixelmatrix.h"
#include "ltcdisplayrgbpanel.h"
#if !defined(CONFIG_LTC_DISABLE_WS28XX)
#include "pixeltype.h"
#endif
#include "softwaretimers.h"
 #include "firmware/debug/debug_debug.h"

namespace rgb
{
static constexpr char kPath[] = "rgb";
static constexpr auto kLength = sizeof(kPath) - 1;
static constexpr auto kHexSize = 7; // 1 byte index followed by 6 bytes hex RGB
} // namespace rgb
namespace master
{
static constexpr char kPath[] = "master";
static constexpr auto kLength = sizeof(kPath) - 1;
static constexpr auto kHexSize = 2;
} // namespace master
namespace showmsg
{
static constexpr char kPath[] = "showmsg";
static constexpr auto kLength = sizeof(kPath) - 1;
} // namespace showmsg
namespace udp
{
static constexpr uint16_t kPort = 0x2812;
}

static TimerHandle_t s_timer_id = kTimerIdNone;
static bool s_show_msg;

static void MessageTimer([[maybe_unused]] TimerHandle_t handle)
{
    s_show_msg = false;
    SoftwareTimerDelete(s_timer_id);
}

LtcDisplayRgb::LtcDisplayRgb(ltc::display::rgb::Type type, ltc::display::rgb::WS28xxType ws28xx_type) : type_(type), ws28xx_type_(ws28xx_type)
{
    DEBUG_ENTRY();
    assert(s_this == nullptr);
    s_this = this;

    colour_[static_cast<uint32_t>(ltc::display::rgb::ColourIndex::TIME)] = ltc::display::rgb::Defaults::kColourTime;
    colour_[static_cast<uint32_t>(ltc::display::rgb::ColourIndex::COLON)] = ltc::display::rgb::Defaults::COLOUR_COLON;
    colour_[static_cast<uint32_t>(ltc::display::rgb::ColourIndex::MESSAGE)] = ltc::display::rgb::Defaults::COLOUR_MESSAGE;
    colour_[static_cast<uint32_t>(ltc::display::rgb::ColourIndex::FPS)] = ltc::display::rgb::Defaults::COLOUR_FPS;
    colour_[static_cast<uint32_t>(ltc::display::rgb::ColourIndex::INFO)] = ltc::display::rgb::Defaults::COLOUR_INFO;
    colour_[static_cast<uint32_t>(ltc::display::rgb::ColourIndex::SOURCE)] = ltc::display::rgb::Defaults::COLOUR_SOURCE;

    DEBUG_EXIT();
}

LtcDisplayRgb::~LtcDisplayRgb()
{
    DEBUG_ENTRY();

    assert(display_rgb_ != nullptr);
    delete display_rgb_;
    display_rgb_ = nullptr;

    DEBUG_EXIT();
}

void LtcDisplayRgb::Init(pixel::Type type)
{
    DEBUG_ENTRY();

    pixel_type_ = type;

    if (type_ == ltc::display::rgb::Type::kRgbpanel)
    {
        display_rgb_ = new LtcDisplayRgbPanel;

        assert(display_rgb_ != nullptr);
        display_rgb_->Init();
    }
    else
    {
        if (ws28xx_type_ == ltc::display::rgb::WS28xxType::SEGMENT)
        {
            display_rgb_ = new LtcDisplayPixel7Segment(type, pixel_map_);
        }
        else
        {
            display_rgb_ = new LtcDisplayPixelMatrix(type, pixel_map_);
        }

        assert(display_rgb_ != nullptr);
    }

    for (uint32_t index = 0; index < static_cast<uint32_t>(ltc::display::rgb::ColourIndex::LAST); index++)
    {
        SetRGB(colour_[index], static_cast<ltc::display::rgb::ColourIndex>(index));
    }

    assert(handle_ == -1);
    handle_ = network::udp::Begin(udp::kPort, StaticCallbackFunction);
    assert(handle_ != -1);

    DEBUG_EXIT();
}

void LtcDisplayRgb::Show(const char* timecode)
{
    if (display_rgb_ == nullptr)
    {
        return;
    }

    if (__builtin_expect((s_show_msg), 0))
    {
        ShowMessage();
        return;
    }

    struct ltc::display::rgb::Colours colours_colons;

    if (colon_blink_mode_ != ltc::display::rgb::ColonBlinkMode::OFF)
    {
        const uint32_t kMillis = hal::Millis();

        if (seconds_previous_ != timecode[ltc::timecode::index::SECONDS_UNITS])
        { // seconds have changed
            seconds_previous_ = timecode[ltc::timecode::index::SECONDS_UNITS];
            colon_blink_millis_ = kMillis;

            colours_colons.red = 0;
            colours_colons.green = 0;
            colours_colons.blue = 0;
        }
        else if (kMillis - colon_blink_millis_ < 1000)
        {
            uint32_t master;

            if (colon_blink_mode_ == ltc::display::rgb::ColonBlinkMode::DOWN)
            {
                master = 255 - ((kMillis - colon_blink_millis_) * 255 / 1000);
            }
            else
            {
                master = ((kMillis - colon_blink_millis_) * 255 / 1000);
            }

            if (!(master_ == 0 || master_ == 255))
            {
                master = (master_ * master) / 255;
            }

            colours_colons.red = static_cast<uint8_t>((master * colours_colons_.red) / 255);
            colours_colons.green = static_cast<uint8_t>((master * colours_colons_.green) / 255);
            colours_colons.blue = static_cast<uint8_t>((master * colours_colons_.blue) / 255);
        }
        else
        {
            if (!(master_ == 0 || master_ == 255))
            {
                colours_colons.red = static_cast<uint8_t>((master_ * colours_colons_.red) / 255);
                colours_colons.green = static_cast<uint8_t>((master_ * colours_colons_.green) / 255);
                colours_colons.blue = static_cast<uint8_t>((master_ * colours_colons_.blue) / 255);
            }
            else
            {
                colours_colons.red = colours_colons_.red;
                colours_colons.green = colours_colons_.green;
                colours_colons.blue = colours_colons_.blue;
            }
        }
    }
    else
    {
        if (!(master_ == 0 || master_ == 255))
        {
            colours_colons.red = static_cast<uint8_t>((master_ * colours_colons_.red) / 255);
            colours_colons.green = static_cast<uint8_t>((master_ * colours_colons_.green) / 255);
            colours_colons.blue = static_cast<uint8_t>((master_ * colours_colons_.blue) / 255);
        }
        else
        {
            colours_colons.red = colours_colons_.red;
            colours_colons.green = colours_colons_.green;
            colours_colons.blue = colours_colons_.blue;
        }
    }

    struct ltc::display::rgb::Colours colours;

    if (!(master_ == 0 || master_ == 255))
    {
        colours.red = static_cast<uint8_t>((master_ * colours_time_.red) / 255);
        colours.green = static_cast<uint8_t>((master_ * colours_time_.green) / 255);
        colours.blue = static_cast<uint8_t>((master_ * colours_time_.blue) / 255);
    }
    else
    {
        colours.red = colours_time_.red;
        colours.green = colours_time_.green;
        colours.blue = colours_time_.blue;
    }

    display_rgb_->Show(timecode, colours, colours_colons);
}

void LtcDisplayRgb::ShowSysTime(const char* systemtime)
{
    if (display_rgb_ == nullptr)
    {
        return;
    }

    if (__builtin_expect((s_show_msg), 0))
    {
        ShowMessage();
        return;
    }

    struct ltc::display::rgb::Colours colours;
    struct ltc::display::rgb::Colours colours_colons;

    if (!(master_ == 0 || master_ == 255))
    {
        colours.red = static_cast<uint8_t>((master_ * colours_time_.red) / 255);
        colours.green = static_cast<uint8_t>((master_ * colours_time_.green) / 255);
        colours.blue = static_cast<uint8_t>((master_ * colours_time_.blue) / 255);
        //
        colours_colons.red = static_cast<uint8_t>((master_ * colours_colons_.red) / 255);
        colours_colons.green = static_cast<uint8_t>((master_ * colours_colons_.green) / 255);
        colours_colons.blue = static_cast<uint8_t>((master_ * colours_colons_.blue) / 255);
    }
    else
    {
        colours.red = colours_time_.red;
        colours.green = colours_time_.green;
        colours.blue = colours_time_.blue;
        //
        colours_colons.red = colours_colons_.red;
        colours_colons.green = colours_colons_.green;
        colours_colons.blue = colours_colons_.blue;
    }

    display_rgb_->ShowSysTime(systemtime, colours, colours_colons);
}

void LtcDisplayRgb::SetMessage(const char* message, uint32_t size)
{
    assert(message != nullptr);

    uint32_t i;
    const char* src = message;
    char* dst = message_;

    for (i = 0; i < std::min(size, static_cast<uint32_t>(sizeof(message_))); i++)
    {
        *dst++ = *src++;
    }

    for (; i < sizeof(message_); i++)
    {
        *dst++ = ' ';
    }

    message_timer_ = hal::Millis();

    if (s_timer_id == kTimerIdNone)
    {
        s_timer_id = SoftwareTimerAdd(kMessageTimeMs, MessageTimer);
    }
    else
    {
        SoftwareTimerChange(s_timer_id, kMessageTimeMs);
    }

    s_show_msg = true;
}

void LtcDisplayRgb::ShowMessage()
{
    if (display_rgb_ == nullptr)
    {
        return;
    }

    struct ltc::display::rgb::Colours colours;

    const auto kMillis = hal::Millis();

    colours.red = static_cast<uint8_t>(((kMillis - message_timer_) * colours_message_.red) / kMessageTimeMs);
    colours.green = static_cast<uint8_t>(((kMillis - message_timer_) * colours_message_.green) / kMessageTimeMs);
    colours.blue = static_cast<uint8_t>(((kMillis - message_timer_) * colours_message_.blue) / kMessageTimeMs);

    display_rgb_->ShowMessage(message_, colours);
}

void LtcDisplayRgb::ShowFPS(ltc::Type type)
{
    if (display_rgb_ == nullptr)
    {
        return;
    }

    display_rgb_->ShowFPS(type, colours_fps_);
}

void LtcDisplayRgb::ShowInfo(const char* info)
{
    if (display_rgb_ == nullptr)
    {
        return;
    }

    display_rgb_->ShowInfo(info, static_cast<uint16_t>(strlen(info)), colours_info_);
}

void LtcDisplayRgb::ShowSource(ltc::Source source)
{
    if (display_rgb_ == nullptr)
    {
        return;
    }

    display_rgb_->ShowSource(source, colours_source_);
}

void LtcDisplayRgb::WriteChar(uint8_t ch, uint8_t pos)
{
    if (display_rgb_ == nullptr)
    {
        return;
    }

    display_rgb_->WriteChar(ch, pos, colours_info_);
}

/**
 * @brief Processes an incoming UDP packet.
 *
 * @param pBuffer Pointer to the packet buffer.
 * @param nSize Size of the packet buffer.
 * @param from_ip IP address of the sender.
 * @param from_port Port number of the sender.
 */
void LtcDisplayRgb::Input(const uint8_t* buffer, uint32_t size, [[maybe_unused]] uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
{
    if (__builtin_expect((memcmp("7seg!", buffer, 5) != 0), 0))
    {
        return;
    }

    if (buffer[size - 1] == '\n')
    {
        DEBUG_PUTS("\'\\n\'");
        size--;
    }

    if (memcmp(&buffer[5], showmsg::kPath, showmsg::kLength) == 0)
    {
        const uint32_t kMsgLength = size - (5 + showmsg::kLength + 1);
        DEBUG_PRINTF("size=%d, nMsgLength=%d [%.*s]", size, kMsgLength, kMsgLength, &buffer[(5 + showmsg::kLength + 1)]);

        if (((kMsgLength > 0) && (kMsgLength <= ltc::display::rgb::kMaxMessageSize)) && (buffer[5 + showmsg::kLength] == '#'))
        {
            SetMessage(reinterpret_cast<const char*>(&buffer[(5 + showmsg::kLength + 1)]), kMsgLength);
            return;
        }

        DEBUG_PUTS("Invalid !showmsg command");
        return;
    }

    if (memcmp(&buffer[5], rgb::kPath, rgb::kLength) == 0)
    {
        if ((size == (5 + rgb::kLength + 1 + rgb::kHexSize)) && (buffer[5 + rgb::kLength] == '#'))
        {
            SetRGB(reinterpret_cast<const char*>(&buffer[(5 + rgb::kLength + 1)]));
            return;
        }

        DEBUG_PUTS("Invalid !rgb command");
        return;
    }

    if (memcmp(&buffer[5], master::kPath, master::kLength) == 0)
    {
        if ((size == (5 + master::kLength + 1 + master::kHexSize)) && (buffer[5 + master::kLength] == '#'))
        {
            master_ = HexadecimalToDecimal(reinterpret_cast<const char*>(&buffer[(5 + master::kLength + 1)]), master::kHexSize);
            return;
        }

        DEBUG_PUTS("Invalid !master command");
        return;
    }

    DEBUG_PUTS("Invalid command");
}

void LtcDisplayRgb::Print()
{
    if (type_ == ltc::display::rgb::Type::kRgbpanel)
    {
#if !defined(CONFIG_LTC_DISABLE_RGB_PANEL)
        puts("Display RGB panel");
#else
        puts("Display RGB panel disabled");
#endif
    }
    else
    {
#if !defined(CONFIG_LTC_DISABLE_WS28XX)
        puts("Display PixelOutput");
        printf(" Type    : %s [%d]\n", pixel::GetType(pixel_type_), static_cast<int>(pixel_type_));
        printf(" Mapping : %s [%d]\n", pixel::GetMap(pixel_map_), static_cast<int>(pixel_map_));
#else
        puts("Display PixelOutput disabled");
#endif
    }

    printf(" Master  : %d\n", master_);
    printf(" RGB     : Character 0x%.6X, Colon 0x%.6X, Message 0x%.6X\n", colour_[static_cast<uint32_t>(ltc::display::rgb::ColourIndex::TIME)],
           colour_[static_cast<uint32_t>(ltc::display::rgb::ColourIndex::COLON)], colour_[static_cast<uint32_t>(ltc::display::rgb::ColourIndex::MESSAGE)]);

    if (display_rgb_ == nullptr)
    {
        puts(" No Init()!");
    }
    else
    {
        display_rgb_->Print();
    }
}
