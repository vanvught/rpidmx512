/**
 * @file lightsetconst.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "lightsetconst.h"

const char LightSetConst::PARAMS_OUTPUT[] = "output";

const char LightSetConst::PARAMS_UNIVERSE[] = "universe";
const char LightSetConst::PARAMS_UNIVERSE_PORT[4][16] = { "universe_port_a", "universe_port_b", "universe_port_c", "universe_port_d" };

const char LightSetConst::PARAMS_MERGE_MODE[] = "merge_mode";
const char LightSetConst::PARAMS_MERGE_MODE_PORT[4][18] = { "merge_mode_port_a", "merge_mode_port_b", "merge_mode_port_c", "merge_mode_port_d" };

const char LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE[] = "enable_no_change_update";

const char LightSetConst::PARAMS_DMX_START_ADDRESS[] = "dmx_start_address";
const char LightSetConst::PARAMS_DMX_SLOT_INFO[] = "dmx_slot_info";
