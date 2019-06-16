/**
 * @file h3_codec.h
 *
 */
/*
 * Based on https://github.com/allwinner-zh/linux-3.4-sunxi/blob/master/sound/soc/sunxi/audiocodec/sun8iw7_sndcodec.c
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#undef NDEBUG
#define LOGIC_ANALYZER

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "debug.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"

#include "h3_codec.h"
#include "h3_ccu.h"
#include "h3_timer.h"
#include "h3.h"
#include "h3_dma.h"
#include "h3_board.h"
#include "h3_gpio.h"

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#define CODEC_BASSADDRESS         (0x01c22c00)
#define SUNXI_DAC_DPC                (0x00)
#define SUNXI_DAC_FIFOC              (0x04)
#define SUNXI_DAC_FIFOS              (0x08)

#define SUNXI_ADC_FIFOC              (0x10)
#define SUNXI_ADC_FIFOS              (0x14)
#define SUNXI_ADC_RXDATA			 (0x18)

#define SUNXI_DAC_TXDATA             (0x20)

#define SUNXI_DAC_DEBUG              (0x48)
#define SUNXI_ADC_DEBUG              (0x4c)

#define SUNXI_DAC_DAP_CTR            (0x60)
#define SUNXI_DAC_DAP_VOL            (0x64)
#define SUNXI_DAC_DAP_COFF           (0x68)
#define SUNXI_DAC_DAP_OPT            (0x6c)

#define SUNXI_DAC_DRC_HHPFC			(0X100)
#define SUNXI_DAC_DRC_LHPFC			(0X104)
#define SUNXI_DAC_DRC_CTRL			(0X108)
#define SUNXI_DAC_DRC_LPFHAT		(0X10C)
#define SUNXI_DAC_DRC_LPFLAT		(0X110)
#define SUNXI_DAC_DRC_RPFHAT		(0X114)
#define SUNXI_DAC_DRC_RPFLAT		(0X118)
#define SUNXI_DAC_DRC_LPFHRT		(0X11C)
#define SUNXI_DAC_DRC_LPFLRT		(0X120)
#define SUNXI_DAC_DRC_RPFHRT		(0X124)
#define SUNXI_DAC_DRC_RPFLRT		(0X128)
#define SUNXI_DAC_DRC_LRMSHAT		(0X12C)
#define SUNXI_DAC_DRC_LRMSLAT		(0X130)
#define SUNXI_DAC_DRC_RRMSHAT		(0X134)
#define SUNXI_DAC_DRC_RRMSLAT		(0X138)
#define SUNXI_DAC_DRC_HCT		(0X13C)
#define SUNXI_DAC_DRC_LCT		(0X140)
#define SUNXI_DAC_DRC_HKC		(0X144)
#define SUNXI_DAC_DRC_LKC		(0X148)
#define SUNXI_DAC_DRC_HOPC		(0X14C)
#define SUNXI_DAC_DRC_LOPC		(0X150)
#define SUNXI_DAC_DRC_HLT		(0X154)
#define SUNXI_DAC_DRC_LLT		(0X158)
#define SUNXI_DAC_DRC_HKI		(0X15C)
#define SUNXI_DAC_DRC_LKI		(0X160)
#define SUNXI_DAC_DRC_HOPL		(0X164)
#define SUNXI_DAC_DRC_LOPL		(0X168)
#define SUNXI_DAC_DRC_HET		(0X16C)
#define	SUNXI_DAC_DRC_LET		(0X170)
#define SUNXI_DAC_DRC_HKE		(0X174)
#define SUNXI_DAC_DRC_LKE		(0X178)
#define SUNXI_DAC_DRC_HOPE		(0X17C)
#define SUNXI_DAC_DRC_LOPE		(0X180)
#define SUNXI_DAC_DRC_HKN		(0X184)
#define SUNXI_DAC_DRC_LKN		(0X188)
#define SUNXI_DAC_DRC_SFHAT		(0X18C)
#define SUNXI_DAC_DRC_SFLAT		(0X190)
#define SUNXI_DAC_DRC_SFHRT		(0X194)
#define	SUNXI_DAC_DRC_SFLRT		(0X198)
#define	SUNXI_DAC_DRC_MXGHS		(0X19C)
#define SUNXI_DAC_DRC_MXGLS		(0X1A0)
#define SUNXI_DAC_DRC_MNGHS		(0X1A4)
#define SUNXI_DAC_DRC_MNGLS		(0X1A8)
#define SUNXI_DAC_DRC_EPSHC		(0X1AC)
#define SUNXI_DAC_DRC_EPSLC		(0X1B0)
#define SUNXI_DAC_DRC_OPT		(0X1B4)
#define SUNXI_DAC_HPF_HG		(0x1B8)
#define SUNXI_DAC_HPF_LG		(0x1BC)


#define SUNXI_ADC_DRC_HHPFC		(0X200)
#define SUNXI_ADC_DRC_LHPFC		(0X204)
#define SUNXI_ADC_DRC_CTRL		(0X208)
#define SUNXI_ADC_DRC_LPFHAT		(0X20C)
#define SUNXI_ADC_DRC_LPFLAT		(0X210)
#define SUNXI_ADC_DRC_RPFHAT		(0X214)
#define SUNXI_ADC_DRC_RPFLAT		(0X218)
#define SUNXI_ADC_DRC_LPFHRT		(0X21C)
#define SUNXI_ADC_DRC_LPFLRT		(0X220)
#define SUNXI_ADC_DRC_RPFHRT		(0X224)
#define SUNXI_ADC_DRC_RPFLRT		(0X228)
#define SUNXI_ADC_DRC_LRMSHAT		(0X22C)
#define SUNXI_ADC_DRC_LRMSLAT		(0X230)
#define SUNXI_ADC_DRC_RRMSHAT		(0X234)
#define SUNXI_ADC_DRC_RRMSLAT		(0X238)
#define SUNXI_ADC_DRC_HCT		(0X23C)
#define SUNXI_ADC_DRC_LCT		(0X240)
#define SUNXI_ADC_DRC_HKC		(0X244)
#define SUNXI_ADC_DRC_LKC		(0X248)
#define SUNXI_ADC_DRC_HOPC		(0X24C)
#define SUNXI_ADC_DRC_LOPC		(0X250)
#define SUNXI_ADC_DRC_HLT		(0X254)
#define SUNXI_ADC_DRC_LLT		(0X258)
#define SUNXI_ADC_DRC_HKI		(0X25C)
#define SUNXI_ADC_DRC_LKI		(0X260)
#define SUNXI_ADC_DRC_HOPL		(0X264)
#define SUNXI_ADC_DRC_LOPL		(0X268)
#define SUNXI_ADC_DRC_HET		(0X26C)
#define SUNXI_ADC_DRC_LET		(0X270)
#define	SUNXI_ADC_DRC_HKE		(0X274)
#define SUNXI_ADC_DRC_LKE		(0X278)
#define SUNXI_ADC_DRC_HOPE		(0X27C)
#define SUNXI_ADC_DRC_LOPE		(0X280)
#define SUNXI_ADC_DRC_HKN		(0X284)
#define SUNXI_ADC_DRC_LKN		(0X288)
#define SUNXI_ADC_DRC_SFHAT		(0X28C)
#define SUNXI_ADC_DRC_SFLAT		(0X290)
#define SUNXI_ADC_DRC_SFHRT		(0X294)
#define SUNXI_ADC_DRC_SFLRT		(0X298)
#define SUNXI_ADC_DRC_MXGHS		(0X29C)
#define SUNXI_ADC_DRC_MXGLS		(0X2A0)
#define SUNXI_ADC_DRC_MNGHS		(0X2A4)
#define SUNXI_ADC_DRC_MNGLS		(0X2A8)
#define SUNXI_ADC_DRC_EPSHC		(0X2AC)
#define SUNXI_ADC_DRC_EPSLC		(0X2B0)
#define SUNXI_ADC_DRC_OPT		(0X2B4)
#define SUNXI_ADC_HPF_HG		(0x2B8)
#define SUNXI_ADC_HPF_LG		(0x2BC)

/*DAC Digital Part Control Register
* codecbase+0x00
* SUNXI_DAC_DPC
*/
#define DAC_EN                    (31)
#define HPF_EN					  (18)
#define DIGITAL_VOL               (12)
#define HUB_EN					  (0)

