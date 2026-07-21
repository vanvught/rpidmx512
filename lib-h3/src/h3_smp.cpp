/**
 * @file h3_smp.c
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

#include <cstdint>

#include "h3_smp.h"
#include "h3.h"
#include "h3_cpu.h"
#include "h3_spinlock.h"
#include "arm/synchronize.h"

namespace h3::smp {
static volatile bool core_is_started;
static start_fn_t start_fn;

void CoreMain() {
    start_fn_t temp_fn = start_fn;
    dmb();

    core_is_started = true;

    temp_fn();

    for (;;) {
    }
}

void StartCore(uint32_t core_number, start_fn_t start) {
    if (core_number == 0 || core_number >= H3_CPU_COUNT) {
        return;
    }

    h3_spinlock_lock(0);

    start_fn = start;

    H3_CPUCFG->PRIVATE0 = (uint32_t)_init_core;

    h3::cpu::On(static_cast<h3::cpu::Cpu>(core_number));

    core_is_started = false;

    while (!core_is_started) { // TODO blocking wait
        dmb();
    }

    h3_spinlock_unlock(0);
}
} // namespace h3::smp

extern "C" {
	void smp_core_main() { // NOLINT
		h3::smp::CoreMain();
	}
}
