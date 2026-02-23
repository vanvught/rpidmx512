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
#include "pixelpatterns.h"
#include "pixeltestpattern.h"
#include "pixeldmxconfiguration.h"
#include "display.h"
#include "displayudf.h"
 #include "firmware/debug/debug_debug.h"

class ArtNetTriggerHandler
{
   public:
    ArtNetTriggerHandler(DmxNodeWith4<CONFIG_DMXNODE_DMX_PORT_OFFSET>* pLightSet32with4, DmxPixelOutputType* pLightSetA)
        : dmxnode_(pLightSet32with4), outputtype_(pLightSetA)
    {
        assert(s_this == nullptr);
        s_this = this;

        ArtNetNode::Get()->SetArtTriggerCallbackFunctionPtr(StaticCallbackFunction);
    }

    ~ArtNetTriggerHandler() = default;

    void static StaticCallbackFunction(const ArtNetTrigger* trigger)
    {
        assert(s_this != nullptr);
        s_this->Handler(trigger);
    }

   private:
    void Handler(const ArtNetTrigger* trigger)
    {
        DEBUG_ENTRY();

        if (trigger->key == ArtTriggerKey::kArtTriggerKeyShow)
        {
            dmxnode_->SetDmxPixel(outputtype_);

            const auto kShow = static_cast<pixelpatterns::Pattern>(trigger->sub_key);

            DEBUG_PRINTF("nShow=%u", static_cast<uint32_t>(kShow));

            if (kShow == PixelTestPattern::Get()->GetPattern())
            {
                DEBUG_EXIT();
                return;
            }

            const auto kIsSet = PixelTestPattern::Get()->SetPattern(kShow);

            DEBUG_PRINTF("kIsSet=%u", static_cast<uint32_t>(kIsSet));

            if (!kIsSet)
            {
                DEBUG_EXIT();
                return;
            }

            if (static_cast<pixelpatterns::Pattern>(kShow) != pixelpatterns::Pattern::kNone)
            {
                dmxnode_->SetDmxPixel(nullptr);
                Display::Get()->ClearLine(6);
                Display::Get()->Printf(6, "%s:%u", PixelPatterns::GetName(kShow), static_cast<uint32_t>(kShow));
            }
            else
            {
                outputtype_->Blackout(true);
                DisplayUdf::Get()->Show();
            }

            DEBUG_EXIT();
            return;
        }

        if (trigger->key == ArtTriggerKey::kArtTriggerUndefined)
        {
            if (trigger->sub_key == 0)
            {
                const auto kIsSet = PixelTestPattern::Get()->SetPattern(pixelpatterns::Pattern::kNone);

                DEBUG_PRINTF("isSet=%u", static_cast<uint32_t>(kIsSet));

                if (!kIsSet)
                {
                    DEBUG_EXIT();
                    return;
                }

                dmxnode_->SetDmxPixel(nullptr);

                auto& pixelDmxConfiguration = PixelDmxConfiguration::Get();
                const auto* pData = &trigger->data[0];
                const uint32_t nColour =
                    pData[0] | (static_cast<uint32_t>(pData[1]) << 8) | (static_cast<uint32_t>(pData[2]) << 16) | (static_cast<uint32_t>(pData[3]) << 24);

                for (uint32_t nPort = 0; nPort < pixelDmxConfiguration.GetOutputPorts(); nPort++)
                {
                    pixel::SetPixelColour(nPort, nColour);
                }

                pixel::Update();
            }
        }
    }

   private:
    DmxNodeWith4<CONFIG_DMXNODE_DMX_PORT_OFFSET>* dmxnode_;
    DmxPixelOutputType* outputtype_;

    static inline ArtNetTriggerHandler* s_this;
};

#endif  // ARTNETTRIGGERHANDLER_H_
