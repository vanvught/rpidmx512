/**
 * @file rdmtod.h
 *
 */
/* Copyright (C) 2017-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMTOD_H_
#define RDMTOD_H_

#include <cstdint>
#include <cstring>
#include <cassert>
#ifndef NDEBUG
#include <cstdio>
#endif

#include "rdmconst.h"
#include "firmware/debug/debug_debug.h"

namespace rdm
{
class Tod
{
   public:
#if !defined(RDM_DISCOVERY_TOD_TABLE_SIZE)
#define RDM_DISCOVERY_TOD_TABLE_SIZE 200U
#endif
    static constexpr uint32_t kTableSize = RDM_DISCOVERY_TOD_TABLE_SIZE;
    static constexpr uint32_t kMutesTableSize = (kTableSize + 32) / 32;
    static constexpr uint32_t kInvalidEntry = static_cast<uint32_t>(~0);

    Tod()
    {
        for (uint32_t i = 0; i < kTableSize; i++)
        {
            memcpy(&tod_[i], UID_ALL, RDM_UID_SIZE);
        }

        for (uint32_t i = 0; i < kMutesTableSize; i++)
        {
            mutes_[i] = 0;
        }
    }

    ~Tod() = default;

    void Reset()
    {
        for (uint32_t i = 0; i < entries_; i++)
        {
            memcpy(&tod_[i], UID_ALL, RDM_UID_SIZE);
        }

        entries_ = 0;

        for (uint32_t i = 0; i < kMutesTableSize; i++)
        {
            mutes_[i] = 0;
        }
    }

    bool AddUid(const uint8_t* uid)
    {
        if (entries_ == kTableSize)
        {
            return false;
        }

        if (Exist(uid))
        {
            return false;
        }

        memcpy(&tod_[entries_++], uid, RDM_UID_SIZE);

        return true;
    }

    uint32_t UidCount() const { return entries_; }

    bool CopyUidEntry(uint32_t index, uint8_t uid[RDM_UID_SIZE])
    {
        if (index > entries_)
        {
            memcpy(uid, UID_ALL, RDM_UID_SIZE);
            return false;
        }

        memcpy(uid, &tod_[index], RDM_UID_SIZE);
        return true;
    }

    void Copy(uint8_t* table)
    {
        DEBUG_ENTRY();
        DEBUG_PRINTF("entries_=%u", static_cast<unsigned int>(entries_));
        assert(table != nullptr);

        const auto* src = reinterpret_cast<const uint8_t*>(tod_);
        auto* dst = table;

        for (uint32_t i = 0; i < (entries_ * RDM_UID_SIZE); i++)
        {
            *dst++ = *src++;
        }

        DEBUG_EXIT();
    }

    bool Delete(const uint8_t* uid)
    {
        bool found = false;
        uint32_t i;

        for (i = 0; i < entries_; i++)
        {
            if (memcmp(&tod_[i], uid, RDM_UID_SIZE) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return false;
        }

        if (i == kTableSize - 1)
        {
            memcpy(&tod_[i], UID_ALL, RDM_UID_SIZE);
        }
        else
        {
            for (; i < entries_; i++)
            {
                memcpy(&tod_[i], &tod_[i + 1], RDM_UID_SIZE);
            }
        }

        entries_--;

        return true;
    }

    bool Exist(const uint8_t* uid)
    {
        for (uint32_t index = 0; index < entries_; index++)
        {
            if (memcmp(&tod_[index], uid, RDM_UID_SIZE) == 0)
            {
                saved_index_ = index;
                return true;
            }
        }

        saved_index_ = kInvalidEntry;
        return false;
    }

    const uint8_t* Next()
    {
        saved_index_++;

        if (saved_index_ == entries_)
        {
            saved_index_ = 0;
        }

        return tod_[saved_index_].uid;
    }

    void Mute()
    {
        if (saved_index_ == kInvalidEntry)
        {
            return;
        }

        const auto kI = saved_index_ / 32;
        const auto kShift = saved_index_ - (kI * 32);

        mutes_[kI] |= (1U << kShift);
    }

    void UnMute()
    {
        if (saved_index_ == kInvalidEntry)
        {
            return;
        }

        const auto kI = saved_index_ / 32;
        const auto kShift = saved_index_ - (kI * 32);

        mutes_[kI] &= ~(1U << kShift);
    }

    void UnMuteAll()
    {
        for (uint32_t i = 0; i < kMutesTableSize; i++)
        {
            mutes_[i] = 0;
        }
    }

    bool IsMuted()
    {
        if (saved_index_ == kInvalidEntry)
        {
            return true;
        }

        const auto kI = saved_index_ / 32;
        const auto kMutes = mutes_[kI];
        const auto kShift = saved_index_ - (kI * 32);

        return (kMutes & (1U << kShift)) == (1U << kShift);
    }

    void Dump([[maybe_unused]] uint32_t count)
    {
#ifndef NDEBUG
        if (count > kTableSize)
        {
            count = kTableSize;
        }

        printf("[%u]\n", static_cast<unsigned int>(count));
        for (uint32_t i = 0; i < count; i++)
        {
            printf("%.2x%.2x:%.2x%.2x%.2x%.2x\n", tod_[i].uid[0], tod_[i].uid[1], tod_[i].uid[2], tod_[i].uid[3], tod_[i].uid[4], tod_[i].uid[5]);
        }
#endif
    }

    void Dump()
    {
#ifndef NDEBUG
        Dump(entries_);
#endif
    }

   private:
    uint32_t entries_{0};
    uint32_t saved_index_{kInvalidEntry};
    uint32_t mutes_[kMutesTableSize];
    struct Uid
    {
        uint8_t uid[RDM_UID_SIZE];
    };
    Uid tod_[kTableSize];
};
} // namespace rdm

#endif // RDMTOD_H_
