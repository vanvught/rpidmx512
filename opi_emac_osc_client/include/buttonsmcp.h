/**
 * @file buttonsmcp.h
 *
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

#ifndef BUTTONSMCP_H_
#define BUTTONSMCP_H_

#include <stdint.h>

#include "buttonsset.h"
#include "oscclientled.h"
#include "oscclient.h"
#include "hal_i2c.h"

class ButtonsMcp final: public ButtonsSet, public OscClientLed
{
   public:
    explicit ButtonsMcp(OscClient* oscclient);

    bool Start() override;
    void Stop() override;
    void Run() override;

    void SetLed(uint32_t led, bool on) override;

   private:
    HAL_I2C hal_i2c_;
    OscClient* oscclient_;
    bool is_connected_{false};
    uint8_t buttons_previous_{0};
    uint8_t port_b_{0};
};

#endif  // BUTTONSMCP_H_
