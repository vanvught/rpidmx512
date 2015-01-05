/**
 * @file bcm2835_emmc.c
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

#include <string.h>

extern int printf(const char *format, ...);

#include "timer.h"

#include "bcm2835.h"
#include "bcm2835_vc.h"
#include "bcm2835_emmc.h"

#include "sd.h"

extern uint32_t sd_commands[];

// SDMA buffer address
#define SDMA_BUFFER     0x6000
#define SDMA_BUFFER_PA  (SDMA_BUFFER + 0xC0000000)

#define EMMC_DEBUG

/* Tracing macros */
#ifdef EMMC_DEBUG
#define EMMC_TRACE(...)     {										\
        printf("EMMC %s:%4d[%s] : ", __FILE__, __LINE__, __func__);	\
        printf(__VA_ARGS__);										\
        printf("\n"); }
#else
#define EMMC_TRACE(...)
#endif


/**
 * @ingroup EMMC
 * Set SD Bus Power bit off in Power Control Register
 * Not implemented in BCM2835 External Mass Media Controller (EMMC)
 */
void bcm2835_emmc_sdcard_power_off(void)
{
#if 0
	uint32_t control0 = BCM2835_EMMC->CONTROL0;
	control0 &= ~BCM2835_EMMC_CONTROL0_POWER_ON;
	BCM2835_EMMC->CONTROL0 = control0;
#endif
}

/**
 * @ingroup EMMC
 * Checking for an inserted card
 * Not implemented in BCM2835 External Mass Media Controller (EMMC)
 * @return
 */
static uint32_t bcm2835_emmc_sdcard_inserted(void)
{
#if 0
	TIMEOUT_WAIT(BCM2835_EMMC->STATUS & BCM2835_EMMC_STATUS_CARD_PRESENT, 500000);
#endif
	uint32_t status_reg = BCM2835_EMMC->STATUS;
#if 0
	if((status_reg & BCM2835_EMMC_STATUS_CARD_PRESENT) == 0)
	{
		printf("EMMC: no card inserted\n");
		return -1;
	}
#endif
	EMMC_TRACE("Status: %08x", status_reg);

	return 0;
}

/**
 * @ingroup EMMC
 * @return
 */
static uint32_t bcm2835_emmc_get_version(void)
{
	uint32_t ver = BCM2835_EMMC->SLOTISR_VER;
	uint32_t vendor = ver >> 24;
	uint32_t sdversion = (ver >> 16) & 0xff;
	uint32_t slot_status = ver & 0xff;
	EMMC_TRACE("vendor %x, sdversion %x, slot_status %x", vendor, sdversion, slot_status);

	return sdversion;
}

/**
 * @ingroup EMMC
 * @return
 */
static uint32_t bcm2835_emmc_do_reset(void)
{
	uint32_t control1 = BCM2835_EMMC->CONTROL1;
	control1 |= BCM2835_EMMC_CONTROL1_RESET_ALL;
	control1 &= ~BCM2835_EMMC_CONTROL1_CLOCK_CARD_EN;
	control1 &= ~BCM2835_EMMC_CONTROL1_CLOCK_INT_EN;
	BCM2835_EMMC->CONTROL1 = control1;

	TIMEOUT_WAIT((BCM2835_EMMC->CONTROL1 & (0x7 << 24)) == 0, 1000000);

	if((BCM2835_EMMC->CONTROL1 & (0x7 << 24)) != 0)
	{
		printf("EMMC: controller did not reset properly\n");
		return -1;
	}

	EMMC_TRACE("control0: %08x, control1: %08x, control2: %08x", BCM2835_EMMC->CONTROL0, BCM2835_EMMC->CONTROL1, BCM2835_EMMC->CONTROL2);

	return 0;
}

/**
 * @ingroup EMMC
 * Reset the CMD line
 *
 * @return 0 if successful; -1 otherwise.
 */
