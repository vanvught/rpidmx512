/**
 * @file osc.h
 *
 */
/* Copyright (C) 2016-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef OSC_H_
#define OSC_H_

#include <cstdint>

#include "oscstring.h"

namespace osc {
namespace type {
/* basic OSC types */
/* 32 bit signed integer. */
inline constexpr char kInt32 = 'i';
/* 32 bit IEEE-754 float. */
inline constexpr char kFloat = 'f';
/* Standard C, NULL terminated string. */
inline constexpr char kString = 's';
/* OSC binary blob type. */
inline constexpr char kBlob = 'b';
/* extended OSC types */
/* 64 bit signed integer. */
inline constexpr char kInt64 = 'h';
/* OSC TimeTag type. */
inline constexpr char kTimetag = 't';
/* 64 bit IEEE-754 double. */
inline constexpr char kDouble = 'd';
/* Standard C, NULL terminated, string. Used in systems which distinguish strings and symbols. */
inline constexpr char kSymbol = 'S';
/* Standard C, 8 bit, char variable. */
inline constexpr char kChar = 'c';
/* A 4 byte MIDI packet. */
inline constexpr char kMidi = 'm';
/* Symbol representing the value True. */
inline constexpr char kTrue = 'T';
/* Symbol representing the value False. */
inline constexpr char kFalse = 'F';
/* Symbol representing the value Nil. */
inline constexpr char kNil = 'N';
/* Symbol representing the value Infinitum. */
inline constexpr char kInfinitum = 'I';
/* */
inline constexpr char kUnknown = '\0';
} // namespace type
namespace port {
inline constexpr uint16_t kDefaultIncoming = 8000;
inline constexpr uint16_t kDefaultOutgoing = 9000;
} // namespace port
} // namespace osc

extern "C" {
int lo_pattern_match(const char*, const char*); // NOLINT
}

#include "oscstring.h"

namespace osc {
inline char* GetPath(void* path, unsigned size) {
    return (StringValidate(path, size) >= 4) ? reinterpret_cast<char*>(path) : nullptr;
}

inline bool IsMatch(const char* str, const char* p) {
    return lo_pattern_match(str, p) == 0 ? false : true;
}
} // namespace osc

#endif // OSC_H_
