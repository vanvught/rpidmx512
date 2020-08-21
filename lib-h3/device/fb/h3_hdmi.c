#pragma GCC push_options
#pragma GCC optimize ("Os")
/**
 * @file h3_hdmi.c
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
/*
 * https://github.com/u-boot/u-boot/blob/master/drivers/video/sunxi/sunxi_dw_hdmi.c
 *
 * Optimized for HDMI/DVI fixed resolution and now can be compiled with
 * -Wpedantic -Wsign-conversion
 *
 */
// SPDX-License-Identifier: GPL-2.0+
/*
 * Allwinner DW HDMI bridge
 *
 * (C) Copyright 2017 Jernej Skrabec <jernej.skrabec@siol.net>
 */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "h3.h"
#include "h3_ccu.h"
#include "h3_hs_timer.h"

#include "display_timing.h"

#include "dw_hdmi.h"

struct sunxi_dw_hdmi_priv {
	struct dw_hdmi hdmi;
	int mux;
};

static struct sunxi_dw_hdmi_priv _sunxi_dw_hdmi_priv;

#define CCU_PLL_VIDEO_CTRL_EN			(1U << 31)
#define CCU_PLL_VIDEO_CTRL_LOCK			(1U << 28)
#define CCU_PLL_VIDEO_CTRL_INTEGER_MODE	(1U << 24)
#define CCU_PLL_VIDEO_CTRL_N_SHIFT		8U
#define CCU_PLL_VIDEO_CTRL_N_MASK		(0x7f << CCU_PLL_VIDEO_CTRL_N_SHIFT)
#define CCU_PLL_VIDEO_CTRL_N(n)			((((n) - 1) & 0x7f) << CCU_PLL_VIDEO_CTRL_N_SHIFT)
#define CCU_PLL_VIDEO_CTRL_M_SHIFT		0U
#define CCU_PLL_VIDEO_CTRL_M_MASK		(0xf << CCU_PLL_VIDEO_CTRL_M_SHIFT)
#define CCU_PLL_VIDEO_CTRL_M(n)			((((n) - 1) & 0xf) << CCU_PLL_VIDEO_CTRL_M_SHIFT)

#define CCU_HDMI_CTRL_GATE				(1U << 31)
#define CCU_HDMI_CTRL_PLL_MASK			(3U << 24)
#define CCU_HDMI_CTRL_PLL_VIDEO			(0U << 24)

#define CCU_HDMI_SLOW_CTRL_DDC_GATE		(1U << 31)

#define CCU_LCD0_CTRL_GATE				(1U << 31)
#define CCU_LCD0_CTRL_M(n)				((((n) - 1) & 0xf) << 0)

#define BIT(nr)                		(1UL << (nr))

static uint32_t hdmi_get_divider(uint32_t clock) {
	/*
	 * Due to missing documentation of HDMI PHY, we know correct
	 * settings only for following four PHY dividers. Select one
	 * based on clock speed.
	 */
	if (clock <= 27000000) {
		return 11;
	}

	if (clock <= 74250000) {
		return 4;
	}

	if (clock <= 148500000) {
		return 2;
	}

	return 1;
}