int bcm2835_emmc_reset_cmd(void)
{
	uint32_t control1 = BCM2835_EMMC ->CONTROL1;
	control1 |= BCM2835_EMMC_CONTROL1_RESET_CMD;
	BCM2835_EMMC ->CONTROL1 = control1;

	TIMEOUT_WAIT((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_RESET_CMD) == 0, 1000000);

	if ((BCM2835_EMMC ->CONTROL1 & BCM2835_EMMC_CONTROL1_RESET_CMD )!= 0){
		printf("EMMC: CMD line did not reset properly\n");
		return -1;
	}

	BCM2835_EMMC ->INTERRUPT = SD_ERR_MASK_CMD_TIMEOUT;

	return 0;
}

/**
 * @ingroup EMMC
 * Reset the CMD line
 *
 * @return 0 if successful; -1 otherwise.
 */
int bcm2835_emmc_reset_dat(void)
{
	uint32_t control1 = BCM2835_EMMC ->CONTROL1;
	control1 |= BCM2835_EMMC_CONTROL1_RESET_DATA;
	BCM2835_EMMC ->CONTROL1 = control1;

	TIMEOUT_WAIT((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_RESET_DATA) == 0, 1000000);

	if ((BCM2835_EMMC ->CONTROL1 & BCM2835_EMMC_CONTROL1_RESET_DATA )!= 0){
		printf("EMMC: DAT line did not reset properly\n");
		return -1;
	}

	return 0;
}

/**
 * @ingroup EMMC
 * Set the clock dividers to generate a target value
 *
 * @param base_clock
 * @param target_rate
 * @return divider
 */
