/* Copyright (C) 2013 by John Cronin <jncronin@tysos.org>
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

/* Provides an interface to the EMMC controller and commands for interacting
 * with an sd card */

/* References:
 *
 * PLSS 	- SD Group Physical Layer Simplified Specification ver 3.00
 * HCSS		- SD Group Host Controller Simplified Specification ver 3.00
 *
 * Broadcom BCM2835 Peripherals Guide
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "block.h"
#include "timer.h"
#include "util.h"

#include <bcm2835.h>
#include <bcm2835_emmc.h>
#include <bcm2835_vc.h>

int printf(const char *format, ...);

#ifdef DEBUG2
#define EMMC_DEBUG
#endif


/* Tracing macros */
#ifdef EMMC_DEBUG
#define EMMC_TRACE(...)     {										\
        printf("EMMC %s:%4d[%s] : ", __FILE__, __LINE__, __func__);	\
        printf(__VA_ARGS__);										\
        printf("\n"); }
#define SD_TRACE(...)     {											\
        printf("SD   %s:%4d[%s] : ", __FILE__, __LINE__, __func__);	\
        printf(__VA_ARGS__);										\
        printf("\n"); }
#else
#define EMMC_TRACE(...)
#define SD_TRACE(...)
#endif


// Configuration options

// Enable 1.8V support
#define SD_1_8V_SUPPORT

// Enable 4-bit support
#define SD_4BIT_DATA

// SD Clock Frequencies (in Hz)
#define SD_CLOCK_ID         4000000
#define SD_CLOCK_NORMAL     50000000
#define SD_CLOCK_HIGH       50000000
#define SD_CLOCK_100        100000000
#define SD_CLOCK_208        208000000

// Enable SDXC maximum performance mode
// Requires 150 mA power so disabled on the RPi for now
//#define SDXC_MAXIMUM_PERFORMANCE

// Enable SDMA support
//#define SDMA_SUPPORT

// SDMA buffer address
#define SDMA_BUFFER     0x6000
#define SDMA_BUFFER_PA  (SDMA_BUFFER + 0xC0000000)

// Enable card interrupts
//#define SD_CARD_INTERRUPTS

// Enable EXPERIMENTAL (and possibly DANGEROUS) SD write support
#define SD_WRITE_SUPPORT

// The particular SDHCI implementation
#define SDHCI_IMPLEMENTATION_GENERIC        0
#define SDHCI_IMPLEMENTATION_BCM_2708       1
#define SDHCI_IMPLEMENTATION                SDHCI_IMPLEMENTATION_BCM_2708

static char driver_name[] = "emmc";
static char device_name[] = "emmc0";	// We use a single device name as there is only one card slot in the RPi

static uint32_t hci_ver = 0;
static uint32_t capabilities_0 = 0;
static uint32_t capabilities_1 = 0;

struct sd_scr
{
    uint32_t    scr[2];
    uint32_t    sd_bus_widths;
    int         sd_version;
};

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
	uint32_t base_clock;
};

#define SD_CMD_INDEX(a)		((a) << 24)
#define SD_CMD_TYPE_NORMAL	0x0
#define SD_CMD_TYPE_SUSPEND	(1 << 22)
#define SD_CMD_TYPE_RESUME	(2 << 22)
#define SD_CMD_TYPE_ABORT	(3 << 22)
#define SD_CMD_TYPE_MASK    (3 << 22)
#define SD_CMD_ISDATA		(1 << 21)
#define SD_CMD_IXCHK_EN		(1 << 20)
#define SD_CMD_CRCCHK_EN	(1 << 19)
#define SD_CMD_RSPNS_TYPE_NONE	0			// For no response
#define SD_CMD_RSPNS_TYPE_136	(1 << 16)		// For response R2 (with CRC), R3,4 (no CRC)
#define SD_CMD_RSPNS_TYPE_48	(2 << 16)		// For responses R1, R5, R6, R7 (with CRC)
#define SD_CMD_RSPNS_TYPE_48B	(3 << 16)		// For responses R1b, R5b (with CRC)
#define SD_CMD_RSPNS_TYPE_MASK  (3 << 16)
#define SD_CMD_MULTI_BLOCK	(1 << 5)
#define SD_CMD_DAT_DIR_HC	0
#define SD_CMD_DAT_DIR_CH	(1 << 4)
#define SD_CMD_AUTO_CMD_EN_NONE	0
#define SD_CMD_AUTO_CMD_EN_CMD12	(1 << 2)
#define SD_CMD_AUTO_CMD_EN_CMD23	(2 << 2)
#define SD_CMD_BLKCNT_EN		(1 << 1)
#define SD_CMD_DMA          1

#define SD_ERR_CMD_TIMEOUT	0
#define SD_ERR_CMD_CRC		1
#define SD_ERR_CMD_END_BIT	2
#define SD_ERR_CMD_INDEX	3
#define SD_ERR_DATA_TIMEOUT	4
#define SD_ERR_DATA_CRC		5
#define SD_ERR_DATA_END_BIT	6
#define SD_ERR_CURRENT_LIMIT	7
#define SD_ERR_AUTO_CMD12	8
#define SD_ERR_ADMA		9
#define SD_ERR_TUNING		10
#define SD_ERR_RSVD		11

#define SD_ERR_MASK_CMD_TIMEOUT		(1 << (16 + SD_ERR_CMD_TIMEOUT))
#define SD_ERR_MASK_CMD_CRC		(1 << (16 + SD_ERR_CMD_CRC))
#define SD_ERR_MASK_CMD_END_BIT		(1 << (16 + SD_ERR_CMD_END_BIT))
#define SD_ERR_MASK_CMD_INDEX		(1 << (16 + SD_ERR_CMD_INDEX))
#define SD_ERR_MASK_DATA_TIMEOUT	(1 << (16 + SD_ERR_CMD_TIMEOUT))
#define SD_ERR_MASK_DATA_CRC		(1 << (16 + SD_ERR_CMD_CRC))
#define SD_ERR_MASK_DATA_END_BIT	(1 << (16 + SD_ERR_CMD_END_BIT))
#define SD_ERR_MASK_CURRENT_LIMIT	(1 << (16 + SD_ERR_CMD_CURRENT_LIMIT))
#define SD_ERR_MASK_AUTO_CMD12		(1 << (16 + SD_ERR_CMD_AUTO_CMD12))
#define SD_ERR_MASK_ADMA		(1 << (16 + SD_ERR_CMD_ADMA))
#define SD_ERR_MASK_TUNING		(1 << (16 + SD_ERR_CMD_TUNING))

