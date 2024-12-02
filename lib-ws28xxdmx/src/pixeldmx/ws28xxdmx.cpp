/**
 * @file ws28xxdmx.cpp
 *
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined (DEBUG_PIXELDMX)
# undef NDEBUG
#endif

#include <cassert>

#include "ws28xxdmx.h"
#include "pixeldmxconfiguration.h"

#if defined (PIXELDMXSTARTSTOP_GPIO)
# include "hal_gpio.h"
#endif

#include "debug.h"

WS28xxDmx::WS28xxDmx() {
		DEBUG_ENTRY

		assert(s_pThis == nullptr);
		s_pThis = this;

		PixelDmxConfiguration::Get().Validate(1);

		m_pWS28xx = new WS28xx();
		assert(m_pWS28xx != nullptr);
		m_pWS28xx->Blackout();

#if defined (PIXELDMXSTARTSTOP_GPIO)
		FUNC_PREFIX(gpio_fsel(PIXELDMXSTARTSTOP_GPIO, GPIO_FSEL_OUTPUT));
		FUNC_PREFIX(gpio_clr(PIXELDMXSTARTSTOP_GPIO));
#endif

		DEBUG_EXIT
	}

WS28xxDmx::~WS28xxDmx() {
		DEBUG_ENTRY

		delete m_pWS28xx;
		m_pWS28xx = nullptr;

		DEBUG_EXIT
	}
