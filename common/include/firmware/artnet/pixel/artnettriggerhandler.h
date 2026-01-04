/**
 * @file artnettriggerhandler.h
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

#ifndef COMMON_FIRMWARE_ARTNET_PIXEL_ARTNETTRIGGERHANDLER_H_
#define COMMON_FIRMWARE_ARTNET_PIXEL_ARTNETTRIGGERHANDLER_H_

#include <cassert>

#include "artnettrigger.h"
#include "artnetnode.h"
#include "dmxnode_outputtype.h"
#include "pixelpatterns.h"
#include "pixeltestpattern.h"
#include "pixeldmxconfiguration.h"
#include "displayudf.h"

class ArtNetTriggerHandler : ArtNetTrigger
{
   public:
    explicit ArtNetTriggerHandler(DmxNodeOutputType* output_type) : dmxnode_output_type_(output_type)
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
        if (trigger->key == ArtTriggerKey::kArtTriggerKeyShow)
        {
            ArtNetNode::Get()->SetOutput(dmxnode_output_type_);

            const auto kShow = static_cast<pixelpatterns::Pattern>(trigger->sub_key);

            if (kShow == PixelTestPattern::Get()->GetPattern())
            {
                return;
            }

            const auto kIsSet = PixelTestPattern::Get()->SetPattern(kShow);

            if (!kIsSet)
            {
                return;
            }

            if (static_cast<pixelpatterns::Pattern>(kShow) != pixelpatterns::Pattern::kNone)
            {
                ArtNetNode::Get()->SetOutput(nullptr);
                Display::Get()->ClearLine(6);
                Display::Get()->Printf(6, "%s:%u", PixelPatterns::GetName(kShow), static_cast<uint32_t>(kShow));
            }
            else
            {
                dmxnode_output_type_->Blackout(true);
                DisplayUdf::Get()->Show();
            }

            return;
        }

        if (trigger->key == ArtTriggerKey::kArtTriggerUndefined)
        {
            if (trigger->sub_key == 0)
            {
                const auto kIsSet = PixelTestPattern::Get()->SetPattern(pixelpatterns::Pattern::kNone);

                if (!kIsSet)
                {
                    return;
                }

                ArtNetNode::Get()->SetOutput(nullptr);

                auto& configuration = PixelDmxConfiguration::Get();
                const auto* trigger_data = &trigger->data[0];
                const uint32_t kColour = trigger_data[0] | (static_cast<uint32_t>(trigger_data[1]) << 8) | (static_cast<uint32_t>(trigger_data[2]) << 16) |
                                         (static_cast<uint32_t>(trigger_data[3]) << 24);

                for (uint32_t active_port = 0; active_port < configuration.GetOutputPorts(); active_port++)
                {
                    pixel::SetPixelColour(active_port, kColour);
                }

                pixel::Update();
            }
        }
    }

   private:
    DmxNodeOutputType* dmxnode_output_type_;

    static inline ArtNetTriggerHandler* s_this;
};

#endif // COMMON_FIRMWARE_ARTNET_PIXEL_ARTNETTRIGGERHANDLER_H_
