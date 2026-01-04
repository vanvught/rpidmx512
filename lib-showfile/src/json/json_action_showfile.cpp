/**
 * @file json_action_showfile.cpp
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

#include "json/json_key.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "showfile.h"

static void SetPlayer(const char* val, uint32_t len)
{
    if (memcmp(val, "play", len) == 0)
    {
        ShowFile::Instance().Play();
        return;
    }

    if (memcmp(val, "stop", len) == 0)
    {
        ShowFile::Instance().Stop();
        return;
    }

    if (memcmp(val, "resume", len) == 0)
    {
        ShowFile::Instance().Resume();
        return;
    }

#if !defined(CONFIG_SHOWFILE_DISABLE_RECORD)
    if (memcmp(val, "record", len) == 0)
    {
        ShowFile::Instance().Record();
        return;
    }
#endif
}

static void SetLoop(const char* val, uint32_t len)
{
    if (len != 1) return;

    ShowFile::Instance().DoLoop(val[0] != '0');
}

static void SetShow(const char* val, uint32_t len)
{
	if (len > 2) return;

	ShowFile::Instance().SetPlayerShowFileCurrent(json::Atoi(val, len));
}

static constexpr json::SimpleKey kPlayer {
    "player",
    6,
    Fnv1a32("player", 6)
};

static constexpr json::SimpleKey kLoop {
    "loop",
    4,
    Fnv1a32("loop", 4)
};

static constexpr json::SimpleKey kShow {
    "show",
    4,
    Fnv1a32("show", 4)
};

static constexpr json::Key kActionKeys[] = {
	json::MakeKey(SetPlayer, kPlayer), 
	json::MakeKey(SetLoop, kLoop),
	json::MakeKey(SetShow, kShow)
};

namespace json::action {
void SetShowFile(const char* buffer, uint32_t buffer_size) {
    ParseJsonWithTable(buffer, buffer_size, kActionKeys);
}
}
