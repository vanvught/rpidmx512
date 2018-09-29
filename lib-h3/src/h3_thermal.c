/**
 * @file h3_thermal.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>

#include "h3.h"
#include "h3_ccu.h"
#include "h3_sid.h"

#define CTRL1_ADC_CALI_EN			(1 << 17)

#define CTRL2_SENSOR_EN				(1 << 0)
	#define CTRL2_SENSOR_ACQ1_SHIFT		16

#define FILTER_ENABLE				(1 << 2)
#define FILTER_TYPE2				(0b00 << 0)
#define FILTER_TYPE4				(0b01 << 0)
#define FILTER_TYPE8				(0b10 << 0)
#define FILTER_TYPE16				(0b11 << 0)

#define ALARM_CTRL_T_HOT_MASK		0x0FFF
	#define ALARM_CTRL_T_HOT_SHIFT		16

#define SHUTDOWN_CTRL_T_HOT_MASK	0x0FFF
	#define SHUTDOWN_CTRL_T_HOT_SHIFT	16

#define INT_CTRL_THERMAL_PER_SHIFT	12
#define INT_CTRL_DATA_IRQ_EN		(1 << 8)
#define INT_CTRL_SHUT_IRQ_EN		(1 << 4)
#define INT_CTRL_ALARM_IRQ_EN		(1 << 0)

#define	TEMP_BASE					217
#define	TEMP_MUL					1000
#define	TEMP_DIV					8253
#define	TEMP_MINUS					1794000

#define	INIT_ALARM_CELCIUS			90
#define	INIT_SHUT_CELCIUS			105

#define	DATA_MASK					0xFFF

// https://github.com/freebsd/freebsd/blob/master/sys/arm/allwinner/aw_thermal.c#L109
#define	H3_ADC_ACQUIRE_TIME	0x3f		// TODO How is this calculated?
#define	H3_FILTER			0x6			// Some other calculation here -> https://github.com/megous/h3-firmware/blob/master/ths.c
#define	H3_INTC				0x191000	// Other values here -> https://github.com/ayufan-pine64/linux-3.10/blob/master/drivers/thermal/sunxi_thermal/sun50i_ths.h

static int to_temp(uint32_t val) {
	return (TEMP_BASE - ((val * TEMP_MUL) / TEMP_DIV));
}

static uint32_t to_reg(int val) {
	return ((TEMP_MINUS - (val * TEMP_DIV)) / TEMP_MUL);
}

int h3_thermal_gettemp(void) {
	const uint32_t value = H3_THS->DATA & DATA_MASK;
	return (value == 0 ? -1 : to_temp(value));
}

int h3_thermal_getshut(void) {
	const uint32_t value = (H3_THS->SHUTDOWN_CTRL >> SHUTDOWN_CTRL_T_HOT_SHIFT) & SHUTDOWN_CTRL_T_HOT_MASK;

	return (to_temp(value));
}

void h3_thermal_setshut(int temp) {
	uint32_t value = H3_THS->SHUTDOWN_CTRL;
	value &= ~(SHUTDOWN_CTRL_T_HOT_MASK << SHUTDOWN_CTRL_T_HOT_SHIFT);
	value |= (to_reg(temp) << SHUTDOWN_CTRL_T_HOT_SHIFT);
	H3_THS->SHUTDOWN_CTRL = value;
}

int h3_thermal_getalarm(void) {
	const uint32_t value = (H3_THS->ALARM_CTRL >> ALARM_CTRL_T_HOT_SHIFT) & ALARM_CTRL_T_HOT_MASK;

	return (to_temp(value));
}

void h3_thermal_setalarm(int temp) {
	uint32_t value = H3_THS->ALARM_CTRL;
	value &= ~(ALARM_CTRL_T_HOT_MASK << ALARM_CTRL_T_HOT_SHIFT);
	value |= (to_reg(temp) << ALARM_CTRL_T_HOT_SHIFT);
	H3_THS->ALARM_CTRL = value;
}

void h3_thermal_init(void) {
	uint32_t value = H3_CCU->THS_CLK;
	value &= ~(CCU_THS_CLK_SRC_MASK);
	value |= (CCU_THS_CLK_SRC_OSC24M << CCU_THS_CLK_SRC_SHIFT);
	value &= ~(CCU_THS_CLK_DIV_RATIO_MASK);
	value |= (CCU_THS_CLK_DIV_RATIO_6 << CCU_THS_CLK_DIV_RATIO_SHIFT);	// 24/6 = 4MHz
	H3_CCU->THS_CLK = value | CCU_THS_CLK_SCLK_GATING;

	H3_CCU->BUS_SOFT_RESET3 |= CCU_BUS_SOFT_RESET3_THS;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING2 &= ~(CCU_BUS_CLK_GATING2_THS);
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING2 |= CCU_BUS_CLK_GATING2_THS;

	uint32_t calib;

	h3_sid_read_tscalib(&calib);

	calib &= DATA_MASK;

	if (calib != 0) {
		H3_THS->CDATA = calib;
	}

	H3_THS->CTRL1 = CTRL1_ADC_CALI_EN;
	H3_THS->CTRL0 = H3_ADC_ACQUIRE_TIME;
	H3_THS->CTRL2 = H3_ADC_ACQUIRE_TIME << CTRL2_SENSOR_ACQ1_SHIFT;

	H3_THS->FILTER = H3_FILTER;

	uint32_t val = H3_THS->STAT;
	H3_THS->STAT = val;

	H3_THS->INT_CTRL = H3_INTC  | INT_CTRL_DATA_IRQ_EN | INT_CTRL_SHUT_IRQ_EN | INT_CTRL_ALARM_IRQ_EN;

	H3_THS->CTRL2 |= CTRL2_SENSOR_EN;

	h3_thermal_setalarm(INIT_ALARM_CELCIUS);
	h3_thermal_setshut(INIT_SHUT_CELCIUS);
}
