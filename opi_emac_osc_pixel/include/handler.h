/**
 * @file handler.h
 */
/* Copyright (C) 2019-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef HANDLER_H_
#define HANDLER_H_

#include <cstdint>

#include "oscserver.h"
#include "oscsimplesend.h"
#include "pixeltype.h"
#include "pixeldmx.h"
#include "firmware/debug/debug_debug.h"

class Handler : public OscServerHandler
{
   public:
    Handler(PixelDmx* pixel_dmx) : pixel_dmx_(pixel_dmx)
    {
        DEBUG_ENTRY();
        DEBUG_EXIT();
    }

    void Blackout() override { pixel_dmx_->Blackout(true); }

    void Update() override { pixel_dmx_->Blackout(false); }

    void Info(int32_t handle, uint32_t remote_ip, uint16_t port_outgoing) override
    {
        OscSimpleSend MsgSendLedType(handle, remote_ip, port_outgoing, "/info/ledtype", "s",
                                     const_cast<char*>(pixel::GetTypeName(PixelConfiguration::Get().GetType())));
        OscSimpleSend MsgSendLedCount(handle, remote_ip, port_outgoing, "/info/ledcount", "i", static_cast<int>(PixelConfiguration::Get().GetCount()));
        OscSimpleSend MsgSendGroupCount(handle, remote_ip, port_outgoing, "/info/groupcount", "i",
                                        static_cast<int>(PixelDmxConfiguration::Get().GetGroupingCount()));
    }

   private:
    PixelDmx* pixel_dmx_;
};

#endif /* HANDLER_H_ */
