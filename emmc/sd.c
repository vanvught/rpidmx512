/**
 * @file emmc.c
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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
#include <string.h>
#include <stdlib.h>
#include "block.h"
#include "util.h"

#include "bcm2835.h"
#include "bcm2835_vc.h"
#include "bcm2835_emmc.h"

#include "sd.h"

uint32_t sd_commands[] = {
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

#define SD_DEBUG

#ifdef SD_DEBUG
int printf(const char *format, ...);
#define SD_TRACE(...)     {											\
        printf("SD   %s:%4d[%s] : ", __FILE__, __LINE__, __func__);	\
        printf(__VA_ARGS__);										\
        printf("\n"); }
#else
#define SD_TRACE(...)
#endif

// Enable 1.8V support
#define SD_1_8V_SUPPORT

// Enable 4-bit support
#define SD_4BIT_DATA

// Enable SDXC maximum performance mode
#define SDXC_MAXIMUM_PERFORMANCE

// Enable EXPERIMENTAL (and possibly DANGEROUS) SD write support
#define SD_WRITE_SUPPORT

static char driver_name[] = "emmc";
static char device_name[] = "emmc0";	// We use a single device name as there is only one card slot in the RPi

struct sd_scr
{
    uint32_t    scr[2];
    uint32_t    sd_bus_widths;
    int         sd_version;
};

static char *sd_versions[] = { "unknown", "1.0 and 1.01", "1.10", "2.00", "3.0x", "4.xx" };

#ifdef SD_DEBUG
static char *err_irpts[] = { "CMD_TIMEOUT", "CMD_CRC", "CMD_END_BIT", "CMD_INDEX",
	"DATA_TIMEOUT", "DATA_CRC", "DATA_END_BIT", "CURRENT_LIMIT",
	"AUTO_CMD12", "ADMA", "TUNING", "RSVD" };
#endif

int sd_read(struct block_device *, uint8_t *, size_t buf_size, uint32_t);
int sd_write(struct block_device *, uint8_t *, size_t buf_size, uint32_t);

/**
 * @ingroup SD
 * @param dev
 */
static void sd_issue_command(struct emmc_block_dev *dev, uint32_t command, uint32_t argument, useconds_t timeout)
{
    // First, handle any pending interrupts
	bcm2835_emmc_handle_interrupts(dev);

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
        bcm2835_emmc_issue_command(dev, sd_commands[APP_CMD], rca, timeout);
        if(dev->last_cmd_success)
        {
            dev->last_cmd = command | IS_APP_CMD;
            bcm2835_emmc_issue_command(dev, sd_acommands[command], argument, timeout);
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
        bcm2835_emmc_issue_command(dev, sd_commands[command], argument, timeout);
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

/**
 * @ingroup SD
 * @return
 */
static int sd_card_sanity_check(void) {
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

    return 0;

}

/**
 * @ingroup SD
 * @param dev
 * @return
 */
int sd_card_init(struct block_device **dev)
{
	if (sd_card_sanity_check() != 0)
		return -1;

    /***********************************************************************/

	uint32_t base_clock = bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_CORE);

	if (bcm2825_emmc_init(base_clock) !=0 )
		return -1;

    /***********************************************************************/

	// Mask off sending interrupts to the ARM
	BCM2835_EMMC->IRPT_EN = 0;
	// Reset interrupts
	BCM2835_EMMC->INTERRUPT = 0xffffffff;
	// Have all interrupts sent to the INTERRUPT register
	uint32_t irpt_mask = 0xffffffff & (~SD_CARD_INTERRUPT);
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

    /***********************************************************************/

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
		if (bcm2835_emmc_reset_cmd() == -1)
			return -1;

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

    // Here we are supposed to check the response to CMD5 (HCSS 3.6) page 100
    // It only returns if the card is a SDIO card
	SD_TRACE("Note that a timeout error on the following command (CMD5) is normal and expected if the card is not a SDIO card");
    sd_issue_command(ret, IO_SET_OP_COND, 0, 10000);

    if(!TIMEOUT(ret))
    {
        if(CMD_TIMEOUT(ret))
        {
            if(bcm2835_emmc_reset_cmd() == -1)
                return -1;

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
    bcm2835_emmc_set_clock(base_clock, SD_CLOCK_NORMAL);

	// A small wait before the voltage switch
	udelay(5000);

#ifdef SD_1_8V_SUPPORT
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
	        bcm2835_emmc_sdcard_power_off();
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
	        bcm2835_emmc_sdcard_power_off();
	        return sd_card_init((struct block_device **)&ret);
	    }
	    SD_TRACE("SD: voltage switch complete");
	}
#endif

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

/**
 * @ingroup SD
 * @param edev
 * @param is_write
 * @param buf
 * @param buf_size
 * @param block_no
 * @return
 */
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
        edev->use_sdma = 0;

        sd_issue_command(edev, command, block_no, 5000000);

		if (SUCCESS(edev))
			break;
		else
		{
			printf("SD: error sending CMD%i, ", command);
			printf("error = %08x.  ", edev->last_error);
			retry_count++;
			if (retry_count < max_retries)
				printf("Retrying...\n");
			else
				printf("Giving up.\n");
		}
	}
	if (retry_count == max_retries)
	{
		edev->card_rca = 0;
		return -1;
	}

	return 0;
}

