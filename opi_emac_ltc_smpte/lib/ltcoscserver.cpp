/**
 * @file ltcoscserver.cpp
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

#if defined(DEBUG_LTCOSCSERVER)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>

#include "../include/ltcoscserver.h"
#if !defined(LTC_NO_DISPLAY_RGB)
#include "ltcdisplayrgb.h"
#endif
#include "arm/ltcmidisystemrealtime.h"
#include "arm/ltcgenerator.h"
#include "arm/ltcoutputs.h"
#include "arm/systimereader.h"
#include "tcnetdisplay.h"
#include "tcnet.h"
#include "osc.h"
#include "oscsimplemessage.h"
#include "firmware/debug/debug_debug.h"

namespace cmd {
static constexpr char kStart[] = "start";
static constexpr char kStop[] = "stop";
static constexpr char kResume[] = "resume";
static constexpr char kRate[] = "rate";
static constexpr char kSet[] = "/set/";
static constexpr char kGoto[] = "goto";
static constexpr char kDirection[] = "direction";
static constexpr char kPitch[] = "pitch";
static constexpr char kForward[] = "forward";
static constexpr char kBackward[] = "backward";
} // namespace cmd

namespace length {
static constexpr uint32_t kStart = sizeof(cmd::kStart) - 1;
static constexpr uint32_t kStop = sizeof(cmd::kStop) - 1;
static constexpr uint32_t kResume = sizeof(cmd::kResume) - 1;
static constexpr uint32_t kRate = sizeof(cmd::kRate) - 1;
static constexpr uint32_t kSet = sizeof(cmd::kSet) - 1;
static constexpr uint32_t kGoto = sizeof(cmd::kGoto) - 1;
static constexpr uint32_t kDirection = sizeof(cmd::kDirection) - 1;
static constexpr uint32_t kPitch = sizeof(cmd::kPitch) - 1;
static constexpr uint32_t kForward = sizeof(cmd::kForward) - 1;
static constexpr uint32_t kBackward = sizeof(cmd::kBackward) - 1;
} // namespace length

namespace tcnet {
namespace cmd {
static constexpr char kPath[] = "tcnet/";
static constexpr char kLayer[] = "layer/";
static constexpr char kType[] = "type";
static constexpr char kTimecode[] = "timecode";
} // namespace cmd
namespace length {
static constexpr auto kPath = sizeof(cmd::kPath) - 1;
static constexpr auto kLayer = sizeof(cmd::kLayer) - 1;
static constexpr auto kType = sizeof(cmd::kType) - 1;
static constexpr auto kTimecode = sizeof(cmd::kTimecode) - 1;
} // namespace length
} // namespace tcnet

namespace midi {
namespace cmd {
static constexpr char kPath[] = "midi/";
static constexpr char kStart[] = "start";
static constexpr char kStop[] = "stop";
static constexpr char kContinue[] = "continue";
static constexpr char kBpm[] = "bpm";
} // namespace cmd
namespace length {
static constexpr uint32_t kPath = sizeof(cmd::kPath) - 1;
static constexpr uint32_t kStart = sizeof(cmd::kStart) - 1;
static constexpr uint32_t kStop = sizeof(cmd::kStop) - 1;
static constexpr uint32_t kResume = sizeof(cmd::kContinue) - 1;
static constexpr uint32_t kBpm = sizeof(cmd::kBpm) - 1;
} // namespace length
} // namespace midi

namespace pixel {
namespace cmd {
static constexpr char kPath[] = "pixel/";
static constexpr char kMaster[] = "master";
static constexpr char kMessage[] = "message";
static constexpr char kInfo[] = "info";
} // namespace cmd
namespace length {
static constexpr uint32_t kPath = sizeof(cmd::kPath) - 1;
static constexpr uint32_t kMaster = sizeof(cmd::kMaster) - 1;
static constexpr uint32_t kMessage = sizeof(cmd::kMessage) - 1;
static constexpr uint32_t kInfo = sizeof(cmd::kInfo) - 1;
} // namespace length
namespace rgb {
namespace cmd {
static constexpr char kPath[] = "rgb/";
static constexpr char kTime[] = "time";
static constexpr char kColon[] = "colon";
static constexpr char kMessage[] = "message";
static constexpr char kFps[] = "fps";
static constexpr char kInfo[] = "info";
} // namespace cmd
namespace length {
static constexpr uint32_t kPath = sizeof(cmd::kPath) - 1;
static constexpr uint32_t kTime = sizeof(cmd::kTime) - 1;
static constexpr uint32_t kColon = sizeof(cmd::kColon) - 1;
static constexpr uint32_t kMessage = sizeof(cmd::kMessage) - 1;
static constexpr uint32_t kFps = sizeof(cmd::kFps) - 1;
static constexpr uint32_t kInfo = sizeof(cmd::kInfo) - 1;
} // namespace length
} // namespace rgb
} // namespace pixel

// "hh/mm/ss/ff" -> length = 11
static constexpr uint32_t kValueLength = 11;
static constexpr uint32_t kFpsValueLength = 2;

LtcOscServer::LtcOscServer() : port_incoming_(osc::port::kDefaultIncoming) {
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    path_length_ = static_cast<uint32_t>(snprintf(path_, sizeof(path_) - 1, "/%s/tc/*", network::iface::HostName()) - 1);

    DEBUG_PRINTF("%d [%s]", path_length_, path_);
    DEBUG_EXIT();
}

void LtcOscServer::Start() {
    DEBUG_ENTRY();

    assert(handle_ == -1);
    handle_ = network::udp::Begin(port_incoming_, StaticCallbackFunction);
    assert(handle_ != -1);

    DEBUG_EXIT();
}

void LtcOscServer::Print() {
    puts("OSC Server");
    printf(" Port : %u\n", port_incoming_);
    printf(" Path : [%s]\n", path_);
}

void LtcOscServer::Input(const uint8_t* data, uint32_t size, [[maybe_unused]] uint32_t from_ip, [[maybe_unused]] uint16_t from_port) {
    if (size <= 4) {
        return;
    }

    const auto* data_char = reinterpret_cast<const char*>(data);

    if (osc::IsMatch(data_char, path_)) {
        const auto kCommandLength = strlen(data_char);

        DEBUG_PRINTF("[%s]:%d %d:|%s|", data_char, static_cast<int>(kCommandLength), path_length_, &data_char[path_length_]);

        // */pitch f
        if (memcmp(&data_char[path_length_], cmd::kPitch, length::kPitch) == 0) {
            OscSimpleMessage msg(data, size);

            if (msg.GetType(0) != osc::type::kFloat) {
                return;
            }

            const auto kValue = msg.GetFloat(0);

            DEBUG_PRINTF("fValue=%f", kValue);

            LtcGenerator::Get()->ActionSetPitch(kValue);

            DEBUG_PUTS("ActionSetPitch");
            return;
        }
        // */start*
        if (memcmp(&data_char[path_length_], cmd::kStart, length::kStart) == 0) {
            if ((kCommandLength == (path_length_ + length::kStart))) {
                LtcGenerator::Get()->ActionStart();
                SystimeReader::Get()->ActionStart();

                DEBUG_PUTS("ActionStart");
            } else if ((kCommandLength == (path_length_ + length::kStart + 1 + kValueLength))) {
                if (data_char[path_length_ + length::kStart] == '/') {
                    const auto kOffset = path_length_ + length::kStart + 1;

                    char timecode[kValueLength];
                    memcpy(timecode, &data_char[kOffset], kValueLength);

                    timecode[2] = ':';
                    timecode[5] = ':';
                    timecode[8] = ':';

                    LtcGenerator::Get()->ActionSetStart(timecode);
                    LtcGenerator::Get()->ActionStop();
                    LtcGenerator::Get()->ActionStart();

                    DEBUG_PRINTF("%.*s", kValueLength, timecode);
                }
            } else if ((kCommandLength == (path_length_ + length::kStart + length::kSet + kValueLength))) {
                if (memcmp(&data_char[path_length_ + length::kStart], cmd::kSet, length::kSet) == 0) {
                    const auto kOffset = path_length_ + length::kStart + length::kSet;

                    char timecode[kValueLength];
                    memcpy(timecode, &data_char[kOffset], kValueLength);

                    timecode[2] = ':';
                    timecode[5] = ':';
                    timecode[8] = ':';

                    LtcGenerator::Get()->ActionSetStart(timecode);

                    DEBUG_PRINTF("%.*s", kValueLength, timecode);
                }
            }
            return;
        }
        // */stop*
        if (memcmp(&data_char[path_length_], cmd::kStop, length::kStop) == 0) {
            if ((kCommandLength == (path_length_ + length::kStop))) {
                LtcGenerator::Get()->ActionStop();
                SystimeReader::Get()->ActionStop();

                DEBUG_PUTS("ActionStop");
            } else if ((kCommandLength == (path_length_ + length::kStop + length::kSet + kValueLength))) {
                if (memcmp(&data_char[path_length_ + length::kStop], cmd::kSet, length::kSet) == 0) {
                    const auto kOffset = path_length_ + length::kStop + length::kSet;

                    char timecode[kValueLength];
                    memcpy(timecode, &data_char[kOffset], kValueLength);

                    timecode[2] = ':';
                    timecode[5] = ':';
                    timecode[8] = ':';

                    LtcGenerator::Get()->ActionSetStop(timecode);

                    DEBUG_PRINTF("%.*s", kValueLength, timecode);
                }
            }

            return;
        }
        // */forward i
        if (memcmp(&data_char[path_length_], cmd::kForward, length::kForward) == 0) {
            OscSimpleMessage msg(data, size);

            if (msg.GetType(0) != osc::type::kInt32) {
                return;
            }

            const auto kValue = msg.GetInt(0);

            if (kValue > 0) {
                LtcGenerator::Get()->ActionForward(static_cast<uint32_t>(kValue));
                DEBUG_PRINTF("ActionForward(%d)", kValue);
            }
            return;
        }
        // */backward i
        if (memcmp(&data_char[path_length_], cmd::kBackward, length::kBackward) == 0) {
            OscSimpleMessage msg(data, size);

            if (msg.GetType(0) != osc::type::kInt32) {
                return;
            }

            const auto kValue = msg.GetInt(0);

            if (kValue > 0) {
                LtcGenerator::Get()->ActionBackward(static_cast<uint32_t>(kValue));
                DEBUG_PRINTF("ActionBackward(%d)", kValue);
            }
            return;
        }
        // */set/*
        if ((kCommandLength == (path_length_ + length::kRate + length::kSet + kFpsValueLength))) {
            if (memcmp(&data_char[path_length_ + length::kRate], cmd::kSet, length::kSet) == 0) {
                const auto kOffset = path_length_ + length::kRate + length::kSet;

                LtcGenerator::Get()->ActionSetRate(&data_char[kOffset]);

                DEBUG_PUTS(&data_char[kOffset]);
                return;
            }
        }
        // */resume
        if ((kCommandLength == (path_length_ + length::kResume)) && (memcmp(&data_char[path_length_], cmd::kResume, length::kResume) == 0)) {
            LtcGenerator::Get()->ActionResume();

            DEBUG_PUTS("ActionResume");
            return;
        }
        // */goto/*
        if ((kCommandLength == (path_length_ + length::kGoto + 1 + kValueLength)) && (memcmp(&data_char[path_length_], cmd::kGoto, length::kGoto) == 0)) {
            if (data_char[path_length_ + length::kGoto] == '/') {
                const auto kOffset = path_length_ + length::kGoto + 1;

                char timecode[kValueLength];
                memcpy(timecode, &data_char[kOffset], kValueLength);

                timecode[2] = ':';
                timecode[5] = ':';
                timecode[8] = ':';

                LtcGenerator::Get()->ActionGoto(timecode);

                DEBUG_PUTS(timecode);
                return;
            }
        }
        // */direction/*
        if ((kCommandLength <= (path_length_ + length::kDirection + 1 + 8)) && (memcmp(&data_char[path_length_], cmd::kDirection, length::kDirection) == 0)) {
            if (data_char[path_length_ + length::kDirection] == '/') {
                const uint32_t kOffset = path_length_ + length::kDirection + 1;
                LtcGenerator::Get()->ActionSetDirection(&data_char[kOffset]);

                DEBUG_PUTS(&data_char[kOffset]);
                return;
            }
        }

        // */tcnet/
        if (memcmp(&data_char[path_length_], tcnet::cmd::kPath, tcnet::length::kPath) == 0) {
            // layer/?
            if (kCommandLength == (path_length_ + tcnet::length::kPath + tcnet::length::kLayer + 1)) {
                if (memcmp(&data_char[path_length_ + tcnet::length::kPath], tcnet::cmd::kLayer, tcnet::length::kLayer) == 0) {
                    const auto kOffset = path_length_ + tcnet::length::kPath + tcnet::length::kLayer;
                    const auto kLayer = tcnet::GetLayer(data_char[kOffset]);

                    TCNet::Get()->SetLayer(kLayer);
                    tcnet::display::show();

                    DEBUG_PRINTF("*/tcnet/layer/%c -> %d", data_char[kOffset], static_cast<int>(kLayer));
                    return;
                }
            }
            // type i
            if (kCommandLength == (path_length_ + tcnet::length::kPath + tcnet::length::kType)) {
                if (memcmp(&data_char[path_length_ + tcnet::length::kPath], tcnet::cmd::kType, tcnet::length::kType) == 0) {
                    OscSimpleMessage msg(data, size);

                    if (msg.GetType(0) != osc::type::kInt32) {
                        return;
                    }

                    const int kValue = msg.GetInt(0);

                    switch (kValue) {
                        case 24:
                            TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::kTimecodeTypeFilm);
                            tcnet::display::show();
                            break;
                        case 25:
                            TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::kTimecodeTypeEbu25Fps);
                            tcnet::display::show();
                            break;
                        case 29:
                            TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::kTimecodeTypeDf);
                            tcnet::display::show();
                            break;
                        case 30:
                            TCNet::Get()->SetTimeCodeType(tcnet::TimeCodeType::kTimecodeTypeSmpte30Fps);
                            tcnet::display::show();
                            break;
                        default:
                            break;
                    }

                    DEBUG_PRINTF("*/tcnet/type -> %d", kValue);
                    return;
                }
            }
            // timecode i
            if (kCommandLength == (path_length_ + tcnet::length::kPath + tcnet::length::kTimecode)) {
                if (memcmp(&data_char[path_length_ + tcnet::length::kPath], tcnet::cmd::kType, tcnet::length::kTimecode) == 0) {
                    OscSimpleMessage msg(data, size);

                    if (msg.GetType(0) != osc::type::kInt32) {
                        return;
                    }

                    const auto kValue = msg.GetInt(0);
                    const auto kUseTimecode = (kValue > 0);

                    TCNet::Get()->SetUseTimeCode(kUseTimecode);
                    tcnet::display::show();

                    DEBUG_PRINTF("*/tcnet/timecode -> %d", static_cast<int>(kUseTimecode));
                    return;
                }
            }
        }

        // */midi/
        if (memcmp(&data_char[path_length_], midi::cmd::kPath, midi::length::kPath) == 0) {
            // */start
            if ((kCommandLength == (path_length_ + midi::length::kPath + midi::length::kStart)) && (memcmp(&data_char[path_length_ + midi::length::kPath], midi::cmd::kStart, midi::length::kStart) == 0)) {
                LtcMidiSystemRealtime::Get()->SendStart();
                DEBUG_PUTS("MIDI Start");
                return;
            }
            // */stop
            if ((kCommandLength == (path_length_ + midi::length::kPath + midi::length::kStop)) && (memcmp(&data_char[path_length_ + midi::length::kPath], midi::cmd::kStop, midi::length::kStop) == 0)) {
                LtcMidiSystemRealtime::Get()->SendStop();
                DEBUG_PUTS("MIDI Stop");
                return;
            }
            // */continue
            if ((kCommandLength == (path_length_ + midi::length::kPath + midi::length::kResume)) && (memcmp(&data_char[path_length_ + midi::length::kPath], midi::cmd::kContinue, midi::length::kResume) == 0)) {
                LtcMidiSystemRealtime::Get()->SendContinue();
                DEBUG_PUTS("MIDI Continue");
                return;
            }
            // */bpm i or */bpm f
            if ((kCommandLength == (path_length_ + midi::length::kPath + midi::length::kBpm)) && (memcmp(&data_char[path_length_ + midi::length::kPath], midi::cmd::kBpm, midi::length::kBpm) == 0)) {
                uint32_t bpm = 0;

                OscSimpleMessage msg(data, size);

                if (msg.GetType(0) == osc::type::kFloat) {
                    const auto kValue = msg.GetFloat(0);
                    if (kValue > 0) {
                        bpm = static_cast<uint32_t>(kValue);
                    }
                } else if (msg.GetType(0) == osc::type::kInt32) {
                    const auto kValue = msg.GetInt(0);
                    if (kValue > 0) {
                        bpm = static_cast<uint32_t>(kValue);
                    }
                }

                LtcMidiSystemRealtime::Get()->SetBPM(bpm);
                LtcOutputs::Get()->ShowBPM(bpm);

                DEBUG_PRINTF("MIDI BPM: %u", bpm);
                return;
            }
        }