static uint32_t bcm2835_emmc_get_clock_divider(uint32_t base_clock, uint32_t target_clock)
{
	int divisor = 0;
	int real_div = divisor;
	uint32_t ret = 0;

	if (base_clock <= target_clock)
		divisor = 1;
	else
	{
		for (divisor = 2; divisor < SDHCI_MAX_DIV_SPEC_300; divisor += 2)
		{
			if ((base_clock / divisor) <= target_clock)
				break;
		}
	}

	real_div = divisor;
	divisor >>= 1;

	int actual_clock;

	if (real_div)
		actual_clock = base_clock / real_div;

	ret |= (divisor & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT;
	ret |= ((divisor & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN) << SDHCI_DIVIDER_HI_SHIFT;

	EMMC_TRACE("base_clock: %i, target_rate: %i, divisor: %08x, actual_clock: %i, ret: %08x", base_clock, target_clock, divisor, actual_clock, ret);

	return ret;
}

/**
 * @ingroup EMMC
 * @param base_clock
 * @param target_rate
 * @return
 */
uint32_t bcm2835_emmc_set_clock(uint32_t target_clock)
{
	uint32_t base_clock = bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_CORE);
	uint32_t divider = bcm2835_emmc_get_clock_divider(base_clock, target_clock);

	// Wait for the command inhibit (CMD and DAT) bits to clear
	while (BCM2835_EMMC ->STATUS & (BCM2835_EMMC_STATUS_CMD_INHIBIT || BCM2835_EMMC_STATUS_DATA_INHIBIT ))
		;

	// Set the SD clock off
	uint32_t control1 = BCM2835_EMMC ->CONTROL1;
	control1 &= ~BCM2835_EMMC_CONTROL1_CLOCK_CARD_EN;
	BCM2835_EMMC ->CONTROL1 = control1;
	udelay(2000);

	// Write the new divider
	control1 &= ~0xffe0;		// Clear old setting + clock generator select
	control1 |= divider;
	BCM2835_EMMC ->CONTROL1 = control1;
	udelay(2000);

	// Enable the SD clock
	control1 |= BCM2835_EMMC_CONTROL1_CLOCK_CARD_EN;
	BCM2835_EMMC ->CONTROL1 = control1;
	udelay(2000);

	EMMC_TRACE("Successfully set clock rate to %i Hz", target_clock);

	return 0;
}

/**
 * @ingroup EMMC
 * @return
 */
static uint32_t bcm2825_emmc_set_id_clock(void)
{
	BCM2835_EMMC->CONTROL2 = 0;

	EMMC_TRACE("Setting clock rate (to something slow)");

	uint32_t control1 = BCM2835_EMMC->CONTROL1;
	control1 |= BCM2835_EMMC_CONTROL1_CLOCK_INT_EN;

	EMMC_TRACE("Set to identification frequency : %d", SD_CLOCK_ID);

	uint32_t base_clock = bcm2835_vc_get_clock_rate(BCM2835_VC_CLOCK_ID_CORE);

	uint32_t f_id = bcm2835_emmc_get_clock_divider(base_clock, SD_CLOCK_ID);

	if(f_id == SD_GET_CLOCK_DIVIDER_FAIL)
	{
		printf("EMMC: unable to get a valid clock divider for ID frequency\n");
		return -1;
	}

	control1 |= f_id;
	control1 |= (7 << 16);		// data timeout = TMCLK * 2^10
	BCM2835_EMMC->CONTROL1 = control1;

	TIMEOUT_WAIT(BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_CLOCK_INT_STABLE, 0x1000000);

	if ((BCM2835_EMMC->CONTROL1 & BCM2835_EMMC_CONTROL1_CLOCK_INT_STABLE ) == 0)
	{
		printf("EMMC: controller's clock did not stabilize within 1 second\n");
		return -1;
	}

	EMMC_TRACE("control0: %08x, control1: %08x", BCM2835_EMMC->CONTROL0, BCM2835_EMMC->CONTROL1);

	EMMC_TRACE("Enabling SD clock");

	udelay(2000);

	control1 = BCM2835_EMMC->CONTROL1;
	control1 |= BCM2835_EMMC_CONTROL1_CLOCK_CARD_EN;
	BCM2835_EMMC->CONTROL1 = control1;

	udelay(2000);

	return 0;
}

/**
 * @ingroup EMMC
 * @return  0 if successful; 1 otherwise.
 */
static int emmc_power_cycle()
{

	if (bcm2835_vc_get_power_state(BCM2835_VC_POWER_ID_SDCARD) != 0)
	{
		int32_t ret = bcm2835_vc_set_power_state(BCM2835_VC_POWER_ID_SDCARD, BCM2835_VC_SET_POWER_STATE_OFF_WAIT);

		if (ret < 0) {
			printf("EMMC: %s : bcm2835_vc_set_power_state() did not return a valid response.\n", __func__);
			return -1;
		}

		if (ret == BCM2835_VC_POWER_STATE_RESP_ON) {
			EMMC_TRACE("Device did not power off successfully (%08x).", ret);
			return 1;
		}

		udelay(5000);
	}

	int32_t ret = bcm2835_vc_set_power_state(BCM2835_VC_POWER_ID_SDCARD, BCM2835_VC_SET_POWER_STATE_ON_WAIT);

	if (ret < 0) {
		printf("EMMC: %s : bcm2835_vc_set_power_state() did not return a valid response.\n", __func__);
		return -1;
	}

	if(ret != BCM2835_VC_POWER_STATE_RESP_ON)
	{
		EMMC_TRACE("Device did not power on successfully (%08x).", ret);
		return 1;
	}

	return 0;
}

/**
 * @ingroup EMMC
 * @param base_clock
 * @return 0 if successful; -1 otherwise.
 */
uint32_t bcm2825_emmc_init(void)
{
	if(emmc_power_cycle() != 0)
	{
		printf("EMMC: BCM2835 controller did not power cycle successfully\n");
		return -1;
	}

	EMMC_TRACE("BCM2835 controller power-cycled");

	if(bcm2835_emmc_get_version() < 2)
	{
		printf("EMMC: only SDHCI versions >= 3.0 are supported\n");
		return -1;
	}

	if(bcm2835_emmc_do_reset() != 0)
	{
		printf("EMMC: controller did not reset properly\n");
		return -1;
	}

	EMMC_TRACE("capabilities: %08x%08x", BCM2835_EMMC->CAPABILITIES_1, BCM2835_EMMC->CAPABILITIES_0);

	if(bcm2835_emmc_sdcard_inserted() != 0)
	{
		printf("EMMC: no card inserted\n");
		return -1;
	}

	if (bcm2825_emmc_set_id_clock() != 0)
	{
		printf("EMMC: controller did not set low clock rate\n");
		return -1;
	}

	return 0;
}

/**
 * @ingroup EMMC
 * @param dev
 * @param cmd_reg
 * @param argument
 * @param timeout
 */
void bcm2835_emmc_issue_command(struct emmc_block_dev *dev, uint32_t cmd_reg, uint32_t argument, uint32_t timeout)
{
    dev->last_cmd_reg = cmd_reg;
    dev->last_cmd_success = 0;

    // This is as per HCSS 3.7.1.1/3.7.2.2

    // Check Command Inhibit
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
               while(BCM2835_EMMC->STATUS & BCM2835_EMMC_STATUS_DATA_INHIBIT)
            	;
        }
    }

    // Is this a DMA transfer?
    int is_sdma = 0;
    if((cmd_reg & SD_CMD_ISDATA) && (dev->use_sdma))
    {
    	EMMC_TRACE("Performing SDMA transfer, current INTERRUPT: %08x", BCM2835_EMMC->INTERRUPT);
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
#ifdef SDCARD_DEBUG
			if(dev->blocks_to_transfer > 1)
				EMMC_TRACE("Multi block transfer, awaiting block %i ready", cur_block);
#endif
			TIMEOUT_WAIT(BCM2835_EMMC->INTERRUPT & (wr_irpt | 0x8000), timeout);
			irpts = BCM2835_EMMC->INTERRUPT;
			BCM2835_EMMC->INTERRUPT = 0xffff0000 | wr_irpt;

            if((irpts & (0xffff0000 | wr_irpt)) != wr_irpt)
            {
            	EMMC_TRACE("Error occurred whilst waiting for data ready interrupt");
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

            EMMC_TRACE("block %i transfer complete", cur_block);

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
            	EMMC_TRACE("Error occurred whilst waiting for transfer complete interrupt");
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
            	EMMC_TRACE("Error: DMA interrupt occured without transfer complete");
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

			if(irpts == 0)
			{
				EMMC_TRACE("Timeout waiting for SDMA transfer to complete");
			}
			else
			{
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
 * @ingroup EMMC
 * @param dev
 */
static void bcm2835_emmc_handle_card_interrupt(struct emmc_block_dev *dev)
{
	EMMC_TRACE("Controller status: %08x", BCM2835_EMMC->STATUS);
	EMMC_TRACE("Get the card status");

    if(dev->card_rca)
    {
    	bcm2835_emmc_issue_command(dev, sd_commands[SEND_STATUS], dev->card_rca << 16, 500000);

        if(FAIL(dev))
		{
        	EMMC_TRACE("Unable to get card status");
		}
		else
		{
			EMMC_TRACE("Card status: %08x", dev->last_r0);
		}
	}
	else
	{
		EMMC_TRACE("No card currently selected");
	}
}

/**
 * @ingroup EMMC
 * @param dev
 */
void bcm2835_emmc_handle_interrupts(struct emmc_block_dev *dev)
{
	uint32_t irpts = BCM2835_EMMC->INTERRUPT;
    uint32_t reset_mask = 0;

    if(irpts & SD_COMMAND_COMPLETE)
    {
    	EMMC_TRACE("Spurious command complete interrupt\n");
        reset_mask |= SD_COMMAND_COMPLETE;
    }

    if(irpts & SD_TRANSFER_COMPLETE)
    {
    	EMMC_TRACE("Spurious transfer complete interrupt");
        reset_mask |= SD_TRANSFER_COMPLETE;
    }

    if(irpts & SD_BLOCK_GAP_EVENT)
    {
    	EMMC_TRACE("Spurious block gap event interrupt");
        reset_mask |= SD_BLOCK_GAP_EVENT;
    }

    if(irpts & SD_DMA_INTERRUPT)
    {
    	EMMC_TRACE("Spurious DMA interrupt");
        reset_mask |= SD_DMA_INTERRUPT;
    }

    if(irpts & SD_BUFFER_WRITE_READY)
    {
    	EMMC_TRACE("Spurious buffer write ready interrupt");
        reset_mask |= SD_BUFFER_WRITE_READY;
        bcm2835_emmc_reset_dat();
    }

    if(irpts & SD_BUFFER_READ_READY)
    {
    	EMMC_TRACE("Spurious buffer read ready interrupt");
        reset_mask |= SD_BUFFER_READ_READY;
        bcm2835_emmc_reset_dat();
    }

    if(irpts & SD_CARD_INSERTION)
    {
    	EMMC_TRACE("Card insertion detected");
        reset_mask |= SD_CARD_INSERTION;
    }

    if(irpts & SD_CARD_REMOVAL)
    {
    	EMMC_TRACE("Card removal detected");
        reset_mask |= SD_CARD_REMOVAL;
        dev->card_removal = 1;
    }

    if(irpts & SD_CARD_INTERRUPT)
    {
    	EMMC_TRACE("Card interrupt detected");
    	bcm2835_emmc_handle_card_interrupt(dev);
        reset_mask |= SD_CARD_INTERRUPT;
    }

    if(irpts & 0x8000)
    {
    	EMMC_TRACE("Spurious error interrupt: %08x", irpts);
        reset_mask |= 0xffff0000;
    }

    BCM2835_EMMC->INTERRUPT = reset_mask;
}

/**
 * @ingroup EMMC
 * Mask off sending interrupts to the ARM.
 * Reset interrupts.
 * Have all interrupts sent to the INTERRUPT register.
 */
void bcm2835_emmc_enable_all_interrupts_not_arm(void)
{
	BCM2835_EMMC->IRPT_EN = 0;
	BCM2835_EMMC->INTERRUPT = 0xffffffff;
    BCM2835_EMMC->IRPT_MASK = 0xffffffff & (~SD_CARD_INTERRUPT);
}

/**
 * @ingroup EMMC
 * Disable card interrupt in host.
 */
uint32_t bcm2835_emmc_disable_card_interrupt(void)
{
    uint32_t old_irpt_mask = BCM2835_EMMC->IRPT_MASK;
    BCM2835_EMMC->IRPT_MASK = old_irpt_mask & (~SD_CARD_INTERRUPT);

    return old_irpt_mask;
}

/**
 * @ingroup EMMC
 * @param old_irpt_mask
 */
void bcm2835_emmc_4bit_mode_change_bit(uint32_t old_irpt_mask)
{
	// Change bit mode for Host
	// TODO
	uint32_t control0 = BCM2835_EMMC ->CONTROL0;
	control0 |= 0x2;
	BCM2835_EMMC ->CONTROL0 = control0;

	// Re-enable card interrupt in host
	BCM2835_EMMC ->IRPT_MASK = old_irpt_mask;

	EMMC_TRACE("Switch to 4-bit complete");

}

/**
 * @ingroup EMMC
 * @param block_size
 */
void bcm2835_emmc_set_block_size(uint32_t block_size)
{
	uint32_t controller_block_size = BCM2835_EMMC->BLKSIZECNT;
	controller_block_size &= (~0xfff);
	controller_block_size |= block_size;
	BCM2835_EMMC->BLKSIZECNT = controller_block_size;
}

/**
 * @ingroup EMMC
 */
void bcm2835_emmc_reset_interrupt_register(void)
{
	BCM2835_EMMC->INTERRUPT = 0xffffffff;
}

