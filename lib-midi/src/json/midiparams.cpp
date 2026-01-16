/**
 * @file midiparams.cpp
 *
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

#include "midi.h"
#include "json/midiparams.h"
#include "json/json_parsehelper.h"
#include "json/midiparamsconst.h"
#include "json/json_parser.h"
#include "configstore.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"

using common::store::midi::Flags;

namespace json
{
MidiParams::MidiParams()
{
    ConfigStore::Instance().Copy(&store_midi, &ConfigurationStore::midi);
}

void MidiParams::SetBaudrate(const char* val, uint32_t len)
{
    const auto kValue = ParseValue<uint32_t>(val, len);

    if (kValue == 0)
    {
        store_midi.baudrate = midi::defaults::BAUDRATE;
    }
    else
    {
        store_midi.baudrate = kValue;
    }
}

void MidiParams::SetActiveSense(const char* val, uint32_t len)
{
    if (len != 1) return;

    store_midi.flags = common::SetFlagValue(store_midi.flags, Flags::Flag::kActiveSense, val[0] != '0');
}

void MidiParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kMidiKeys);
    ConfigStore::Instance().Store(&store_midi, &ConfigurationStore::midi);

#ifndef NDEBUG
    Dump();
#endif
}

void MidiParams::Set()
{
#ifndef NDEBUG
    Dump();
#endif
}

void MidiParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::MidiParamsConst::kFileName);
    printf(" %s=%s [%u]\n", MidiParamsConst::kBaudrate.name, store_midi.baudrate);
    printf(" %s=%u\n", MidiParamsConst::kActiveSense.name, common::IsFlagSet(store_midi.flags, Flags::Flag::kActiveSense));
}
} // namespace json