/*DAC FIFO Control Register
* codecbase+0x04
* SUNXI_DAC_FIFOC
*/
#define DAC_FS					  (29)
#define FIR_VER					  (28)
#define SEND_LASAT                 (26)
#define FIFO_MODE              	  (24)
#define DAC_DRQ_CLR_CNT           (21)
#define TX_TRI_LEVEL              (8)
#define ADDA_LOOP_EN			  (7)
#define DAC_MONO_EN               (6)
#define TX_SAMPLE_BITS            (5)
#define DAC_DRQ_EN                (4)
#define FIFO_FLUSH 				(0)

/*ADC FIFO Control Register
* codecbase+0x10
* SUNXI_ADC_FIFOC
*/
#define ADFS					  (29)
#define EN_AD                	  (28)
#define RX_FIFO_MODE              (24)
#define ADCFDT					  (17)
#define ADCDFEN					  (16)
#define RX_FIFO_TRG_LEVEL         (8)
#define ADC_MONO_EN               (7)
#define RX_SAMPLE_BITS            (6)
#define ADC_DRQ_EN                (4)
#define ADC_FIFO_FLUSH (0)

#define SUNXI_R_PRCM_PBASE 0x01f01400

#define ADDA_PR_CFG_REG     	  (SUNXI_R_PRCM_PBASE+0x1c0)
#define LINEOUT_PA_GAT			  (0x00)
#define LOMIXSC					  (0x01)
#define ROMIXSC					  (0x02)
#define DAC_PA_SRC				  (0x03)
#define LINEIN_GCTR				  (0x05)
#define MIC_GCTR				  (0x06)
#define PAEN_CTR				  (0x07)
#define LINEOUT_VOLC			  (0x09)
#define MIC2G_LINEOUT_CTR		  (0x0A)
#define MIC1G_MICBIAS_CTR		  (0x0B)
#define LADCMIXSC		  		  (0x0C)
#define RADCMIXSC				  (0x0D)
#define ADC_AP_EN				  (0x0F)
#define ADDA_APT0				  (0x10)
#define ADDA_APT1				  (0x11)
#define ADDA_APT2				  (0x12)
#define BIAS_DA16_CTR0			  (0x13)
#define BIAS_DA16_CTR1			  (0x14)
#define DA16CAL					  (0x15)
#define DA16VERIFY				  (0x16)
#define BIASCALI				  (0x17)
#define BIASVERIFY	(0x18)

