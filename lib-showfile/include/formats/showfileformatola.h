/**
 * @file showfileformatola.h
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef FORMATS_SHOWFILEFORMATOLA_H_
#define FORMATS_SHOWFILEFORMATOLA_H_

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O2")
#endif

#include <cstdio>
#include <cassert>

#include "showfileprotocol.h"
 #include "firmware/debug/debug_debug.h"

#define SHOWFILE_PREFIX "show"
#define SHOWFILE_SUFFIX ".txt"

namespace showfile
{
inline constexpr uint32_t kFileNameLength = sizeof(SHOWFILE_PREFIX "NN" SHOWFILE_SUFFIX) - 1U;
inline constexpr int32_t kFileMaxNumber = 99;
} // namespace showfile

class ShowFileFormat : ShowFileProtocol
{
   public:
    ShowFileFormat()
    {
        DEBUG_ENTRY();

        assert(s_this == nullptr);
        s_this = this;

        for (uint32_t index = 0; index < sizeof(m_digitsTable); index = index + 2)
        {
            const auto kValue = index / 2;
            m_digitsTable[index] = '0' + static_cast<char>(kValue / 10U);
            m_digitsTable[index + 1] = '0' + static_cast<char>(kValue % 10U);
        }

        ShowFileProtocol::Start();

        DEBUG_EXIT();
    }

    void ShowFileStart()
    {
        DEBUG_ENTRY();

        m_nDelayMillis = 0;
        m_nLastMillis = 0;

        fseek(m_pShowFile, 0L, SEEK_SET);

        m_OlaState = OlaState::IDLE;

        DEBUG_EXIT();
    }

    void ShowFileStop()
    {
        DEBUG_ENTRY();

        DEBUG_EXIT();
    }

    void ShowFileResume()
    {
        DEBUG_ENTRY();

        m_nDelayMillis = 0;
        m_nLastMillis = 0;

        DEBUG_EXIT();
    }

    void ShowFileRecord()
    {
        DEBUG_ENTRY();
        DEBUG_PRINTF("m_pShowFile%snullptr", m_pShowFile != nullptr ? "!=" : "==");

        if (m_pShowFile != nullptr)
        {
            fputs("OLA Show\n", m_pShowFile);
#ifndef NDEBUG
            perror("fputs");
#endif
            m_OlaState = OlaState::RECORD_FIRST;
        }
        else
        {
            m_OlaState = OlaState::IDLE;
        }

        ShowFileProtocol::Record();

        DEBUG_EXIT();
    }

    void ShowFilePrint()
    {
        puts(" Format: OLA");
        ShowFileProtocol::Print();
    }

    void ShowFileRun(const bool doRun)
    {
        if (doRun)
        {
            Run();
        }

        ShowFileProtocol::Run();
    }

    void DoRunCleanupProcess(const bool bDoRun) { ShowFileProtocol::DoRunCleanupProcess(bDoRun); }

    void ShowfileWrite(const uint8_t* pDmxData, const uint32_t nSize, const uint32_t nUniverse, const uint32_t millis)
    {
        if (m_OlaState == OlaState::RECORD_FIRST)
        {
            m_OlaState = OlaState::RECORDING;
        }
        else
        {
            auto* p = m_buffer;
            p += FastItoaUniverse(millis - m_nLastMillis, p);
            *p++ = '\n';
            *p = '\0';
            fputs(m_buffer, m_pShowFile);
#ifndef NDEBUG
            perror("fputs");
            printf("[%s]", m_buffer);
#endif
        }

        m_nLastMillis = millis;

        auto* p = m_buffer;

        p += FastItoaUniverse(nUniverse & 0xFFFF, p);
        *p++ = ' ';

        for (uint32_t nIndex = 0; nIndex < nSize; nIndex++)
        {
            p += fast_itoa_dmx(pDmxData[nIndex] & 0xFF, p);
            *p++ = ',';
        }

        *--p = '\n';
        *++p = '\0';

        fputs(m_buffer, m_pShowFile);
#ifndef NDEBUG
        perror("fputs");
        printf("[%s]", m_buffer);
#endif
    }

    void BlackOut()
    {
#if defined(CONFIG_SHOWFILE_ENABLE_MASTER)
        ShowFileProtocol::DmxBlackout();
#endif
    }

    void SetMaster([[maybe_unused]] const uint32_t nMaster)
    {
#if defined(CONFIG_SHOWFILE_ENABLE_MASTER)
        ShowFileProtocol::DmxMaster(nMaster);
#endif
    }

    bool IsSyncDisabled() { return ShowFileProtocol::IsSyncDisabled(); }

    static ShowFileFormat* Get() { return s_this; }

   private:
    void Run();
    /*
     * Using a lookup table to convert binary numbers from 0 to 99
     * into ascii characters as described by Andrei Alexandrescu in
     * https://www.facebook.com/notes/facebook-engineering/three-optimization-tips-for-c/10151361643253920/
     */
    uint32_t FastItoaUniverse(uint32_t universe, char* pDestination)
    {
        uint32_t n = 0;

        if (universe >= 10000)
        {
            const auto kIndex = (universe % 100) * 2;
            pDestination[3] = m_digitsTable[kIndex];
            pDestination[4] = m_digitsTable[kIndex + 1];
            universe /= 100;
            n = 5;
        }
        else if (universe >= 1000)
        {
            const auto kIndex = (universe % 100) * 2;
            pDestination[2] = m_digitsTable[kIndex];
            pDestination[3] = m_digitsTable[kIndex + 1];
            universe /= 100;
            n = 4;
        }

        if (universe >= 100)
        {
            const auto kIndex = (universe % 100) * 2;
            pDestination[1] = m_digitsTable[kIndex];
            pDestination[2] = m_digitsTable[kIndex + 1];
            universe /= 100;
            if (n == 0)
            {
                n = 2;
            }
        }

        if (universe < 10)
        {
            pDestination[0] = '0' + static_cast<char>(universe);
            n++;
        }
        else
        {
            const auto kIndex = universe * 2;
            pDestination[0] = m_digitsTable[kIndex];
            pDestination[1] = m_digitsTable[kIndex + 1];
            if (n == 0)
            {
                n = 2;
            }
        }

        return n;
    }

    uint32_t fast_itoa_dmx(uint32_t nDmxValue, char* pDestination)
    {
        uint32_t n = 0;

        if (nDmxValue >= 100)
        {
            const auto nIndex = (nDmxValue % 100) * 2;
            pDestination[1] = m_digitsTable[nIndex];
            pDestination[2] = m_digitsTable[nIndex + 1];
            nDmxValue /= 100;
            n = 2;
        }

        if (nDmxValue < 10)
        {
            pDestination[0] = '0' + static_cast<char>(nDmxValue);
            n++;
        }
        else
        {
            const auto nIndex = nDmxValue * 2;
            pDestination[0] = m_digitsTable[nIndex];
            pDestination[1] = m_digitsTable[nIndex + 1];
            n = 2;
        }

        return n;
    }

