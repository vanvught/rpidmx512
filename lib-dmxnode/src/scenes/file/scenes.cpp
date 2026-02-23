/**
 * @file scenes.cpp
 *
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:infogd32-dmx.org
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

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cstdint>
#include <cstdio>
#include <cassert>
#include <unistd.h>

#include "dmxnode.h"
#include "firmware/debug/debug_debug.h"

namespace dmxnode::scenes
{

static constexpr char kFileName[] = "failsafe.bin";

static FILE* s_file;

void WriteStart()
{
    DEBUG_ENTRY();

    if ((s_file = fopen(kFileName, "r+")) == nullptr)
    {
        perror("fopen r+");

        if ((s_file = fopen(kFileName, "w+")) == nullptr)
        {
            perror("fopen w+");

            DEBUG_EXIT();
            return;
        }

        for (uint32_t i = 0; i < dmxnode::scenes::kBytesNeeded; i++)
        {
            if (fputc(0xFF, s_file) == EOF)
            {
                perror("fputc(0xFF, file)"); // Same as erasing a flash memory device

                if (fclose(s_file) != 0)
                {
                    perror("flcose");
                }

                s_file = nullptr;
                DEBUG_EXIT();
                return;
            }
        }

        if (fflush(s_file) != 0)
        {
            perror("fflush");
        }
    }

    DEBUG_EXIT();
}

void Write(uint32_t port_index, const uint8_t* data)
{
    DEBUG_ENTRY();
    assert(port_index < dmxnode::kMaxPorts);
    assert(data != nullptr);

    if (fseek(s_file, static_cast<long int>(port_index * dmxnode::kUniverseSize), SEEK_SET) != 0)
    {
        perror("fseek");
        DEBUG_EXIT();
        return;
    }

    if (fwrite(data, 1, dmxnode::kUniverseSize, s_file) != dmxnode::kUniverseSize)
    {
        perror("fwrite");
        DEBUG_EXIT();
        return;
    }

    DEBUG_EXIT();
}

void WriteEnd()
{
    DEBUG_ENTRY();

    if (s_file != nullptr)
    {
        if (fclose(s_file) != 0)
        {
            perror("flcose");
        }
        s_file = nullptr;
    }

    DEBUG_EXIT();
}

void ReadStart()
{
    DEBUG_ENTRY();

    if ((s_file = fopen(kFileName, "r")) == nullptr)
    {
        perror("fopen r");
        DEBUG_EXIT();
    }

    DEBUG_EXIT();
}

void Read(uint32_t port_index, uint8_t* data)
{
    DEBUG_ENTRY();
    assert(port_index < dmxnode::kMaxPorts);
    assert(data != nullptr);

    if (s_file == nullptr)
    {
        DEBUG_EXIT();
        return;
    }

    if (fseek(s_file, static_cast<long int>(port_index * dmxnode::kUniverseSize), SEEK_SET) != 0)
    {
        perror("fseek");
        DEBUG_EXIT();
        return;
    }

    if (fread(data, 1, dmxnode::kUniverseSize, s_file) != dmxnode::kUniverseSize)
    {
        perror("fread");
        DEBUG_EXIT();
        return;
    }

    DEBUG_EXIT();
}

void ReadEnd()
{
    DEBUG_ENTRY();

    if (s_file != nullptr)
    {
        if (fclose(s_file) != 0)
        {
            perror("flcose");
        }
        s_file = nullptr;
    }

    DEBUG_EXIT();
}
} // namespace dmxnode::scenes
