/**
 * @file buttonsgpio.cpp
 *
 */
/* Copyright (C) 2019-2026 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cassert>

#include "buttonsgpio.h"
#include "oscclient.h"
#include "gpio.h"
#include "firmware/debug/debug_debug.h"

#define BUTTON(x) ((buttons_ >> x) & 0x01)
#define BUTTON_STATE(x) ((buttons_ & (1U << x)) == (1U << x))

#define BUTTON0_GPIO GPIO_EXT_13
#define BUTTON1_GPIO GPIO_EXT_11
#define BUTTON2_GPIO GPIO_EXT_22
#define BUTTON3_GPIO GPIO_EXT_15

#define INT_MASK ((1U << BUTTON0_GPIO) | (1U << BUTTON1_GPIO) | (1U << BUTTON2_GPIO) | (1U << BUTTON3_GPIO))

#define LED0_GPIO GPIO_EXT_7
#define LED1_GPIO GPIO_EXT_12
#define LED2_GPIO GPIO_EXT_26
#define LED3_GPIO GPIO_EXT_18

ButtonsGpio::ButtonsGpio(OscClient* pOscClient) : oscclient_(pOscClient) {
    assert(oscclient_ != nullptr);
}

bool ButtonsGpio::Start() {
    gpio::Fsel(LED0_GPIO, GPIO_FSEL_OUTPUT);
    gpio::Fsel(LED1_GPIO, GPIO_FSEL_OUTPUT);
    gpio::Fsel(LED2_GPIO, GPIO_FSEL_OUTPUT);
    gpio::Fsel(LED3_GPIO, GPIO_FSEL_OUTPUT);

    gpio::Fsel(BUTTON0_GPIO, GPIO_FSEL_EINT);
    gpio::Fsel(BUTTON1_GPIO, GPIO_FSEL_EINT);
    gpio::Fsel(BUTTON2_GPIO, GPIO_FSEL_EINT);
    gpio::Fsel(BUTTON3_GPIO, GPIO_FSEL_EINT);

    gpio::SetPud(BUTTON0_GPIO, gpio::Pull::kUp);
    gpio::SetPud(BUTTON1_GPIO, gpio::Pull::kUp);
    gpio::SetPud(BUTTON2_GPIO, gpio::Pull::kUp);
    gpio::SetPud(BUTTON3_GPIO, gpio::Pull::kUp);

    gpio::IntCfg(BUTTON0_GPIO, gpio::IntConfig::kNegEdge);
    gpio::IntCfg(BUTTON1_GPIO, gpio::IntConfig::kNegEdge);
    gpio::IntCfg(BUTTON2_GPIO, gpio::IntConfig::kNegEdge);
    gpio::IntCfg(BUTTON3_GPIO, gpio::IntConfig::kNegEdge);

    H3_PIO_PA_INT->CTL |= INT_MASK;
    H3_PIO_PA_INT->STA = INT_MASK;
    H3_PIO_PA_INT->DEB = (0x0 << 0) | (0x7U << 4);

    buttons_count_ = 4;

    return true;
}

void ButtonsGpio::Stop() {
    gpio::Fsel(BUTTON0_GPIO, GPIO_FSEL_DISABLE);
    gpio::Fsel(BUTTON1_GPIO, GPIO_FSEL_DISABLE);
    gpio::Fsel(BUTTON2_GPIO, GPIO_FSEL_DISABLE);
    gpio::Fsel(BUTTON3_GPIO, GPIO_FSEL_DISABLE);

    gpio::Fsel(LED0_GPIO, GPIO_FSEL_DISABLE);
    gpio::Fsel(LED1_GPIO, GPIO_FSEL_DISABLE);
    gpio::Fsel(LED2_GPIO, GPIO_FSEL_DISABLE);
    gpio::Fsel(LED3_GPIO, GPIO_FSEL_DISABLE);
}

void ButtonsGpio::Run() {
    buttons_ = H3_PIO_PA_INT->STA & INT_MASK;

    if (__builtin_expect((buttons_ != 0), 0)) {
        H3_PIO_PA_INT->STA = INT_MASK;

        DEBUG_PRINTF("%d-%d-%d-%d", BUTTON(BUTTON0_GPIO), BUTTON(BUTTON1_GPIO), BUTTON(BUTTON2_GPIO), BUTTON(BUTTON3_GPIO));

        if (BUTTON_STATE(BUTTON0_GPIO)) {
            oscclient_->SendCmd(0);
            DEBUG_PUTS("BUTTON0_GPIO");
        }

        if (BUTTON_STATE(BUTTON1_GPIO)) {
            oscclient_->SendCmd(1);
            DEBUG_PUTS("BUTTON1_GPIO");
        }

        if (BUTTON_STATE(BUTTON2_GPIO)) {
            oscclient_->SendCmd(2);
            DEBUG_PUTS("BUTTON2_GPIO");
        }

        if (BUTTON_STATE(BUTTON3_GPIO)) {
            oscclient_->SendCmd(3);
            DEBUG_PUTS("BUTTON3_GPIO");
        }
    }
}

void ButtonsGpio::SetLed(uint32_t led, bool on) {
    DEBUG_PRINTF("led%d %s", led, on ? "On" : "Off");

    switch (led) {
        case 0:
            on ? gpio::Set(LED0_GPIO) : gpio::Clr(LED0_GPIO);
            break;
        case 1:
            on ? gpio::Set(LED1_GPIO) : gpio::Clr(LED1_GPIO);
            break;
        case 2:
            on ? gpio::Set(LED2_GPIO) : gpio::Clr(LED2_GPIO);
            break;
        case 3:
            on ? gpio::Set(LED3_GPIO) : gpio::Clr(LED3_GPIO);
            break;
        default:
            break;
    }
}
