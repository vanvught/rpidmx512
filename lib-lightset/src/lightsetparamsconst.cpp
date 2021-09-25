/**
 * @file lightsetconst.cpp
 *
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "lightsetparamsconst.h"

const char LightSetParamsConst::PARAMS_OUTPUT[] = "output";

const char LightSetParamsConst::UNIVERSE[] = "universe";
const char LightSetParamsConst::UNIVERSE_PORT[4][16] = {
		"universe_port_a",
		"universe_port_b",
		"universe_port_c",
		"universe_port_d" };

const char LightSetParamsConst::MERGE_MODE[] = "merge_mode";
const char LightSetParamsConst::MERGE_MODE_PORT[4][18] = {
		"merge_mode_port_a",
		"merge_mode_port_b",
		"merge_mode_port_c",
		"merge_mode_port_d" };

const char LightSetParamsConst::DIRECTION[4][18] = {
		"direction_port_a",
		"direction_port_b",
		"direction_port_c",
		"direction_port_d" };

const char LightSetParamsConst::START_UNI_PORT[8][18] = {
		"start_uni_port_1",
		"start_uni_port_2",
		"start_uni_port_3",
		"start_uni_port_4",
		"start_uni_port_5",
		"start_uni_port_6",
		"start_uni_port_7",
		"start_uni_port_8" };

const char LightSetParamsConst::DMX_START_ADDRESS[] = "dmx_start_address";
const char LightSetParamsConst::DMX_SLOT_INFO[] = "dmx_slot_info";

const char LightSetParamsConst::TEST_PATTERN[] = "test_pattern";
