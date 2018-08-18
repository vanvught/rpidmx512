/**
 * @file sd.c
 *
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 * Based on
 * https://github.com/jncronin/rpi-boot/blob/master/emmc.c
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

#include "bcm2835.h"
#include "bcm2835_vc.h"
#include "bcm2835_emmc.h"

#include "device/sd.h"
#include "device/sdhci.h"

#include "util.h"

#define TIMEOUT_WAIT(stop_if_true, usec) 						\
do {															\
	uint32_t micros_now = BCM2835_ST->CLO;						\
	do {														\
		if(stop_if_true)										\
			break;												\
	} while( BCM2835_ST->CLO - micros_now < (uint32_t)usec);	\
} while(0);

struct sd_scr {
	uint32_t scr[2];
	uint32_t sd_bus_widths;
	int sd_version;
};

struct block_device {
	size_t block_size;
};

struct emmc_block_dev {
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
	int card_removal;
};

#define SD_CLOCK_ID         	4000000
#define SD_CLOCK_NORMAL     	25000000
#define SD_CLOCK_HIGH       	50000000
#define SD_CLOCK_100        	100000000
#define SD_CLOCK_208        	208000000

#define SD_VER_UNKNOWN      	0
#define SD_VER_1            	1
#define SD_VER_1_1          	2
#define SD_VER_2            	3
#define SD_VER_3            	4
#define SD_VER_4            	5

#define SD_CMD_INDEX(a)				(uint32_t)((a) << 24)
#define SD_CMD_TYPE_NORMAL			0x0
#define SD_CMD_TYPE_SUSPEND			(1 << 22)
#define SD_CMD_TYPE_RESUME			(2 << 22)
#define SD_CMD_TYPE_ABORT			(3 << 22)
#define SD_CMD_TYPE_MASK			(3 << 22)
#define SD_CMD_ISDATA				(1 << 21)
#define SD_CMD_IXCHK_EN				(1 << 20)
#define SD_CMD_CRCCHK_EN			(1 << 19)
#define SD_CMD_RSPNS_TYPE_NONE		0			// For no response
#define SD_CMD_RSPNS_TYPE_136		(1 << 16)	// For response R2 (with CRC), R3,4 (no CRC)
#define SD_CMD_RSPNS_TYPE_48		(2 << 16)	// For responses R1, R5, R6, R7 (with CRC)
#define SD_CMD_RSPNS_TYPE_48B		(3 << 16)	// For responses R1b, R5b (with CRC)
#define SD_CMD_RSPNS_TYPE_MASK 		(3 << 16)
#define SD_CMD_MULTI_BLOCK			(1 << 5)
#define SD_CMD_DAT_DIR_HC			0
#define SD_CMD_DAT_DIR_CH			(1 << 4)
#define SD_CMD_AUTO_CMD_EN_NONE		0
#define SD_CMD_AUTO_CMD_EN_CMD12	(1 << 2)
#define SD_CMD_AUTO_CMD_EN_CMD23	(2 << 2)
#define SD_CMD_BLKCNT_EN			(1 << 1)
#define SD_CMD_DMA          		1

#define SD_ERR_CMD_TIMEOUT		0
#define SD_ERR_CMD_CRC			1
#define SD_ERR_CMD_END_BIT		2
#define SD_ERR_CMD_INDEX		3
#define SD_ERR_DATA_TIMEOUT		4
#define SD_ERR_DATA_CRC			5
#define SD_ERR_DATA_END_BIT		6
#define SD_ERR_CURRENT_LIMIT	7
#define SD_ERR_AUTO_CMD12		8
#define SD_ERR_ADMA				9
#define SD_ERR_TUNING			10
#define SD_ERR_RSVD				11

#define SD_ERR_MASK_CMD_TIMEOUT		(1 << (16 + SD_ERR_CMD_TIMEOUT))
#define SD_ERR_MASK_CMD_CRC			(1 << (16 + SD_ERR_CMD_CRC))
#define SD_ERR_MASK_CMD_END_BIT		(1 << (16 + SD_ERR_CMD_END_BIT))
#define SD_ERR_MASK_CMD_INDEX		(1 << (16 + SD_ERR_CMD_INDEX))
#define SD_ERR_MASK_DATA_TIMEOUT	(1 << (16 + SD_ERR_CMD_TIMEOUT))
#define SD_ERR_MASK_DATA_CRC		(1 << (16 + SD_ERR_CMD_CRC))
#define SD_ERR_MASK_DATA_END_BIT	(1 << (16 + SD_ERR_CMD_END_BIT))
#define SD_ERR_MASK_CURRENT_LIMIT	(1 << (16 + SD_ERR_CMD_CURRENT_LIMIT))
#define SD_ERR_MASK_AUTO_CMD12		(1 << (16 + SD_ERR_CMD_AUTO_CMD12))
#define SD_ERR_MASK_ADMA			(1 << (16 + SD_ERR_CMD_ADMA))
#define SD_ERR_MASK_TUNING			(1 << (16 + SD_ERR_CMD_TUNING))

#define SD_RESP_NONE        SD_CMD_RSPNS_TYPE_NONE
#define SD_RESP_R1          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R1b         (SD_CMD_RSPNS_TYPE_48B | SD_CMD_CRCCHK_EN)
#define SD_RESP_R2          (SD_CMD_RSPNS_TYPE_136 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R3          SD_CMD_RSPNS_TYPE_48
#define SD_RESP_R4          SD_CMD_RSPNS_TYPE_136
#define SD_RESP_R5          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R5b         (SD_CMD_RSPNS_TYPE_48B | SD_CMD_CRCCHK_EN)
#define SD_RESP_R6          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)
#define SD_RESP_R7          (SD_CMD_RSPNS_TYPE_48 | SD_CMD_CRCCHK_EN)

#define SD_DATA_READ        (SD_CMD_ISDATA | SD_CMD_DAT_DIR_CH)
#define SD_DATA_WRITE       (SD_CMD_ISDATA | SD_CMD_DAT_DIR_HC)

#define SD_CMD_RESERVED(a)  (uint32_t)0xffffffff

#define SD_COMMAND_COMPLETE		(1 << 0)
#define SD_TRANSFER_COMPLETE	(1 << 1)
#define SD_BLOCK_GAP_EVENT		(1 << 2)
#define SD_DMA_INTERRUPT		(1 << 3)
#define SD_BUFFER_WRITE_READY	(1 << 4)
#define SD_BUFFER_READ_READY	(1 << 5)
#define SD_CARD_INSERTION		(1 << 6)
#define SD_CARD_REMOVAL			(1 << 7)
#define SD_CARD_INTERRUPT		(1 << 8)

#define SUCCESS(a)          	(a->last_cmd_success)
#define FAIL(a)            	 	(a->last_cmd_success == 0)
#define TIMEOUT(a)         		(FAIL(a) && (a->last_error == 0))
#define CMD_TIMEOUT(a)     		(FAIL(a) && (a->last_error & (1 << 16)))
#define CMD_CRC(a)        	 	(FAIL(a) && (a->last_error & (1 << 17)))
#define CMD_END_BIT(a)     		(FAIL(a) && (a->last_error & (1 << 18)))
#define CMD_INDEX(a)       		(FAIL(a) && (a->last_error & (1 << 19)))
#define DATA_TIMEOUT(a)  		(FAIL(a) && (a->last_error & (1 << 20)))
#define DATA_CRC(a)				(FAIL(a) && (a->last_error & (1 << 21)))
#define DATA_END_BIT(a)			(FAIL(a) && (a->last_error & (1 << 22)))
#define CURRENT_LIMIT(a)		(FAIL(a) && (a->last_error & (1 << 23)))
#define ACMD12_ERROR(a)			(FAIL(a) && (a->last_error & (1 << 24)))
#define ADMA_ERROR(a)			(FAIL(a) && (a->last_error & (1 << 25)))
#define TUNING_ERROR(a)			(FAIL(a) && (a->last_error & (1 << 26)))

// The actual command indices
#define GO_IDLE_STATE           0
#define ALL_SEND_CID            2
#define SEND_RELATIVE_ADDR      3
#define SET_DSR                 4
#define IO_SET_OP_COND          5
#define SWITCH_FUNC             6
#define SELECT_CARD             7
#define DESELECT_CARD           7
#define SELECT_DESELECT_CARD    7
#define SEND_IF_COND            8
#define SEND_CSD                9
#define SEND_CID                10
#define VOLTAGE_SWITCH          11
#define STOP_TRANSMISSION       12
#define SEND_STATUS             13
#define GO_INACTIVE_STATE       15
#define SET_BLOCKLEN            16
#define READ_SINGLE_BLOCK       17
#define READ_MULTIPLE_BLOCK     18
#define SEND_TUNING_BLOCK       19
#define SPEED_CLASS_CONTROL     20
#define SET_BLOCK_COUNT         23
#define WRITE_BLOCK             24
#define WRITE_MULTIPLE_BLOCK    25
#define PROGRAM_CSD             27
#define SET_WRITE_PROT          28
#define CLR_WRITE_PROT          29
#define SEND_WRITE_PROT         30
#define ERASE_WR_BLK_START      32
#define ERASE_WR_BLK_END        33
#define ERASE                   38
#define LOCK_UNLOCK             42
#define APP_CMD                 55
#define GEN_CMD                 56

#define IS_APP_CMD              0x80000000
#define ACMD(a)                 (a | IS_APP_CMD)
#define SET_BUS_WIDTH           (6 | IS_APP_CMD)
#define SD_STATUS               (13 | IS_APP_CMD)
#define SEND_NUM_WR_BLOCKS      (22 | IS_APP_CMD)
#define SET_WR_BLK_ERASE_COUNT  (23 | IS_APP_CMD)
#define SD_SEND_OP_COND         (41 | IS_APP_CMD)
#define SET_CLR_CARD_DETECT     (42 | IS_APP_CMD)
#define SEND_SCR                (51 | IS_APP_CMD)

#define SD_GET_CLOCK_DIVIDER_FAIL	0xffffffff

const uint32_t sd_commands[] __attribute__((aligned(4))) = {
    SD_CMD_INDEX(0),
    SD_CMD_RESERVED(1),
    SD_CMD_INDEX(2) | SD_RESP_R2,
    SD_CMD_INDEX(3) | SD_RESP_R6,
    SD_CMD_INDEX(4),
    SD_CMD_INDEX(5) | SD_RESP_R4,
    SD_CMD_INDEX(6) | SD_RESP_R1,
    SD_CMD_INDEX(7) | SD_RESP_R1b,
    SD_CMD_INDEX(8) | SD_RESP_R7,
    SD_CMD_INDEX(9) | SD_RESP_R2,
    SD_CMD_INDEX(10) | SD_RESP_R2,
    SD_CMD_INDEX(11) | SD_RESP_R1,
    SD_CMD_INDEX(12) | SD_RESP_R1b | SD_CMD_TYPE_ABORT,
    SD_CMD_INDEX(13) | SD_RESP_R1,
    SD_CMD_RESERVED(14),
    SD_CMD_INDEX(15),
    SD_CMD_INDEX(16) | SD_RESP_R1,
    SD_CMD_INDEX(17) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_INDEX(18) | SD_RESP_R1 | SD_DATA_READ | SD_CMD_MULTI_BLOCK | SD_CMD_BLKCNT_EN,
    SD_CMD_INDEX(19) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_INDEX(20) | SD_RESP_R1b,
    SD_CMD_RESERVED(21),
    SD_CMD_RESERVED(22),
    SD_CMD_INDEX(23) | SD_RESP_R1,
    SD_CMD_INDEX(24) | SD_RESP_R1 | SD_DATA_WRITE,
    SD_CMD_INDEX(25) | SD_RESP_R1 | SD_DATA_WRITE | SD_CMD_MULTI_BLOCK | SD_CMD_BLKCNT_EN,
    SD_CMD_RESERVED(26),
    SD_CMD_INDEX(27) | SD_RESP_R1 | SD_DATA_WRITE,
    SD_CMD_INDEX(28) | SD_RESP_R1b,
    SD_CMD_INDEX(29) | SD_RESP_R1b,
    SD_CMD_INDEX(30) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_RESERVED(31),
    SD_CMD_INDEX(32) | SD_RESP_R1,
    SD_CMD_INDEX(33) | SD_RESP_R1,
    SD_CMD_RESERVED(34),
    SD_CMD_RESERVED(35),
    SD_CMD_RESERVED(36),
    SD_CMD_RESERVED(37),
    SD_CMD_INDEX(38) | SD_RESP_R1b,
    SD_CMD_RESERVED(39),
    SD_CMD_RESERVED(40),
    SD_CMD_RESERVED(41),
    SD_CMD_RESERVED(42) | SD_RESP_R1,
    SD_CMD_RESERVED(43),
    SD_CMD_RESERVED(44),
    SD_CMD_RESERVED(45),
    SD_CMD_RESERVED(46),
    SD_CMD_RESERVED(47),
    SD_CMD_RESERVED(48),
    SD_CMD_RESERVED(49),
    SD_CMD_RESERVED(50),
    SD_CMD_RESERVED(51),
    SD_CMD_RESERVED(52),
    SD_CMD_RESERVED(53),
    SD_CMD_RESERVED(54),
    SD_CMD_INDEX(55) | SD_RESP_R1,
    SD_CMD_INDEX(56) | SD_RESP_R1 | SD_CMD_ISDATA,
    SD_CMD_RESERVED(57),
    SD_CMD_RESERVED(58),
    SD_CMD_RESERVED(59),
    SD_CMD_RESERVED(60),
    SD_CMD_RESERVED(61),
    SD_CMD_RESERVED(62),
    SD_CMD_RESERVED(63)
};

static const uint32_t sd_acommands[] __attribute__((aligned(4))) = {
    SD_CMD_RESERVED(0),
    SD_CMD_RESERVED(1),
    SD_CMD_RESERVED(2),
    SD_CMD_RESERVED(3),
    SD_CMD_RESERVED(4),
    SD_CMD_RESERVED(5),
    SD_CMD_INDEX(6) | SD_RESP_R1,
    SD_CMD_RESERVED(7),
    SD_CMD_RESERVED(8),
    SD_CMD_RESERVED(9),
    SD_CMD_RESERVED(10),
    SD_CMD_RESERVED(11),
    SD_CMD_RESERVED(12),
    SD_CMD_INDEX(13) | SD_RESP_R1,
    SD_CMD_RESERVED(14),
    SD_CMD_RESERVED(15),
    SD_CMD_RESERVED(16),
    SD_CMD_RESERVED(17),
    SD_CMD_RESERVED(18),
    SD_CMD_RESERVED(19),
    SD_CMD_RESERVED(20),
    SD_CMD_RESERVED(21),
    SD_CMD_INDEX(22) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_INDEX(23) | SD_RESP_R1,
    SD_CMD_RESERVED(24),
    SD_CMD_RESERVED(25),
    SD_CMD_RESERVED(26),
    SD_CMD_RESERVED(27),
    SD_CMD_RESERVED(28),
    SD_CMD_RESERVED(29),
    SD_CMD_RESERVED(30),
    SD_CMD_RESERVED(31),
    SD_CMD_RESERVED(32),
    SD_CMD_RESERVED(33),
    SD_CMD_RESERVED(34),
    SD_CMD_RESERVED(35),
    SD_CMD_RESERVED(36),
    SD_CMD_RESERVED(37),
    SD_CMD_RESERVED(38),
    SD_CMD_RESERVED(39),
    SD_CMD_RESERVED(40),
    SD_CMD_INDEX(41) | SD_RESP_R3,
    SD_CMD_INDEX(42) | SD_RESP_R1,
    SD_CMD_RESERVED(43),
    SD_CMD_RESERVED(44),
    SD_CMD_RESERVED(45),
    SD_CMD_RESERVED(46),
    SD_CMD_RESERVED(47),
    SD_CMD_RESERVED(48),
    SD_CMD_RESERVED(49),
    SD_CMD_RESERVED(50),
    SD_CMD_INDEX(51) | SD_RESP_R1 | SD_DATA_READ,
    SD_CMD_RESERVED(52),
    SD_CMD_RESERVED(53),
    SD_CMD_RESERVED(54),
    SD_CMD_RESERVED(55),
    SD_CMD_RESERVED(56),
    SD_CMD_RESERVED(57),
    SD_CMD_RESERVED(58),
    SD_CMD_RESERVED(59),
    SD_CMD_RESERVED(60),
    SD_CMD_RESERVED(61),
    SD_CMD_RESERVED(62),
    SD_CMD_RESERVED(63)
};

static uint32_t base_clock;
static struct sd_scr sdcard_scr  __attribute__((aligned(4)));
static struct emmc_block_dev block_dev __attribute__((aligned(4)));

#define MIN_FREQ 400000
#define BCM2835_EMMC_WRITE_DELAY       (((2 * 1000000) / MIN_FREQ) + 1)

//#define EMMC_DEBUG
//#define SD_DEBUG

#ifdef DEBUG
#define EMMC_DEBUG
#define SD_DEBUG
#endif

#if defined (EMMC_DEBUG) || defined (SD_DEBUG)
extern int printf(const char *format, ...);
#endif

#ifdef EMMC_DEBUG
#define EMMC_TRACE(...)     {										\
        printf("EMMC %s:%4d[%s] : ", __FILE__, __LINE__, __func__);	\
        printf(__VA_ARGS__);										\
        printf("\n"); }
#else
#define EMMC_TRACE(...)
#endif

#ifdef SD_DEBUG
#define SD_TRACE(...)     {											\
        printf("SD   %s:%4d[%s] : ", __FILE__, __LINE__, __func__);	\
        printf(__VA_ARGS__);										\
        printf("\n"); }
#else
#define SD_TRACE(...)
#endif

// Enable 1.8V support
//#define SD_1_8V_SUPPORT

// Enable 4-bit support
#define SD_4BIT_DATA

// Enable SDXC maximum performance mode
#define SDXC_MAXIMUM_PERFORMANCE

// Enable EXPERIMENTAL (and possibly DANGEROUS) SD write support
//#define SD_WRITE_SUPPORT

#ifdef SD_DEBUG
static char *sd_versions[] = { "unknown", "1.0 and 1.01", "1.10", "2.00", "3.0x", "4.xx" };

static char *err_irpts[] = { "CMD_TIMEOUT", "CMD_CRC", "CMD_END_BIT", "CMD_INDEX",
	"DATA_TIMEOUT", "DATA_CRC", "DATA_END_BIT", "CURRENT_LIMIT",
	"AUTO_CMD12", "ADMA", "TUNING", "RSVD" };
#endif

/**
 * @ingroup EMMC
 * @return
 */