#define SD_COMMAND_COMPLETE     1
#define SD_TRANSFER_COMPLETE    (1 << 1)
#define SD_BLOCK_GAP_EVENT      (1 << 2)
#define SD_DMA_INTERRUPT        (1 << 3)
#define SD_BUFFER_WRITE_READY   (1 << 4)
#define SD_BUFFER_READ_READY    (1 << 5)
#define SD_CARD_INSERTION       (1 << 6)
#define SD_CARD_REMOVAL         (1 << 7)
#define SD_CARD_INTERRUPT       (1 << 8)

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

#define SD_CMD_RESERVED(a)  0xffffffff

#define SUCCESS(a)          (a->last_cmd_success)
#define FAIL(a)             (a->last_cmd_success == 0)
#define TIMEOUT(a)          (FAIL(a) && (a->last_error == 0))
#define CMD_TIMEOUT(a)      (FAIL(a) && (a->last_error & (1 << 16)))
#define CMD_CRC(a)          (FAIL(a) && (a->last_error & (1 << 17)))
#define CMD_END_BIT(a)      (FAIL(a) && (a->last_error & (1 << 18)))
#define CMD_INDEX(a)        (FAIL(a) && (a->last_error & (1 << 19)))
#define DATA_TIMEOUT(a)     (FAIL(a) && (a->last_error & (1 << 20)))
#define DATA_CRC(a)         (FAIL(a) && (a->last_error & (1 << 21)))
#define DATA_END_BIT(a)     (FAIL(a) && (a->last_error & (1 << 22)))
#define CURRENT_LIMIT(a)    (FAIL(a) && (a->last_error & (1 << 23)))
#define ACMD12_ERROR(a)     (FAIL(a) && (a->last_error & (1 << 24)))
#define ADMA_ERROR(a)       (FAIL(a) && (a->last_error & (1 << 25)))
#define TUNING_ERROR(a)     (FAIL(a) && (a->last_error & (1 << 26)))

#define SD_VER_UNKNOWN      0
#define SD_VER_1            1
#define SD_VER_1_1          2
#define SD_VER_2            3
#define SD_VER_3            4
#define SD_VER_4            5

static char *sd_versions[] = { "unknown", "1.0 and 1.01", "1.10", "2.00", "3.0x", "4.xx" };

#ifdef EMMC_DEBUG
static char *err_irpts[] = { "CMD_TIMEOUT", "CMD_CRC", "CMD_END_BIT", "CMD_INDEX",
	"DATA_TIMEOUT", "DATA_CRC", "DATA_END_BIT", "CURRENT_LIMIT",
	"AUTO_CMD12", "ADMA", "TUNING", "RSVD" };
#endif

int sd_read(struct block_device *, uint8_t *, size_t buf_size, uint32_t);
int sd_write(struct block_device *, uint8_t *, size_t buf_size, uint32_t);

