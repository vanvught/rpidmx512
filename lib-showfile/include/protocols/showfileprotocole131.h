/**
 * @file showfileprotocole131.h
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

#ifndef PROTOCOLS_SHOWFILEPROTOCOLE131_H_
#define PROTOCOLS_SHOWFILEPROTOCOLE131_H_

#include <cstdint>

#include "e131controller.h"
 #include "firmware/debug/debug_debug.h"

class ShowFileProtocol
{
   public:
    ShowFileProtocol()
    {
        DEBUG_ENTRY();

        DEBUG_EXIT();
    }

    void SetSynchronizationAddress(uint16_t synchronization_address) { e131_controller_.SetSynchronizationAddress(synchronization_address); }

    void Start()
    {
        DEBUG_ENTRY();

        e131_controller_.Start();

        DEBUG_EXIT();
    }

    void Stop()
    {
        DEBUG_ENTRY();

        e131_controller_.Stop();

        DEBUG_EXIT();
    }

    void Record()
    {
        DEBUG_ENTRY();

        DEBUG_EXIT();
    }

    void DmxOut(uint16_t universe, const uint8_t* dmx_data, uint32_t length) { e131_controller_.HandleDmxOut(universe, dmx_data, length); }
    void DmxSync() { e131_controller_.HandleSync(); }
    void DmxBlackout() { e131_controller_.HandleBlackout(); }
    void DmxMaster(uint32_t master) { e131_controller_.SetMaster(master); }

    void DoRunCleanupProcess([[maybe_unused]] bool do_run) {}

    void Run()
    {
        // Nothing todo here
    }

    bool IsSyncDisabled() { return (e131_controller_.GetSynchronizationAddress() == 0); }

    void Print() { e131_controller_.Print(); }

   private:
    E131Controller e131_controller_;
};

#endif  // PROTOCOLS_SHOWFILEPROTOCOLE131_H_
