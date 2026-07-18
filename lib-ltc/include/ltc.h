/**
 * @file ltc.h
 *
 */
/* Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LTC_H_
#define LTC_H_

#include <cstdint>
#include <bit>
#include <time.h>
#include <cstring>
#include <cassert>


namespace ltc {
enum class Source : uint8_t {
    LTC,       //
    ARTNET,    //
    MIDI,      //
    TCNET,     //
    INTERNAL,  //
    APPLEMIDI, //
    SYSTIME,   //
    ETC,       //
    UNDEFINED  //
};

inline constexpr uint32_t kSourceMaxNameLength = 9; // Included '\0'

inline constexpr const char kSource[static_cast<uint32_t>(ltc::Source::UNDEFINED)][kSourceMaxNameLength] = {
    "ltc",      //
    "artnet",   //
    "midi",     //
    "tcnet",    //
    "internal", //
    "rtp-midi", //
    "systime",  //
    "etc"       //
};

[[nodiscard]] inline constexpr const char* GetSourceType(ltc::Source source) {
    if (source < ltc::Source::UNDEFINED) {
        return kSource[static_cast<uint32_t>(source)];
    }

    return "Undefined";
}

inline Source GetSourceType(const char* string) {
    assert(string != nullptr);
    uint8_t index = 0;

    for (const char (&source)[kSourceMaxNameLength] : kSource) {
        if (strcasecmp(string, source) == 0) {
            return static_cast<Source>(index);
        }
        ++index;
    }

    return ltc::Source::LTC;
}

enum class Type : uint8_t {
    FILM,         //
    EBU,          //
    DF,           //
    SMPTE,        //
    UNKNOWN,      //
    INVALID = 255 //
};

extern Type g_Type;

struct TimeCode {
    uint8_t frames;  ///< Frames time. 0 – 29 depending on mode.
    uint8_t seconds; ///< Seconds. 0 - 59.
    uint8_t minutes; ///< Minutes. 0 - 59.
    uint8_t hours;   ///< Hours. 0 - 59.
    uint8_t type;    ///< 0 = Film (24fps) , 1 = EBU (25fps), 2 = DF (29.97fps), 3 = SMPTE (30fps)
} __attribute__((packed));

struct DisabledOutputs {
    bool bOled;
    bool bMax7219;
    bool bMidi;
    bool bArtNet;
    bool bLtc;
    bool bEtc;
    bool bNtp;
    bool bRtpMidi;
    bool bWS28xx;
    bool bRgbPanel;
};

extern uint32_t g_disabled_outputs;

struct Destination {
    enum class Output : uint32_t {
        DISPLAY_OLED = (1U << 0),
        MAX7219 = (1U << 1),
        MIDI = (1U << 2),
        ARTNET = (1U << 3),
        LTC = (1U << 4),
        ETC = (1U << 5),
        NTP_SERVER = (1U << 6),
        RTPMIDI = (1U << 7),
        WS28XX = (1U << 8),
        RGBPANEL = (1U << 9)
    };

    static constexpr char kOutputString[][14] = {
		"Display OLED", 
		"Max7219",    
		"DIN-MIDI", 
		"Art-Net",     
		"LTC",                                 
		"ETC",          
		"NTP Server", 
		"RTP-MIDI", 
		"PixelOutput", 
		"RGB panel"
	};

    static constexpr const char* GetOutputString(Output output) {
        const auto kIndex = std::countr_zero(static_cast<uint32_t>(output));
        return kOutputString[kIndex];
    }

    static void SetDisabled(Output output, bool disable = true) {
        if (disable) {
            g_disabled_outputs |= static_cast<uint32_t>(output);
        }
    }

    static bool IsDisabled(Output output) { return (g_disabled_outputs & static_cast<uint32_t>(output)) == static_cast<uint32_t>(output); }
    static bool IsEnabled(Output output) { return !IsDisabled(output); }
};

namespace systemtime::index {
inline constexpr auto HOURS = 0;
inline constexpr auto HOURS_TENS = 0;
inline constexpr auto HOURS_UNITS = 1;
inline constexpr auto COLON_1 = 2;
inline constexpr auto MINUTES = 3;
inline constexpr auto MINUTES_TENS = 3;
inline constexpr auto MINUTES_UNITS = 4;
inline constexpr auto COLON_2 = 5;
inline constexpr auto SECONDS = 6;
inline constexpr auto SECONDS_TENS = 6;
inline constexpr auto SECONDS_UNITS = 7;
} // namespace systemtime::index

namespace timecode {
inline constexpr auto CODE_MAX_LENGTH = 11;
inline constexpr auto TYPE_MAX_LENGTH = 11;
inline constexpr auto RATE_MAX_LENGTH = 2;
inline constexpr auto SYSTIME_MAX_LENGTH = CODE_MAX_LENGTH;

namespace index {
inline constexpr auto HOURS = 0;
inline constexpr auto HOURS_TENS = 0;
inline constexpr auto HOURS_UNITS = 1;
inline constexpr auto COLON_1 = 2;
inline constexpr auto MINUTES = 3;
inline constexpr auto MINUTES_TENS = 3;
inline constexpr auto MINUTES_UNITS = 4;
inline constexpr auto COLON_2 = 5;
inline constexpr auto SECONDS = 6;
inline constexpr auto SECONDS_TENS = 6;
inline constexpr auto SECONDS_UNITS = 7;
inline constexpr auto COLON_3 = 8;
inline constexpr auto FRAMES = 9;
inline constexpr auto FRAMES_TENS = 9;
inline constexpr auto FRAMES_UNITS = 10;
} // namespace index
} // namespace timecode

const char* GetType();
const char* GetType(ltc::Type type);
ltc::Type GetType(uint8_t fps);
void SetType(uint8_t fps);

void InitTimecode(char* timecode);
void InitSystemtime(char* systemtime);

void ItoaBase10(const struct ltc::TimeCode* ltc_timecode, char* timecode);
void ItoaBase10(const struct tm* localtime, char* systemtime);

bool ParseTimecode(const char* timecode, uint8_t fps, struct ltc::TimeCode* ltc_timecode);
bool ParseTimecodeRate(const char* timecode_rate, uint8_t& fps);
} // namespace ltc

extern volatile uint32_t gv_ltc_updates_per_second;
extern volatile uint32_t gv_ltc_updates_previous;
extern volatile uint32_t gv_ltc_updates;
extern volatile bool gv_ltc_timecode_available;
extern volatile uint32_t gv_ltc_timecode_counter;
extern struct ltc::TimeCode g_ltc_timecode;

#if defined(H3)
#define PLATFORM_LTC_ARM
#include "arm/h3/h3_platform_ltc.h" // IWYU pragma: keep
#elif defined(GD32)
#define PLATFORM_LTC_ARM
#include "arm/gd32/gd32_platform_ltc.h" // IWYU pragma: keep
#endif

#endif // LTC_H_
