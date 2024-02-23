/**
 * emac.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstdio>

#include "gd32.h"

#include "debug.h"

#define RMII_MODE

void enet_gpio_config() {
	DEBUG_ENTRY

	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOD);
	rcu_periph_clock_enable(RCU_GPIOE);
	rcu_periph_clock_enable(RCU_GPIOG);
	rcu_periph_clock_enable(RCU_GPIOH);

	gpio_af_set(GPIOA, GPIO_AF_0, GPIO_PIN_8);
	gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_8);

	/* enable SYSCFG clock */
	rcu_periph_clock_enable(RCU_SYSCFG);

#ifdef MII_MODE

#ifdef PHY_CLOCK_MCO
	/* output HXTAL clock (25MHz) on CKOUT0 pin(PA8) to clock the PHY */
	rcu_ckout0_config(RCU_CKOUT0SRC_HXTAL, RCU_CKOUT0_DIV1);
#endif /* PHY_CLOCK_MCO */

#ifdef USE_ENET0
	syscfg_enet_phy_interface_config(ENET0, SYSCFG_ENET_PHY_MII);
#endif /* USE_ENET0 */
#ifdef USE_ENET1
	syscfg_enet_phy_interface_config(ENET1, SYSCFG_ENET_PHY_MII);
#endif /* USE_ENET1 */

#elif defined RMII_MODE
	/* choose DIV12 to get 50MHz from 600MHz on CKOUT0 pin (PA8) to clock the PHY */
	rcu_ckout0_config(RCU_CKOUT0SRC_PLL0P, RCU_CKOUT0_DIV12);

#ifdef USE_ENET0
	syscfg_enet_phy_interface_config(ENET0, SYSCFG_ENET_PHY_RMII);
#endif /* USE_ENET0 */
#ifdef USE_ENET1
	syscfg_enet_phy_interface_config(ENET1, SYSCFG_ENET_PHY_RMII);
#endif /* USE_ENET1 */

#endif /* MII_MODE */

#ifdef USE_ENET0
#ifdef MII_MODE

	/* PA1: ETH0_MII_RX_CLK */
	gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_1);

	/* PA2: ETH0_MDIO */
	gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_2);

	/* PA7: ETH0_MII_RX_DV */
	gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_7);

	gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_1);
	gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_2);
	gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_7);

	/* PB8: ETH0_MII_TXD3 */
	gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_8);

	/* PB10: ETH0_MII_RX_ER */
	gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);
	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_10);

	gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_8);
	gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_10);

	/* PC1: ETH0_MDC */
	gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_1);

	/* PC2: ETH0_MII_TXD2 */
	gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_2);

	/* PC3: ETH0_MII_TX_CLK */
	gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_3);

	/* PC4: ETH0_MII_RXD0 */
	gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_4);

	/* PC5: ETH0_MII_RXD1 */
	gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_5);

	gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_1);
	gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_2);
	gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_3);
	gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_4);
	gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_5);

	/* PH2: ETH0_MII_CRS */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_2);

	/* PH3: ETH0_MII_COL */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_3);

	/* PH6: ETH0_MII_RXD2 */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_6);

	/* PH7: ETH0_MII_RXD3 */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_7);

	gpio_af_set(GPIOH, GPIO_AF_11, GPIO_PIN_2);
	gpio_af_set(GPIOH, GPIO_AF_11, GPIO_PIN_3);
	gpio_af_set(GPIOH, GPIO_AF_11, GPIO_PIN_6);
	gpio_af_set(GPIOH, GPIO_AF_11, GPIO_PIN_7);

	/* PG11: ETH0_MII_TX_EN */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_11);

	/* PG13: ETH0_MII_TXD0 */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_13);

	/* PG14: ETH0_MII_TXD1 */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_14);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_14);

	gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_11);
	gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_13);
	gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_14);

	/* PD8: ETH0_INT */
	gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_8);

