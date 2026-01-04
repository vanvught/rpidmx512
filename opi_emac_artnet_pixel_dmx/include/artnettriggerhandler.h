/**
 * @file artnettriggerhandler.h
 *
 */
/* Copyright (C) 2024-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef ARTNETTRIGGERHANDLER_H_
#define ARTNETTRIGGERHANDLER_H_

#include <cassert>

#include "artnettrigger.h"
#include "artnetnode.h"
#include "dmxnode_outputtype.h"
#include "dmxnodewith4.h"
#include "pixeltestpattern.h"
#include "display.h"
#include "displayudf.h"

class ArtNetTriggerHandler
{
   public:
    ArtNetTriggerHandler(DmxNodeWith4<CONFIG_DMXNODE_DMX_PORT_OFFSET>* pDmxNodeWith4, DmxPixelOutputType* pDmxPixelOutputType)
        : m_pDmxNodeWith4(pDmxNodeWith4), m_pDmxPixelOutputType(pDmxPixelOutputType)
    {
        assert(s_this == nullptr);
        s_this = this;

        ArtNetNode::Get()->SetArtTriggerCallbackFunctionPtr(StaticCallbackFunction);
    }

    ~ArtNetTriggerHandler() = default;

    void static StaticCallbackFunction(const ArtNetTrigger* pArtNetTrigger)
    {
        assert(s_this != nullptr);
        s_this->Handler(pArtNetTrigger);
    }

   private:
    void Handler(const ArtNetTrigger* pArtNetTrigger)
    {
        if (pArtNetTrigger->key == ArtTriggerKey::kArtTriggerKeyShow)
        {
            m_pDmxNodeWith4->SetDmxPixel(m_pDmxPixelOutputType);

            const auto nShow = static_cast<pixelpatterns::Pattern>(pArtNetTrigger->sub_key);

            if (nShow == PixelTestPattern::Get()->GetPattern())
            {
                return;
            }

            const auto isSet = PixelTestPattern::Get()->SetPattern(nShow);

            if (!isSet)
            {
                return;
            }

            if (static_cast<pixelpatterns::Pattern>(nShow) != pixelpatterns::Pattern::kNone)
            {
                m_pDmxNodeWith4->SetDmxPixel(nullptr);
                Display::Get()->ClearLine(6);
                Display::Get()->Printf(6, "%s:%u", PixelPatterns::GetName(nShow), static_cast<uint32_t>(nShow));
            }
            else
            {
                m_pDmxPixelOutputType->Blackout(true);
                DisplayUdf::Get()->Show();
            }

            return;
        }

        if (pArtNetTrigger->key == ArtTriggerKey::kArtTriggerUndefined)
        {
            if (pArtNetTrigger->sub_key == 0)
            {
                const auto kIsSet = PixelTestPattern::Get()->SetPattern(pixelpatterns::Pattern::kNone);

                if (!kIsSet)
                {
                    return;
                }

                m_pDmxNodeWith4->SetDmxPixel(nullptr);

                const auto* pData = &pArtNetTrigger->data[0];
                const uint32_t nColour =
                    pData[0] | (static_cast<uint32_t>(pData[1]) << 8) | (static_cast<uint32_t>(pData[2]) << 16) | (static_cast<uint32_t>(pData[3]) << 24);

                pixel::SetPixelColour(0, nColour);
                pixel::Update();
            }
        }
    }

   private:
    DmxNodeWith4<CONFIG_DMXNODE_DMX_PORT_OFFSET>* m_pDmxNodeWith4;
    DmxPixelOutputType* m_pDmxPixelOutputType;

    static inline ArtNetTriggerHandler* s_this;
};

#endif  // ARTNETTRIGGERHANDLER_H_
