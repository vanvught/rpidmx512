/**
 * @file rdmqueuedmessage.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMQUEUEDMESSAGE_H_
#define RDMQUEUEDMESSAGE_H_

#include <cstdint>

#include "e120.h"

struct TRdmQueuedMessage
{
    uint8_t command_class;     ///< 21
    uint8_t param_id[2];       ///< 22, 23
    uint8_t param_data_length; ///< 24	PDL	Range 0 to 231
    uint8_t param_data[231];   ///< 25,,,,	PD	6.2.3 Message Length
};

class RDMQueuedMessage
{
   public:
    RDMQueuedMessage();
    ~RDMQueuedMessage();

    uint8_t GetMessageCount() const;

    void Handler(uint8_t* rdm_data);

    bool Add(const struct TRdmQueuedMessage* msg);

   private:
    void Copy(struct TRdmMessage* rdm_message, uint32_t index);

   private:
    uint8_t message_count_{0};
    bool is_never_queued_{true};

    struct TRdmQueuedMessage* queued_message_;
};

#endif  // RDMQUEUEDMESSAGE_H_
