#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wpedantic"
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 * Copyright 2017 Jernej Skrabec <jernej.skrabec@siol.net>
 */
/*
 * https://github.com/u-boot/u-boot/blob/master/drivers/video/dw_hdmi.c
 *
 * Optimized for HDMI/DVI fixed resolution
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

extern int uart0_printf(const char* fmt, ...);
#define debug	uart0_printf
#define printf	uart0_printf

#include "display_timing.h"

#include "dw_hdmi.h"
#include "media_bus_format.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static const u16 csc_coeff_default[3][4] = {
	{ 0x2000, 0x0000, 0x0000, 0x0000 },
	{ 0x0000, 0x2000, 0x0000, 0x0000 },
	{ 0x0000, 0x0000, 0x2000, 0x0000 }
};

static const u16 csc_coeff_rgb_in_eitu601[3][4] = {
	{ 0x2591, 0x1322, 0x074b, 0x0000 },
	{ 0x6535, 0x2000, 0x7acc, 0x0200 },
	{ 0x6acd, 0x7534, 0x2000, 0x0200 }
};

static const u16 csc_coeff_rgb_out_eitu601[3][4] = {
	{ 0x2000, 0x6926, 0x74fd, 0x010e },
	{ 0x2000, 0x2cdd, 0x0000, 0x7e9a },
	{ 0x2000, 0x0000, 0x38b4, 0x7e3b }
};

static void dw_hdmi_write(struct dw_hdmi *hdmi, u8 val, int offset) {
	switch (hdmi->reg_io_width) {
	case 1:
		*(volatile unsigned char *)(hdmi->ioaddr + offset) = val;
		break;
	default:
		debug("reg_io_width has unsupported width!\n");
		break;
	}
}

static u8 dw_hdmi_read(struct dw_hdmi *hdmi, int offset) {
	switch (hdmi->reg_io_width) {
	case 1:
		return *(volatile unsigned char *)(hdmi->ioaddr + offset);
	default:
		debug("reg_io_width has unsupported width!\n");
		break;
	}

	return 0;
}

static u8 (*hdmi_read)(struct dw_hdmi *hdmi, int offset) = dw_hdmi_read;
static void (*hdmi_write)(struct dw_hdmi *hdmi, u8 val, int offset) = dw_hdmi_write;

static void hdmi_mod(struct dw_hdmi *hdmi, unsigned reg, u8 mask, u8 data) {
	u8 val = hdmi_read(hdmi, reg) & ~mask;
	val |= data & mask;
	hdmi_write(hdmi, val, reg);
}

static bool hdmi_bus_fmt_is_rgb(unsigned int bus_format) {
	switch (bus_format) {
	case MEDIA_BUS_FMT_RGB888_1X24:
	case MEDIA_BUS_FMT_RGB101010_1X30:
	case MEDIA_BUS_FMT_RGB121212_1X36:
	case MEDIA_BUS_FMT_RGB161616_1X48:
		return true;

	default:
		return false;
	}
}

static bool hdmi_bus_fmt_is_yuv444(unsigned int bus_format) {
	switch (bus_format) {
	case MEDIA_BUS_FMT_YUV8_1X24:
	case MEDIA_BUS_FMT_YUV10_1X30:
	case MEDIA_BUS_FMT_YUV12_1X36:
	case MEDIA_BUS_FMT_YUV16_1X48:
		return true;

	default:
		return false;
	}
}

static bool hdmi_bus_fmt_is_yuv422(unsigned int bus_format) {
	switch (bus_format) {
	case MEDIA_BUS_FMT_UYVY8_1X16:
	case MEDIA_BUS_FMT_UYVY10_1X20:
	case MEDIA_BUS_FMT_UYVY12_1X24:
		return true;

	default:
		return false;
	}
}

static int hdmi_bus_fmt_color_depth(unsigned int bus_format) {
	switch (bus_format) {
	case MEDIA_BUS_FMT_RGB888_1X24:
	case MEDIA_BUS_FMT_YUV8_1X24:
	case MEDIA_BUS_FMT_UYVY8_1X16:
	case MEDIA_BUS_FMT_UYYVYY8_0_5X24:
		return 8;

	case MEDIA_BUS_FMT_RGB101010_1X30:
	case MEDIA_BUS_FMT_YUV10_1X30:
	case MEDIA_BUS_FMT_UYVY10_1X20:
	case MEDIA_BUS_FMT_UYYVYY10_0_5X30:
		return 10;

	case MEDIA_BUS_FMT_RGB121212_1X36:
	case MEDIA_BUS_FMT_YUV12_1X36:
	case MEDIA_BUS_FMT_UYVY12_1X24:
	case MEDIA_BUS_FMT_UYYVYY12_0_5X36:
		return 12;

	case MEDIA_BUS_FMT_RGB161616_1X48:
	case MEDIA_BUS_FMT_YUV16_1X48:
	case MEDIA_BUS_FMT_UYYVYY16_0_5X48:
		return 16;

	default:
		return 0;
	}
}

static int is_color_space_decimation(struct dw_hdmi *hdmi) {
	if (!hdmi_bus_fmt_is_yuv422(hdmi->hdmi_data.enc_out_bus_format))
		return 0;

	if (hdmi_bus_fmt_is_rgb(hdmi->hdmi_data.enc_in_bus_format)
			|| hdmi_bus_fmt_is_yuv444(hdmi->hdmi_data.enc_in_bus_format))
		return 1;

	return 0;
}

static void hdmi_av_composer(struct dw_hdmi *hdmi, const struct display_timing *edid) {
	bool mdataenablepolarity = true;
	uint inv_val;
	uint hbl;
	uint vbl;

	hbl = edid->hback_porch.typ + edid->hfront_porch.typ + edid->hsync_len.typ;
	vbl = edid->vback_porch.typ + edid->vfront_porch.typ + edid->vsync_len.typ;

	/* set up hdmi_fc_invidconf */
	inv_val = HDMI_FC_INVIDCONF_HDCP_KEEPOUT_INACTIVE;

	inv_val |= (
			edid->flags & DISPLAY_FLAGS_VSYNC_HIGH ?
					HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_HIGH :
					HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_LOW);

	inv_val |= (
			edid->flags & DISPLAY_FLAGS_HSYNC_HIGH ?
					HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_HIGH :
					HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_LOW);

	inv_val |= (
			mdataenablepolarity ?
					HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_HIGH :
					HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_LOW);

	inv_val |= (
			edid->hdmi_monitor ?
					HDMI_FC_INVIDCONF_DVI_MODEZ_HDMI_MODE :
					HDMI_FC_INVIDCONF_DVI_MODEZ_DVI_MODE);

	inv_val |= HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_LOW;

	inv_val |= HDMI_FC_INVIDCONF_IN_I_P_PROGRESSIVE;

	hdmi_write(hdmi, inv_val, HDMI_FC_INVIDCONF);

	/* set up horizontal active pixel width */
	hdmi_write(hdmi, edid->hactive.typ >> 8, HDMI_FC_INHACTV1);
	hdmi_write(hdmi, edid->hactive.typ, HDMI_FC_INHACTV0);

	/* set up vertical active lines */
	hdmi_write(hdmi, edid->vactive.typ >> 8, HDMI_FC_INVACTV1);
	hdmi_write(hdmi, edid->vactive.typ, HDMI_FC_INVACTV0);

	/* set up horizontal blanking pixel region width */
	hdmi_write(hdmi, hbl >> 8, HDMI_FC_INHBLANK1);
	hdmi_write(hdmi, hbl, HDMI_FC_INHBLANK0);

	/* set up vertical blanking pixel region width */
	hdmi_write(hdmi, vbl, HDMI_FC_INVBLANK);

	/* set up hsync active edge delay width (in pixel clks) */
	hdmi_write(hdmi, edid->hfront_porch.typ >> 8, HDMI_FC_HSYNCINDELAY1);
	hdmi_write(hdmi, edid->hfront_porch.typ, HDMI_FC_HSYNCINDELAY0);

	/* set up vsync active edge delay (in lines) */
	hdmi_write(hdmi, edid->vfront_porch.typ, HDMI_FC_VSYNCINDELAY);

	/* set up hsync active pulse width (in pixel clks) */
	hdmi_write(hdmi, edid->hsync_len.typ >> 8, HDMI_FC_HSYNCINWIDTH1);
	hdmi_write(hdmi, edid->hsync_len.typ, HDMI_FC_HSYNCINWIDTH0);

	/* set up vsync active edge delay (in lines) */
	hdmi_write(hdmi, edid->vsync_len.typ, HDMI_FC_VSYNCINWIDTH);
}