/*
*	0x00 LINEOUT_PA_GAT
*/
#define PA_CLK_GC		(7)

/*
*	0x01 LOMIXSC
*/
#define LMIXMUTE				  (0)
#define LMIXMUTEDACR			  (0)
#define LMIXMUTEDACL			  (1)
#define LMIXMUTELINEINL			  (2)
#define LMIXMUTEMIC2BOOST		  (5)
#define LMIXMUTEMIC1BOOST		  (6)

/*
*	0x02 ROMIXSC
*/
#define RMIXMUTE				  (0)
#define RMIXMUTEDACL			  (0)
#define RMIXMUTEDACR			  (1)
#define RMIXMUTELINEINR			  (2)
#define RMIXMUTEMIC2BOOST		  (5)
#define RMIXMUTEMIC1BOOST	(6)

/*
*	0x03 DAC_PA_SRC
*/
#define DACAREN			(7)
#define DACALEN			(6)
#define RMIXEN			(5)
#define LMIXEN	(4)

/*
*	0x07 PAEN_CTR
*/
#define LINEOUTEN		 (7)
#define PA_ANTI_POP_CTRL (2)

/*
*	0x09 LINEOUT_VOLC
*/
#define LINEOUTVOL	(3)

/*
*	0x0A MIC2G_LINEOUT_CTR
*/
#define MIC2AMPEN		(7)
#define MIC2BOOST		(4)
#define LINEOUTL_EN		(3)
#define LINEOUTR_EN		(2)
#define LINEOUTL_SS		(1)
#define LINEOUTR_SS		(0)

#define SCLK_1X_GATING	(1 << 31)

/*                            ns  nw  ks  kw  ms  mw  ps  pw  d1s  d1w  d2s  d2w  {frac  out  mode}  en-s   sdmss  sdmsw  	sdmpat      sdmval
SUNXI_CLK_FACTORS(pll_audio,  8,  7,  0,  0,  0,  5,  16, 4,  0,   0,   0,   0,    0,    0,   0,     31,   24,     1,       PLL_AUDIOPAT,0xc0010d84);
*/

#define PLL_FACTOR_M_MASK	0x1F
#define PLL_FACTOR_M_SHIFT	0

#define PLL_FACTOR_N_MASK	0x7F
#define PLL_FACTOR_N_SHIFT	8

#define PLL_FACTOR_P_MASK	0x0F
#define PLL_FACTOR_P_SHIFT	16

#define PLL_LOCK					(1 << 28)	// Read only, 1 indicates that the PLL has been stable
#define PLL_SDM_ENABLE				(1 << 24)
#define PLL_ENABLE					(1 << 31)

#define writel(v,a) (*(volatile uint32_t *)(a) = (v))
#define readl(a)	(*(volatile uint32_t *)(a))

#define msleep(x)	__msdelay(x)

#define	ARM_DMA_ALIGN	64

#define	CONFIG_BUFSIZE		(16 * 1024)
#define CONFIG_TX_DESCR_NUM	(1)
#define TX_TOTAL_BUFSIZE	(CONFIG_BUFSIZE * CONFIG_TX_DESCR_NUM)

struct coherent_region {
	struct sunxi_dma_lli lli[CONFIG_TX_DESCR_NUM];
	int16_t txbuffer[TX_TOTAL_BUFSIZE] ALIGNED;
};

static struct coherent_region *p_coherent_region = (struct coherent_region *)(H3_MEM_COHERENT_REGION + MEGABYTE/2);

