/**
 * @file bcm2835_emmc.c
 *
 */

extern int printf(const char *format, ...);

#include "timer.h"

#include "bcm2835.h"
#include "bcm2835_vc.h"
#include "bcm2835_emmc.h"

#include "sdcard.h"

extern uint32_t hci_ver;

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
 *
 * @return
 */
int bcm2835_emmc_power_off(void)
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
int bcm2835_emmc_power_on(void)
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
 * @ingroup EMMC
 * Reset the CMD line
 *
 * @return
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

	return 0;
}

/**
 * @ingroup EMMC
 * Reset the CMD line
 *
 * @return
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
 * @return
 */
uint32_t bcm2835_emmc_get_clock_divider(uint32_t base_clock, uint32_t target_rate)
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
 * @ingroup EMMC
 * @param base_clock
 * @param target_rate
 * @return
 */
uint32_t bcm2835_emmc_switch_clock_rate(uint32_t base_clock, uint32_t target_rate)
{
  uint32_t divider = bcm2835_emmc_get_clock_divider(base_clock, target_rate);

  if (divider == SD_GET_CLOCK_DIVIDER_FAIL)
    {
      printf("EMMC: couldn't get a valid divider for target rate %i Hz\n", target_rate);
      return -1;
    }

  // Wait for the command inhibit (CMD and DAT) bits to clear
  while (BCM2835_EMMC ->STATUS & (BCM2835_EMMC_STATUS_CMD_INHIBIT || BCM2835_EMMC_STATUS_DATA_INHIBIT ))
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