static int is_color_space_conversion(struct dw_hdmi *hdmi) {
	return hdmi->hdmi_data.enc_in_bus_format
			!= hdmi->hdmi_data.enc_out_bus_format;
}

static void dw_hdmi_update_csc_coeffs(struct dw_hdmi *hdmi)
{
	const u16 (*csc_coeff)[3][4] = &csc_coeff_default;
	unsigned int i;
	u32 csc_scale = 1;

	if (is_color_space_conversion(hdmi)) {
		if (hdmi_bus_fmt_is_rgb(hdmi->hdmi_data.enc_out_bus_format)) {
			csc_coeff = &csc_coeff_rgb_out_eitu601;
		} else if (hdmi_bus_fmt_is_rgb(
					hdmi->hdmi_data.enc_in_bus_format)) {
			csc_coeff = &csc_coeff_rgb_in_eitu601;
			csc_scale = 0;
		}
	}

	/* The CSC registers are sequential, alternating MSB then LSB */
	for (i = 0; i < ARRAY_SIZE(csc_coeff_default[0]); i++) {
		u16 coeff_a = (*csc_coeff)[0][i];
		u16 coeff_b = (*csc_coeff)[1][i];
		u16 coeff_c = (*csc_coeff)[2][i];

		hdmi_write(hdmi, coeff_a & 0xff, HDMI_CSC_COEF_A1_LSB + i * 2);
		hdmi_write(hdmi, coeff_a >> 8, HDMI_CSC_COEF_A1_MSB + i * 2);
		hdmi_write(hdmi, coeff_b & 0xff, HDMI_CSC_COEF_B1_LSB + i * 2);
		hdmi_write(hdmi, coeff_b >> 8, HDMI_CSC_COEF_B1_MSB + i * 2);
		hdmi_write(hdmi, coeff_c & 0xff, HDMI_CSC_COEF_C1_LSB + i * 2);
		hdmi_write(hdmi, coeff_c >> 8, HDMI_CSC_COEF_C1_MSB + i * 2);
	}

	hdmi_mod(hdmi, HDMI_CSC_SCALE, HDMI_CSC_SCALE_CSCSCALE_MASK, csc_scale);
}