static uint32_t read_prcm_wvalue(uint32_t addr) {
	unsigned int reg;
	reg = readl(ADDA_PR_CFG_REG);
	reg |= (0x1 << 28);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg &= ~(0x1 << 24);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg &= ~(0x1f << 16);
	reg |= (addr << 16);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg &= (0xff << 0);

	return reg;
}

static void write_prcm_wvalue(uint32_t addr, uint32_t val) {
	uint32_t reg;
	reg = readl(ADDA_PR_CFG_REG);
	reg |= (0x1 << 28);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg &= ~(0x1f << 16);
	reg |= (addr << 16);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg &= ~(0xff << 8);
	reg |= (val << 8);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg |= (0x1 << 24);
	writel(reg, ADDA_PR_CFG_REG);

	reg = readl(ADDA_PR_CFG_REG);
	reg &= ~(0x1 << 24);
	writel(reg, ADDA_PR_CFG_REG);
}

static void codec_wrreg_prcm_bits(unsigned short reg, uint32_t mask, uint32_t value) {
	uint32_t old, new;
	old = read_prcm_wvalue(reg);
	new = (old & ~mask) | value;
	write_prcm_wvalue(reg, new);
}

static void codec_wr_prcm_control(uint32_t reg, uint32_t mask, uint32_t shift, uint32_t val) {
	uint32_t reg_val;
	reg_val = val << shift;
	mask = mask << shift;
	codec_wrreg_prcm_bits(reg, mask, reg_val);
}

#define codec_rdreg(reg)		readl((CODEC_BASSADDRESS +(reg)))
#define codec_wrreg(reg,val) 	writel((val),(CODEC_BASSADDRESS+(reg)))

static int codec_wrreg_bits(unsigned short reg, uint32_t mask, uint32_t value) {
	uint32_t old, new;

	old = codec_rdreg(reg);
	new = (old & ~mask) | value;
	codec_wrreg(reg, new);

	return 0;
}

static int codec_wr_control(uint32_t reg, uint32_t mask, uint32_t shift, uint32_t val) {
	uint32_t reg_val;
	reg_val = val << shift;
	mask = mask << shift;
	codec_wrreg_bits(reg, mask, reg_val);
	return 0;
}

