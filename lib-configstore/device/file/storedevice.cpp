/**
 * @file storedevice.cpp
 *
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <cassert>
#include <unistd.h>

#include "configstoredevice.h"
 #include "firmware/debug/debug_debug.h"

static constexpr auto kFlashSectorSize = 4096U;
static constexpr auto kFlashSize = (512 * kFlashSectorSize);
static constexpr char kFlashFileName[] = "spiflash.bin";
static constexpr auto kBlockWriteLength = 1024;

enum class State
{
    kIdle,
    kRunning,
    kError
};

static FILE* s_file;
static State s_state = State::kIdle;
static uint32_t s_index = 0;

StoreDevice::StoreDevice()
{
    DEBUG_ENTRY();

    if ((s_file = fopen(kFlashFileName, "r+")) == nullptr)
    {
        perror("fopen r+");

        s_file = fopen(kFlashFileName, "w+");

        for (uint32_t i = 0; i < kFlashSize; i++)
        {
            if (fputc(0xFF, s_file) == EOF)
            {
                perror("fputc(0xFF, file)");
                DEBUG_EXIT();
                return;
            }
        }

        if (fflush(s_file) != 0)
        {
            perror("fflush");
        }
    }

    detected_ = true;

    DEBUG_EXIT();
}

StoreDevice::~StoreDevice()
{
    DEBUG_ENTRY();

    if (s_file != nullptr)
    {
        fclose(s_file);
        s_file = nullptr;
    }

    DEBUG_EXIT();
}

uint32_t StoreDevice::GetSize() const
{
    return kFlashSize;
}

uint32_t StoreDevice::GetSectorSize() const
{
    return kFlashSectorSize;
}

bool StoreDevice::Read(uint32_t offset, uint32_t length, uint8_t* buffer, storedevice::Result& result)
{
    assert(s_file != nullptr);
    DEBUG_ENTRY();
    DEBUG_PRINTF("offset=%u, length=%u", offset, length);

    if (fseek(s_file, static_cast<long int>(offset), SEEK_SET) != 0)
    {
        result = storedevice::Result::kError;
        perror("fseek");
        DEBUG_EXIT();
        return true;
    }

    if (fread(buffer, 1, length, s_file) != length)
    {
        result = storedevice::Result::kError;
        perror("fread");
        DEBUG_EXIT();
        return true;
    }

    DEBUG_EXIT();
    result = storedevice::Result::kOk;
    return true;
}

bool StoreDevice::Erase(uint32_t offset, uint32_t length, storedevice::Result& result)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("offset=%u, length=%u, s_State=%d", offset, length, static_cast<int>(s_state));
    assert(s_state != State::kError);
    assert(s_file != nullptr);

    if (s_state == State::kIdle)
    {
        if (offset % kFlashSectorSize || length % kFlashSectorSize)
        {
            s_state = State::kError;
            result = storedevice::Result::kError;
            DEBUG_PUTS("Erase offset/length not multiple of erase size");
            DEBUG_EXIT();
            return true;
        }

        if (fseek(s_file, static_cast<long int>(offset), SEEK_SET) != 0)
        {
            s_state = State::kError;
            result = storedevice::Result::kError;
            perror("fseek");
            DEBUG_EXIT();
            return true;
        }

        s_index = 0;
        s_state = State::kRunning;
        result = storedevice::Result::kOk;
        DEBUG_EXIT();
        return false;
    }

    if (s_state == State::kRunning)
    {
        for (uint32_t i = 0; i < length; i++)
        {
            if (fputc(0xFF, s_file) == EOF)
            {
                s_state = State::kError;
                result = storedevice::Result::kError;
                perror("fputc(0xFF, file)");
                DEBUG_EXIT();
                return true;
            }
        }

        s_state = State::kIdle;
        result = storedevice::Result::kOk;

        if (fflush(s_file) != 0)
        {
            perror("fflush");
        }

        sync();
        DEBUG_EXIT();
        return true;
    }

    DEBUG_EXIT();
    assert(0);
    return true;
}

bool StoreDevice::Write(uint32_t offset, uint32_t length, const uint8_t* buffer, storedevice::Result& result)
{
    DEBUG_ENTRY();
    DEBUG_PRINTF("s_State=%u, offset=%u, length=%u [%u]", static_cast<unsigned int>(s_state), offset, length, s_index);
    assert(s_state != State::kError);
    assert(s_file != nullptr);

    if (s_state == State::kIdle)
    {
        if (fseek(s_file, static_cast<long int>(offset), SEEK_SET) != 0)
        {
            s_state = State::kError;
            result = storedevice::Result::kError;
            perror("fseek");
            DEBUG_EXIT();
            return true;
        }

        s_index = 0;
        s_state = State::kRunning;
        result = storedevice::Result::kOk;
        DEBUG_EXIT();
        return false;
    }
    else if (s_state == State::kRunning)
    {
        uint32_t block_write_length = length - s_index;

        if (block_write_length > kBlockWriteLength)
        {
            block_write_length = kBlockWriteLength;
        }

        if (fwrite(&buffer[s_index], 1, block_write_length, s_file) != block_write_length)
        {
            s_state = State::kError;
            result = storedevice::Result::kError;
            perror("fwrite");
            DEBUG_EXIT();
            return true;
        }

        s_index += block_write_length;

        if (s_index == length)
        {
            s_state = State::kIdle;

            if (fflush(s_file) != 0)
            {
                perror("fflush");
            }

            DEBUG_EXIT();
            return true;
        }

        result = storedevice::Result::kOk;
        DEBUG_EXIT();
        return false;
    }

    DEBUG_EXIT();
    return true;
}
