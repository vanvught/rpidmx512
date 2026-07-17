/**
 * @file showfileparamsconst.h
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef JSON_SHOWFILEPARAMSCONST_H_
#define JSON_SHOWFILEPARAMSCONST_H_

#include "json/json_key.h"

namespace json {
struct ShowFileParamsConst {
    static constexpr char kFileName[] = "showfile.json";

    static constexpr auto kShow = json::MakeSimpleKey("show");
    static constexpr auto kOptionAutoPlay = json::MakeSimpleKey("auto_play");
    static constexpr auto kOptionLoop = json::MakeSimpleKey("loop");
    static constexpr auto kOptionSacnSyncUniverse = json::MakeSimpleKey("sync_universe");
    static constexpr auto kOptionArtnetDisableUnicast = json::MakeSimpleKey("disable_unicast");
};
} // namespace json

#endif // JSON_SHOWFILEPARAMSCONST_H_