void dacdrc_config(void) {
	//codec_wr_control(SUNXI_DAC_DRC_HHPFC    , 0xffff, 0,    0x00000000);
	//codec_wr_control(SUNXI_DAC_DRC_LHPFC    , 0xffff, 0,    0x00000000);

	codec_wr_control(SUNXI_DAC_DRC_CTRL     , 0xffff, 0,    0x00000000);

	codec_wr_control(SUNXI_DAC_DRC_LPFHAT   , 0xffff, 0,    0x00000000); // Left
	codec_wr_control(SUNXI_DAC_DRC_LPFLAT   , 0xffff, 0,    0x00000000);

	codec_wr_control(SUNXI_DAC_DRC_RPFHAT   , 0xffff, 0,    0x0000000B);
	codec_wr_control(SUNXI_DAC_DRC_RPFLAT   , 0xffff, 0,    0x000077EF);

	codec_wr_control(SUNXI_DAC_DRC_LPFHRT   , 0xffff, 0,    0x00000000); // Left
	codec_wr_control(SUNXI_DAC_DRC_LPFLRT   , 0xffff, 0,    0x00000000);

	codec_wr_control(SUNXI_DAC_DRC_RPFHRT   , 0xffff, 0,    0x000000FF);
	codec_wr_control(SUNXI_DAC_DRC_RPFLRT   , 0xffff, 0,    0x0000E1F8);

	codec_wr_control(SUNXI_DAC_DRC_LRMSHAT  , 0xffff, 0,    0x00000000); // Left
	codec_wr_control(SUNXI_DAC_DRC_LRMSLAT  , 0xffff, 0,    0x00000000);

	codec_wr_control(SUNXI_DAC_DRC_RRMSHAT  , 0xffff, 0,    0x00000001);
	codec_wr_control(SUNXI_DAC_DRC_RRMSLAT  , 0xffff, 0,    0x00002BAF);

	codec_wr_control(SUNXI_DAC_DRC_HCT      , 0xffff, 0,    0x000006A4); // 0x000004FB
	codec_wr_control(SUNXI_DAC_DRC_LCT      , 0xffff, 0,    0x0000D3C0); // 0x00009ED0

	codec_wr_control(SUNXI_DAC_DRC_HKC      , 0xffff, 0,    0x00000100);
	codec_wr_control(SUNXI_DAC_DRC_LKC      , 0xffff, 0,    0x00000000);
	codec_wr_control(SUNXI_DAC_DRC_HOPC     , 0xffff, 0,    0x0000FBD8);
	codec_wr_control(SUNXI_DAC_DRC_LOPC     , 0xffff, 0,    0x0000FBA8);
	codec_wr_control(SUNXI_DAC_DRC_HLT      , 0xffff, 0,    0x00000352);
	codec_wr_control(SUNXI_DAC_DRC_LLT      , 0xffff, 0,    0x000069E0);
	codec_wr_control(SUNXI_DAC_DRC_HKI      , 0xffff, 0,    0x00000080);
	codec_wr_control(SUNXI_DAC_DRC_LKI      , 0xffff, 0,    0x00000000);
	codec_wr_control(SUNXI_DAC_DRC_HOPL     , 0xffff, 0,    0x0000FD82);
	codec_wr_control(SUNXI_DAC_DRC_LOPL     , 0xffff, 0,    0x00003098);
	codec_wr_control(SUNXI_DAC_DRC_HET      , 0xffff, 0,    0x00000779);
	codec_wr_control(SUNXI_DAC_DRC_LET      , 0xffff, 0,    0x00006E38);
	codec_wr_control(SUNXI_DAC_DRC_HKE      , 0xffff, 0,    0x00000100);
	codec_wr_control(SUNXI_DAC_DRC_LKE      , 0xffff, 0,    0x00000000);
	codec_wr_control(SUNXI_DAC_DRC_HOPE     , 0xffff, 0,    0x0000F906);
	codec_wr_control(SUNXI_DAC_DRC_LOPE     , 0xffff, 0,    0x000021A9);
	codec_wr_control(SUNXI_DAC_DRC_HKN      , 0xffff, 0,    0x00000122);
	codec_wr_control(SUNXI_DAC_DRC_LKN      , 0xffff, 0,    0x00002222);
	codec_wr_control(SUNXI_DAC_DRC_SFHAT    , 0xffff, 0,    0x00000002);
	codec_wr_control(SUNXI_DAC_DRC_SFLAT    , 0xffff, 0,    0x00005600);
	codec_wr_control(SUNXI_DAC_DRC_SFHRT    , 0xffff, 0,    0x00000000);
	codec_wr_control(SUNXI_DAC_DRC_SFLRT    , 0xffff, 0,    0x00000F04);
	codec_wr_control(SUNXI_DAC_DRC_MXGHS    , 0xffff, 0,    0x0000FE56);
	codec_wr_control(SUNXI_DAC_DRC_MXGLS    , 0xffff, 0,    0x0000CB0F);
	codec_wr_control(SUNXI_DAC_DRC_MNGHS    , 0xffff, 0,    0x0000F95B);
	codec_wr_control(SUNXI_DAC_DRC_MNGLS    , 0xffff, 0,    0x00002C3F);
	codec_wr_control(SUNXI_DAC_DRC_EPSHC    , 0xffff, 0,    0x00000000);
	codec_wr_control(SUNXI_DAC_DRC_EPSLC    , 0xffff, 0,    0x0000640C);
}

void dacdrc_enable(bool on) {
	if (on) {
		codec_wr_control( SUNXI_DAC_DAP_CTR, 0x1, 15, 1);
		codec_wr_control( SUNXI_DAC_DAP_CTR, 0x1, 31, 1);

	} else {
		codec_wr_control( SUNXI_DAC_DAP_CTR, 0x1, 15, 0);
		codec_wr_control( SUNXI_DAC_DAP_CTR, 0x1, 31, 0);
	}

}

static void codec_init(uint32_t lineout_vol) {
	codec_wr_control(SUNXI_DAC_FIFOC, 0x1, FIFO_FLUSH, 0x1);

	codec_wr_prcm_control(PAEN_CTR, 0x1, LINEOUTEN, 0x1);

	codec_wr_prcm_control(LINEOUT_VOLC, 0x1f, LINEOUTVOL, lineout_vol);

	codec_wr_prcm_control(DAC_PA_SRC, 0x1, DACAREN, 0x1);	// Internal Analog Right channel DAC enable
	codec_wr_prcm_control(DAC_PA_SRC, 0x1, DACALEN, 0x1);	// Internal Analog Left channel DAC enable
	codec_wr_prcm_control(DAC_PA_SRC, 0x1, RMIXEN, 0x1);	// Right Analog Output Mixer Enable
	codec_wr_prcm_control(DAC_PA_SRC, 0x1, LMIXEN, 0x1);	// Left Analog Output Mixer Enable

	codec_wr_prcm_control(MIC2G_LINEOUT_CTR, 0x1, LINEOUTL_EN, 0x1); // Enable Line-out Left
	codec_wr_prcm_control(MIC2G_LINEOUT_CTR, 0x1, LINEOUTR_EN, 0x1); // Enable Line-out Right
	codec_wr_prcm_control(MIC2G_LINEOUT_CTR, 0x1, LINEOUTR_SS, 0x1); // Differential output

	codec_wr_prcm_control(LOMIXSC, 0x1, LMIXMUTEDACL, 0x1);
	codec_wr_prcm_control(ROMIXSC, 0x1, RMIXMUTEDACR, 0x1);
}