static void hdmi_phy_init(void) {
	uint32_t tmo;
	uint32_t tmp;

	/*
	 * HDMI PHY settings are taken as-is from Allwinner BSP code.
	 * There is no documentation.
	 */
	H3_HDMI_PHY->CTRL = 0;

	H3_HDMI_PHY->CTRL |= BIT(0);
	udelay(5);

	H3_HDMI_PHY->CTRL |= BIT(16);
	H3_HDMI_PHY->CTRL |= BIT(1);
	udelay(10);

	H3_HDMI_PHY->CTRL |= BIT(2);
	udelay(5);

	H3_HDMI_PHY->CTRL |= BIT(3);
	udelay(40);

	H3_HDMI_PHY->CTRL |= BIT(19);
	udelay(100);

	H3_HDMI_PHY->CTRL |= BIT(18);

	H3_HDMI_PHY->CTRL |= (7U << 4);

	/* Note that Allwinner code doesn't fail in case of timeout */
	tmo = h3_hs_timer_lo_us() + 2000;

	while ((H3_HDMI_PHY->STATUS & 0x80) == 0) {
		if (h3_hs_timer_lo_us() > tmo) {
			printf("Warning: HDMI PHY init timeout!\n");
			break;
		}
	}

	H3_HDMI_PHY->CTRL |= (0xf << 8);
	H3_HDMI_PHY->CTRL |= BIT(7);

	H3_HDMI_PHY->PLL = 0x39dc5040;
	H3_HDMI_PHY->CLK = 0x80084343;
	udelay(10000);

	H3_HDMI_PHY->UNK3 = 1;
	H3_HDMI_PHY->PLL |= BIT(25);
	udelay(100000);

	tmp = (H3_HDMI_PHY->STATUS & 0x1f800) >> 11;

	H3_HDMI_PHY->PLL |= (BIT(31) | BIT(30));
	H3_HDMI_PHY->PLL |= tmp;

	H3_HDMI_PHY->CTRL = 0x01FF0F7F;
	H3_HDMI_PHY->UNK1 = 0x80639000;
	H3_HDMI_PHY->UNK2 = 0x0F81C405;

	/* enable read access to HDMI controller */
	H3_HDMI_PHY->READ_EN = 0x54524545;
	/* descramble register offsets */
	H3_HDMI_PHY->UNSCRAMBLE = 0x42494E47;
}

static int hdmi_get_plug_in_status(void) {
	return !!(H3_HDMI_PHY->STATUS & (1U << 19));
}

static int hdmi_wait_for_hpd(void) {
	uint32_t start;

	start = h3_hs_timer_lo_us();

	do {
		if (hdmi_get_plug_in_status()) {
			return 0;
		}
		udelay(100);
	} while ((h3_hs_timer_lo_us() - start) < 300);

	return -1;
}

static void hdmi_phy_set(uint32_t clock) {
	uint32_t div = hdmi_get_divider(clock);
	uint32_t tmp;

	/*
	 * Unfortunately, we don't know much about those magic
	 * numbers. They are taken from Allwinner BSP driver.
	 */
	switch (div) {
	case 1:
		H3_HDMI_PHY->PLL = 0x30dc5fc0;
		H3_HDMI_PHY->CLK = 0x800863C0;
		udelay(1000 * 10);

		H3_HDMI_PHY->UNK3 = 0x00000001;
		H3_HDMI_PHY->PLL |= BIT(25);
		udelay(1000 * 200);

		tmp = (H3_HDMI_PHY->STATUS & 0x1f800) >> 11;
		H3_HDMI_PHY->PLL |= (BIT(31) | BIT(30));

		if (tmp < 0x3d) {
			H3_HDMI_PHY->PLL |= (tmp + 2);
		} else {
			H3_HDMI_PHY->PLL |= 0x3f;
		}
		udelay(1000 * 100);

		H3_HDMI_PHY->CTRL = 0x01FFFF7F;
		H3_HDMI_PHY->UNK1 = 0x8063b000;
		H3_HDMI_PHY->UNK2 = 0x0F8246B5;
		break;
	case 2:
		H3_HDMI_PHY->PLL = 0x39dc5040;
		H3_HDMI_PHY->CLK = 0x80084381;
		udelay(1000 * 10);

		H3_HDMI_PHY->UNK3 = 0x00000001;
		H3_HDMI_PHY->PLL |= BIT(25);
		udelay(1000 * 100);

		tmp = (H3_HDMI_PHY->STATUS & 0x1f800) >> 11;
		H3_HDMI_PHY->PLL |= (BIT(31) | BIT(30));
		H3_HDMI_PHY->PLL |= tmp;
		H3_HDMI_PHY->CTRL = 0x01FFFF7F;
		H3_HDMI_PHY->UNK1 = 0x8063a800;
		H3_HDMI_PHY->UNK2 = 0x0F81C485;
		break;
	case 4:
		H3_HDMI_PHY->PLL = 0x39dc5040;
		H3_HDMI_PHY->CLK = 0x80084343;
		udelay(1000 * 10);

		H3_HDMI_PHY->UNK3 = 0x00000001;
		H3_HDMI_PHY->PLL |= BIT(25);
		udelay(1000 * 100);

		tmp = (H3_HDMI_PHY->STATUS & 0x1f800) >> 11;
		H3_HDMI_PHY->PLL |=  (BIT(31) | BIT(30));
		H3_HDMI_PHY->PLL |= tmp;
		H3_HDMI_PHY->CTRL = 0x01FFFF7F;
		H3_HDMI_PHY->UNK1 = 0x8063b000;
		H3_HDMI_PHY->UNK2 = 0x0F81C405;
		break;
	case 11:
		H3_HDMI_PHY->PLL = 0x39dc5040;
		H3_HDMI_PHY->CLK = 0x8008430a;
		udelay(1000 * 10);

		H3_HDMI_PHY->UNK3 = 0x00000001;
		H3_HDMI_PHY->PLL |= BIT(25);
		udelay(1000 * 100);

		tmp = (H3_HDMI_PHY->STATUS & 0x1f800) >> 11;
		H3_HDMI_PHY->PLL |= (BIT(31) | BIT(30));
		H3_HDMI_PHY->PLL |= tmp;
		H3_HDMI_PHY->CTRL = 0x01FFFF7F;
		H3_HDMI_PHY->UNK1 = 0x8063b000;
		H3_HDMI_PHY->UNK2 = 0x0F81C405;
		break;
	}
}