/**
 * @ingroup SD
 * @param edev
 * @return
 */
static int sd_ensure_data_mode(struct emmc_block_dev *edev)
{
	if (edev->card_rca == 0)
	{
		int ret = sd_card_init((struct block_device **) &edev);
		if (ret != 0)
			return ret;
	}

	SD_TRACE("Obtaining status register for card_rca %08x: ", edev->card_rca);

	sd_issue_command(edev, SEND_STATUS, edev->card_rca << 16, 500000);
	if (FAIL(edev))
	{
		printf("SD: ensure_data_mode() error sending CMD13\n");
		edev->card_rca = 0;
		return -1;
	}

	uint32_t status = edev->last_r0;
	uint32_t cur_state = (status >> 9) & 0xf;

	SD_TRACE("status %i", cur_state);

	if (cur_state == 3)
	{
		// Currently in the stand-by state - select it
		sd_issue_command(edev, SELECT_CARD, edev->card_rca << 16, 500000);
		if (FAIL(edev))
		{
			printf("SD: ensure_data_mode() no response from CMD17\n");
			edev->card_rca = 0;
			return -1;
		}
	}
	else if (cur_state == 5)
	{
		// In the data transfer state - cancel the transmission
		sd_issue_command(edev, STOP_TRANSMISSION, 0, 500000);
		if (FAIL(edev))
		{
			printf("SD: ensure_data_mode() no response from CMD12\n");
			edev->card_rca = 0;
			return -1;
		}

		// Reset the data circuit
		bcm2835_emmc_reset_dat();
	}
	else if (cur_state != 4)
	{
		// Not in the transfer state - re-initialize
		int ret = sd_card_init((struct block_device **) &edev);
		if (ret != 0)
			return ret;
	}

	// Check again that we're now in the correct mode
	if (cur_state != 4)
	{
		SD_TRACE("Re-checking status: ");

		sd_issue_command(edev, SEND_STATUS, edev->card_rca << 16, 500000);
		if (FAIL(edev))
		{
			printf("SD: ensure_data_mode() no response from CMD13\n");
			edev->card_rca = 0;
			return -1;
		}
		status = edev->last_r0;
		cur_state = (status >> 9) & 0xf;

		SD_TRACE("%i", cur_state);

		if (cur_state != 4)
		{
			printf("SD: unable to initialize SD card to data mode (state %i)\n", cur_state);
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
 * @ingroup SD
 * @param dev
 * @param buf
 * @param buf_size
 * @param block_no
 * @return
 */
int sd_write(struct block_device *dev, uint8_t *buf, size_t buf_size,   uint32_t block_no)
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
