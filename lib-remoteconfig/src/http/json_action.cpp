/**
 * @file json_action.cpp
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>

#include "display.h"
#include "board_statusled.h"
#include "board.h"
#include "json/json_key.h"
#include "json/json_parser.h"

static void SetDisplay(const char* val, uint32_t len) {
    if (len != 1) return;

    Display::Get()->SetSleep(val[0] == '0');
}

static void SetIdentify(const char* val, uint32_t len) {
    if (len != 1) return;

    if (val[0] != '0') {
        board::statusled::SetMode(board::statusled::Mode::kFast);
    } else {
        board::statusled::SetMode(board::statusled::Mode::kNormal);
    }
}

// TODO (a) Subject for deletion
static void SetReboot(const char* val, uint32_t len) {
    if (len != 1) return;
    if (val[0] != '0') board::Reboot();
}

static constexpr auto kDisplay = json::MakeSimpleKey("display");
static constexpr auto kIdentify = json::MakeSimpleKey("identify");
static constexpr auto kReboot = json::MakeSimpleKey("reboot");

static constexpr json::Key kActionKeys[] = {json::MakeKey(SetDisplay, kDisplay), json::MakeKey(SetIdentify, kIdentify), json::MakeKey(SetReboot, kReboot)};

namespace json::action {
void Set(const char* buffer, uint32_t buffer_size) {
    ParseJsonWithTable(buffer, buffer_size, kActionKeys);
}
} // namespace json::action
