/**
 * @file pixelpatterns.h
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
/**
 * Based on https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 */

#ifndef PIXELPATTERNS_H_
#define PIXELPATTERNS_H_

#include <cstdint>
#include <algorithm>

#include "pixel.h"
#include "pixelconfiguration.h"
#include "hal_millis.h"
 #include "firmware/debug/debug_debug.h"

namespace pixelpatterns
{
#if defined(PIXELPATTERNS_MULTI)
#if !defined(CONFIG_DMXNODE_PIXEL_MAX_PORTS)
#define CONFIG_DMXNODE_PIXEL_MAX_PORTS 8U
#endif
static constexpr uint32_t kMaxPorts = CONFIG_DMXNODE_PIXEL_MAX_PORTS;
#else
static constexpr uint32_t kMaxPorts = 1;
#endif

enum class Pattern
{
    kNone,
    kRainbowCycle,
    kTheaterChase,
    kColorWipe,
    kFade,
    kLast
};

inline constexpr char kPatternName[static_cast<uint32_t>(pixelpatterns::Pattern::kLast)][14] = {"None", "Rainbow cycle", "Theater chase", "Colour wipe",
                                                                                                "Fade"};

enum class Direction
{
    kForward,
    kReverse
};
} // namespace pixelpatterns

class PixelPatterns
{
   public:
    explicit PixelPatterns(uint32_t active_ports)
    {
        DEBUG_ENTRY();
        DEBUG_PRINTF("nActivePorts=%u", active_ports);

        active_ports = std::min(pixelpatterns::kMaxPorts, active_ports);

        DEBUG_EXIT();
    }

    ~PixelPatterns() = default;

    static const char* GetName(pixelpatterns::Pattern pattern)
    {
        if (pattern < pixelpatterns::Pattern::kLast)
        {
            return pixelpatterns::kPatternName[static_cast<uint32_t>(pattern)];
        }

        return "Unknown";
    }

    inline uint32_t GetActivePorts() const { return s_active_ports; }

    void RainbowCycle(uint32_t port_index, uint32_t interval, pixelpatterns::Direction direction = pixelpatterns::Direction::kForward)
    {
        Clear(port_index);

        s_port_config[port_index].active_pattern = pixelpatterns::Pattern::kRainbowCycle;
        s_port_config[port_index].interval = interval;
        s_port_config[port_index].total_steps = 255;
        s_port_config[port_index].pixel_index = 0;
        s_port_config[port_index].direction = direction;
    }

    void TheaterChase(uint32_t port_index, uint32_t colour1, uint32_t colour2, uint32_t interval,
                      pixelpatterns::Direction direction = pixelpatterns::Direction::kForward)
    {
        Clear(port_index);

        s_port_config[port_index].active_pattern = pixelpatterns::Pattern::kTheaterChase;
        s_port_config[port_index].interval = interval;
        s_port_config[port_index].total_steps = PixelConfiguration::Get().GetCount();
        s_port_config[port_index].colour1 = colour1;
        s_port_config[port_index].colour2 = colour2;
        s_port_config[port_index].pixel_index = 0;
        s_port_config[port_index].direction = direction;
    }

    void ColourWipe(uint32_t port_index, uint32_t colour, uint32_t interval, pixelpatterns::Direction direction = pixelpatterns::Direction::kForward)
    {
        Clear(port_index);

        s_port_config[port_index].active_pattern = pixelpatterns::Pattern::kColorWipe;
        s_port_config[port_index].interval = interval;
        s_port_config[port_index].total_steps = PixelConfiguration::Get().GetCount();
        s_port_config[port_index].colour1 = colour;
        s_port_config[port_index].pixel_index = 0;
        s_port_config[port_index].direction = direction;
    }

    void Fade(uint32_t port_index, uint32_t colour1, uint32_t colour2, uint32_t steps, uint32_t interval,
              pixelpatterns::Direction direction = pixelpatterns::Direction::kForward)
    {
        Clear(port_index);

        s_port_config[port_index].active_pattern = pixelpatterns::Pattern::kFade;
        s_port_config[port_index].interval = interval;
        s_port_config[port_index].total_steps = steps;
        s_port_config[port_index].colour1 = colour1;
        s_port_config[port_index].colour2 = colour2;
        s_port_config[port_index].pixel_index = 0;
        s_port_config[port_index].direction = direction;
    }

    void None(uint32_t port_index)
    {
        DEBUG_ENTRY();
        DEBUG_PRINTF("port_index=%u", port_index);

        Clear(port_index);

        s_port_config[port_index].active_pattern = pixelpatterns::Pattern::kNone;

        DEBUG_EXIT();
    }

    void Run()
    {
        if (pixel::IsUpdating())
        {
            return;
        }

        auto is_updated = false;
        const auto kMillis = hal::Millis();

        for (uint32_t i = 0; i < s_active_ports; i++)
        {
            is_updated |= PortUpdate(i, kMillis);
        }

        if (is_updated)
        {
            pixel::Update();
        }
    }

   private:
    void RainbowCycleUpdate(uint32_t port_index)
    {
        const auto kIndex = s_port_config[port_index].pixel_index;

        for (uint32_t i = 0; i < PixelConfiguration::Get().GetCount(); i++)
        {
            pixel::SetPixelColour(port_index, i, Wheel(((i * 256U / PixelConfiguration::Get().GetCount()) + kIndex) & 0xFF));
        }

        Increment(port_index);
    }

