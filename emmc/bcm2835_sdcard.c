/**
 * @file bcm2835_sdcard.c
 *
 */

#include "bcm2835.h"
#include "bcm2835_emmc.h"
#include "bcm2835_sdcard.h"

/**
 * @ingroup SDCARD
 */
void bcm2835_sdcard_power_off(void)
{
	uint32_t control0 = BCM2835_EMMC->CONTROL0;
	control0 &= ~BCM2835_EMMC_CONTROL0_POWER_ON; // Set SD Bus Power bit off in Power Control Register
	BCM2835_EMMC->CONTROL0 = control0;
}
