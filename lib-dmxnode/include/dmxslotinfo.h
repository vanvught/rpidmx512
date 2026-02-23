/**
 * @file dmxslotinfo.h
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXSLOTINFO_H_
#define DMXSLOTINFO_H_

#include <cstdint>
#include <cstdio>
#include <cctype>
#include <cassert>

#include "dmxnode.h"
#include "common/utils/utils_hex.h"
 #include "firmware/debug/debug_debug.h"

class DmxSlotInfo
{
   public:
    DmxSlotInfo(dmxnode::SlotInfo* slot_info, uint32_t size) : slot_info_(slot_info), size_(size)
    {
        DEBUG_ENTRY();

        assert(slot_info != nullptr);
        assert(size != 0);

        for (uint32_t i = 0; i < size_; i++)
        {
            slot_info_[i].type = 0x00;       // ST_PRIMARY
            slot_info_[i].category = 0xFFFF; // SD_UNDEFINED
        }

        DEBUG_EXIT();
    }

    ~DmxSlotInfo()
    {
        DEBUG_ENTRY();

        if (slot_info_ != nullptr)
        {
            delete[] slot_info_;
            slot_info_ = nullptr;
        }

        if (to_string_ != nullptr)
        {
            delete[] to_string_;
            to_string_ = nullptr;
        }

        DEBUG_EXIT();
    }

    void FromString(const char* string, uint32_t& mask)
    {
        assert(string != nullptr);

        auto* slot_info_raw = const_cast<char*>(string);
        mask = 0;

        for (uint32_t i = 0; i < size_; i++)
        {
            auto is_set = false;
            dmxnode::SlotInfo slot_info;

            if (slot_info_raw == nullptr)
            {
                break;
            }

            slot_info_raw = Parse(slot_info_raw, is_set, slot_info);

            if (is_set)
            {
                slot_info_[i].type = slot_info.type;
                slot_info_[i].category = slot_info.category;
                mask |= (1U << i);
            }
        }

        DEBUG_PRINTF("mask=0x%x", static_cast<int>(mask));
    }

    const char* ToString(uint32_t mask)
    {
        if (to_string_ == nullptr)
        {
            to_string_ = new char[size_ * 7];
            assert(to_string_ != nullptr);

            to_string_[0] = '\0';
        }

        DEBUG_PRINTF("mask=0x%x", mask);

        if (mask == 0)
        {
            to_string_[0] = '\0';
            return to_string_;
        }

        auto* p = to_string_;

        for (uint32_t i = 0; i < size_; i++)
        {
            if ((mask & 0x1) == 0x1)
            {
                const auto kType = slot_info_[i].type;
                const auto kCategory = slot_info_[i].category;

                auto append_hex = [&](uint32_t nybble) 
                { 
					*p++ = common::hex::ToCharUppercase(nybble); 
				};

                append_hex((kType & 0xF0) >> 4);
                append_hex(kType & 0x0F);
                *p++ = ':';
                append_hex((kCategory & 0xF000) >> 12);
                append_hex((kCategory & 0x0F00) >> 8);
                append_hex((kCategory & 0x00F0) >> 4);
                append_hex(kCategory & 0x000F);
                *p++ = ',';
            }

            mask = mask >> 1;
        }

        p--;
        *p = '\0';

        assert(static_cast<uint32_t>(p - to_string_) <= (size_ * 7U));

        return to_string_;
    }

    void Dump()
    {
        for (uint32_t i = 0; i < size_; i++)
        {
            printf("  Slot:%u %.2X:%.4X\n", static_cast<unsigned int>(i), slot_info_[i].type, slot_info_[i].category);
        }
    }

   private:
    char* Parse(char* s, bool& is_valid, dmxnode::SlotInfo& slot_info)
    {
        assert(s != nullptr);

        auto* b = s;
        uint8_t i = 0;

        uint16_t tmp = 0;

        while ((i < 2) && (*b != ':'))
        {
            if (isxdigit(*b) == 0)
            {
                is_valid = false;
                return nullptr;
            }

            const auto kNibble = *b > '9' ? static_cast<uint8_t>((*b | 0x20) - 'a' + 10) : static_cast<uint8_t>(*b - '0');
            tmp = static_cast<uint16_t>((tmp << 4) | kNibble);
            b++;
            i++;
        }

        if ((i != 2) && (*b != ':'))
        {
            is_valid = false;
            return nullptr;
        }

        slot_info.type = static_cast<uint8_t>(tmp);

        i = 0;
        tmp = 0;

        b++;

        while ((i < 4) && (*b != ',') && (*b != '\0'))
        {
            if (isxdigit(*b) == 0)
            {
                is_valid = false;
                return nullptr;
            }

            const auto kNibble = *b > '9' ? static_cast<uint8_t>((*b | 0x20) - 'a' + 10) : static_cast<uint8_t>(*b - '0');
            tmp = static_cast<uint16_t>((tmp << 4) | kNibble);
            b++;
            i++;
        }

        if (i != 4)
        {
            is_valid = false;
            return nullptr;
        }

        if ((*b != ',') && (*b != ' ') && (*b != '\0'))
        {
            is_valid = false;
            return nullptr;
        }

        slot_info.category = tmp;

        is_valid = true;

        if (*b == '\0')
        {
            return nullptr;
        }

        return ++b;
    }

   private:
    dmxnode::SlotInfo* slot_info_;
    uint32_t size_;
    char* to_string_{nullptr};
};

#endif  // DMXSLOTINFO_H_