static int is_color_space_interpolation(struct dw_hdmi *hdmi)
{
	if (!hdmi_bus_fmt_is_yuv422(hdmi->hdmi_data.enc_in_bus_format))
		return 0;

	if (hdmi_bus_fmt_is_rgb(hdmi->hdmi_data.enc_out_bus_format) ||
	    hdmi_bus_fmt_is_yuv444(hdmi->hdmi_data.enc_out_bus_format))
		return 1;

	return 0;
}

static void hdmi_video_csc(struct dw_hdmi *hdmi)
{
	int color_depth = 0;
	int interpolation = HDMI_CSC_CFG_INTMODE_DISABLE;
	int decimation = 0;

	/* YCC422 interpolation to 444 mode */
	if (is_color_space_interpolation(hdmi))
		interpolation = HDMI_CSC_CFG_INTMODE_CHROMA_INT_FORMULA1;
	else if (is_color_space_decimation(hdmi))
		decimation = HDMI_CSC_CFG_DECMODE_CHROMA_INT_FORMULA3;

	switch (hdmi_bus_fmt_color_depth(hdmi->hdmi_data.enc_out_bus_format)) {
	case 8:
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_24BPP;
		break;
	case 10:
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_30BPP;
		break;
	case 12:
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_36BPP;
		break;
	case 16:
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_48BPP;
		break;

	default:
		return;
	}

	/* Configure the CSC registers */
	hdmi_write(hdmi, interpolation | decimation, HDMI_CSC_CFG);

	hdmi_mod(hdmi, HDMI_CSC_SCALE, HDMI_CSC_SCALE_CSC_COLORDE_PTH_MASK,
		 color_depth);

	dw_hdmi_update_csc_coeffs(hdmi);
}

/* hdmi initialization step b.4 */
static void hdmi_enable_video_path(struct dw_hdmi *hdmi, bool audio) {
	uint clkdis;

	/* control period minimum duration */
	hdmi_write(hdmi, 12, HDMI_FC_CTRLDUR);
	hdmi_write(hdmi, 32, HDMI_FC_EXCTRLDUR);
	hdmi_write(hdmi, 1, HDMI_FC_EXCTRLSPAC);

	/* set to fill tmds data channels */
	hdmi_write(hdmi, 0x0b, HDMI_FC_CH0PREAM);
	hdmi_write(hdmi, 0x16, HDMI_FC_CH1PREAM);
	hdmi_write(hdmi, 0x21, HDMI_FC_CH2PREAM);

	hdmi_write(hdmi, HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_BYPASS,
	HDMI_MC_FLOWCTRL);

	/* enable pixel clock and tmds data path */
	clkdis = 0x7f;
	clkdis &= ~HDMI_MC_CLKDIS_PIXELCLK_DISABLE;
	hdmi_write(hdmi, clkdis, HDMI_MC_CLKDIS);

	clkdis &= ~HDMI_MC_CLKDIS_TMDSCLK_DISABLE;
	hdmi_write(hdmi, clkdis, HDMI_MC_CLKDIS);

	/* Enable csc path */
	if (is_color_space_conversion(hdmi)) {
		clkdis &= ~HDMI_MC_CLKDIS_CSCCLK_DISABLE;
		hdmi_write(hdmi, clkdis, HDMI_MC_CLKDIS);
	}

	/* Enable color space conversion if needed */
	if (is_color_space_conversion(hdmi))
		hdmi_write(hdmi, HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_IN_PATH,
		HDMI_MC_FLOWCTRL);
	else
		hdmi_write(hdmi, HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_BYPASS,
		HDMI_MC_FLOWCTRL);

	if (audio) {
		clkdis &= ~HDMI_MC_CLKDIS_AUDCLK_DISABLE;
		hdmi_write(hdmi, clkdis, HDMI_MC_CLKDIS);
	}
}