static uint32_t emmc_get_version(void) {
	const uint32_t ver = BCM2835_EMMC->SLOTISR_VER;
	uint32_t sdversion = (ver & BCM2835_EMMC_SLOTISR_VER_SDVERSION_MASK) >> BCM2835_EMMC_SLOTISR_VER_SDVERSION_SHIFT;
#ifdef EMMC_DEBUG
	uint32_t vendor = (ver & BCM2835_EMMC_SLOTISR_VER_VENDOR_MASK) >> BCM2835_EMMC_SLOTISR_VER_VENDOR_SHIFT;
	uint32_t slot_status = ver & BCM2835_EMMC_SLOTISR_VER_SLOT_STATUS_MASK;
	EMMC_TRACE("vendor %x, sdversion %x, slot_status %x", (uint8_t)vendor, (uint8_t)sdversion, (uint8_t)slot_status);
#endif

	return sdversion;
}

/**
 * @ingroup EMMC
 *
 * @param target_rate
 * @return divider
 */
static uint32_t emmc_get_clock_divider(const uint32_t target_clock) {
	uint32_t targetted_divisor = 0;

	if (target_clock > base_clock) {
		targetted_divisor = 1;
	}
	else {
		targetted_divisor = base_clock / target_clock;
		uint32_t mod = base_clock % target_clock;
		if (mod) {
			targetted_divisor++;
		}
	}

    int divisor = -1;
    int first_bit;

	for (first_bit = 31; first_bit >= 0; first_bit--) {
		uint32_t bit_test = (1 << first_bit);
		if (targetted_divisor & bit_test) {
			divisor = first_bit;
			targetted_divisor &= ~bit_test;
			if (targetted_divisor) {
				// The divisor is not a power-of-two, increase it
				divisor++;
			}
			break;
		}
	}

    if(divisor == -1) {
        divisor = 31;
    }

    if(divisor >= 32) {
        divisor = 31;
    }

    if(divisor != 0) {
        divisor = (1 << (divisor - 1));
    }

    if(divisor >= 0x400) {
        divisor = 0x3ff;
    }

	uint32_t ret = (divisor & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT;
	ret |= ((divisor & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN) << SDHCI_DIVIDER_HI_SHIFT;

#ifdef EMMC_DEBUG
	int denominator = 1;
	if (divisor != 0) {
		denominator = divisor * 2;
	}
	int actual_clock = base_clock / denominator;
	EMMC_TRACE("target_clock: %ld, divisor: %x, actual_clock: %d, ret: %x0", target_clock, divisor, actual_clock, (int)ret);
#endif

	return ret;
}

/**
 * @ingroup EMMC
 * @param target_rate
 * @return
 */
static uint32_t emmc_set_clock(const uint32_t target_clock) {
	const uint32_t divider = emmc_get_clock_divider(target_clock);

    if (divider == SD_GET_CLOCK_DIVIDER_FAIL) {
    	EMMC_TRACE("Couldn't get a valid divider for target clock %d Hz", target_clock);
		return -1;
	}

	// Wait for the command inhibit (CMD and DAT) bits to clear
	TIMEOUT_WAIT(!(BCM2835_EMMC->STATUS & (BCM2835_EMMC_STATUS_CMD_INHIBIT || BCM2835_EMMC_STATUS_DATA_INHIBIT)), 1000000);

	if ((BCM2835_EMMC->STATUS & (BCM2835_EMMC_STATUS_CMD_INHIBIT || BCM2835_EMMC_STATUS_DATA_INHIBIT)) != 0) {
		EMMC_TRACE("Timeout waiting for inhibit flags. Status %08x", BCM2835_EMMC->STATUS);
		return -1;
	}

	// Set the SD clock off
	uint32_t control1 = BCM2835_EMMC->CONTROL1;
	control1 &= ~BCM2835_EMMC_CONTROL1_CLK_EN;
	BCM2835_EMMC->CONTROL1 = control1;
	udelay(BCM2835_EMMC_WRITE_DELAY);

	// Write the new divider
	control1 &= ~0xffe0;		// Clear old setting + clock generator select
	control1 |= divider;
	BCM2835_EMMC->CONTROL1 = control1;
	udelay(BCM2835_EMMC_WRITE_DELAY);

	// Enable the SD clock
	control1 |= BCM2835_EMMC_CONTROL1_CLK_EN;
	BCM2835_EMMC->CONTROL1 = control1;
	udelay(BCM2835_EMMC_WRITE_DELAY);

	TIMEOUT_WAIT(BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_CLK_STABLE, 1000000);

	if ((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_CLK_STABLE) == 0) {
		EMMC_TRACE("Controller's clock did not stabilize within 1 second");
		return -1;
	}

	EMMC_TRACE("Successfully set clock rate to %d Hz", target_clock);

	return 0;
}

/**
 * @ingroup EMMC
 * @return
 */
static uint32_t emmc_reset(void) {
	BCM2835_EMMC->CONTROL0 = (uint32_t) 0;
	BCM2835_EMMC->CONTROL2 = (uint32_t) 0;

	 // Send reset host controller and wait for complete.
	uint32_t control1 = BCM2835_EMMC->CONTROL1;
	control1 |= BCM2835_EMMC_CONTROL1_SRST_HC;
	BCM2835_EMMC->CONTROL1 = control1;
	udelay(BCM2835_EMMC_WRITE_DELAY);

	TIMEOUT_WAIT((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_SRST_HC) == 0, 1000000);

	if ((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_SRST_HC) != 0) {
		EMMC_TRACE("Controller failed to reset");
		return -1;
	}

	// Enable internal clock and set data timeout.
	control1 = BCM2835_EMMC->CONTROL1;
	control1 |= BCM2835_EMMC_CONTROL1_CLK_INTLEN;
	control1 |= BCM2835_EMMC_CONTROL1_DATA_TOUNIT_MAX;
	BCM2835_EMMC->CONTROL1 = control1;
	udelay(BCM2835_EMMC_WRITE_DELAY);

	EMMC_TRACE("control0: %08x, control1: %08x, control2: %08x", BCM2835_EMMC->CONTROL0, BCM2835_EMMC->CONTROL1, BCM2835_EMMC->CONTROL2);

	if (emmc_set_clock(SD_CLOCK_ID) < 0) {
		return -1;
	}

	// Mask off sending interrupts to the ARM.
	// Reset interrupts.
	// Have all interrupts sent to the INTERRUPT register.
	BCM2835_EMMC->IRPT_EN = 0;
	BCM2835_EMMC->INTERRUPT = 0xffffffff;
	BCM2835_EMMC->IRPT_MASK = 0xffffffff & (~SD_CARD_INTERRUPT);
	udelay(BCM2835_EMMC_WRITE_DELAY);

	return 0;
}

/**
 * @ingroup EMMC
 * Reset the CMD line
 *
 * @return 0 if successful; -1 otherwise.
 */
static int emmc_reset_cmd(void) {
	uint32_t control1 = BCM2835_EMMC->CONTROL1;
	control1 |= BCM2835_EMMC_CONTROL1_RESET_CMD;
	BCM2835_EMMC->CONTROL1 = control1;
	udelay(BCM2835_EMMC_WRITE_DELAY);

	TIMEOUT_WAIT((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_RESET_CMD) == 0, 1000000);

	if ((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_RESET_CMD) != 0) {
		EMMC_TRACE("CMD line did not reset properly");
		return -1;
	}

	BCM2835_EMMC->INTERRUPT = SD_ERR_MASK_CMD_TIMEOUT;
	udelay(BCM2835_EMMC_WRITE_DELAY);

	return 0;
}

/**
 * @ingroup EMMC
 * Reset the CMD line
 *
 * @return 0 if successful; -1 otherwise.
 */
static int emmc_reset_dat(void) {
	uint32_t control1 = BCM2835_EMMC->CONTROL1;
	control1 |= BCM2835_EMMC_CONTROL1_RESET_DATA;
	BCM2835_EMMC->CONTROL1 = control1;
	udelay(BCM2835_EMMC_WRITE_DELAY);

	TIMEOUT_WAIT((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_RESET_DATA) == 0, 1000000);

	if ((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_RESET_DATA) != 0) {
		EMMC_TRACE("DAT line did not reset properly");
		return -1;
	}

	return 0;
}

#ifdef SD_1_8V_SUPPORT
static uint32_t emmc_switch_voltage(void) {
	EMMC_TRACE("Switching to 1.8V mode");

    EMMC_TRACE("Voltage switch complete");

    return 0;
}
#endif

/**
 * @ingroup EMMC
 * @param dev
 * @param cmd_reg
 * @param argument
 * @param timeout
 */
static void emmc_issue_command(uint32_t cmd_reg, uint32_t argument, uint32_t timeout) {
	struct emmc_block_dev *dev = &block_dev;

    dev->last_cmd_reg = cmd_reg;
    dev->last_cmd_success = 0;

    // This is as per HCSS 3.7.1.1/3.7.2.2

    // Check Command Inhibit
    while(BCM2835_EMMC->STATUS & BCM2835_EMMC_STATUS_CMD_INHIBIT)
    	;

	// Is the command with busy?
	if ((cmd_reg & SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B) {
		// With busy
		// Is is an abort command?
		if ((cmd_reg & SD_CMD_TYPE_MASK) != SD_CMD_TYPE_ABORT) {
			// Not an abort command
			// Wait for the data line to be free
			while (BCM2835_EMMC->STATUS & BCM2835_EMMC_STATUS_DATA_INHIBIT)
				;
		}
	}

    // Set block size and block count
    // For now, block size = 512 bytes, block count = 1,
	if (dev->blocks_to_transfer > 0xffff) {
		EMMC_TRACE("Blocks_to_transfer too great (%i)", dev->blocks_to_transfer);
		dev->last_cmd_success = 0;
		return;
	}

    uint32_t blksizecnt = dev->block_size | (dev->blocks_to_transfer << 16);
    BCM2835_EMMC->BLKSIZECNT = blksizecnt;

    // Set argument 1 reg
    BCM2835_EMMC->ARG1 = argument;

    // Set command reg
    BCM2835_EMMC->CMDTM = cmd_reg;
	udelay(BCM2835_EMMC_WRITE_DELAY);

    EMMC_TRACE("Wait for command complete interrupt");
    TIMEOUT_WAIT(BCM2835_EMMC->INTERRUPT & 0x8001, timeout);

    // Save the interrupt status
    uint32_t irpts = BCM2835_EMMC->INTERRUPT;

    // Clear command complete status
    BCM2835_EMMC->INTERRUPT = 0xffff0001;

    // Test for errors
	if ((irpts & 0xffff0001) != 0x1) {
		EMMC_TRACE("Error occurred whilst waiting for command complete interrupt");
		dev->last_error = irpts & 0xffff0000;
		dev->last_interrupt = irpts;
		return;
	}

    // Get response data
	switch (cmd_reg & SD_CMD_RSPNS_TYPE_MASK) {
	case SD_CMD_RSPNS_TYPE_48:
	case SD_CMD_RSPNS_TYPE_48B:
		dev->last_r0 = BCM2835_EMMC->RESP0;
		break;
	case SD_CMD_RSPNS_TYPE_136:
		dev->last_r0 = BCM2835_EMMC->RESP0;
		dev->last_r1 = BCM2835_EMMC->RESP1;
		dev->last_r2 = BCM2835_EMMC->RESP2;
		dev->last_r3 = BCM2835_EMMC->RESP3;
		break;
	}

    // If with data, wait for the appropriate interrupt
    if(cmd_reg & SD_CMD_ISDATA) {
        uint32_t wr_irpt;
        int is_write = 0;

		if (cmd_reg & SD_CMD_DAT_DIR_CH) {
			wr_irpt = (1 << 5);     // read
		} else {
			is_write = 1;
			wr_irpt = (1 << 4);     // write
		}

        int cur_block = 0;
        uint32_t *cur_buf_addr = (uint32_t *)dev->buf;
		while (cur_block < dev->blocks_to_transfer) {
#ifdef SDCARD_DEBUG
			if(dev->blocks_to_transfer > 1)
				EMMC_TRACE("Multi block transfer, awaiting block %i ready", cur_block);
#endif
			TIMEOUT_WAIT(BCM2835_EMMC->INTERRUPT & (wr_irpt | 0x8000), timeout);
			irpts = BCM2835_EMMC->INTERRUPT;
			BCM2835_EMMC->INTERRUPT = 0xffff0000 | wr_irpt;

			if ((irpts & (0xffff0000 | wr_irpt)) != wr_irpt) {
            	EMMC_TRACE("Error occurred whilst waiting for data ready interrupt");
                dev->last_error = irpts & 0xffff0000;
                dev->last_interrupt = irpts;
                return;
            }

            // Transfer the block
            size_t cur_byte_no = 0;
			while (cur_byte_no < dev->block_size) {
				if (is_write) {
					BCM2835_EMMC->DATA = *cur_buf_addr;
				} else {
					*cur_buf_addr = BCM2835_EMMC->DATA;
				}
				cur_byte_no += 4;
				cur_buf_addr++;
			}

            EMMC_TRACE("block %d transfer complete", cur_block);

            cur_block++;
        }
    }

    // Wait for transfer complete (set if read/write transfer or with busy)
    if((((cmd_reg & SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B) || (cmd_reg & SD_CMD_ISDATA))) {
        // First check command inhibit (DAT) is not already 0
		if ((BCM2835_EMMC->STATUS & BCM2835_EMMC_STATUS_DATA_INHIBIT) == 0) {
			BCM2835_EMMC->INTERRUPT = 0xffff0002;
		} else {
        	TIMEOUT_WAIT(BCM2835_EMMC->INTERRUPT & 0x8002, timeout);
        	irpts = BCM2835_EMMC->INTERRUPT;
        	BCM2835_EMMC->INTERRUPT = 0xffff0002;

            // Handle the case where both data timeout and transfer complete
            //  are set - transfer complete overrides data timeout: HCSS 2.2.17
			if (((irpts & 0xffff0002) != 0x2) && ((irpts & 0xffff0002) != 0x100002)) {
            	EMMC_TRACE("Error occurred whilst waiting for transfer complete interrupt");
                dev->last_error = irpts & 0xffff0000;
                dev->last_interrupt = irpts;
                return;
            }

            BCM2835_EMMC->INTERRUPT = 0xffff0002;
        }
    }

    // Return success
    dev->last_cmd_success = 1;
}

/**
 * @ingroup EMMC
 * @param dev
 */
static void emmc_handle_card_interrupt(struct emmc_block_dev *dev) {

	EMMC_TRACE("Controller status: %08x", BCM2835_EMMC->STATUS); EMMC_TRACE("Get the card status");

	if (dev->card_rca) {
		emmc_issue_command(sd_commands[SEND_STATUS], dev->card_rca << 16, 500000);

		if (FAIL(dev)) {
			EMMC_TRACE("Unable to get card status");
		} else {
			EMMC_TRACE("Card status: %08x", dev->last_r0);
		}
	} else {
		EMMC_TRACE("No card currently selected");
	}
}

/**
 * @ingroup EMMC
 * @param dev
 */
static void emmc_handle_interrupts() {
	struct emmc_block_dev *dev = &block_dev;

	const uint32_t irpts = BCM2835_EMMC->INTERRUPT;
	uint32_t reset_mask = 0;

	if (irpts & SD_COMMAND_COMPLETE) {
		EMMC_TRACE("Spurious command complete interrupt");
		reset_mask |= SD_COMMAND_COMPLETE;
	}

	if (irpts & SD_TRANSFER_COMPLETE) {
		EMMC_TRACE("Spurious transfer complete interrupt");
		reset_mask |= SD_TRANSFER_COMPLETE;
	}

	if (irpts & SD_BLOCK_GAP_EVENT) {
		EMMC_TRACE("Spurious block gap event interrupt");
		reset_mask |= SD_BLOCK_GAP_EVENT;
	}

	if (irpts & SD_DMA_INTERRUPT) {
		EMMC_TRACE("Spurious DMA interrupt");
		reset_mask |= SD_DMA_INTERRUPT;
	}

	if (irpts & SD_BUFFER_WRITE_READY) {
		EMMC_TRACE("Spurious buffer write ready interrupt");
		reset_mask |= SD_BUFFER_WRITE_READY;
		emmc_reset_dat();
	}

	if (irpts & SD_BUFFER_READ_READY) {
		EMMC_TRACE("Spurious buffer read ready interrupt");
		reset_mask |= SD_BUFFER_READ_READY;
		emmc_reset_dat();
	}

	if (irpts & SD_CARD_INSERTION) {
		EMMC_TRACE("Card insertion detected");
		reset_mask |= SD_CARD_INSERTION;
	}

	if (irpts & SD_CARD_REMOVAL) {
		EMMC_TRACE("Card removal detected");
		reset_mask |= SD_CARD_REMOVAL;
		dev->card_removal = 1;
	}

	if (irpts & SD_CARD_INTERRUPT) {
		EMMC_TRACE("Card interrupt detected");
		emmc_handle_card_interrupt(dev);
		reset_mask |= SD_CARD_INTERRUPT;
	}

	if (irpts & 0x8000) {
		EMMC_TRACE("Spurious error interrupt: %08x", irpts);
		reset_mask |= 0xffff0000;
	}

	BCM2835_EMMC->INTERRUPT = reset_mask;
}

/**
 * @ingroup EMMC
 * Disable card interrupt in host.
 */
static uint32_t emmc_disable_card_interrupt(void) {
	uint32_t old_irpt_mask = BCM2835_EMMC->IRPT_MASK;
	BCM2835_EMMC->IRPT_MASK = old_irpt_mask & (~SD_CARD_INTERRUPT);

	return old_irpt_mask;
}

/**
 * @ingroup EMMC
 * @param old_irpt_mask
 */
static void emmc_4bit_mode_change_bit(uint32_t old_irpt_mask) {
	uint32_t control0 = BCM2835_EMMC->CONTROL0;
	control0 |= BCM2835_EMMC_CONTROL0_USE_4BITBUS;
	BCM2835_EMMC->CONTROL0 = control0;
	BCM2835_EMMC->IRPT_MASK = old_irpt_mask;
	udelay(BCM2835_EMMC_WRITE_DELAY);
}

/**
 * @ingroup EMMC
 *
 * Please note that the EMMC module restricts the maximum block size to the size of the internal data FIFO which is 1k bytes.
 *
 * @param block_size
 */
static void emmc_set_block_size(uint32_t block_size) {
	uint32_t controller_block_size = BCM2835_EMMC->BLKSIZECNT;
	controller_block_size &= ~BCM2835_EMMC_BLKSIZECNT_BLKSIZE_MASK;
	controller_block_size |= block_size;
	BCM2835_EMMC->BLKSIZECNT = controller_block_size;
	udelay(BCM2835_EMMC_WRITE_DELAY);
}

/**
 * @ingroup EMMC
 */
static void emmc_reset_interrupt_register(void) {
	BCM2835_EMMC->INTERRUPT = 0xffffffff;
}

#if 0
#define GPIO_CD    47

/**
 * @ingroup EMMC
 *
 */
void bcm2835_emmc_init_gpio_cd(void) {
	// Enable input
	uint32_t value = BCM2835_GPIO->GPFSEL4;
	value &= ~(7 << 21);
	value |= BCM2835_GPIO_FSEL_INPT << 21;
	BCM2835_GPIO->GPFSEL4 = value;
	// Enable pull-up
	BCM2835_GPIO->GPPUD = (uint32_t)BCM2835_GPIO_PUD_UP;
	udelay(10);
	BCM2835_GPIO->GPPUDCLK1 = (uint32_t)(1 << (GPIO_CD % 32));
	udelay(10);
	BCM2835_GPIO->GPPUD = (uint32_t)BCM2835_GPIO_PUD_OFF;
	BCM2835_GPIO->GPPUDCLK1 = (uint32_t)(0 << (GPIO_CD % 32));
}

/**
 * @ingroup EMMC
 *
 * Is only working on models with the none micro-sdcard
 *
 * @return
 */
bool bcm2835_emmc_is_sdcard_inserted(void) {
	return (BCM2835_GPIO->GPLEV1 & (1 << (GPIO_CD % 32))) == 0;
}
#endif

/**
 * @ingroup SD
 * @param dev
 * @param command
 * @param argument
 * @param timeout
 */
static void sd_issue_command(uint32_t command, uint32_t argument, uint32_t timeout) {
	struct emmc_block_dev *dev = &block_dev;

	// First, handle any pending interrupts
	emmc_handle_interrupts();

	// Stop the command issue if it was the card remove interrupt that was handled
	if (dev->card_removal) {
		dev->last_cmd_success = 0;
		return;
	}

	if (command & IS_APP_CMD) {
		command &= 0xff;

		SD_TRACE("Issuing command ACMD%d", command);

		if (sd_acommands[command] == SD_CMD_RESERVED(0)) {
			SD_TRACE("Invalid command ACMD%d", command);
			dev->last_cmd_success = 0;
			return;
		}

		dev->last_cmd = APP_CMD;

		uint32_t rca = 0;

		if (dev->card_rca) {
			rca = dev->card_rca << 16;
		}

		emmc_issue_command(sd_commands[APP_CMD], rca, timeout);

		if (dev->last_cmd_success) {
			dev->last_cmd = command | IS_APP_CMD;
			emmc_issue_command(sd_acommands[command], argument, timeout);
		}
	} else {
		SD_TRACE("Issuing command CMD%d", command);

		if (sd_commands[command] == SD_CMD_RESERVED(0)) {
			SD_TRACE("Invalid command CMD%d", command);
			dev->last_cmd_success = 0;
			return;
		}

		dev->last_cmd = command;

		emmc_issue_command(sd_commands[command], argument,	timeout);
	}

#ifdef SD_DEBUG
	if(FAIL(dev))
	{
		SD_TRACE("Error issuing command: interrupts %08x: ", dev->last_interrupt);

		if(dev->last_error == 0) {
			SD_TRACE("TIMEOUT");
		}
		else
		{
			int i;
			for(i = 0; i < SD_ERR_RSVD; i++)
			{
				if(dev->last_error & (1 << (i + 16)))
				{
					SD_TRACE(err_irpts[i]);
				}
			}
		}
	}
	else
	SD_TRACE("Command completed successfully");
#endif
}

/**
 * @ingroup SD
 * @return 0 if successful; -1 otherwise.
 */
static int sd_card_sanity_check(void) {

	if (sizeof(sd_commands) != (64 * sizeof(uint32_t))) {
		SD_TRACE("Fatal error, sd_commands of incorrect size: %d expected %d", sizeof(sd_commands), 64 * sizeof(uint32_t));
        return -1;
    }

	if (sizeof(sd_acommands) != (64 * sizeof(uint32_t))) {
		SD_TRACE("Fatal error, sd_acommands of incorrect size: %d expected %d", sizeof(sd_acommands), 64 * sizeof(uint32_t));
        return -1;
    }

    return 0;
}

/**
 * @ingroup SD
 * @param edev
 * @param is_write
 * @param buf
 * @param buf_size
 * @param block_no
 * @return
 */
static int sd_do_data_command(int is_write, uint8_t *buf, size_t buf_size, uint32_t block_no) {
	struct emmc_block_dev *edev = &block_dev;

	// PLSS table 4.20 - SDSC cards use byte addresses rather than block addresses
	if(!edev->card_supports_sdhc) {
		block_no *= 512;
	}

	// This is as per HCSS 3.7.2.1
	if (buf_size < edev->block_size) {
		SD_TRACE("buffer size (%d) is less than block size (%d)", buf_size, edev->block_size);
        return -1;
	}

	edev->blocks_to_transfer = buf_size / edev->block_size;

	if (buf_size % edev->block_size) {
		SD_TRACE("buffer size (%d) is not an exact multiple of block size (%d)", buf_size, edev->block_size);
		return -1;
	}

	edev->buf = buf;

	// Decide on the command to use
	int command;

	if (is_write) {
		if (edev->blocks_to_transfer > 1) {
			command = WRITE_MULTIPLE_BLOCK;
		} else {
			command = WRITE_BLOCK;
		}
	} else {
		if (edev->blocks_to_transfer > 1) {
			command = READ_MULTIPLE_BLOCK;
		} else {
			command = READ_SINGLE_BLOCK;
		}
	}

	int retry_count = 0;
	int max_retries = 3;

	while (retry_count < max_retries) {
		sd_issue_command(command, block_no, 5000000);

		if (SUCCESS(edev)) {
			break;
		} else {
			SD_TRACE("Error sending CMD%d, edev->last_error = %08x", command, edev->last_error);
			retry_count++;

			if (retry_count < max_retries) {
				SD_TRACE("Retrying...%d", retry_count);
			} else {
				SD_TRACE("Giving up...%d", max_retries);
			}
		}
	}

	if (retry_count == max_retries) {
		edev->card_rca = 0;
		return -1;
	}

	return 0;
}

/**
 * @ingroup SD
 * @param dev
 * @return
 */
int sd_card_init(void) {
	if (sd_card_sanity_check() != 0) {
		return SD_ERROR;
	}

	if (emmc_get_version() < 2) {
		SD_TRACE("Only SDHCI versions >= 3.0 are supported");
		return SD_ERROR;
	}

	base_clock = bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_EMMC);

	SD_TRACE("base_clock = %d", base_clock);

	if (base_clock < 0) {
		SD_TRACE("Could not get the base clock");
		return SD_ERROR;
	}

	if (emmc_reset() != 0) {
		SD_TRACE("Controller did not reset properly");
		return SD_ERROR;
	}

	struct emmc_block_dev *ret = &block_dev;
	memset(ret, 0, sizeof(struct emmc_block_dev));

	ret->bd.block_size = 512;

    /***********************************************************************/
	// Send CMD0 to the card (reset to idle state)
	sd_issue_command(GO_IDLE_STATE, 0, 500000);

	if (FAIL(ret)) {
		SD_TRACE("No CMD0 response");
		return SD_ERROR;
	}
	// Send CMD8 to the card
	// Voltage supplied = 0x1 = 2.7-3.6V (standard)
	// Check pattern = 10101010b (as per PLSS 4.3.13) = 0xAA
	SD_TRACE("Note a timeout error on the following command (CMD8) is normal and expected if the SD card version is less than 2.0");
	sd_issue_command(SEND_IF_COND, 0x1aa, 500000);

	int v2_later = 0;

	if (TIMEOUT(ret)) {
		v2_later = 0;
	} else if (CMD_TIMEOUT(ret)) {
		if (emmc_reset_cmd() == -1) {
			return SD_ERROR;
		}
		v2_later = 0;
	} else if (FAIL(ret)) {
		SD_TRACE("Failure sending CMD8 (%08x)", ret->last_interrupt);
		return SD_ERROR;
	} else {
		if ((ret->last_r0 & 0xfff) != 0x1aa) {
			SD_TRACE("Unusable card, CMD8 response %08x", ret->last_r0);
			return SD_ERROR;
		} else {
			v2_later = 1;
		}
	}

	// Here we are supposed to check the response to CMD5 (HCSS 3.6) page 100
	// It only returns if the card is a SDIO card
	SD_TRACE("Note that a timeout error on the following command (CMD5) is normal and expected if the card is not a SDIO card");
	sd_issue_command(IO_SET_OP_COND, 0, 10000);

	if (!TIMEOUT(ret)) {
		if (CMD_TIMEOUT(ret)) {
			if (emmc_reset_cmd() == -1) {
				return SD_ERROR;
			}
		} else {
			SD_TRACE("SDIO card detected - not currently supported, CMD5 returned %08x", ret->last_r0);
			return SD_ERROR;
		}
	}

    SD_TRACE("Call an inquiry ACMD41 (voltage window = 0) to get the OCR");
    sd_issue_command(ACMD(41), 0, 500000);

	if (FAIL(ret)) {
		SD_TRACE("Inquiry ACMD41 failed");
		return SD_ERROR;
	}

    SD_TRACE("Inquiry ACMD41 returned %08x", ret->last_r0);

	// Call initialization ACMD41
	int card_is_busy = 1;
	while (card_is_busy) {
		uint32_t v2_flags = 0;
		if (v2_later) {
	        // Set SDHC support
	        v2_flags |= (1 << 30);

	        // Set 1.8v support
#ifdef SD_1_8V_SUPPORT
	        if(!ret->failed_voltage_switch)
                v2_flags |= (1 << 24);
#endif

            // Enable SDXC maximum performance
#ifdef SDXC_MAXIMUM_PERFORMANCE
            v2_flags |= (1 << 28);
#endif
	    }

	    sd_issue_command(ACMD(41), 0x00ff8000 | v2_flags, 500000);

		if (FAIL(ret)) {
			SD_TRACE("Error issuing ACMD41");
			return SD_ERROR;
		}

		if ((ret->last_r0 >> 31) & 0x1) {
			// Initialization is complete
			ret->card_ocr = (ret->last_r0 >> 8) & 0xffff;
			ret->card_supports_sdhc = (ret->last_r0 >> 30) & 0x1;

#ifdef SD_1_8V_SUPPORT
			if(!ret->failed_voltage_switch)
			ret->card_supports_18v = (ret->last_r0 >> 24) & 0x1;
#endif

			card_is_busy = 0;
		} else {
			SD_TRACE("Card is busy, retrying");
			udelay(500000);
		}
	}

	SD_TRACE("Card identified: OCR: %04x, 1.8v support: %d, SDHC support: %d", ret->card_ocr, ret->card_supports_18v, ret->card_supports_sdhc);

    // At this point, we know the card is definitely an SD card, so will definitely
	//  support SDR12 mode which runs at 25 MHz
    emmc_set_clock(SD_CLOCK_NORMAL);

#ifdef SD_1_8V_SUPPORT
	// A small wait before the voltage switch
	udelay(5000);

	// Switch to 1.8V mode if possible
	if(ret->card_supports_18v)
	{
		SD_TRACE("SD: switching to 1.8V mode\n");
	    // As per HCSS 3.6.1
	    // Send VOLTAGE_SWITCH
	    sd_issue_command(ret, VOLTAGE_SWITCH, 0, 500000);
	    if(FAIL(ret))
	    {
	    	SD_TRACE("SD: error issuing VOLTAGE_SWITCH");
	        ret->failed_voltage_switch = 1;
	        bcm2835_emmc_sdcard_power_off();
	        return sd_card_init((struct block_device **)&ret);
	    }

	    // Disable SD clock
	    uint32_t control1 = BCM2835_EMMC->CONTROL1;
	    control1 &= ~BCM2835_EMMC_CLOCK_CARD_EN;
	    BCM2835_EMMC->CONTROL1 = control1;

	    // Check DAT[3:0]
	    uint32_t status_reg = BCM2835_EMMC->STATUS;
	    uint32_t dat30 = (status_reg >> 20) & 0xf;

	    if(dat30 != 0)
	    {
	    	SD_TRACE("DAT[3:0] did not settle to 0");
	        ret->failed_voltage_switch = 1;
	        bcm2835_emmc_sdcard_power_off();
	        return sd_card_init((struct block_device **)&ret);
	    }

	    // Set 1.8V signal enable to 1
	    uint32_t control0 = BCM2835_EMMC->CONTROL0;
	    control0 |= (1 << 8);
	    BCM2835_EMMC->CONTROL0 = control0;

	    // Wait 5 ms
	    udelay(5000);

	    // Check the 1.8V signal enable is set
	    control0 = BCM2835_EMMC->CONTROL0;
	    if(((control0 >> 8) & 0x1) == 0)
	    {
	    	SD_TRACE("Controller did not keep 1.8V signal enable high");
	        ret->failed_voltage_switch = 1;
	        bcm2835_emmc_sdcard_power_off();
	        return sd_card_init((struct block_device **)&ret);
	    }

	    // Re-enable the SD clock
	    control1 = BCM2835_EMMC->CONTROL1;
	    control1 |= (1 << 2);
	    BCM2835_EMMC->CONTROL1 = control1;

	    // Wait 1 ms
	    udelay(10000);

	    // Check DAT[3:0]
		status_reg = BCM2835_EMMC->STATUS;
	    dat30 = (status_reg >> 20) & 0xf;
	    if(dat30 != 0xf)
	    {
	    	SD_TRACE("DAT[3:0] did not settle to 1111b (%01x)", dat30);
	        ret->failed_voltage_switch = 1;
	        bcm2835_emmc_sdcard_power_off();
	        return sd_card_init((struct block_device **)&ret);
	    }
	    SD_TRACE("SD: voltage switch complete");
	}
#endif

	SD_TRACE("Send CMD2 to get the cards CID");
	sd_issue_command(ALL_SEND_CID, 0, 500000);

	if (FAIL(ret)) {
		SD_TRACE("Error sending ALL_SEND_CID");
		return SD_ERROR;
	}

#ifdef SD_DEBUG
	uint32_t card_cid_0 = ret->last_r0;
	uint32_t card_cid_1 = ret->last_r1;
	uint32_t card_cid_2 = ret->last_r2;
	uint32_t card_cid_3 = ret->last_r3;
	SD_TRACE("Card CID: %08x%08x%08x%08x", card_cid_3, card_cid_2, card_cid_1, card_cid_0);
#endif

	SD_TRACE("Send CMD3 to enter the data state");
	sd_issue_command(SEND_RELATIVE_ADDR, 0, 500000);

	if (FAIL(ret)) {
		SD_TRACE("Error sending SEND_RELATIVE_ADDR");
		return SD_ERROR;
	}

	uint32_t cmd3_resp = ret->last_r0;
	SD_TRACE("CMD3 response: %08x", cmd3_resp);

	ret->card_rca = (cmd3_resp >> 16) & 0xffff;
	uint32_t crc_error = (cmd3_resp >> 15) & 0x1;
	uint32_t illegal_cmd = (cmd3_resp >> 14) & 0x1;
	uint32_t error = (cmd3_resp >> 13) & 0x1;
	uint32_t status = (cmd3_resp >> 9) & 0xf;
	uint32_t ready = (cmd3_resp >> 8) & 0x1;

	if (crc_error) {
		SD_TRACE("CRC error");
		return SD_ERROR;
	}

	if (illegal_cmd) {
		SD_TRACE("Illegal command");
		return SD_ERROR;
	}

	if (error) {
		SD_TRACE("Generic error");
		return SD_ERROR;
	}

	if (!ready) {
		SD_TRACE("Not ready for data");
		return SD_ERROR;
	}

	SD_TRACE("RCA: %04x", ret->card_rca);

	// Now select the card (toggles it to transfer state)
	sd_issue_command(SELECT_CARD, ret->card_rca << 16, 500000);

	if (FAIL(ret)) {
		SD_TRACE("Error sending CMD7");
		return SD_ERROR;
	}

	uint32_t cmd7_resp = ret->last_r0;
	status = (cmd7_resp >> 9) & 0xf;

	SD_TRACE("Status (%d)", status);

	if ((status != 2) && (status != 3) && (status != 4)) {
		SD_TRACE("Invalid status (%d)", status);
		return SD_ERROR;
	}

	// If not an SDHC card, ensure BLOCKLEN is 512 bytes
	if (!ret->card_supports_sdhc) {
		sd_issue_command(SET_BLOCKLEN, 512, 500000);
		SD_TRACE("CMD16 response: %08x, Statud %d", ret->last_r0, (ret->last_r0 >> 9) & 0xf);
		if (FAIL(ret)) {
			SD_TRACE("Error sending SET_BLOCKLEN");
			return SD_ERROR;
		}
	}

	ret->block_size = 512;

	emmc_set_block_size(ret->block_size);

	// Get the cards SCR register
	// SEND_SCR command is like a READ_SINGLE but for a block of 8 bytes.
	// Ensure that any data operation has completed before reading the block.
	ret->scr = &sdcard_scr;
	ret->buf = &ret->scr->scr[0];
	ret->block_size = 8;
	ret->blocks_to_transfer = 1;

	sd_issue_command(SEND_SCR, 0, 500000);
	SD_TRACE("ACMD51 response: %08x, Status %d", ret->last_r0, (ret->last_r0 >> 9) & 0xf);

	ret->block_size = 512;

	if (FAIL(ret)) {
		SD_TRACE("Error sending SEND_SCR");
		return SD_ERROR;
	}

	// Determine card version
	// Note that the SCR is big-endian
	uint32_t scr0 = __builtin_bswap32(ret->scr->scr[0]);

	ret->scr->sd_version = SD_VER_UNKNOWN;

	uint32_t sd_spec = (scr0 >> (56 - 32)) & 0xf;
	uint32_t sd_spec3 = (scr0 >> (47 - 32)) & 0x1;
	uint32_t sd_spec4 = (scr0 >> (42 - 32)) & 0x1;

	ret->scr->sd_bus_widths = (scr0 >> (48 - 32)) & 0xf;

	if (sd_spec == 0) {
		ret->scr->sd_version = SD_VER_1;
	} else if (sd_spec == 1) {
		ret->scr->sd_version = SD_VER_1_1;
	} else if (sd_spec == 2) {
		if (sd_spec3 == 0) {
			ret->scr->sd_version = SD_VER_2;
		} else if (sd_spec3 == 1) {
			if (sd_spec4 == 0) {
				ret->scr->sd_version = SD_VER_3;
			} else if (sd_spec4 == 1) {
				ret->scr->sd_version = SD_VER_4;
			}
		}
	}

	SD_TRACE("&scr: %08x", &ret->scr->scr[0]);
	SD_TRACE("SCR[0]: %08x, SCR[1]: %08x", ret->scr->scr[0], ret->scr->scr[1]);
	SD_TRACE("SCR: %08x%08x", __builtin_bswap32(ret->scr->scr[0]), __builtin_bswap32(ret->scr->scr[1]));
	SD_TRACE("SCR: version %s, bus_widths %01x", sd_versions[ret->scr->sd_version], ret->scr->sd_bus_widths);

#ifdef SD_4BIT_DATA
	if (ret->scr->sd_bus_widths & 0x4) {
		SD_TRACE("Switching to 4-bit data mode");

		uint32_t old_irpt_mask = emmc_disable_card_interrupt();

		// Send ACMD6 to change the card's bit mode
		sd_issue_command(SET_BUS_WIDTH, 0x2, 500000);

		if (FAIL(ret)) {
			SD_TRACE("Switch to 4-bit data mode failed");
		} else {
			emmc_4bit_mode_change_bit(old_irpt_mask);
			SD_TRACE("Switch to 4-bit complete");
		}
	}
#endif

    SD_TRACE("Found a valid version %s SD card", sd_versions[ret->scr->sd_version]);
	SD_TRACE("Setup successful (status %d)", (ret->last_r0 >> 9) & 0xf);

	emmc_reset_interrupt_register();

	return SD_OK;
}

/**
 * @ingroup SD
 * @param edev
 * @return
 */
static int sd_ensure_data_mode(void) {
	struct emmc_block_dev *edev = &block_dev;

	if (edev->card_rca == 0) {
		int ret = sd_card_init();

		if (ret != 0) {
			return ret;
		}
	}

	SD_TRACE("Obtaining status register for edev->card_rca = %08x", edev->card_rca);

	sd_issue_command(SEND_STATUS, edev->card_rca << 16, 500000);

	if (FAIL(edev)) {
		SD_TRACE("Error sending CMD13");
		edev->card_rca = 0;
		return -1;
	}

	uint32_t status = edev->last_r0;
	uint32_t cur_state = (status >> 9) & 0xf;

	SD_TRACE("status %d", cur_state);

	if (cur_state == 3) {
		// Currently in the stand-by state - select it
		sd_issue_command(SELECT_CARD, edev->card_rca << 16, 500000);
		if (FAIL(edev)) {
			SD_TRACE("No response from CMD17");
			edev->card_rca = 0;
			return -1;
		}
	} else if (cur_state == 5) {
		// In the data transfer state - cancel the transmission
		sd_issue_command(STOP_TRANSMISSION, 0, 500000);
		if (FAIL(edev)) {
			SD_TRACE("No response from CMD12");
			edev->card_rca = 0;
			return -1;
		}

		// Reset the data circuit
		emmc_reset_dat();
	} else if (cur_state != 4) {
		// Not in the transfer state - re-initialize
		int ret = sd_card_init();

		if (ret != 0) {
			return ret;
		}
	}

	// Check again that we're now in the correct mode
	if (cur_state != 4) {
		SD_TRACE("Re-checking status: ");

		sd_issue_command(SEND_STATUS, edev->card_rca << 16, 500000);

		if (FAIL(edev)) {
			SD_TRACE("No response from CMD13");
			edev->card_rca = 0;
			return -1;
		}

		status = edev->last_r0;
		cur_state = (status >> 9) & 0xf;

		SD_TRACE("%d", cur_state);

		if (cur_state != 4) {
			SD_TRACE("Unable to initialize SD card to data mode (state %d)", cur_state);
			edev->card_rca = 0;
			return -1;
		}
	}

	return 0;
}

/**
 * @ingroup SD
 * @param dev
 * @param buf
 * @param buf_size
 * @param block_no
 * @return
 */
int sd_read(uint8_t *buf, size_t buf_size, uint32_t block_no) {
	if (sd_ensure_data_mode() != 0) {
		return -1;
	}

	SD_TRACE("Card ready, reading from block %u", block_no);

	if (sd_do_data_command(0, buf, buf_size, block_no) < 0) {
		return -1;
	}

	SD_TRACE("Data read successful");

	return buf_size;
}

#ifdef SD_WRITE_SUPPORT
/**
 * @ingroup SD
 * @param dev
 * @param buf
 * @param buf_size
 * @param block_no
 * @return
 */
int sd_write(uint8_t *buf, size_t buf_size, uint32_t block_no) {
	if (sd_ensure_data_mode() != 0) {
		return -1;
	}

	SD_TRACE("Card ready, writing to block %u", block_no);

	if (sd_do_data_command(1, buf, buf_size, block_no) < 0) {
		return -1;
	}

	SD_TRACE("Write data successful");

	return buf_size;
}
#endif

