/**
 * @file mmc_bsp.c
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
/**
 * Original code : https://github.com/allwinner-zh/bootloader/blob/master/basic_loader/boot0/load_boot1_from_sdmmc/bsp_mmc_for_boot/mmc_bsp.c
 */
/*
 * (C) Copyright 2007-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdint.h>

#include "mmc_internal.h"

#include "h3.h"
#include "h3_timer.h"

#include "util.h"

#include "debug.h"

// CCU register
#define SDMMC_CLK_SCLK_GATING		(1 << 31)	///< 1 = Clock is ON. SCLK = Clock source/Divider N/ Divider M

#define BUS_CLK_GATING0_MMC0_GATE	(1 << 8)
#define BUS_CLK_GATING0_MMC1_GATE	(1 << 9)
#define BUS_CLK_GATING0_MMC2_GATE	(1 << 10)

// SD_MMCx register
#define GCTL_SOFT_RST			(1 << 0)	///< Reset SD/MMC controller
#define GCTL_FIFO_RST			(1 << 1)	///< Reset FIFO
#define GCTL_DMA_RST			(1 << 2)	///< DMA Reset
	#define GCTL_RESET	(GCTL_SOFT_RST | GCTL_FIFO_RST | GCTL_DMA_RST)
#define GCTL_FIFO_AC_MODE_AHB	(1 << 31)	///< FIFO Access Mode AHB bus

	#define CKC_CCLK_DIV_MASK		(0xFF << 0)
#define CKC_CCLK_ENABLE			(1 << 16)	///< Card Clock on
#define CKC_CCLK_CTRL			(1 << 17)	///< Turn off card when FSM in IDLE state
#define CKC_CCLK_MASK_DATA0		(1 << 31)	///< Mask Data0 when update clock

#define BWD_CARD_WID_1BIT		(0b00)
#define BWD_CARD_WID_4BIT		(0b01)
#define BWD_CARD_WID_8BIT		(0b10)

#define HWRST_HW_RESET			(0 << 0)
#define HWRST_HW_ACTIVE			(1 << 0)

#define STA_FIFO_EMPTY			(1 << 2)
#define STA_FIFO_FULL			(1 << 3)

	#define CMD_CMD_IDX_MASK		(0x3F << 0)
#define CMD_RESP_RCV			(1 << 6)	///< Command with Response
#define CMD_LONG_RESP			(1 << 7)	///< Long Response (136 bits)
#define CMD_CHK_RESP_CRC		(1 << 8)	///< Check Response CRC
#define CMD_DATA_TRANS			(1 << 9)	///< Data Transfer with data transfer
#define CMD_TRANS_WRITE			(1 << 10)	///< Transfer direction Write operation
#define CMD_TRANS_MODE_STREAM	(1 << 11)	///< Transfer Mode Stream data transfer command
#define CMD_STOP_CMD_FLAG		(1 << 12)	///< Send Stop CMD Automatically (CMD12)
#define CMD_WAIT_PRE_OVER		(1 << 13)	///< Wait for data transfer completion before sending current command
#define CMD_SEND_INIT_SEQ		(1 << 15)	///< Send initialization sequence before sending this command
#define CMD_PRG_CLOCK			(1 << 21)	///< Change Card Clock
#define CMD_CMD_LOAD			(1 << 31)	///< Start Command

#define RIS_RESPONSE_ERROR		(1 << 1)	///< Response Error (no response or response CRC error)
#define RIS_CMD_COMPLETE		(1 << 2)	///< Command Complete
#define RIS_TRANS_COMPLETE		(1 << 3)	///< Data Transfer Complete
#define RIS_TRANSMIT_REQ		(1 << 4)	///< Data Transmit Request
#define RIS_RECEIVE_REQ			(1 << 5)	///< Data Receive Request
#define RIS_RESP_CRC_ERROR		(1 << 6)	///< Response CRC error
#define RIS_DATA_CRC_ERROR		(1 << 7)	///< Data CRC error
#define RIS_RESP_TIMEOUT		(1 << 8)	///< Response timeout/Boot ACK received
#define RIS_DATA_TIMEOUT		(1 << 9)	///< Data timeout/Boot data start
#define RIS_DATA_STAR_TIMEOUT	(1 << 10)	///< Data starvation timeout(HTO)/V1.8 Switch Done
#define RIS_FIFO_UNDERRUN		(1 << 11)	///< FIFO under run/overflow
#define RIS_CMD_BUSY			(1 << 12)	///< Command Busy and illagal write
#define RIS_DATA_START_ERROR	(1 << 13)	///< Data Start Error
#define RIS_AUTO_CMD_DONE		(1 << 14)	///< Auto command done
#define RIS_DATA_ENDBIT_ERROR	(1 << 15)	///< Data End-bit error
//0xbbc2 0b1011101111000010
#define RIS_RAW_ISTA	(RIS_DATA_ENDBIT_ERROR |RIS_DATA_START_ERROR | RIS_CMD_BUSY | RIS_FIFO_UNDERRUN | RIS_DATA_TIMEOUT | RIS_RESP_TIMEOUT | RIS_DATA_CRC_ERROR | RIS_RESP_CRC_ERROR | RIS_RESPONSE_ERROR)

#ifndef NDEBUG
static void dumphex32(const char *reg_name, char *base, uint32_t len) {
	uint32_t i;

	mmcmsg("Dump %s registers:", reg_name);

	for (i = 0; i < len; i += 4) {
		mmcmsg("\n%p : ", base + i);
		mmcmsg("%p ", *(volatile uint32_t *)(base + i));
	}

	mmcmsg("\n");
}
#else
 #define  dumphex32(fmt...)
#endif

struct sunxi_mmc_host {
	uint32_t  mclk;
	uint32_t  fatal_err;
} static _aw_mmc_host;

static struct mmc mmc_dev;

static void mmc_ccu_init(void) {
	DEBUG_ENTRY

	uint32_t value = H3_CCU->BUS_CLK_GATING0;
	value |= BUS_CLK_GATING0_MMC0_GATE;
	H3_CCU->BUS_CLK_GATING0 = value;

	value = H3_CCU->BUS_SOFT_RESET0;
	value |= BUS_CLK_GATING0_MMC0_GATE;
	H3_CCU->BUS_SOFT_RESET0 = value;

	H3_CCU->SDMMC0_CLK = SDMMC_CLK_SCLK_GATING; // Clock ON, SRC=OSC24M, N = 1, M = 1
	_aw_mmc_host.mclk = 24000000;

	DEBUG_EXIT
}

static int mmc_update_clk(void) {
	DEBUG_ENTRY

	int timeout = 0xfffff;

	H3_SD_MMC0->CMD = CMD_CMD_LOAD | CMD_PRG_CLOCK | CMD_WAIT_PRE_OVER;

	while ((H3_SD_MMC0->CMD & CMD_CMD_LOAD) && timeout--)
		;

	if (timeout < 0) {
		mmcinfo("update clk failed\n");
		dumphex32("MMC0_BASE", (char *) H3_SD_MMC0_BASE, 0x100);
		DEBUG_EXIT
		return -1;
	}

	uint32_t value = H3_SD_MMC0->RIS;
	H3_SD_MMC0->RIS = value;

	DEBUG_EXIT
	return 0;
}

static int mmc_config_clock(unsigned clk) {
	DEBUG_ENTRY

	mmcdbg("clk %d\n",clk);

	uint32_t value = H3_SD_MMC0->CKC;
	value &= ~CKC_CCLK_ENABLE;
	H3_SD_MMC0->CKC = value;

	if (mmc_update_clk()) {
		mmcinfo("disable clock failed\n");
		DEBUG_EXIT
		return -1;
	}

	H3_CCU->SDMMC0_CLK = 0;

	mmcdbg("H3_CCU->SDMMC0_CLK %x\n",H3_CCU->SDMMC0_CLK);

	// SCLK = OSC24M / N / M
	if (clk <= 400000) {
		_aw_mmc_host.mclk = 400000;
		H3_CCU->SDMMC0_CLK = 0x0002000f; // N = 4, M = 16
	} else { // FIXME Get PERIPH0 clock and set DIVs accordingly
		_aw_mmc_host.mclk = 12000000;
		H3_CCU->SDMMC0_CLK = 0x00000001; // N = 1, M = 2
	}

	mmcdbg("H3_CCU->SDMMC0_CLK %x\n",H3_CCU->SDMMC0_CLK);

	H3_CCU->SDMMC0_CLK |= SDMMC_CLK_SCLK_GATING;

	mmcdbg("H3_CCU->SDMMC0_CLK %x\n",H3_CCU->SDMMC0_CLK);

	value &= ~CKC_CCLK_DIV_MASK;
	H3_SD_MMC0->CKC = value;

	if (mmc_update_clk()) {
		mmcinfo("Change Divider Factor failed\n");
		DEBUG_EXIT
		return -1;
	}

	H3_SD_MMC0->CKC |= (CKC_CCLK_ENABLE | CKC_CCLK_CTRL);

	if (mmc_update_clk()) {
		mmcinfo("Re-enable clock failed\n");
		DEBUG_EXIT
		return -1;
	}

	DEBUG_EXIT
	return 0;
}

// Called by mmc.c
static void mmc_set_ios(struct mmc *mmc) {
	DEBUG_ENTRY

	mmcdbg("ios: bus_width: %d, clock: %d\n", mmc->bus_width, mmc->clock);

	if (mmc->clock && mmc_config_clock(mmc->clock)) {
		msg("[mmc]: " "*** update clock failed\n");
		_aw_mmc_host.fatal_err = 1;
		DEBUG_EXIT
		return;
	}

	if (mmc->bus_width == 8) {
		H3_SD_MMC0->BWD = BWD_CARD_WID_8BIT;
	} else if (mmc->bus_width == 4) {
		H3_SD_MMC0->BWD = BWD_CARD_WID_4BIT;
	} else {
		H3_SD_MMC0->BWD = BWD_CARD_WID_1BIT;
	}

	DEBUG_EXIT
}

// Called by mmc.c
static int mmc_core_init(void) {
	DEBUG_ENTRY

	// Reset controller
	H3_SD_MMC0->GCTL = GCTL_RESET;
	while (H3_SD_MMC0->GCTL & GCTL_RESET)
		;

	// Release MMC reset signal
	H3_SD_MMC0->HWRST = HWRST_HW_ACTIVE;
	H3_SD_MMC0->HWRST = HWRST_HW_RESET;
	__msdelay(1);
	H3_SD_MMC0->HWRST = HWRST_HW_ACTIVE;

	DEBUG_EXIT
	return 0;
}

static int mmc_trans_data_by_cpu(struct mmc *mmc, struct mmc_data *data) {
	uint32_t i;
	uint32_t byte_cnt = data->blocksize * data->blocks;
	uint32_t *buff;
	int32_t timeout = 0xffffff;

	if (data->flags & MMC_DATA_READ) {
		buff = (uint32_t *) data->b.dest;
		for (i = 0; i < (byte_cnt >> 2); i++) {

			while (--timeout && (H3_SD_MMC0->STA & STA_FIFO_EMPTY))
				;

			if (timeout <= 0) {
				mmcinfo("read transfer by cpu failed\n");
				return -1;
			}

			buff[i] = H3_SD_MMC0->FIFO;
			timeout = 0xffffff;
		}
	} else {
		buff = (uint32_t *) data->b.src;
		for (i = 0; i < (byte_cnt >> 2); i++) {

			while (--timeout && (H3_SD_MMC0->STA & STA_FIFO_FULL))
				;

			if (timeout <= 0) {
				mmcinfo("write transfer by cpu failed\n");
				return -1;
			}

			H3_SD_MMC0->FIFO = buff[i];
			timeout = 0xffffff;
		}
	}

	return 0;
}

// Called by mmc.c
static int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data) {
	uint32_t cmdval = CMD_CMD_LOAD;
	uint32_t timeout = 0;
	uint32_t status = 0;
	int32_t error = 0;

	if (_aw_mmc_host.fatal_err){
		mmcinfo("Found fatal err,so no send cmd\n");
		return -1;
	}

	if (cmd->resp_type & MMC_RSP_BUSY) {
		mmcdbg("cmd %d check rsp busy\n", cmd->cmdidx);
	}

	if (cmd->cmdidx == 12) { // TODO What is this 12? MMC_CMD_STOP_TRANSMISSION ?
		return 0;
	}

	if (!cmd->cmdidx) {
		cmdval |= CMD_SEND_INIT_SEQ;
	}

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		cmdval |= CMD_RESP_RCV;
	}

	if (cmd->resp_type & MMC_RSP_136) {
		cmdval |= CMD_LONG_RESP;
	}

	if (cmd->resp_type & MMC_RSP_CRC) {
		cmdval |= CMD_CHK_RESP_CRC;
	}

	if (data) {
		if ((uint32_t) data->b.dest & 0x3) {
			mmcinfo("dest is not 4 byte align\n");
			error = -1;
			goto out;
		}

		cmdval |= CMD_DATA_TRANS | CMD_WAIT_PRE_OVER;

		if (data->flags & MMC_DATA_WRITE) {
			cmdval |= CMD_TRANS_WRITE;
		}

		if (data->blocks > 1) {
			cmdval |= CMD_STOP_CMD_FLAG;
		}

		H3_SD_MMC0->BKS = data->blocksize;
		H3_SD_MMC0->BYC = data->blocks * data->blocksize;

	}

	mmcdbg("cmd %d(0x%x), arg 0x%x\n", cmd->cmdidx, cmdval|cmd->cmdidx, cmd->cmdarg);
	H3_SD_MMC0->ARG = cmd->cmdarg;

	if (!data) {
		H3_SD_MMC0->CMD = cmdval | (cmd->cmdidx & CMD_CMD_IDX_MASK);
	}

	if (data) {
		int ret = 0;
		mmcdbg("trans data %d bytes\n", data->blocksize * data->blocks);

		H3_SD_MMC0->GCTL |= GCTL_FIFO_AC_MODE_AHB;
		H3_SD_MMC0->CMD = cmdval | (cmd->cmdidx & CMD_CMD_IDX_MASK);

		ret = mmc_trans_data_by_cpu(mmc, data);

		if (ret) {
			mmcinfo("Transfer failed\n");

			error = H3_SD_MMC0->RIS & RIS_RAW_ISTA;

			if (!error) {
				error = 0xffffffff;
			}

			goto out;
		}
	}

	timeout = 0xffffff;

	do {
		status = H3_SD_MMC0->RIS;

		if (!timeout-- || (status & RIS_RAW_ISTA)) {
			error = status & RIS_RAW_ISTA;

			if(!error) {
				error = 0xffffffff;
			}

			mmcinfo("cmd %d timeout, err %x\n", cmd->cmdidx, error);

			goto out;
		}
	} while (!(status & RIS_CMD_COMPLETE));

	if (data) {
		uint32_t done = 0;
		timeout = 0xffff;

		do {
			status = H3_SD_MMC0->RIS;

			if (!timeout-- || (status & RIS_RAW_ISTA)) {
				error = status & RIS_RAW_ISTA;

				if(!error) {
					error = 0xffffffff;
				}

				mmcinfo("data timeout, err %x\n", error);

				goto out;
			}

			if (data->blocks > 1) {
				done = status & RIS_AUTO_CMD_DONE;
			} else {
				done = status & RIS_TRANS_COMPLETE;
			}

		} while (!done);
	}

	if (cmd->resp_type & MMC_RSP_BUSY) {
		timeout = 0x4ffffff;
		do {
			status = H3_SD_MMC0->STA;

			if (!timeout--) {
				error = -1;
				mmcinfo("busy timeout\n");
				goto out;
			}
		} while (status & RIS_DATA_TIMEOUT);
	}

	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = H3_SD_MMC0->RESP3;
		cmd->response[1] = H3_SD_MMC0->RESP2;
		cmd->response[2] = H3_SD_MMC0->RESP1;
		cmd->response[3] = H3_SD_MMC0->RESP0;
		mmcdbg("resp 0x%x 0x%x 0x%x 0x%x\n", cmd->response[3], cmd->response[2], cmd->response[1], cmd->response[0]);
	} else {
		cmd->response[0] = H3_SD_MMC0->RESP0;
		mmcdbg("resp 0x%x\n", cmd->response[0]);
	}

out:

	if (error) {
		dumphex32("MMC0_BASE", (char *) H3_SD_MMC0_BASE, 0x100);

		H3_SD_MMC0->GCTL = GCTL_RESET;
		while (H3_SD_MMC0->GCTL & GCTL_RESET)
			;

		mmc_update_clk();
		mmcinfo("cmd %d err %x\n", cmd->cmdidx, error);
	}

	H3_SD_MMC0->RIS = 0xffffffff;

	if (error) {
		return -1;
	}

	return 0;
}

int sunxi_mmc_init(void) {
	DEBUG_ENTRY

	mmcinfo("mmc driver ver %s\n",__DATE__ " " __TIME__);

	memset(&mmc_dev, 0, sizeof(struct mmc));
	memset(&_aw_mmc_host, 0, sizeof(struct sunxi_mmc_host));

	strcpy(mmc_dev.name, "SUNXI SD/MMC");

	mmc_dev.priv = &_aw_mmc_host;

	mmc_dev.send_cmd = mmc_send_cmd;
	mmc_dev.set_ios = mmc_set_ios;
	mmc_dev.init = mmc_core_init;

	mmc_dev.voltages = MMC_VDD_29_30 | MMC_VDD_30_31 | MMC_VDD_31_32
			| MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36;
	mmc_dev.host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_HC;
	mmc_dev.host_caps |= MMC_MODE_4BIT;
	mmc_dev.f_min = 400000;
	mmc_dev.f_max = 25000000;
	mmc_dev.control_num = 0;

	mmc_ccu_init();

	int ret = mmc_register(0, &mmc_dev);

	if (ret < 0) {
		mmcinfo("register failed\n",0);
		DEBUG_EXIT
		return -1;
	}

	DEBUG_EXIT

	return mmc_dev.lba;
}