/* workaround to clear the overflow condition */
static void hdmi_clear_overflow(struct dw_hdmi *hdmi) {
	uint val, count;

	/* tmds software reset */
	hdmi_write(hdmi, (u8) ~HDMI_MC_SWRSTZ_TMDSSWRST_REQ, HDMI_MC_SWRSTZ);

	val = hdmi_read(hdmi, HDMI_FC_INVIDCONF);

	for (count = 0; count < 4; count++)
		hdmi_write(hdmi, val, HDMI_FC_INVIDCONF);
}

static void hdmi_video_packetize(struct dw_hdmi *hdmi) {
	u32 output_select = HDMI_VP_CONF_OUTPUT_SELECTOR_BYPASS;
	u32 remap_size = HDMI_VP_REMAP_YCC422_16BIT;
	u32 color_depth = 0;
	uint val, vp_conf;

	/* set the packetizer registers */
	val = ((color_depth << HDMI_VP_PR_CD_COLOR_DEPTH_OFFSET)
			& HDMI_VP_PR_CD_COLOR_DEPTH_MASK)
			| ((0 << HDMI_VP_PR_CD_DESIRED_PR_FACTOR_OFFSET)
					& HDMI_VP_PR_CD_DESIRED_PR_FACTOR_MASK);
	hdmi_write(hdmi, val, HDMI_VP_PR_CD);

	hdmi_mod(hdmi, HDMI_VP_STUFF, HDMI_VP_STUFF_PR_STUFFING_MASK,
			HDMI_VP_STUFF_PR_STUFFING_STUFFING_MODE);

	/* data from pixel repeater block */
	vp_conf = HDMI_VP_CONF_PR_EN_DISABLE
			| HDMI_VP_CONF_BYPASS_SELECT_VID_PACKETIZER;

	hdmi_mod(hdmi, HDMI_VP_CONF,
			HDMI_VP_CONF_PR_EN_MASK | HDMI_VP_CONF_BYPASS_SELECT_MASK, vp_conf);

	hdmi_mod(hdmi, HDMI_VP_STUFF, HDMI_VP_STUFF_IDEFAULT_PHASE_MASK,
			1 << HDMI_VP_STUFF_IDEFAULT_PHASE_OFFSET);

	hdmi_write(hdmi, remap_size, HDMI_VP_REMAP);

	vp_conf = HDMI_VP_CONF_BYPASS_EN_ENABLE | HDMI_VP_CONF_PP_EN_DISABLE
			| HDMI_VP_CONF_YCC422_EN_DISABLE;

	hdmi_mod(hdmi, HDMI_VP_CONF,
			HDMI_VP_CONF_BYPASS_EN_MASK | HDMI_VP_CONF_PP_EN_ENMASK
					| HDMI_VP_CONF_YCC422_EN_MASK, vp_conf);

	hdmi_mod(hdmi, HDMI_VP_STUFF,
			HDMI_VP_STUFF_PP_STUFFING_MASK | HDMI_VP_STUFF_YCC422_STUFFING_MASK,
			HDMI_VP_STUFF_PP_STUFFING_STUFFING_MODE
					| HDMI_VP_STUFF_YCC422_STUFFING_STUFFING_MODE);

	hdmi_mod(hdmi, HDMI_VP_CONF, HDMI_VP_CONF_OUTPUT_SELECTOR_MASK,
			output_select);
}

/*
 * this submodule is responsible for the video data synchronization.
 * for example, for rgb 4:4:4 input, the data map is defined as
 *			pin{47~40} <==> r[7:0]
 *			pin{31~24} <==> g[7:0]
 *			pin{15~8}  <==> b[7:0]
 */