static void clock_set_pll_video_factors(uint32_t m, uint32_t n) {
	/* VIDEO rate = 24000000 * n / m */
	H3_CCU->PLL_VIDEO_CTRL = CCU_PLL_VIDEO_CTRL_EN | CCU_PLL_VIDEO_CTRL_INTEGER_MODE | CCU_PLL_VIDEO_CTRL_N(n) | CCU_PLL_VIDEO_CTRL_M(m);

	while (!(H3_CCU->PLL_VIDEO_CTRL & CCU_PLL_VIDEO_CTRL_LOCK))
		;
}

static uint32_t clock_get_pll_video(void) {
	uint32_t rval = H3_CCU->PLL_VIDEO_CTRL;

	const uint32_t n = ((rval & CCU_PLL_VIDEO_CTRL_N_MASK) >> CCU_PLL_VIDEO_CTRL_N_SHIFT) + 1;
	const uint32_t m = ((rval & CCU_PLL_VIDEO_CTRL_M_MASK) >> CCU_PLL_VIDEO_CTRL_M_SHIFT) + 1;

	/* Multiply by 1000 after dividing by m to avoid integer overflows */
	return (24000 * n / m) * 1000;
}

static void hdmi_pll_set(uint32_t clk_khz) {
	uint32_t value, n, m, div = 0, diff;
	uint32_t best_n = 0, best_m = 0, best_diff = 0x0FFFFFFF;

	div = hdmi_get_divider(clk_khz * 1000);

	/*
	 * Find the lowest divider resulting in a matching clock. If there
	 * is no match, pick the closest lower clock, as monitors tend to
	 * not sync to higher frequencies.
	 */
	for (m = 1; m <= 16; m++) {
		n = (m * div * clk_khz) / 24000;

		if ((n >= 1) && (n <= 128)) {
			value = (24000 * n) / m / div;
			diff = clk_khz - value;
			if (diff < best_diff) {
				best_diff = diff;
				best_m = m;
				best_n = n;
			}
		}
	}

	clock_set_pll_video_factors(best_m, best_n);

	printf("dotclock: %dkHz = %dkHz: (24MHz * %d) / %d / %d\n", clk_khz, (clock_get_pll_video() / 1000) / div, best_n, best_m, div);
}

/*
 * The LCD0 module is used for HDMI
 */

void h3_lcdc_init(void);
void h3_lcdc_tcon1_mode_set(const struct display_timing *mode);
void h3_lcdc_enable(uint32_t depth);