static void clk_set_rate_codec(uint32_t rate) {
	uint32_t value;
	uint32_t factorn;
	uint32_t factorm;
	uint32_t factorp;
	uint32_t sdmval;

    if(rate == 22579200) {
        factorn = 6;
        factorm = 0;
        factorp = 7;
		sdmval = 0xc0010d84;
    } else { //if(rate == 24576000) {
        factorn = 13;
        factorm = 0;
        factorp = 13;
		sdmval = 0xc000ac02;
    }

	value = PLL_ENABLE;
	value |= PLL_SDM_ENABLE;
	value |= (factorn << PLL_FACTOR_N_SHIFT);
	value |= (factorm << PLL_FACTOR_M_SHIFT);
	value |= (factorp << PLL_FACTOR_P_SHIFT);
	H3_CCU->PLL_AUDIO_CTRL = value;

	H3_CCU->PLL_AUDIO_PAT_CTRL = sdmval;

	do {
		value = H3_CCU->PLL_AUDIO_CTRL;
	} while (!(value & PLL_LOCK));
}

/*
SUNXI_CLK_PERIPH(name,    mux_reg,    mux_shift, mux_width, div_reg,    div_mshift, div_mwidth, div_nshift, div_nwidth, gate_flags, enable_reg, reset_reg, bus_gate_reg, drm_gate_reg, enable_shift, reset_shift, bus_gate_shift, dram_gate_shift, lock,com_gate,com_gate_off)
SUNXI_CLK_PERIPH(adda,    0,           0,        0,         0,          0,          0,          0,          0,          0,          ADDA_CFG,   BUS_RST3,  BUS_GATE2,    0,           31,            0,           0,              0,               &clk_lock,NULL,             0);
*/

static void clk_set_rate_codec_module(uint32_t rate) {
	H3_CCU->AC_DIG_CLK = SCLK_1X_GATING;

	H3_CCU->BUS_SOFT_RESET3 |= CCU_BUS_SOFT_RESET3_AC;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING2 &= ~CCU_BUS_CLK_GATING2_AC_DIG;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING2 |= CCU_BUS_CLK_GATING2_AC_DIG;
}

static void codec_prepare(uint32_t rate) {
	codec_wr_control(SUNXI_DAC_DPC, 0x1, DAC_EN, 0x1);
	codec_wr_prcm_control(LINEOUT_PA_GAT, 0x1, PA_CLK_GC, 0x0);

	codec_wr_control(SUNXI_DAC_FIFOC, 0x1, DAC_DRQ_EN, 0x1);

	/* Flush the TX FIFO */
	codec_wr_control(SUNXI_DAC_FIFOC, 0x1, FIFO_FLUSH, 0x1);

	/* Set TX FIFO Empty Trigger Level */
	codec_wr_control(SUNXI_DAC_FIFOC, 0x3f, TX_TRI_LEVEL, 0xf);

	if (rate > 32000) {
		/* Use 64 bits FIR filter */
		codec_wr_control(SUNXI_DAC_FIFOC, 0x1, FIR_VER, 0x0);
	} else {
		/* Use 32 bits FIR filter */
		codec_wr_control(SUNXI_DAC_FIFOC, 0x1, FIR_VER, 0x1);
	}

	/* Send zeros when we have an underrun */
	codec_wr_control(SUNXI_DAC_FIFOC, 0x1, SEND_LASAT, 0x0);
}