static void hdmi_video_sample(struct dw_hdmi *hdmi)
{
	u32 color_format;
	uint val;

	switch (hdmi->hdmi_data.enc_in_bus_format) {
	case MEDIA_BUS_FMT_RGB888_1X24:
		color_format = 0x01;
		break;
	case MEDIA_BUS_FMT_RGB101010_1X30:
		color_format = 0x03;
		break;
	case MEDIA_BUS_FMT_RGB121212_1X36:
		color_format = 0x05;
		break;
	case MEDIA_BUS_FMT_RGB161616_1X48:
		color_format = 0x07;
		break;
	case MEDIA_BUS_FMT_YUV8_1X24:
	case MEDIA_BUS_FMT_UYYVYY8_0_5X24:
		color_format = 0x09;
		break;
	case MEDIA_BUS_FMT_YUV10_1X30:
	case MEDIA_BUS_FMT_UYYVYY10_0_5X30:
		color_format = 0x0B;
		break;
	case MEDIA_BUS_FMT_YUV12_1X36:
	case MEDIA_BUS_FMT_UYYVYY12_0_5X36:
		color_format = 0x0D;
		break;
	case MEDIA_BUS_FMT_YUV16_1X48:
	case MEDIA_BUS_FMT_UYYVYY16_0_5X48:
		color_format = 0x0F;
		break;
	case MEDIA_BUS_FMT_UYVY8_1X16:
		color_format = 0x16;
		break;
	case MEDIA_BUS_FMT_UYVY10_1X20:
		color_format = 0x14;
		break;
	case MEDIA_BUS_FMT_UYVY12_1X24:
		color_format = 0x12;
		break;
	default:
		color_format = 0x01;
		break;
	}

	val = HDMI_TX_INVID0_INTERNAL_DE_GENERATOR_DISABLE |
	      ((color_format << HDMI_TX_INVID0_VIDEO_MAPPING_OFFSET) &
	      HDMI_TX_INVID0_VIDEO_MAPPING_MASK);

	hdmi_write(hdmi, val, HDMI_TX_INVID0);

	/* enable tx stuffing: when de is inactive, fix the output data to 0 */
	val = HDMI_TX_INSTUFFING_BDBDATA_STUFFING_ENABLE |
	      HDMI_TX_INSTUFFING_RCRDATA_STUFFING_ENABLE |
	      HDMI_TX_INSTUFFING_GYDATA_STUFFING_ENABLE;
	hdmi_write(hdmi, val, HDMI_TX_INSTUFFING);
	hdmi_write(hdmi, 0x0, HDMI_TX_GYDATA0);
	hdmi_write(hdmi, 0x0, HDMI_TX_GYDATA1);
	hdmi_write(hdmi, 0x0, HDMI_TX_RCRDATA0);
	hdmi_write(hdmi, 0x0, HDMI_TX_RCRDATA1);
	hdmi_write(hdmi, 0x0, HDMI_TX_BCBDATA0);
	hdmi_write(hdmi, 0x0, HDMI_TX_BCBDATA1);
}

int dw_hdmi_enable(struct dw_hdmi *hdmi, const struct display_timing *edid) {
	int ret;

	debug("%s, mode info : clock %d hdis %d vdis %d\n",
			edid->hdmi_monitor ? "hdmi" : "dvi", edid->pixelclock.typ,
			edid->hactive.typ, edid->vactive.typ);

	hdmi_av_composer(hdmi, edid);

	ret = hdmi->phy_set(hdmi, edid->pixelclock.typ);

	if (ret)
		return ret;

	hdmi_enable_video_path(hdmi, edid->hdmi_monitor);

	hdmi_video_packetize(hdmi);
	hdmi_video_csc(hdmi);
	hdmi_video_sample(hdmi);

	hdmi_clear_overflow(hdmi);

	return 0;
}

void __attribute__((cold)) dw_hdmi_init(struct dw_hdmi *hdmi) {
	uint ih_mute;

	/*
	 * boot up defaults are:
	 * hdmi_ih_mute   = 0x03 (disabled)
	 * hdmi_ih_mute_* = 0x00 (enabled)
	 *
	 * disable top level interrupt bits in hdmi block
	 */
	ih_mute = /*hdmi_read(hdmi, HDMI_IH_MUTE) |*/
	HDMI_IH_MUTE_MUTE_WAKEUP_INTERRUPT | HDMI_IH_MUTE_MUTE_ALL_INTERRUPT;

	hdmi_write(hdmi, ih_mute, HDMI_IH_MUTE);

	/* enable i2c master done irq */
	hdmi_write(hdmi, ~0x04, HDMI_I2CM_INT);

	/* enable i2c client nack % arbitration error irq */
	hdmi_write(hdmi, ~0x44, HDMI_I2CM_CTLINT);
}