#if !defined(LTC_NO_DISPLAY_RGB)
        // */pixel/
        if (memcmp(&data_char[path_length_], pixel::cmd::kPath, pixel::length::kPath) == 0) {
            // pixel/master i
            if (kCommandLength == (path_length_ + pixel::length::kPath + pixel::length::kMaster)) {
                if (memcmp(&data_char[path_length_ + pixel::length::kPath], pixel::cmd::kMaster, pixel::length::kMaster) == 0) {
                    OscSimpleMessage msg(data, size);

                    if (msg.GetType(0) != osc::type::kInt32) {
                        return;
                    }

                    const auto kValue = static_cast<uint8_t>(msg.GetInt(0));

                    LtcDisplayRgb::Get()->SetMaster(kValue);

                    DEBUG_PRINTF("*/pixel/master -> %d", static_cast<int>(static_cast<uint8_t>(kValue)));
                    return;
                }
            }
            // pixel/message string
            if (kCommandLength == (path_length_ + pixel::length::kPath + pixel::length::kMessage)) {
                if (memcmp(&data_char[path_length_ + pixel::length::kPath], pixel::cmd::kMessage, pixel::length::kMessage) == 0) {
                    OscSimpleMessage msg(data, size);

                    if (msg.GetType(0) != osc::type::kString) {
                        return;
                    }

                    const auto* string = msg.GetString(0);
                    const auto kSize = strlen(string);

                    LtcDisplayRgb::Get()->SetMessage(string, kSize);

                    DEBUG_PRINTF("*/pixel/message -> [%.*s]", kSize, string);
                    return;
                }
            }
            // pixel/info string
            if (kCommandLength == (path_length_ + pixel::length::kPath + pixel::length::kInfo)) {
                if (memcmp(&data_char[path_length_ + pixel::length::kPath], pixel::cmd::kInfo, pixel::length::kInfo) == 0) {
                    OscSimpleMessage msg(data, size);

                    if (msg.GetType(0) != osc::type::kString) {
                        return;
                    }

                    const auto* string = msg.GetString(0);

                    LtcDisplayRgb::Get()->ShowInfo(string);

                    DEBUG_PRINTF("*/pixel/info -> [%.*s]", 8, string);
                    return;
                }
            }
            // pixel/rgb/*
            if (kCommandLength > (path_length_ + pixel::length::kPath + pixel::rgb::length::kPath)) {
                if (memcmp(&data_char[path_length_ + pixel::length::kPath], pixel::rgb::cmd::kPath, pixel::rgb::length::kPath) == 0) {
                    // pixel/rgb/time iii
                    if (kCommandLength == (path_length_ + pixel::length::kPath + pixel::rgb::length::kPath + pixel::rgb::length::kTime)) {
                        if (memcmp(&data_char[path_length_ + pixel::length::kPath + pixel::rgb::length::kPath], pixel::rgb::cmd::kTime, pixel::rgb::length::kTime) == 0) {
                            SetWS28xxRGB(data, size, ltc::display::rgb::ColourIndex::TIME);
                            return;
                        }
                    }
                    // pixel/rgb/colon iii
                    if (kCommandLength == (path_length_ + pixel::length::kPath + pixel::rgb::length::kPath + pixel::rgb::length::kColon)) {
                        if (memcmp(&data_char[path_length_ + pixel::length::kPath + pixel::rgb::length::kPath], pixel::rgb::cmd::kColon, pixel::rgb::length::kColon) == 0) {
                            SetWS28xxRGB(data, size, ltc::display::rgb::ColourIndex::COLON);
                            return;
                        }
                    }
                    // pixel/rgb/message iii
                    if (kCommandLength == (path_length_ + pixel::length::kPath + pixel::rgb::length::kPath + pixel::rgb::length::kMessage)) {
                        if (memcmp(&data_char[path_length_ + pixel::length::kPath + pixel::rgb::length::kPath], pixel::rgb::cmd::kMessage, pixel::rgb::length::kMessage) == 0) {
                            SetWS28xxRGB(data, size, ltc::display::rgb::ColourIndex::MESSAGE);
                            return;
                        }
                    }
                    // pixel/rgb/fps iii
                    if (kCommandLength == (path_length_ + pixel::length::kPath + pixel::rgb::length::kPath + pixel::rgb::length::kFps)) {
                        if (memcmp(&data_char[path_length_ + pixel::length::kPath + pixel::rgb::length::kPath], pixel::rgb::cmd::kFps, pixel::rgb::length::kFps) == 0) {
                            SetWS28xxRGB(data, size, ltc::display::rgb::ColourIndex::FPS);
                            return;
                        }
                    }
                    // pixel/rgb/info iii
                    if (kCommandLength == (path_length_ + pixel::length::kPath + pixel::rgb::length::kPath + pixel::rgb::length::kInfo)) {
                        if (memcmp(&data_char[path_length_ + pixel::length::kPath + pixel::rgb::length::kPath], pixel::rgb::cmd::kInfo, pixel::rgb::length::kInfo) == 0) {
                            SetWS28xxRGB(data, size, ltc::display::rgb::ColourIndex::INFO);
                            return;
                        }
                    }
                    return;
                }
            }
        }
#endif
    }
}

#if !defined(LTC_NO_DISPLAY_RGB)
void LtcOscServer::SetWS28xxRGB(const uint8_t* data, uint32_t size, ltc::display::rgb::ColourIndex index) {
    OscSimpleMessage msg(data, size);

    if (msg.GetArgc() == 3) {
        const auto kRed = static_cast<uint8_t>(msg.GetInt(0));
        const auto kGreen = static_cast<uint8_t>(msg.GetInt(1));
        const auto kBlue = static_cast<uint8_t>(msg.GetInt(2));

        LtcDisplayRgb::Get()->SetRGB(kRed, kGreen, kBlue, index);

        DEBUG_PRINTF("*/pixel/rgb/[%d] -> %d %d %d", static_cast<int>(index), static_cast<int>(kRed), static_cast<int>(kGreen), static_cast<int>(kBlue));
    } else {
        DEBUG_PUTS("Invalid pixel/rgb/*");
    }
}
#endif