static void codec_hw_params(uint32_t rate, uint32_t channels) {
	uint32_t reg_val;

	/* Set DAC sample rate */
	switch (rate) {
	case 44100:
		clk_set_rate_codec(22579200);
		clk_set_rate_codec_module(22579200);
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val &= ~(7 << 29);
		reg_val |= (0 << 29);
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		break;
	case 22050:
		clk_set_rate_codec(22579200);
		clk_set_rate_codec_module(22579200);
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val &= ~(7 << 29);
		reg_val |= (2 << 29);
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		break;
	case 11025:
		clk_set_rate_codec(22579200);
		clk_set_rate_codec_module(22579200);
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val &= ~(7 << 29);
		reg_val |= (4 << 29);
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		break;
	case 48000:
		clk_set_rate_codec(24576000);
		clk_set_rate_codec_module(24576000);
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val &= ~(7 << 29);
		reg_val |= (0 << 29);
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		break;
	case 96000:
		clk_set_rate_codec(24576000);
		clk_set_rate_codec_module(24576000);
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val &= ~(7 << 29);
		reg_val |= (7 << 29);
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		break;
	case 192000:
		clk_set_rate_codec(24576000);
		clk_set_rate_codec_module(24576000);
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val &= ~(7 << 29);
		reg_val |= (6 << 29);
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		break;
	case 32000:
		clk_set_rate_codec(24576000);
		clk_set_rate_codec_module(24576000);
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val &= ~(7 << 29);
		reg_val |= (1 << 29);
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		break;
	case 24000:
		clk_set_rate_codec(24576000);
		clk_set_rate_codec_module(24576000);
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val &= ~(7 << 29);
		reg_val |= (2 << 29);
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		break;
	case 16000:
		clk_set_rate_codec(24576000);
		clk_set_rate_codec_module(24576000);
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val &= ~(7 << 29);
		reg_val |= (3 << 29);
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		break;
	case 12000:
		clk_set_rate_codec(24576000);
		clk_set_rate_codec_module(24576000);
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val &= ~(7 << 29);
		reg_val |= (4 << 29);
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		break;
	case 8000:
		clk_set_rate_codec(24576000);
		clk_set_rate_codec_module(24576000);
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val &= ~(7 << 29);
		reg_val |= (5 << 29);
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		break;
	default:
		clk_set_rate_codec(24576000);
		clk_set_rate_codec_module(24576000);
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val &= ~(7 << 29);
		reg_val |= (0 << 29);
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		break;
	}

	/* Set the number of channels we want to use */
	if (channels == 1) {
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val |= (1 << 6);		// TODO use define
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
	} else {
		reg_val = readl(CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
		reg_val &= ~(1 << 6);
		writel(reg_val, CODEC_BASSADDRESS + SUNXI_DAC_FIFOC);
	}

	/* Set the number of sample bits to 16 bits */
	codec_wr_control(SUNXI_DAC_FIFOC ,0x1, TX_SAMPLE_BITS, 0x0);

	/* Set TX FIFO mode to repeat the MSB */
	codec_wr_control(SUNXI_DAC_FIFOC ,0x1, FIFO_MODE, 0x1);

	/* DMA_SLAVE_BUSWIDTH_2_BYTES */

	codec_prepare(rate);
}

static void __attribute__((interrupt("FIQ"))) fiq_handler(void) {
	dmb();

#ifdef LOGIC_ANALYZER
	h3_gpio_set(6);
#endif

	H3_DMA->IRQ_PEND0 |= H3_DMA->IRQ_PEND0;

	H3_GIC_CPUIF->EOI = H3_DMA_IRQn;
	gic_unpend(H3_DMA_IRQn);
	isb();

	udelay(10);

#ifdef LOGIC_ANALYZER
	h3_gpio_clr(6);
#endif
	dmb();
}

void h3_codec_begin(void) {
#ifdef LOGIC_ANALYZER
	h3_gpio_fsel(6, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(6);
#endif

	/*
	 * DMA setup
	 */

	uint32_t i;

	int16_t *txbuffs = &p_coherent_region->txbuffer[0];

	for (i = 0; i < TX_TOTAL_BUFSIZE; i++) {
		p_coherent_region->txbuffer[i] = 0;
	}

	for (i = 0; i < CONFIG_TX_DESCR_NUM; i++) {
		struct sunxi_dma_lli *lli = &p_coherent_region->lli[i];

		lli->cfg = DMA_CHAN_CFG_DST_IO_MODE | DMA_CHAN_CFG_SRC_LINEAR_MODE
					| DMA_CHAN_CFG_SRC_DRQ(DRQSRC_SDRAM) | DMA_CHAN_CFG_SRC_WIDTH(1) | DMA_CHAN_CFG_SRC_BURST(1)
					| DMA_CHAN_CFG_DST_DRQ(DRQDST_AUDIO_CODEC) | DMA_CHAN_CFG_DST_WIDTH(1) | DMA_CHAN_CFG_DST_BURST(1);
		lli->src = (uint32_t) &txbuffs[i * CONFIG_BUFSIZE];
		lli->dst = (uint32_t) CODEC_BASSADDRESS + SUNXI_DAC_TXDATA;
		lli->len = CONFIG_BUFSIZE;
		lli->para = DMA_NORMAL_WAIT;

		if (i > 0) {
			struct sunxi_dma_lli *lli_prev = &p_coherent_region->lli[i - 1];
			lli_prev->p_lli_next = (uint32_t) lli;
		}
	}

	struct sunxi_dma_lli *lli_last = &p_coherent_region->lli[CONFIG_TX_DESCR_NUM - 1];
	lli_last->p_lli_next =  (uint32_t) &p_coherent_region->lli[0];

	__disable_fiq();

	arm_install_handler((unsigned) fiq_handler, ARM_VECTOR(ARM_VECTOR_FIQ));

	gic_fiq_config(H3_DMA_IRQn, GIC_CORE0);

	H3_CCU->BUS_SOFT_RESET0 |= CCU_BUS_SOFT_RESET0_DMA;
	H3_CCU->BUS_CLK_GATING0 |= CCU_BUS_CLK_GATING0_DMA;

	H3_DMA->IRQ_PEND0 |= H3_DMA->IRQ_PEND0;
	H3_DMA->IRQ_PEND1 |= H3_DMA->IRQ_PEND1;

	H3_DMA->IRQ_EN0 = DMA_IRQ_EN0_DMA0_PKG_IRQ_EN;

	H3_DMA_CHL0->DESC_ADDR = (uint32_t) &p_coherent_region->lli[0];

	isb();

	/**
	 * Codec setup
	 */

	__enable_fiq();

	codec_init(31);

	codec_hw_params(48000, 1);

	/*
	 * Stop issuing DRQ when we have room for less than 16 samples
	 * in our TX FIFO
	 */
	codec_wr_control(SUNXI_DAC_FIFOC, 0x3, DAC_DRQ_CLR_CNT, 0x3);
}

void h3_codec_start(void) {
	writel(0, CODEC_BASSADDRESS+SUNXI_DAC_DAP_CTR);

#ifndef NDEBUG
	uint32_t i;

	for (i = 0; i < CONFIG_TX_DESCR_NUM; i++) {
		const struct sunxi_dma_lli *lli = &p_coherent_region->lli[i];
		h3_dma_dump_lli(lli);
	}

	uint32_t value = H3_CCU->PLL_AUDIO_CTRL;
	const uint32_t n = (value >> PLL_FACTOR_N_SHIFT) & PLL_FACTOR_N_MASK;
	const uint32_t p = (value >> PLL_FACTOR_P_SHIFT) & PLL_FACTOR_P_MASK;
	const uint32_t m = (value >> PLL_FACTOR_M_SHIFT) & PLL_FACTOR_M_MASK;

	uint32_t freq = H3_F_24M;
	freq *= (n + 1);
	freq /= (p + 1);
	freq /= (m + 1);

	printf("================\n");
	printf("H3_CCU->PLL_AUDIO_CTRL=%p ", H3_CCU->PLL_AUDIO_CTRL);
	debug_print_bits(H3_CCU->PLL_AUDIO_CTRL);
	printf("CCU_PLL_AUDIO=%ld n=%d,m=%d,p=%d\n", (long int) freq, n, m, p);
	printf("================\n");

	printf("AC_DAC_DPC=%p ", readl(CODEC_BASSADDRESS));
	debug_print_bits(readl(CODEC_BASSADDRESS));

	printf("AC_DAC_FIFOC=%p ", readl(CODEC_BASSADDRESS+SUNXI_DAC_FIFOC));
	debug_print_bits(readl(CODEC_BASSADDRESS+SUNXI_DAC_FIFOC));

	printf("AC_DAC_FIFOS=%p ", readl(CODEC_BASSADDRESS+SUNXI_DAC_FIFOS));
	debug_print_bits(readl(CODEC_BASSADDRESS+SUNXI_DAC_FIFOS));

	printf("AC_DAC_DAP_CTR=%p ", readl(CODEC_BASSADDRESS+SUNXI_DAC_DAP_CTR));
	debug_print_bits(readl(CODEC_BASSADDRESS+SUNXI_DAC_DAP_CTR));

	printf("AC_DAC_DRC_CTRL=%p ", readl(CODEC_BASSADDRESS+SUNXI_DAC_DRC_CTRL));
	debug_print_bits(readl(CODEC_BASSADDRESS+SUNXI_DAC_DRC_CTRL));

	printf("================\n");

	printf("DAC_PA_SRC=%p ", read_prcm_wvalue(DAC_PA_SRC));
	debug_print_bits(read_prcm_wvalue(DAC_PA_SRC));

	printf("MIC2G_LINEOUT_CTR=%p ", read_prcm_wvalue(MIC2G_LINEOUT_CTR));
	debug_print_bits(read_prcm_wvalue(MIC2G_LINEOUT_CTR));

	printf("================\n");

	//debug_dump(p_coherent_region->txbuffer, p_coherent_region->lli[0].len);
#endif

	H3_DMA_CHL0->EN = DMA_CHAN_ENABLE_START;

	h3_dma_dump_chl(H3_DMA_CHL0_BASE);
}

int16_t *h3_codec_get_pointer(uint32_t index, uint32_t length) {
	assert(index < CONFIG_TX_DESCR_NUM);
	assert((length * 2) < CONFIG_BUFSIZE);

	struct sunxi_dma_lli *lli = &p_coherent_region->lli[index];

	lli->len = length * 2;

	const int16_t *txbuffs = &p_coherent_region->txbuffer[0];

	return (int16_t *)&txbuffs[index * CONFIG_BUFSIZE];
}