static void hdmi_lcdc_init(const struct display_timing *edid, uint32_t bpp) {
	uint32_t div = hdmi_get_divider(edid->pixelclock.typ);

	H3_CCU->BUS_SOFT_RESET1 |= CCU_BUS_SOFT_RESET1_TCON0;
	H3_CCU->BUS_CLK_GATING1 |= CCU_BUS_CLK_GATING1_TCON0;

	H3_CCU->TCON0_CLK = CCU_LCD0_CTRL_GATE | CCU_LCD0_CTRL_M(div);

	h3_lcdc_init();
	h3_lcdc_tcon1_mode_set(edid);
	h3_lcdc_enable(bpp);
}

static int hdmi_phy_cfg(__attribute__((unused)) struct dw_hdmi *hdmi, uint32_t mpixelclock) {
	hdmi_pll_set(mpixelclock / 1000);
	hdmi_phy_set(mpixelclock);

	return 0;
}

__attribute__((cold)) int h3_hdmi_enable(uint32_t panel_bpp, const struct display_timing *edid) {
	struct sunxi_dw_hdmi_priv *priv = &_sunxi_dw_hdmi_priv;
	int ret;

	ret = dw_hdmi_enable(&priv->hdmi, edid);

	if (ret) {
		printf("!!\n");
		return ret;
	}

	hdmi_lcdc_init(edid, panel_bpp);

	if (edid->flags & DISPLAY_FLAGS_VSYNC_LOW) {
		H3_HDMI_PHY->POL |= 0x200;
	}

	if (edid->flags & DISPLAY_FLAGS_HSYNC_LOW) {
		H3_HDMI_PHY->POL |= 0x100;
	}

	H3_HDMI_PHY->CTRL |= (0xf << 12);

	/*
	 * This is last hdmi access before boot, so scramble addresses
	 * again or othwerwise BSP driver won't work. Dummy read is
	 * needed or otherwise last write doesn't get written correctly.
	 */
	(void) *(volatile uint8_t *)H3_HDMI_BASE;

	H3_HDMI_PHY->UNSCRAMBLE = 0;

	return 0;
}

static void clock_set_pll_video(uint32_t clk) {
	if (clk == 0) {
		H3_CCU->PLL_VIDEO_CTRL &= (~CCU_PLL_VIDEO_CTRL_EN);
		return;
	}

	/* VIDEO rate = 3000000 * m */
	H3_CCU->PLL_VIDEO_CTRL = CCU_PLL_VIDEO_CTRL_EN | CCU_PLL_VIDEO_CTRL_INTEGER_MODE | CCU_PLL_VIDEO_CTRL_M(clk / 3000000);
}

/*
 * The MUX0 module is used for HDMI
 */

int __attribute__((cold)) h3_hdmi_probe(void) {
	struct sunxi_dw_hdmi_priv *priv = &_sunxi_dw_hdmi_priv;
	int ret;

	clock_set_pll_video(297000000);

	H3_CCU->HDMI_CLK = ((H3_CCU->HDMI_CLK & (~CCU_HDMI_CTRL_PLL_MASK)) | CCU_HDMI_CTRL_PLL_VIDEO);

	H3_CCU->BUS_SOFT_RESET1 |= CCU_BUS_SOFT_RESET1_HDMI;
	H3_CCU->BUS_SOFT_RESET1 |= CCU_BUS_SOFT_RESET1_HDMI2;
	H3_CCU->BUS_CLK_GATING1 |= CCU_BUS_CLK_GATING1_HDMI;

	H3_CCU->HDMI_SLOW_CLK |= (CCU_HDMI_SLOW_CTRL_DDC_GATE);
	H3_CCU->HDMI_CLK |= (CCU_HDMI_CTRL_GATE);

	hdmi_phy_init();

	ret = hdmi_wait_for_hpd();

	if (ret < 0) {
		printf("hdmi can not get hpd signal\n");
		return -1;
	}

	memset(&priv->hdmi, 0, sizeof(struct dw_hdmi));

	priv->hdmi.ioaddr = H3_HDMI_BASE;
	priv->hdmi.i2c_clk_high = 0xd8;
	priv->hdmi.i2c_clk_low = 0xfe;
	priv->hdmi.reg_io_width = 1;
	priv->hdmi.phy_set = hdmi_phy_cfg;
	priv->mux = 0;

	dw_hdmi_init(&priv->hdmi);

	return 0;
}