#elif defined RMII_MODE

	/* PA1: ETH0_RMII_REF_CLK */
	gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_1);

	/* PA2: ETH0_MDIO */
	gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_2);

	/* PA7: ETH0_RMII_CRS_DV */
	gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_7);

	gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_1);
	gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_2);
	gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_7);

	/* PG11: ETH0_RMII_TX_EN */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_11);

	/* PB12: ETH0_RMII_TXD0 */
	gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_12);

	/* PG12: ETH0_RMII_TXD1 */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_12);

	gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_11);
	gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_12);
	gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_12);

	/* PC1: ETH0_MDC */
	gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_1);

	/* PC4: ETH0_RMII_RXD0 */
	gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_4);

	/* PC5: ETH0_RMII_RXD1 */
	gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_5);

	gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_1);
	gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_4);
	gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_5);

#endif /* MII_MODE */
#endif /* USE_ENET0 */

#ifdef USE_ENET1
#ifdef MII_MODE

	/* PH6: ETH1_MII_RXD2 */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_6);

	/* PH7: ETH1_MII_RXD3 */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_7);

	/* PH8: ETH1_MII_RXD0 */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_8);

	/* PH9: ETH1_MII_RXD1 */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_9);

	/* PH10: ETH1_MII_RX_ER */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_10);

	/* PH11: ETH1_MII_RX_DV */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_11);

	/* PH12: ETH1_MII_RX_CLK */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_12);

	/* PH13: ETH1_MII_COL */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_13);

	/* PH14: ETH1_MDIO */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_14);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_14);

	/* PH15: ETH1_MII_CRS */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_15);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_15);

	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_6);
	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_7);
	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_8);
	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_9);
	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_10);
	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_11);
	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_12);
	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_13);
	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_14);
	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_15);

	/* PG6: ETH1_MDC */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_6);

	/* PG9: ETH1_MII_TX_CLK */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_9);

	/* PG11: ETH1_MII_TX_EN */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_11);

	/* PG12: ETH1_MII_TXD2 */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_12);

	/* PG13: ETH1_MII_TXD0 */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_13);

	/* PG14: ETH1_MII_TXD1 */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_14);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_14);

	/* PG15: ETH1_MII_TXD3 */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_15);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_15);

	gpio_af_set(GPIOG, GPIO_AF_6, GPIO_PIN_6);
	gpio_af_set(GPIOG, GPIO_AF_6, GPIO_PIN_9);
	gpio_af_set(GPIOG, GPIO_AF_6, GPIO_PIN_11);
	gpio_af_set(GPIOG, GPIO_AF_6, GPIO_PIN_12);
	gpio_af_set(GPIOG, GPIO_AF_6, GPIO_PIN_13);
	gpio_af_set(GPIOG, GPIO_AF_6, GPIO_PIN_14);
	gpio_af_set(GPIOG, GPIO_AF_6, GPIO_PIN_15);

	/* PE1: ETH1_INT */
	gpio_mode_set(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_1);

#elif defined RMII_MODE

	/* PH8: ETH1_RMII_RXD0 */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_8);

	/* PH9: ETH1_RMII_RXD1 */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_9);

	/* PH11: ETH1_RMII_CRS_DV */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_11);

	/* PH12: ETH1_RMII_REF_CLK */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_12);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_12);

	/* PH14: ETH1_MDIO */
	gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_14);
	gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_14);


	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_8);
	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_9);
	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_11);
	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_12);
	gpio_af_set(GPIOH, GPIO_AF_6, GPIO_PIN_14);

	/* PG6: ETH1_MDC */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_6);

	/* PG11: ETH1_RMII_TX_EN */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_11);

	/* PG13: ETH1_RMII_TXD0 */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_13);

	/* PG14: ETH1_RMII_TXD1 */
	gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_14);
	gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_14);

	gpio_af_set(GPIOG, GPIO_AF_6, GPIO_PIN_6);
	gpio_af_set(GPIOG, GPIO_AF_6, GPIO_PIN_11);
	gpio_af_set(GPIOG, GPIO_AF_6, GPIO_PIN_13);
	gpio_af_set(GPIOG, GPIO_AF_6, GPIO_PIN_14);

#endif /* MII_MODE */
#endif /* USE_ENET1 */
	DEBUG_EXIT
}
