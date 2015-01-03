/**
 * @file bcm2835_emmc.h
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifndef BCM2835_EMMC_H_
#define BCM2835_EMMC_H_

#include "stdint.h"

// STATUS, offset 0x24
#define BCM2835_EMMC_STATUS_CMD_INHIBIT		((uint32_t)0x00000001)	///< SDHCI_PRESENT_STATE 0x24, SDHCI_CMD_INHIBIT 0x00000001
#define BCM2835_EMMC_STATUS_DATA_INHIBIT	((uint32_t)0x00000002)	///< SDHCI_PRESENT_STATE 0x24, SDHCI_DATA_INHIBIT 0x00000002

// CONTROL0, offset 0x28
#define BCM2835_EMMC_CONTROL0_POWER_ON		((uint32_t)(1 << 8))	///< SDHCI_POWER_CONTROL 0x29, SDHCI_POWER_ON 0x01

// CONTROL1, offset 0x2C
#define BCM2835_EMMC_CLOCK_INT_EN			((uint32_t)0x0001)		///< SDHCI_CLOCK_CONTROL  0x2C, SDHCI_CLOCK_INT_EN 0x0001
#define BCM2835_EMMC_CLOCK_INT_STABLE		((uint32_t)0x0002)		///< SDHCI_CLOCK_CONTROL  0x2C, SDHCI_CLOCK_INT_STABLE 0x0002
#define BCM2835_EMMC_CLOCK_CARD_EN			((uint32_t)0x0004)		///< SDHCI_CLOCK_CONTROL  0x2C, SDHCI_CLOCK_CARD_EN	0x0004
#define BCM2835_EMMC_CONTROL1_RESET_ALL 	((uint32_t)(1 << 24))	///< SDHCI_SOFTWARE_RESET 0x2F, SDHCI_RESET_ALL 0x01
#define BCM2835_EMMC_CONTROL1_RESET_CMD 	((uint32_t)(1 << 25))	///< SDHCI_SOFTWARE_RESET 0x2F, SDHCI_RESET_CMD 0x02
#define BCM2835_EMMC_CONTROL1_RESET_DATA	((uint32_t)(1 << 26))	///< SDHCI_SOFTWARE_RESET 0x2F, SDHCI_RESET_DATA 0x04

/*
 * End of controller registers.
 */

// TODO Move to sdhci.h
#define SDHCI_DIVIDER_SHIFT		8
#define SDHCI_DIVIDER_HI_SHIFT	6
#define SDHCI_DIV_MASK			0xFF
#define SDHCI_DIV_MASK_LEN		8
#define SDHCI_DIV_HI_MASK		0x300

#define SDHCI_MAX_DIV_SPEC_200	256
#define SDHCI_MAX_DIV_SPEC_300	2046

extern int bcm2835_emmc_power_on(void);
extern int bcm2835_emmc_power_off(void);
extern int bcm2835_emmc_reset_cmd(void);
extern int bcm2835_emmc_reset_dat(void);
extern uint32_t bcm2835_emmc_get_clock_divider(uint32_t, uint32_t);
extern uint32_t bcm2835_emmc_switch_clock_rate(uint32_t, uint32_t);

#endif /* BCM2835_EMMC_H_ */