#define UINT_DIGITS 12
    char* ItoaWithLinefeed(uint32_t i)
    {
        m_buffer[UINT_DIGITS + 1] = '\n';
        m_buffer[UINT_DIGITS + 2] = '\0';
        char* p = m_buffer + UINT_DIGITS;
        do
        {
            *--p = '0' + static_cast<char>(i % 10);
            i /= 10;
        } while (i != 0);
        return p;
    }

   protected:
    int32_t show_file_current_{showfile::kFileMaxNumber + 1};
    bool m_bDoLoop{false};
    FILE* m_pShowFile{nullptr};

   private:
    enum class OlaState
    {
        IDLE,
        PARSING_DMX,
        TIME_WAITING,
        RECORD_FIRST,
        RECORDING
    };

    enum class OlaParseCode
    {
        FAILED,
        TIME,
        DMX,
        NONE,
        EOFILE
    };

    OlaParseCode GetNextLine();
    OlaParseCode ParseLine(const char* pLine);
    OlaParseCode ParseDmxData(const char* pLine);

   private:
    OlaParseCode m_OlaParseCode{OlaParseCode::FAILED};
    OlaState m_OlaState{OlaState::IDLE};
    char m_buffer[2048];
    char m_digitsTable[200];
    uint32_t m_nDelayMillis{0};
    uint32_t m_nLastMillis{0};
    uint32_t m_nDmxDataLength{0};
    uint16_t universe_{0};
    uint8_t dmx_data_[512];

    static ShowFileFormat* s_this;
};

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif

#endif /* FORMATS_SHOWFILEFORMATOLA_H_ */