static uint32_t sd_commands[] = {
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

static uint32_t sd_acommands[] = {
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


/**
 *
 * @return
 */
inline static int bcm2835_emmc_power_off()
{
	int32_t ret = bcm2835_vc_set_power_state(BCM2835_VC_POWER_ID_SDCARD, BCM2835_VC_SET_POWER_STATE_OFF_WAIT);

	if (ret < 0) {
		printf("EMMC: %s : bcm2835_vc_set_power_state() did not return a valid response.\n", __func__);
		return -1;
	}
	// TODO
	if ((ret & 0x3) != 0 ) {
		EMMC_TRACE("Device did not power off successfully (%08x).", ret);
		return 1;
	}

	return 0;
}

/**
 *
 * @return
 */
inline static int bcm2835_emmc_power_on()
{
	int32_t ret = bcm2835_vc_set_power_state(BCM2835_VC_POWER_ID_SDCARD, BCM2835_VC_SET_POWER_STATE_ON_WAIT);

	if (ret < 0) {
		printf("EMMC: %s : bcm2835_vc_set_power_state() did not return a valid response.\n", __func__);
		return -1;
	}
	// TODO
	if((ret & 0x3) != 1)
	{
		EMMC_TRACE("Device did not power on successfully (%08x).", ret);
		return 1;
	}

	return 0;
}

/**
 *
 * @return
 */
inline static int bcm2835_emmc_power_cycle()
{
	if(bcm2835_emmc_power_off() < 0)
		return -1;

	udelay(5000);

	return bcm2835_emmc_power_on();
}

/**
 *
 */
static void sd_power_off()
{
	uint32_t control0 = BCM2835_EMMC->CONTROL0;
	control0 &= ~BCM2835_EMMC_CONTROL0_POWER_ON; // Set SD Bus Power bit off in Power Control Register
	BCM2835_EMMC->CONTROL0 = control0;
}

/**
 * Set the clock dividers to generate a target value
 *
 * @param base_clock
 * @param target_rate
 * @return
 */
inline static uint32_t bcm2835_emmc_get_clock_divider(uint32_t base_clock, uint32_t target_rate)
{
	int divisor = 0;
	int real_div = divisor;
	uint32_t ret = 0;

	if (target_rate == 0)
		return SD_GET_CLOCK_DIVIDER_FAIL;

	if (hci_ver >= 2)
	{
		if (base_clock <= target_rate)
			divisor = 1;
		else
		{
			for (divisor = 2; divisor < SDHCI_MAX_DIV_SPEC_300; divisor += 2)
			{
				if ((base_clock / divisor) <= target_rate)
					break;
			}
		}

		real_div = divisor;
		divisor >>= 1;

		int actual_clock;

		if (real_div)
			actual_clock = base_clock/ real_div;

		ret |= (divisor & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT;
		ret |= ((divisor & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN) << SDHCI_DIVIDER_HI_SHIFT;

		EMMC_TRACE("base_clock: %i, target_rate: %i, divisor: %08x, actual_clock: %i, ret: %08x", base_clock, target_rate, divisor, actual_clock, ret);

		return ret;
	}
	else
	{
		printf("EMMC: unsupported host version\n");
		return SD_GET_CLOCK_DIVIDER_FAIL;
	}

}

/**
 *
 * @param base_clock
 * @param target_rate
 * @return
 */
static int bcm2835_emmc_switch_clock_rate(uint32_t base_clock, uint32_t target_rate)
{
    uint32_t divider = bcm2835_emmc_get_clock_divider(base_clock, target_rate);

    if(divider == SD_GET_CLOCK_DIVIDER_FAIL)
    {
        printf("EMMC: couldn't get a valid divider for target rate %i Hz\n", target_rate);
        return -1;
    }

    // Wait for the command inhibit (CMD and DAT) bits to clear
	while (BCM2835_EMMC ->STATUS & (BCM2835_EMMC_STATUS_CMD_INHIBIT || BCM2835_EMMC_STATUS_DATA_INHIBIT))
		;

    // Set the SD clock off
    uint32_t control1 = BCM2835_EMMC ->CONTROL1;
    control1 &= ~BCM2835_EMMC_CLOCK_CARD_EN;
    BCM2835_EMMC ->CONTROL1 = control1;
    udelay(2000);

    // Write the new divider
	control1 &= ~0xffe0;		// Clear old setting + clock generator select
    control1 |= divider;
    BCM2835_EMMC ->CONTROL1 = control1;
    udelay(2000);

    // Enable the SD clock
    control1 |= BCM2835_EMMC_CLOCK_CARD_EN;
    BCM2835_EMMC ->CONTROL1 = control1;
    udelay(2000);

    EMMC_TRACE("Successfully set clock rate to %i Hz", target_rate);

    return 0;
}

/**
 * Reset the CMD line
 *
 * @return
 */
static int sd_reset_cmd()
{
	uint32_t control1 = BCM2835_EMMC->CONTROL1;

	control1 |= BCM2835_EMMC_CONTROL1_RESET_CMD;
	BCM2835_EMMC->CONTROL1 = control1;
	TIMEOUT_WAIT((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_RESET_CMD) == 0, 1000000);

	if((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_RESET_CMD) != 0)
	{
		printf("EMMC: CMD line did not reset properly\n");
		return -1;
	}

	return 0;
}

/**
 * Reset the CMD line
 *
 * @return
 */
static int sd_reset_dat()
{
	uint32_t control1 = BCM2835_EMMC->CONTROL1;

	control1 |= BCM2835_EMMC_CONTROL1_RESET_DATA;
	BCM2835_EMMC->CONTROL1 = control1;
	TIMEOUT_WAIT((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_RESET_DATA) == 0, 1000000);

	if((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_RESET_DATA) != 0)
	{
		printf("EMMC: DAT line did not reset properly\n");
		return -1;
	}

	return 0;
}


/**
 *
 * @param dev
 * @param cmd_reg
 * @param argument
 * @param timeout
 */
static void sd_issue_command_int(struct emmc_block_dev *dev, uint32_t cmd_reg, uint32_t argument, useconds_t timeout)
{
    dev->last_cmd_reg = cmd_reg;
    dev->last_cmd_success = 0;

    // This is as per HCSS 3.7.1.1/3.7.2.2

    // Check Command Inhibit
    // TODO
    while(BCM2835_EMMC->STATUS & BCM2835_EMMC_STATUS_CMD_INHIBIT)
    	;

    // Is the command with busy?
    if((cmd_reg & SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B)
    {
        // With busy
        // Is is an abort command?
        if((cmd_reg & SD_CMD_TYPE_MASK) != SD_CMD_TYPE_ABORT)
        {
            // Not an abort command
            // Wait for the data line to be free
        	// TODO
            while(BCM2835_EMMC->STATUS & BCM2835_EMMC_STATUS_DATA_INHIBIT)
            	;
        }
    }

    // Is this a DMA transfer?
    int is_sdma = 0;
    if((cmd_reg & SD_CMD_ISDATA) && (dev->use_sdma))
    {
    	SD_TRACE("Performing SDMA transfer, current INTERRUPT: %08x", BCM2835_EMMC->INTERRUPT);
        is_sdma = 1;
    }

    if(is_sdma)
    {
        // Set system address register (ARGUMENT2 in RPi)
        // We need to define a 4 kiB aligned buffer to use here
        // Then convert its virtual address to a bus address
        BCM2835_EMMC->ARG2 = SDMA_BUFFER_PA;
    }

    // Set block size and block count
    // For now, block size = 512 bytes, block count = 1,
    //  host SDMA buffer boundary = 4 kiB
    if(dev->blocks_to_transfer > 0xffff)
    {
        printf("SD: blocks_to_transfer too great (%i)\n", dev->blocks_to_transfer);
        dev->last_cmd_success = 0;
        return;
    }
    uint32_t blksizecnt = dev->block_size | (dev->blocks_to_transfer << 16);
    BCM2835_EMMC->BLKSIZECNT = blksizecnt;

    // Set argument 1 reg
    BCM2835_EMMC->ARG1 = argument;

    if(is_sdma)
    {
        // Set Transfer mode register
        cmd_reg |= SD_CMD_DMA;
    }

    // Set command reg
    BCM2835_EMMC->CMDTM = cmd_reg;

    udelay(2000);

    EMMC_TRACE("Wait for command complete interrupt");
    TIMEOUT_WAIT(BCM2835_EMMC->INTERRUPT & 0x8001, timeout);
    uint32_t irpts = BCM2835_EMMC->INTERRUPT;

    // Clear command complete status
    BCM2835_EMMC->INTERRUPT = 0xffff0001;

    // Test for errors
    if((irpts & 0xffff0001) != 0x1)
    {
        EMMC_TRACE("Error occurred whilst waiting for command complete interrupt");
        dev->last_error = irpts & 0xffff0000;
        dev->last_interrupt = irpts;
        return;
    }

    // Get response data
    switch(cmd_reg & SD_CMD_RSPNS_TYPE_MASK)
    {
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
    if((cmd_reg & SD_CMD_ISDATA) && (is_sdma == 0))
    {
        uint32_t wr_irpt;
        int is_write = 0;
        if(cmd_reg & SD_CMD_DAT_DIR_CH)
            wr_irpt = (1 << 5);     // read
        else
        {
            is_write = 1;
            wr_irpt = (1 << 4);     // write
        }

        int cur_block = 0;
        uint32_t *cur_buf_addr = (uint32_t *)dev->buf;
        while(cur_block < dev->blocks_to_transfer)
        {
#ifdef EMMC_DEBUG
			if(dev->blocks_to_transfer > 1)
				SD_TRACE("Multi block transfer, awaiting block %i ready", cur_block);
#endif
			///< AvV
            ///< TIMEOUT_WAIT(mmio_read(EMMC_BASE + EMMC_INTERRUPT) & (wr_irpt | 0x8000), timeout);
			TIMEOUT_WAIT(BCM2835_EMMC->INTERRUPT & (wr_irpt | 0x8000), timeout); ///< AvV
            ///< irpts = mmio_read(EMMC_BASE + EMMC_INTERRUPT);
			irpts = BCM2835_EMMC->INTERRUPT;///< AvV
            ///< mmio_write(EMMC_BASE + EMMC_INTERRUPT, 0xffff0000 | wr_irpt);
			BCM2835_EMMC->INTERRUPT = 0xffff0000 | wr_irpt;

            if((irpts & (0xffff0000 | wr_irpt)) != wr_irpt)
            {
            	SD_TRACE("Error occurred whilst waiting for data ready interrupt");
                dev->last_error = irpts & 0xffff0000;
                dev->last_interrupt = irpts;
                return;
            }

            // Transfer the block
            size_t cur_byte_no = 0;
            while(cur_byte_no < dev->block_size)
            {
                if(is_write)
				{
                	BCM2835_EMMC->DATA = *cur_buf_addr;
				}
                else
				{
                	*cur_buf_addr = BCM2835_EMMC->DATA;
				}
                cur_byte_no += 4;
                cur_buf_addr++;
            }

            SD_TRACE("block %i transfer complete", cur_block);

            cur_block++;
        }
    }

    // Wait for transfer complete (set if read/write transfer or with busy)
    if((((cmd_reg & SD_CMD_RSPNS_TYPE_MASK) == SD_CMD_RSPNS_TYPE_48B) ||
       (cmd_reg & SD_CMD_ISDATA)) && (is_sdma == 0))
    {
        // First check command inhibit (DAT) is not already 0
        if((BCM2835_EMMC->STATUS & BCM2835_EMMC_STATUS_DATA_INHIBIT) == 0)
        {
        	BCM2835_EMMC->INTERRUPT = 0xffff0002;
        }
        else
        {
        	TIMEOUT_WAIT(BCM2835_EMMC->INTERRUPT & 0x8002, timeout);
        	irpts = BCM2835_EMMC->INTERRUPT;
        	BCM2835_EMMC->INTERRUPT = 0xffff0002;

            // Handle the case where both data timeout and transfer complete
            //  are set - transfer complete overrides data timeout: HCSS 2.2.17
            if(((irpts & 0xffff0002) != 0x2) && ((irpts & 0xffff0002) != 0x100002))
            {
                SD_TRACE("Error occurred whilst waiting for transfer complete interrupt");
                dev->last_error = irpts & 0xffff0000;
                dev->last_interrupt = irpts;
                return;
            }

            BCM2835_EMMC->INTERRUPT = 0xffff0002;
        }
    }
    else if (is_sdma)
    {
        // For SDMA transfers, we have to wait for either transfer complete,
        //  DMA int or an error

        // First check command inhibit (DAT) is not already 0
        if((BCM2835_EMMC->STATUS & BCM2835_EMMC_STATUS_DATA_INHIBIT) == 0)
        {
        	BCM2835_EMMC->INTERRUPT = 0xffff000a;
        }
        else
        {
           	TIMEOUT_WAIT(BCM2835_EMMC->INTERRUPT & 0x800a, timeout);
        	irpts = BCM2835_EMMC->INTERRUPT;
        	BCM2835_EMMC->INTERRUPT = 0xffff000a;

            // Detect errors
            if((irpts & 0x8000) && ((irpts & 0x2) != 0x2))
            {
            	EMMC_TRACE("Error occurred whilst waiting for transfer complete interrupt\n");
                dev->last_error = irpts & 0xffff0000;
                dev->last_interrupt = irpts;
                return;
            }

            // Detect DMA interrupt without transfer complete
            // Currently not supported - all block sizes should fit in the
            //  buffer
            if((irpts & 0x8) && ((irpts & 0x2) != 0x2))
            {
            	SD_TRACE("Error: DMA interrupt occured without transfer complete");
                dev->last_error = irpts & 0xffff0000;
                dev->last_interrupt = irpts;
                return;
            }

            // Detect transfer complete
            if(irpts & 0x2)
            {
            	EMMC_TRACE("SDMA transfer complete");
                // Transfer the data to the user buffer
                memcpy(dev->buf, (const void *)SDMA_BUFFER, dev->block_size);
            }
            else
            {
#ifdef EMMC_DEBUG
            	EMMC_TRACE("Unknown error");

                if(irpts == 0) {
                	EMMC_TRACE("Timeout waiting for SDMA transfer to complete");
                }
                else {
                	EMMC_TRACE("Unknown SDMA transfer error");
                }

                EMMC_TRACE("INTERRUPT: %08x, STATUS %08x", irpts, BCM2835_EMMC->STATUS);
#endif
                // TODO
                if((irpts == 0) && ((BCM2835_EMMC->STATUS & 0x3) == 0x2))
                {
                	EMMC_TRACE("The data transfer is ongoing, we should attempt to stop it");
                    BCM2835_EMMC->CMDTM = sd_commands[STOP_TRANSMISSION];
                }
                dev->last_error = irpts & 0xffff0000;
                dev->last_interrupt = irpts;
                return;
            }
        }
    }

    // Return success
    dev->last_cmd_success = 1;
}

/**
 *
 * @param dev
 */
static void sd_handle_card_interrupt(struct emmc_block_dev *dev)
{
    EMMC_TRACE("Controller status: %08x", BCM2835_EMMC->STATUS);
    SD_TRACE("Get the card status");

    if(dev->card_rca)
    {
        sd_issue_command_int(dev, sd_commands[SEND_STATUS], dev->card_rca << 16, 500000);

        if(FAIL(dev))
		{
        	SD_TRACE("Unable to get card status");
		}
		else
		{
			SD_TRACE("Card status: %08x", dev->last_r0);
		}
	}
	else
	{
		SD_TRACE("No card currently selected");
	}
}

// TODO
/**
 *
 * @param dev
 */
static void sd_handle_interrupts(struct emmc_block_dev *dev)
{
	uint32_t irpts = BCM2835_EMMC->INTERRUPT;
    uint32_t reset_mask = 0;

    if(irpts & SD_COMMAND_COMPLETE)
    {
    	SD_TRACE("Spurious command complete interrupt\n");
        reset_mask |= SD_COMMAND_COMPLETE;
    }

    if(irpts & SD_TRANSFER_COMPLETE)
    {
    	SD_TRACE("Spurious transfer complete interrupt");
        reset_mask |= SD_TRANSFER_COMPLETE;
    }

    if(irpts & SD_BLOCK_GAP_EVENT)
    {
    	SD_TRACE("Spurious block gap event interrupt");
        reset_mask |= SD_BLOCK_GAP_EVENT;
    }

    if(irpts & SD_DMA_INTERRUPT)
    {
    	SD_TRACE("Spurious DMA interrupt");
        reset_mask |= SD_DMA_INTERRUPT;
    }

    if(irpts & SD_BUFFER_WRITE_READY)
    {
    	SD_TRACE("Spurious buffer write ready interrupt");
        reset_mask |= SD_BUFFER_WRITE_READY;
        sd_reset_dat();
    }

    if(irpts & SD_BUFFER_READ_READY)
    {
    	SD_TRACE("Spurious buffer read ready interrupt");
        reset_mask |= SD_BUFFER_READ_READY;
        sd_reset_dat();
    }

    if(irpts & SD_CARD_INSERTION)
    {
    	SD_TRACE("Card insertion detected");
        reset_mask |= SD_CARD_INSERTION;
    }

    if(irpts & SD_CARD_REMOVAL)
    {
    	SD_TRACE("Card removal detected");
        reset_mask |= SD_CARD_REMOVAL;
        dev->card_removal = 1;
    }

    if(irpts & SD_CARD_INTERRUPT)
    {
    	SD_TRACE("Card interrupt detected");
        sd_handle_card_interrupt(dev);
        reset_mask |= SD_CARD_INTERRUPT;
    }

    if(irpts & 0x8000)
    {
    	SD_TRACE("Spurious error interrupt: %08x", irpts);
        reset_mask |= 0xffff0000;
    }

    BCM2835_EMMC->INTERRUPT = reset_mask;
}

static void sd_issue_command(struct emmc_block_dev *dev, uint32_t command, uint32_t argument, useconds_t timeout)
{
    // First, handle any pending interrupts
    sd_handle_interrupts(dev);

    // Stop the command issue if it was the card remove interrupt that was
    //  handled
    if(dev->card_removal)
    {
        dev->last_cmd_success = 0;
        return;
    }

    // Now run the appropriate commands by calling sd_issue_command_int()
    if(command & IS_APP_CMD)
    {
        command &= 0xff;

        SD_TRACE("Issuing command ACMD%i", command);

        if(sd_acommands[command] == SD_CMD_RESERVED(0))
        {
            printf("SD: invalid command ACMD%i\n", command);
            dev->last_cmd_success = 0;
            return;
        }
        dev->last_cmd = APP_CMD;

        uint32_t rca = 0;
        if(dev->card_rca)
            rca = dev->card_rca << 16;
        sd_issue_command_int(dev, sd_commands[APP_CMD], rca, timeout);
        if(dev->last_cmd_success)
        {
            dev->last_cmd = command | IS_APP_CMD;
            sd_issue_command_int(dev, sd_acommands[command], argument, timeout);
        }
    }
    else
    {
    	SD_TRACE("Issuing command CMD%i", command);

        if(sd_commands[command] == SD_CMD_RESERVED(0))
        {
            printf("SD: invalid command CMD%i\n", command);
            dev->last_cmd_success = 0;
            return;
        }

        dev->last_cmd = command;
        sd_issue_command_int(dev, sd_commands[command], argument, timeout);
    }

#ifdef EMMC_DEBUG
    if(FAIL(dev))
    {
    	SD_TRACE("Error issuing command: interrupts %08x: ", dev->last_interrupt);

        if(dev->last_error == 0) {
        	SD_TRACE("TIMEOUT");
        }
        else
        {
            for(int i = 0; i < SD_ERR_RSVD; i++)
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

int sd_card_init(struct block_device **dev)
{
    if(sizeof(sd_commands) != (64 * sizeof(uint32_t)))
    {
        printf("EMMC: fatal error, sd_commands of incorrect size: %i expected %i\n", sizeof(sd_commands), 64 * sizeof(uint32_t));
        return -1;
    }

    if(sizeof(sd_acommands) != (64 * sizeof(uint32_t)))
    {
        printf("EMMC: fatal error, sd_acommands of incorrect size: %i expected %i\n", sizeof(sd_acommands), 64 * sizeof(uint32_t));
        return -1;
    }

	if(bcm2835_emmc_power_cycle() != 0)
	{
		printf("EMMC: BCM2835 controller did not power cycle successfully\n");
		return -1;
	}

	EMMC_TRACE("BCM2835 controller power-cycled");

	EMMC_TRACE("Read the controller version");

	uint32_t ver = BCM2835_EMMC->SLOTISR_VER;
	uint32_t vendor = ver >> 24;
	uint32_t sdversion = (ver >> 16) & 0xff;
	uint32_t slot_status = ver & 0xff;

	EMMC_TRACE("vendor %x, sdversion %x, slot_status %x", vendor, sdversion, slot_status);

	hci_ver = sdversion;

	if(hci_ver < 2)
	{
		printf("EMMC: only SDHCI versions >= 3.0 are supported\n");
		return -1;
	}

	EMMC_TRACE("Resetting controller");

	uint32_t control1 = BCM2835_EMMC->CONTROL1;
	control1 |= (1 << 24);
	// TODO
	// Disable clock
	control1 &= ~(1 << 2);
	control1 &= ~(1 << 0);
	BCM2835_EMMC->CONTROL1 = control1;

	TIMEOUT_WAIT((BCM2835_EMMC->CONTROL1 & (0x7 << 24)) == 0, 1000000);

	if((BCM2835_EMMC->CONTROL1 & (0x7 << 24)) != 0)
	{
		printf("EMMC: controller did not reset properly\n");
		return -1;
	}

	EMMC_TRACE("control0: %08x, control1: %08x, control2: %08x", BCM2835_EMMC->CONTROL0, BCM2835_EMMC->CONTROL1, BCM2835_EMMC->CONTROL2);

	capabilities_0 = BCM2835_EMMC->CAPABILITIES_0;
	capabilities_1 = BCM2835_EMMC->CAPABILITIES_1;

	EMMC_TRACE("capabilities: %08x%08x", capabilities_1, capabilities_0);

	// TODO
	EMMC_TRACE("Checking for an inserted card");
	TIMEOUT_WAIT(BCM2835_EMMC->STATUS & (1 << 16), 500000);

	uint32_t status_reg = BCM2835_EMMC->STATUS;

	if((status_reg & (1 << 16)) == 0)
	{
		printf("EMMC: no card inserted\n");
		return -1;
	}

	EMMC_TRACE("Status: %08x", status_reg);

	BCM2835_EMMC->CONTROL2 = 0;

	uint32_t base_clock = bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_CORE);

	if (base_clock == 0)
	{
		printf("EMMC: assuming clock rate to be 100MHz\n");
		base_clock = 100000000;
	}

	EMMC_TRACE("Setting clock rate (to something slow)");

	control1 = BCM2835_EMMC->CONTROL1;
	control1 |= BCM2835_EMMC_CLOCK_INT_EN;

	EMMC_TRACE("Set to identification frequency : %d", SD_CLOCK_ID);

	uint32_t f_id = bcm2835_emmc_get_clock_divider(base_clock, SD_CLOCK_ID);

	if(f_id == SD_GET_CLOCK_DIVIDER_FAIL)
	{
		printf("EMMC: unable to get a valid clock divider for ID frequency\n");
		return -1;
	}

	control1 |= f_id;
	control1 |= (7 << 16);		// data timeout = TMCLK * 2^10
	BCM2835_EMMC->CONTROL1 = control1;

	TIMEOUT_WAIT(BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CLOCK_INT_STABLE, 0x1000000);

	if ((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CLOCK_INT_STABLE ) == 0)
	{
		printf("EMMC: controller's clock did not stabilize within 1 second\n");
		return -1;
	}

	EMMC_TRACE("control0: %08x, control1: %08x", BCM2835_EMMC->CONTROL0, BCM2835_EMMC->CONTROL1);

	EMMC_TRACE("Enabling SD clock");

	udelay(2000);

	control1 = BCM2835_EMMC->CONTROL1;
	control1 |= BCM2835_EMMC_CLOCK_CARD_EN;
	BCM2835_EMMC->CONTROL1 = control1;

	udelay(2000);

	// Mask off sending interrupts to the ARM
	BCM2835_EMMC->IRPT_EN = 0;
	// Reset interrupts
	BCM2835_EMMC->INTERRUPT = 0xffffffff;
	// Have all interrupts sent to the INTERRUPT register
	uint32_t irpt_mask = 0xffffffff & (~SD_CARD_INTERRUPT);
#ifdef SD_CARD_INTERRUPTS
    irpt_mask |= SD_CARD_INTERRUPT;
#endif
    BCM2835_EMMC->IRPT_MASK = irpt_mask;

	udelay(2000);

    // Prepare the device structure
	struct emmc_block_dev *ret;
	if(*dev == NULL)
		ret = (struct emmc_block_dev *)malloc(sizeof(struct emmc_block_dev));
	else
		ret = (struct emmc_block_dev *)*dev;

	memset(ret, 0, sizeof(struct emmc_block_dev));
	ret->bd.driver_name = driver_name;
	ret->bd.device_name = device_name;
	ret->bd.block_size = 512;
	ret->bd.read = sd_read;
#ifdef SD_WRITE_SUPPORT
    ret->bd.write = sd_write;
#endif
    ret->bd.supports_multiple_block_read = 1;
    ret->bd.supports_multiple_block_write = 1;
	ret->base_clock = base_clock;

	// Send CMD0 to the card (reset to idle state)
	sd_issue_command(ret, GO_IDLE_STATE, 0, 500000);
	if(FAIL(ret))
	{
        printf("SD: no CMD0 response\n");
        return -1;
	}

	// Send CMD8 to the card
	// Voltage supplied = 0x1 = 2.7-3.6V (standard)
	// Check pattern = 10101010b (as per PLSS 4.3.13) = 0xAA
	SD_TRACE("Note a timeout error on the following command (CMD8) is normal and expected if the SD card version is less than 2.0");
	sd_issue_command(ret, SEND_IF_COND, 0x1aa, 500000);

	int v2_later = 0;

	if(TIMEOUT(ret))
        v2_later = 0;
    else if(CMD_TIMEOUT(ret))
	{
		if (sd_reset_cmd() == -1)
			return -1;
		BCM2835_EMMC ->INTERRUPT = SD_ERR_MASK_CMD_TIMEOUT;
		v2_later = 0;
	}
    else if(FAIL(ret))
    {
        printf("SD: failure sending CMD8 (%08x)\n", ret->last_interrupt);
        return -1;
    }
    else
    {
        if((ret->last_r0 & 0xfff) != 0x1aa)
        {
            printf("SD: unusable card\n");
            SD_TRACE("CMD8 response %08x", ret->last_r0);
            return -1;
        }
        else
            v2_later = 1;
    }

    // Here we are supposed to check the response to CMD5 (HCSS 3.6)
    // It only returns if the card is a SDIO card
	SD_TRACE("Note that a timeout error on the following command (CMD5) is normal and expected if the card is not a SDIO card");
    sd_issue_command(ret, IO_SET_OP_COND, 0, 10000);

    if(!TIMEOUT(ret))
    {
        if(CMD_TIMEOUT(ret))
        {
            if(sd_reset_cmd() == -1)
                return -1;

            BCM2835_EMMC->INTERRUPT = SD_ERR_MASK_CMD_TIMEOUT;
        }
        else
        {
            printf("SD: SDIO card detected - not currently supported\n");
            SD_TRACE("CMD5 returned %08x", ret->last_r0);
            return -1;
        }
    }

    SD_TRACE("Call an inquiry ACMD41 (voltage window = 0) to get the OCR");
    sd_issue_command(ret, ACMD(41), 0, 500000);

    if(FAIL(ret))
    {
        printf("SD: inquiry ACMD41 failed\n");
        return -1;
    }

    SD_TRACE("Inquiry ACMD41 returned %08x", ret->last_r0);

	// Call initialization ACMD41
	int card_is_busy = 1;
	while(card_is_busy)
	{
	    uint32_t v2_flags = 0;
	    if(v2_later)
	    {
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

	    sd_issue_command(ret, ACMD(41), 0x00ff8000 | v2_flags, 500000);

	    if(FAIL(ret))
	    {
	        printf("SD: error issuing ACMD41\n");
	        return -1;
	    }

	    if((ret->last_r0 >> 31) & 0x1)
	    {
	        // Initialization is complete
	        ret->card_ocr = (ret->last_r0 >> 8) & 0xffff;
	        ret->card_supports_sdhc = (ret->last_r0 >> 30) & 0x1;

#ifdef SD_1_8V_SUPPORT
	        if(!ret->failed_voltage_switch)
                ret->card_supports_18v = (ret->last_r0 >> 24) & 0x1;
#endif

	        card_is_busy = 0;
	    }
	    else
	    {
	    	SD_TRACE("Card is busy, retrying");
            udelay(500000);
	    }
	}

	SD_TRACE("Card identified: OCR: %04x, 1.8v support: %i, SDHC support: %i", ret->card_ocr, ret->card_supports_18v, ret->card_supports_sdhc);

    // At this point, we know the card is definitely an SD card, so will definitely
	//  support SDR12 mode which runs at 25 MHz
    bcm2835_emmc_switch_clock_rate(base_clock, SD_CLOCK_NORMAL);

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
			sd_power_off();
	        return sd_card_init((struct block_device **)&ret);
	    }

	    // TODO
	    // Disable SD clock
	    control1 = BCM2835_EMMC->CONTROL1;
	    control1 &= ~(1 << 2);
	    BCM2835_EMMC->CONTROL1 = control1;

	    // Check DAT[3:0]
	    status_reg = BCM2835_EMMC->STATUS;
	    uint32_t dat30 = (status_reg >> 20) & 0xf;

	    if(dat30 != 0)
	    {
	    	SD_TRACE("DAT[3:0] did not settle to 0");
	        ret->failed_voltage_switch = 1;
			sd_power_off();
	        return sd_card_init((struct block_device **)&ret);
	    }

	    // Set 1.8V signal enable to 1
	    // TODO
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
			sd_power_off();
	        return sd_card_init((struct block_device **)&ret);
	    }

	    // Re-enable the SD clock
	    // TODO
	    control1 = BCM2835_EMMC->CONTROL1;
	    control1 |= (1 << 2);
	    BCM2835_EMMC->CONTROL1 = control1;

	    // Wait 1 ms
	    udelay(10000);

	    // Check DAT[3:0]
	    // TODO
		status_reg = BCM2835_EMMC->STATUS;
	    dat30 = (status_reg >> 20) & 0xf;
	    if(dat30 != 0xf)
	    {
	    	SD_TRACE("DAT[3:0] did not settle to 1111b (%01x)", dat30);
	        ret->failed_voltage_switch = 1;
			sd_power_off();
	        return sd_card_init((struct block_device **)&ret);
	    }
	    SD_TRACE("SD: voltage switch complete");
	}

	SD_TRACE("Send CMD2 to get the cards CID");
	sd_issue_command(ret, ALL_SEND_CID, 0, 500000);

	if(FAIL(ret))
	{
	    printf("SD: error sending ALL_SEND_CID\n");
	    return -1;
	}

	uint32_t card_cid_0 = ret->last_r0;
	uint32_t card_cid_1 = ret->last_r1;
	uint32_t card_cid_2 = ret->last_r2;
	uint32_t card_cid_3 = ret->last_r3;

	SD_TRACE("Card CID: %08x%08x%08x%08x", card_cid_3, card_cid_2, card_cid_1, card_cid_0);

	uint32_t *dev_id = (uint32_t *)malloc(4 * sizeof(uint32_t));
	dev_id[0] = card_cid_0;
	dev_id[1] = card_cid_1;
	dev_id[2] = card_cid_2;
	dev_id[3] = card_cid_3;
	ret->bd.device_id = (uint8_t *)dev_id;
	ret->bd.dev_id_len = 4 * sizeof(uint32_t);

	SD_TRACE("Send CMD3 to enter the data state");
	sd_issue_command(ret, SEND_RELATIVE_ADDR, 0, 500000);

	if(FAIL(ret))
    {
        printf("SD: error sending SEND_RELATIVE_ADDR\n");
        free(ret);
        return -1;
    }

	uint32_t cmd3_resp = ret->last_r0;
	SD_TRACE("CMD3 response: %08x", cmd3_resp);

	ret->card_rca = (cmd3_resp >> 16) & 0xffff;
	uint32_t crc_error = (cmd3_resp >> 15) & 0x1;
	uint32_t illegal_cmd = (cmd3_resp >> 14) & 0x1;
	uint32_t error = (cmd3_resp >> 13) & 0x1;
	uint32_t status = (cmd3_resp >> 9) & 0xf;
	uint32_t ready = (cmd3_resp >> 8) & 0x1;

	if(crc_error)
	{
		printf("SD: CRC error\n");
		free(ret);
		free(dev_id);
		return -1;
	}

	if(illegal_cmd)
	{
		printf("SD: illegal command\n");
		free(ret);
		free(dev_id);
		return -1;
	}

	if(error)
	{
		printf("SD: generic error\n");
		free(ret);
		free(dev_id);
		return -1;
	}

	if(!ready)
	{
		printf("SD: not ready for data\n");
		free(ret);
		free(dev_id);
		return -1;
	}

	SD_TRACE("RCA: %04x", ret->card_rca);

	// Now select the card (toggles it to transfer state)
	sd_issue_command(ret, SELECT_CARD, ret->card_rca << 16, 500000);

	if(FAIL(ret))
	{
	    printf("SD: error sending CMD7\n");
	    free(ret);
	    return -1;
	}

	uint32_t cmd7_resp = ret->last_r0;
	status = (cmd7_resp >> 9) & 0xf;

	if((status != 3) && (status != 4))
	{
		printf("SD: invalid status (%i)\n", status);
		free(ret);
		free(dev_id);
		return -1;
	}

	// If not an SDHC card, ensure BLOCKLEN is 512 bytes
	if(!ret->card_supports_sdhc)
	{
	    sd_issue_command(ret, SET_BLOCKLEN, 512, 500000);
	    if(FAIL(ret))
	    {
	        printf("SD: error sending SET_BLOCKLEN\n");
	        free(ret);
	        return -1;
	    }
	}

	ret->block_size = 512;

	uint32_t controller_block_size = BCM2835_EMMC->BLKSIZECNT;
	controller_block_size &= (~0xfff);
	controller_block_size |= 0x200;
	BCM2835_EMMC->BLKSIZECNT = controller_block_size;

	// Get the cards SCR register
	ret->scr = (struct sd_scr *)malloc(sizeof(struct sd_scr));
	ret->buf = &ret->scr->scr[0];
	ret->block_size = 8;
	ret->blocks_to_transfer = 1;

	sd_issue_command(ret, SEND_SCR, 0, 500000);
	ret->block_size = 512;

	if(FAIL(ret))
	{
	    printf("SD: error sending SEND_SCR\n");
	    free(ret->scr);
        free(ret);
	    return -1;
	}

	// Determine card version
	// Note that the SCR is big-endian
	uint32_t scr0 = byte_swap(ret->scr->scr[0]);

	ret->scr->sd_version = SD_VER_UNKNOWN;

	uint32_t sd_spec = (scr0 >> (56 - 32)) & 0xf;
	uint32_t sd_spec3 = (scr0 >> (47 - 32)) & 0x1;
	uint32_t sd_spec4 = (scr0 >> (42 - 32)) & 0x1;

	ret->scr->sd_bus_widths = (scr0 >> (48 - 32)) & 0xf;

	if(sd_spec == 0)
        ret->scr->sd_version = SD_VER_1;
    else if(sd_spec == 1)
        ret->scr->sd_version = SD_VER_1_1;
    else if(sd_spec == 2)
    {
        if(sd_spec3 == 0)
            ret->scr->sd_version = SD_VER_2;
        else if(sd_spec3 == 1)
        {
            if(sd_spec4 == 0)
                ret->scr->sd_version = SD_VER_3;
            else if(sd_spec4 == 1)
                ret->scr->sd_version = SD_VER_4;
        }
    }

	SD_TRACE("&scr: %08x", &ret->scr->scr[0]);
	SD_TRACE("SCR[0]: %08x, SCR[1]: %08x", ret->scr->scr[0], ret->scr->scr[1]);;
	SD_TRACE("SCR: %08x%08x", byte_swap(ret->scr->scr[0]), byte_swap(ret->scr->scr[1]));
	SD_TRACE("SCR: version %s, bus_widths %01x", sd_versions[ret->scr->sd_version], ret->scr->sd_bus_widths);

    if(ret->scr->sd_bus_widths & 0x4)
    {
        // Set 4-bit transfer mode (ACMD6)
        // See HCSS 3.4 for the algorithm
#ifdef SD_4BIT_DATA
    	SD_TRACE("Switching to 4-bit data mode");
        // Disable card interrupt in host
        uint32_t old_irpt_mask = BCM2835_EMMC->IRPT_MASK;
        uint32_t new_iprt_mask = old_irpt_mask & ~(1 << 8);
        BCM2835_EMMC->IRPT_MASK = new_iprt_mask;

        // Send ACMD6 to change the card's bit mode
        sd_issue_command(ret, SET_BUS_WIDTH, 0x2, 500000);
        if(FAIL(ret))
            printf("SD: switch to 4-bit data mode failed\n");
        else
        {
            // Change bit mode for Host
        	// TODO
        	uint32_t control0 = BCM2835_EMMC->CONTROL0;
            control0 |= 0x2;
            BCM2835_EMMC->CONTROL0 = control0;

            // Re-enable card interrupt in host
            BCM2835_EMMC->IRPT_MASK = old_irpt_mask;

            SD_TRACE("Switch to 4-bit complete");
        }
#endif
    }

    SD_TRACE("Found a valid version %s SD card", sd_versions[ret->scr->sd_version]);
	SD_TRACE("Setup successful (status %i)", status);

	// Reset interrupt register
	BCM2835_EMMC->INTERRUPT = 0xffffffff;

	*dev = (struct block_device *)ret;

	return 0;
}

static int sd_ensure_data_mode(struct emmc_block_dev *edev)
{
	if(edev->card_rca == 0)
	{
		// Try again to initialise the card
		int ret = sd_card_init((struct block_device **)&edev);
		if(ret != 0)
			return ret;
	}

	SD_TRACE("Obtaining status register for card_rca %08x: ", edev->card_rca);

    sd_issue_command(edev, SEND_STATUS, edev->card_rca << 16, 500000);
    if(FAIL(edev))
    {
        printf("SD: ensure_data_mode() error sending CMD13\n");
        edev->card_rca = 0;
        return -1;
    }

	uint32_t status = edev->last_r0;
	uint32_t cur_state = (status >> 9) & 0xf;

	SD_TRACE("status %i", cur_state);

	if(cur_state == 3)
	{
		// Currently in the stand-by state - select it
		sd_issue_command(edev, SELECT_CARD, edev->card_rca << 16, 500000);
		if(FAIL(edev))
		{
			printf("SD: ensure_data_mode() no response from CMD17\n");
			edev->card_rca = 0;
			return -1;
		}
	}
	else if(cur_state == 5)
	{
		// In the data transfer state - cancel the transmission
		sd_issue_command(edev, STOP_TRANSMISSION, 0, 500000);
		if(FAIL(edev))
		{
			printf("SD: ensure_data_mode() no response from CMD12\n");
			edev->card_rca = 0;
			return -1;
		}

		// Reset the data circuit
		sd_reset_dat();
	}
	else if(cur_state != 4)
	{
		// Not in the transfer state - re-initialize
		int ret = sd_card_init((struct block_device **)&edev);
		if(ret != 0)
			return ret;
	}

	// Check again that we're now in the correct mode
	if(cur_state != 4)
	{
		SD_TRACE("Re-checking status: ");

        sd_issue_command(edev, SEND_STATUS, edev->card_rca << 16, 500000);
        if(FAIL(edev))
		{
			printf("SD: ensure_data_mode() no response from CMD13\n");
			edev->card_rca = 0;
			return -1;
		}
		status = edev->last_r0;
		cur_state = (status >> 9) & 0xf;

		SD_TRACE("%i", cur_state);

		if(cur_state != 4)
		{
			printf("SD: unable to initialize SD card to data mode (state %i)\n", cur_state);
			edev->card_rca = 0;
			return -1;
		}
	}

	return 0;
}

#ifdef SDMA_SUPPORT
// We only support DMA transfers to buffers aligned on a 4 kiB boundary
static int sd_suitable_for_dma(void *buf)
{
    if((uintptr_t)buf & 0xfff)
        return 0;
    else
        return 1;
}
#endif

static int sd_do_data_command(struct emmc_block_dev *edev, int is_write, uint8_t *buf, size_t buf_size, uint32_t block_no)
{
	// PLSS table 4.20 - SDSC cards use byte addresses rather than block addresses
	if(!edev->card_supports_sdhc)
		block_no *= 512;

	// This is as per HCSS 3.7.2.1
	if(buf_size < edev->block_size)
	{
	    printf("SD: do_data_command() called with buffer size (%i) less than block size (%i)\n", buf_size, edev->block_size);
        return -1;
	}

	edev->blocks_to_transfer = buf_size / edev->block_size;
	if(buf_size % edev->block_size)
	{
	    printf("SD: do_data_command() called with buffer size (%i) not an exact multiple of block size (%i)\n", buf_size, edev->block_size);
        return -1;
	}
	edev->buf = buf;

	// Decide on the command to use
	int command;
	if(is_write)
	{
	    if(edev->blocks_to_transfer > 1)
            command = WRITE_MULTIPLE_BLOCK;
        else
            command = WRITE_BLOCK;
	}
	else
    {
        if(edev->blocks_to_transfer > 1)
            command = READ_MULTIPLE_BLOCK;
        else
            command = READ_SINGLE_BLOCK;
    }

	int retry_count = 0;
	int max_retries = 3;
	while(retry_count < max_retries)
	{
#ifdef SDMA_SUPPORT
	    // use SDMA for the first try only
	    if((retry_count == 0) && sd_suitable_for_dma(buf))
            edev->use_sdma = 1;
        else
        {
#ifdef EMMC_DEBUG
            printf("SD: retrying without SDMA\n");
#endif
            edev->use_sdma = 0;
        }
#else
        edev->use_sdma = 0;
#endif

        sd_issue_command(edev, command, block_no, 5000000);

        if(SUCCESS(edev))
            break;
        else
        {
            printf("SD: error sending CMD%i, ", command);
            printf("error = %08x.  ", edev->last_error);
            retry_count++;
            if(retry_count < max_retries)
                printf("Retrying...\n");
            else
                printf("Giving up.\n");
        }
	}
	if(retry_count == max_retries)
    {
        edev->card_rca = 0;
        return -1;
    }

    return 0;
}

/**
 *
 * @param dev
 * @param buf
 * @param buf_size
 * @param block_no
 * @return
 */
int sd_read(struct block_device *dev, uint8_t *buf, size_t buf_size, uint32_t block_no)
{
	struct emmc_block_dev *edev = (struct emmc_block_dev *) dev;

	if (sd_ensure_data_mode(edev) != 0)
		return -1;

	SD_TRACE("Card ready, reading from block %u", block_no);

	if (sd_do_data_command(edev, 0, buf, buf_size, block_no) < 0)
		return -1;

	SD_TRACE("Data read successful");

	return buf_size;
}

#ifdef SD_WRITE_SUPPORT
/**
 *
 * @param dev
 * @param buf
 * @param buf_size
 * @param block_no
 * @return
 */
int sd_write(struct block_device *dev, uint8_t *buf, size_t buf_size,	uint32_t block_no)
{
	struct emmc_block_dev *edev = (struct emmc_block_dev *) dev;

	if (sd_ensure_data_mode(edev) != 0)
		return -1;

	SD_TRACE("Card ready, writing to block %u", block_no);

	if (sd_do_data_command(edev, 1, buf, buf_size, block_no) < 0)
		return -1;

	SD_TRACE("Write read successful");

	return buf_size;
}
#endif

