/**
 * @file hardware.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of thnDmxDataDirecte Software, and to permit persons to whom the Software is
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

#if defined (DEBUG_HAL)
# undef NDEBUG
#endif

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <time.h>
#include <uuid/uuid.h>
#include <cassert>

#include "hardware.h"

#include "h3.h"
#include "h3_watchdog.h"
#include "h3_gpio.h"
#include "h3_board.h"
#include "irq_timer.h"

#include "arm/arm.h"
#include "arm/gic.h"
#include "arm/synchronize.h"

#if defined (DEBUG_I2C)
# include "i2cdetect.h"
#endif

#if !defined(DISABLE_RTC)
# include "hwclock.h"
#endif

#include "logic_analyzer.h"

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# if __GNUC__ > 8
#  pragma GCC target ("general-regs-only")
# endif
#endif

static void EXTIA_IRQHandler() {
	DEBUG_PUTS("EXTIA_IRQHandler");

	H3_PIO_PA_INT->STA = ~0;
}

static void EXTIG_IRQHandler() {
	DEBUG_PUTS("EXTIG_IRQHandler");

	H3_PIO_PG_INT->STA = ~0;
}

static void __attribute__((interrupt("IRQ"))) IRQ_Handler() {
	__DMB();

	const auto nIRQ = GICInterface->AIAR;
	IRQHandler_t const handler = IRQ_GetHandler(nIRQ);

	if (handler != nullptr) {
		handler();
	}

	GICInterface->AEOIR = nIRQ;
	const auto nIndex = nIRQ / 32;
	const auto nMask = 1U << (nIRQ % 32);
	GICDistributor->ICPENDR[nIndex] = nMask;

	__DMB();
}

#pragma GCC pop_options

namespace net {
void net_shutdown();
}  // namespace net

namespace hal {
void reboot_handler();
}  // namespace hardware

namespace soc {
#if defined (ORANGE_PI)
	static constexpr char NAME[] = "H2+";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
#elif defined(ORANGE_PI_ONE)
	static constexpr char NAME[] = "H3";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
#else
# error Platform not supported
#endif
}

namespace cpu {
	static constexpr char NAME[] = "Cortex-A7";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
}

namespace machine {
	static constexpr char NAME[] = "arm";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
}

namespace sysname {
	static constexpr char NAME[] = "Baremetal";
	static constexpr auto NAME_LENGTH = sizeof(NAME) - 1;
}

void hardware_init();

Hardware::Hardware() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	hardware_init();

#if defined (DEBUG_I2C)
	i2c_detect();
#endif

#if !defined(DISABLE_RTC)
	m_HwClock.RtcProbe();
	m_HwClock.Print();
	m_HwClock.HcToSys();
#endif

	hardware_led_set(1);

	logic_analyzer::init();

	IRQ_SetHandler(H3_PA_EINT_IRQn, EXTIA_IRQHandler);
//	gic_irq_config(H3_PA_EINT_IRQn, GIC_CORE0);
//
	IRQ_SetHandler(H3_PG_EINT_IRQn, EXTIG_IRQHandler);
//	gic_irq_config(H3_PG_EINT_IRQn, GIC_CORE0);

	arm_install_handler((unsigned) IRQ_Handler, ARM_VECTOR(ARM_VECTOR_IRQ));
}

const char *Hardware::GetMachine(uint8_t &nLength) {
	nLength = machine::NAME_LENGTH;
	return machine::NAME;
}

const char *Hardware::GetSysName(uint8_t &nLength) {
	nLength = sysname::NAME_LENGTH;
	return sysname::NAME;
}

const char *Hardware::GetBoardName(uint8_t &nLength) {
	nLength = sizeof(H3_BOARD_NAME)  - 1;
	return H3_BOARD_NAME;
}

const char *Hardware::GetCpuName(uint8_t &nLength) {
	nLength = cpu::NAME_LENGTH;
	return cpu::NAME;
}

const char *Hardware::GetSocName(uint8_t &nLength) {
	nLength = soc::NAME_LENGTH;
	return soc::NAME;
}

void configstore_commit();

bool Hardware::Reboot() {
	puts("Rebooting ...");
	
	h3_watchdog_disable();

	configstore_commit();

#if !defined(DISABLE_RTC)
	m_HwClock.SysToHc();
#endif

	hal::reboot_handler();
	net::net_shutdown();

	clean_data_cache();
	invalidate_data_cache();

	h3_gpio_fsel(EXT_SPI_MOSI, GPIO_FSEL_INPUT);
	h3_gpio_set_pud(EXT_SPI_MOSI, GPIO_PULL_DOWN);
	h3_gpio_fsel(EXT_SPI_CLK, GPIO_FSEL_INPUT);
	h3_gpio_set_pud(EXT_SPI_CLK, GPIO_PULL_DOWN);
	h3_gpio_fsel(EXT_SPI_CS, GPIO_FSEL_INPUT);
	h3_gpio_set_pud(EXT_SPI_CS, GPIO_PULL_DOWN);

	SetMode(hardware::ledblink::Mode::REBOOT);

	h3_watchdog_enable();

	for (;;) {
		Run();
	}

	__builtin_unreachable();
	return true;
}