    void TheaterChaseUpdate(uint32_t port_index)
    {
        const auto kColour1 = s_port_config[port_index].colour1;
        const auto kColour2 = s_port_config[port_index].colour2;
        const auto kPixelIndex = s_port_config[port_index].pixel_index;

        for (uint32_t i = 0; i < PixelConfiguration::Get().GetCount(); i++)
        {
            if ((i + kPixelIndex) % 3 == 0)
            {
                pixel::SetPixelColour(port_index, i, kColour1);
            }
            else
            {
                pixel::SetPixelColour(port_index, i, kColour2);
            }
        }

        Increment(port_index);
    }

    void ColourWipeUpdate(uint32_t port_index)
    {
        const auto kColour1 = s_port_config[port_index].colour1;
        const auto kIndex = s_port_config[port_index].pixel_index;

        pixel::SetPixelColour(port_index, kIndex, kColour1);
        Increment(port_index);
    }

    void FadeUpdate(uint32_t port_index)
    {
        const auto& config = s_port_config[port_index];

        const pixel::PixelColours kColor1(config.colour1);
        const pixel::PixelColours kColor2(config.colour2);

        const auto kTotalSteps = config.total_steps;
        const auto kIndex = config.pixel_index;
        const auto kInvIndex = kTotalSteps - kIndex;

        const auto kInterp = [=](uint8_t a, uint8_t b) -> uint8_t { 
			return static_cast<uint8_t>((a * kInvIndex + b * kIndex) / kTotalSteps); 
		};

        const auto kR = kInterp(kColor1.Red(), kColor2.Red());
        const auto kG = kInterp(kColor1.Green(), kColor2.Green());
        const auto kB = kInterp(kColor1.Blue(), kColor2.Blue());

        pixel::SetPixelColour(port_index, pixel::GetColour(kR, kG, kB));

        Increment(port_index);
    }

    bool PortUpdate(uint32_t port_index, uint32_t millis)
    {
        if ((millis - s_port_config[port_index].last_update) < s_port_config[port_index].interval)
        {
            return false;
        }

        s_port_config[port_index].last_update = millis;

        switch (s_port_config[port_index].active_pattern)
        {
            case pixelpatterns::Pattern::kRainbowCycle:
                RainbowCycleUpdate(port_index);
                break;
            case pixelpatterns::Pattern::kTheaterChase:
                TheaterChaseUpdate(port_index);
                break;
            case pixelpatterns::Pattern::kColorWipe:
                ColourWipeUpdate(port_index);
                break;
            case pixelpatterns::Pattern::kFade:
                FadeUpdate(port_index);
                break;
            default:
                return false;
                break;
        }

        return true;
    }

    uint32_t Wheel(uint8_t wheel_position)
    {
        wheel_position = static_cast<uint8_t>(255U - wheel_position);

        if (wheel_position < 85)
        {
            return pixel::GetColour(static_cast<uint8_t>(255U - wheel_position * 3), 0, static_cast<uint8_t>(wheel_position * 3));
        }
        else if (wheel_position < 170U)
        {
            wheel_position = static_cast<uint8_t>(wheel_position - 85U);
            return pixel::GetColour(0, static_cast<uint8_t>(wheel_position * 3), static_cast<uint8_t>(255U - wheel_position * 3));
        }
        else
        {
            wheel_position = static_cast<uint8_t>(wheel_position - 170U);
            return pixel::GetColour(static_cast<uint8_t>(wheel_position * 3), static_cast<uint8_t>(255U - wheel_position * 3), 0);
        }
    }

    void Increment(uint32_t port_index)
    {
        if (s_port_config[port_index].direction == pixelpatterns::Direction::kForward)
        {
            s_port_config[port_index].pixel_index++;
            if (s_port_config[port_index].pixel_index == s_port_config[port_index].total_steps)
            {
                s_port_config[port_index].pixel_index = 0;
            }
        }
        else
        {
            if (s_port_config[port_index].pixel_index > 0)
            {
                s_port_config[port_index].pixel_index--;
            }
            if (s_port_config[port_index].pixel_index == 0)
            {
                s_port_config[port_index].pixel_index = s_port_config[port_index].total_steps - 1;
            }
        }
    }

    void Reverse(uint32_t port_index)
    {
        if (s_port_config[port_index].direction == pixelpatterns::Direction::kForward)
        {
            s_port_config[port_index].direction = pixelpatterns::Direction::kReverse;
            s_port_config[port_index].pixel_index = s_port_config[port_index].total_steps - 1;
        }
        else
        {
            s_port_config[port_index].direction = pixelpatterns::Direction::kForward;
            s_port_config[port_index].pixel_index = 0;
        }
    }

    uint32_t DimColour(uint32_t colour)
    {
        const pixel::PixelColours kC(colour);
        return pixel::GetColour(static_cast<uint8_t>(kC.Red() >> 1), static_cast<uint8_t>(kC.Green() >> 1), static_cast<uint8_t>(kC.Blue() >> 1));
    }

    void Clear(uint32_t port_index) { pixel::SetPixelColour(port_index, 0); }

   private:
    static inline uint32_t s_active_ports;

    struct PortConfig
    {
        uint32_t last_update;
        uint32_t interval;
        uint32_t colour1;
        uint32_t colour2;
        uint32_t total_steps;
        uint32_t pixel_index;
        pixelpatterns::Direction direction;
        pixelpatterns::Pattern active_pattern;
    };

    static inline PortConfig s_port_config[pixelpatterns::kMaxPorts];
};

#endif  // PIXELPATTERNS_H_
