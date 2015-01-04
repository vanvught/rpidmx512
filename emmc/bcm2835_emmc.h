/**
 * @file bcm2835_emmc.h
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#include "block.h"
#include "timer.h"

struct emmc_block_dev
{
	struct block_device bd;
	uint32_t card_supports_sdhc;
	uint32_t card_supports_18v;
	uint32_t card_ocr;
	uint32_t card_rca;
	uint32_t last_interrupt;
	uint32_t last_error;

	struct sd_scr *scr;

	int failed_voltage_switch;

    uint32_t last_cmd_reg;
    uint32_t last_cmd;
	uint32_t last_cmd_success;
	uint32_t last_r0;
	uint32_t last_r1;
	uint32_t last_r2;
	uint32_t last_r3;

	void *buf;
	int blocks_to_transfer;
	size_t block_size;
	int use_sdma;
	int card_removal;
};

// STATUS, offset 0x24
#define BCM2835_EMMC_STATUS_CMD_INHIBIT		((uint32_t)0x00000001)	///< SDHCI_PRESENT_STATE 0x24, SDHCI_CMD_INHIBIT 0x00000001
#define BCM2835_EMMC_STATUS_DATA_INHIBIT	((uint32_t)0x00000002)	///< SDHCI_PRESENT_STATE 0x24, SDHCI_DATA_INHIBIT 0x00000002
#define BCM2835_EMMC_CARD_PRESENT     		((uint32_t)0x00010000)	///< SDHCI_PRESENT_STATE 0x24, SDHCI_CARD_PRESENT 0x00010000

// CONTROL0, offset 0x28
#define BCM2835_EMMC_CONTROL0_POWER_ON		((uint32_t)(1 << 8))	///< SDHCI_POWER_CONTROL 0x29, SDHCI_POWER_ON 0x01

// CONTROL1, offset 0x2C
#define BCM2835_EMMC_CLOCK_INT_EN			((uint32_t)0x0001)		///< SDHCI_CLOCK_CONTROL  0x2C, SDHCI_CLOCK_INT_EN 0x0001
#define BCM2835_EMMC_CLOCK_INT_STABLE		((uint32_t)0x0002)		///< SDHCI_CLOCK_CONTROL  0x2C, SDHCI_CLOCK_INT_STABLE 0x0002
#define BCM2835_EMMC_CLOCK_CARD_EN			((uint32_t)0x0004)		///< SDHCI_CLOCK_CONTROL  0x2C, SDHCI_CLOCK_CARD_EN	0x0004
#define BCM2835_EMMC_RESET_ALL 				((uint32_t)(1 << 24))	///< SDHCI_SOFTWARE_RESET 0x2F, SDHCI_RESET_ALL 0x01
#define BCM2835_EMMC_RESET_CMD 				((uint32_t)(1 << 25))	///< SDHCI_SOFTWARE_RESET 0x2F, SDHCI_RESET_CMD 0x02
#define BCM2835_EMMC_RESET_DATA				((uint32_t)(1 << 26))	///< SDHCI_SOFTWARE_RESET 0x2F, SDHCI_RESET_DATA 0x04

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

extern int bcm2835_emmc_reset_cmd(void);
extern int bcm2835_emmc_reset_dat(void);
extern uint32_t bcm2825_emmc_set_id_clock(uint32_t);
extern uint32_t bcm2835_emmc_set_clock(uint32_t, uint32_t);
extern uint32_t bcm2825_emmc_init(uint32_t);
extern void bcm2835_emmc_handle_interrupts(struct emmc_block_dev *);
extern void bcm2835_emmc_issue_command(struct emmc_block_dev *dev, uint32_t cmd_reg, uint32_t argument, useconds_t timeout);
extern void bcm2835_emmc_sdcard_power_off(void);

#endif /* BCM2835_EMMC_H_ */
