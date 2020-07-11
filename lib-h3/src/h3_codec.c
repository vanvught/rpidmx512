/**
 * @file h3_codec.h
 *
 */
/*
 * Based on https://github.com/allwinner-zh/linux-3.4-sunxi/blob/master/sound/soc/sunxi/audiocodec/sun8iw7_sndcodec.c
 * Based on https://elixir.bootlin.com/linux/latest/source/sound/soc/sunxi/sun4i-codec.c
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

//TODO This could be moved to h3.h
#define WRREG_BITS(reg,mask,value)			reg = ((reg) & ~(mask)) | (value);
#define WR_CONTROL(reg,mask,shift,value)	WRREG_BITS(reg,((uint32_t)mask << shift),((uint32_t)value << shift))

/*
 * DAC Digital Part Control Register
 * DAC_DPC
 */
#define DAC_EN			(31)
#define HPF_EN			(18)
#define DIGITAL_VOL		(12)
#define HUB_EN			(0)

/*
 * DAC FIFO Control Register
 * DAC_FIFOC
 */
#define DAC_FS			(29)
#define FIR_VER			(28)
#define SEND_LASAT		(26)
#define FIFO_MODE		(24)
#define DAC_DRQ_CLR_CNT	(21)
#define TX_TRI_LEVEL	(8)
#define ADDA_LOOP_EN	(7)
#define DAC_MONO_EN		(6)
#define TX_SAMPLE_BITS	(5)
#define DAC_DRQ_EN		(4)
#define FIFO_FLUSH		(0)

/*
 * PRCM
 * AUDIO_CFG 0x1C0
 */
#define LINEOUT_PA_GAT		(0x00)
	#define PA_CLK_GC			(7)
#define LOMIXSC				(0x01)
	#define LMIXMUTEDACR		(0)
	#define LMIXMUTEDACL		(1)
#define ROMIXSC				(0x02)
	#define RMIXMUTEDACL		(0)
	#define RMIXMUTEDACR		(1)
#define DAC_PA_SRC			(0x03)
	#define DACAREN				(7)
	#define DACALEN				(6)
	#define RMIXEN				(5)
	#define LMIXEN				(4)
#define PAEN_CTR			(0x07)
	#define LINEOUTEN			(7)
#define LINEOUT_VOLC		(0x09)
	#define LINEOUTVOL			(3)
#define MIC2G_LINEOUT_CTR	(0x0A)
	#define MIC2AMPEN			(7)
	#define MIC2BOOST			(4)
	#define LINEOUTL_EN			(3)
	#define LINEOUTR_EN			(2)
	#define LINEOUTL_SS			(1)
	#define LINEOUTR_SS			(0)

#define SCLK_1X_GATING		(1U << 31)

#define PLL_FACTOR_M_MASK	0x1F
#define PLL_FACTOR_M_SHIFT	0

#define PLL_FACTOR_N_MASK	0x7F
#define PLL_FACTOR_N_SHIFT	8

#define PLL_FACTOR_P_MASK	0x0F
#define PLL_FACTOR_P_SHIFT	16

#define PLL_LOCK			(1U << 28)	// Read only, 1 indicates that the PLL has been stable
#define PLL_SDM_ENABLE		(1U << 24)
#define PLL_ENABLE			(1U << 31)

#define	CONFIG_BUFSIZE				(8 * 1024)
#define CONFIG_TX_DESCR_NUM			(1U << 1)		// INFO This cannot be changed without rewriting FIQ handler
#define CONFIG_TX_DESCR_NUM_MASK    (CONFIG_TX_DESCR_NUM - 1)

static volatile uint32_t circular_buffer_index_head;
static volatile uint32_t circular_buffer_index_tail;
static uint32_t circular_buffer_size;
#ifndef NDEBUG
 static volatile bool circular_buffer_full;
#endif

#define CIRCULAR_BUFFER_INDEX_ENTRIES	(1U << 1)
#define CIRCULAR_BUFFER_INDEX_MASK 		(CIRCULAR_BUFFER_INDEX_ENTRIES - 1)

static int16_t circular_buffer[CIRCULAR_BUFFER_INDEX_ENTRIES][CONFIG_BUFSIZE] ALIGNED;

struct coherent_region {
	struct sunxi_dma_lli lli[CONFIG_TX_DESCR_NUM];
	int16_t txbuffer[CONFIG_TX_DESCR_NUM][CONFIG_BUFSIZE] ALIGNED;
};

static struct coherent_region *p_coherent_region = (struct coherent_region *)(H3_MEM_COHERENT_REGION + MEGABYTE/2);

static uint32_t read_prcm_wvalue(uint32_t addr) {
	uint32_t reg;

	reg = H3_PRCM->AUDIO_CFG;
	reg |= (0x1 << 28);
	H3_PRCM->AUDIO_CFG = reg;

	reg = H3_PRCM->AUDIO_CFG;
	reg &= ~(0x1U << 24);
	H3_PRCM->AUDIO_CFG = reg;

	reg = H3_PRCM->AUDIO_CFG;
	reg &= ~(0x1fU << 16);
	reg |= (addr << 16);
	H3_PRCM->AUDIO_CFG = reg;

	reg = H3_PRCM->AUDIO_CFG;
	reg &= (0xff << 0);

	return reg;
}

static void write_prcm_wvalue(uint32_t addr, uint32_t val) {
	uint32_t reg;

	reg = H3_PRCM->AUDIO_CFG;
	reg |= (0x1 << 28);
	H3_PRCM->AUDIO_CFG = reg;

	reg = H3_PRCM->AUDIO_CFG;
	reg &= ~(0x1fU << 16);
	reg |= (addr << 16);
	H3_PRCM->AUDIO_CFG = reg;

	reg = H3_PRCM->AUDIO_CFG;
	reg &= ~(0xffU << 8);
	reg |= (val << 8);
	H3_PRCM->AUDIO_CFG = reg;

	reg = H3_PRCM->AUDIO_CFG;
	reg |= (0x1 << 24);
	H3_PRCM->AUDIO_CFG = reg;

	reg = H3_PRCM->AUDIO_CFG;
	reg &= ~(0x1U << 24);
	H3_PRCM->AUDIO_CFG = reg;
}

static void codec_wrreg_prcm_bits(uint32_t reg, uint32_t mask, uint32_t value) {
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

static void codec_init(uint32_t lineout_vol) {
	WR_CONTROL(H3_AC->DAC_FIFOC, 0x1, FIFO_FLUSH, 0x1);

	codec_wr_prcm_control(PAEN_CTR, 0x1, LINEOUTEN, 0x1);

	codec_wr_prcm_control(LINEOUT_VOLC, 0x1f, LINEOUTVOL, lineout_vol);

	codec_wr_prcm_control(DAC_PA_SRC, 0x1, DACAREN, 0x1);				// Internal Analog Right channel DAC enable
	codec_wr_prcm_control(DAC_PA_SRC, 0x1, DACALEN, 0x1);				// Internal Analog Left channel DAC enable
	codec_wr_prcm_control(DAC_PA_SRC, 0x1, RMIXEN, 0x1);				// Right Analog Output Mixer Enable
	codec_wr_prcm_control(DAC_PA_SRC, 0x1, LMIXEN, 0x1);				// Left Analog Output Mixer Enable

	codec_wr_prcm_control(MIC2G_LINEOUT_CTR, 0x1, LINEOUTL_EN, 0x1);	// Enable Line-out Left
	codec_wr_prcm_control(MIC2G_LINEOUT_CTR, 0x1, LINEOUTR_EN, 0x1);	// Enable Line-out Right
	codec_wr_prcm_control(MIC2G_LINEOUT_CTR, 0x1, LINEOUTR_SS, 0x1);	// Differential output

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

static void clk_set_rate_codec_module(void) {
	H3_CCU->AC_DIG_CLK = SCLK_1X_GATING;

	H3_CCU->BUS_SOFT_RESET3 |= CCU_BUS_SOFT_RESET3_AC;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING2 &= ~CCU_BUS_CLK_GATING2_AC_DIG;
	udelay(1000); // 1ms
	H3_CCU->BUS_CLK_GATING2 |= CCU_BUS_CLK_GATING2_AC_DIG;
}

static void codec_prepare(uint32_t rate) {
	WR_CONTROL(H3_AC->DAC_DPC, 0x1, DAC_EN, 0x1);

	codec_wr_prcm_control(LINEOUT_PA_GAT, 0x1, PA_CLK_GC, 0x0);

	WR_CONTROL(H3_AC->DAC_FIFOC, 0x1, DAC_DRQ_EN, 0x1);

	/* Flush the TX FIFO */
	WR_CONTROL(H3_AC->DAC_FIFOC, 0x1, FIFO_FLUSH, 0x1);

	/* Set TX FIFO Empty Trigger Level */
	WR_CONTROL(H3_AC->DAC_FIFOC, 0x3f, TX_TRI_LEVEL, 0xf);

	if (rate > 32000) {
		/* Use 64 bits FIR filter */
		WR_CONTROL(H3_AC->DAC_FIFOC, 0x1, FIR_VER, 0x0);
	} else {
		/* Use 32 bits FIR filter */
		WR_CONTROL(H3_AC->DAC_FIFOC, 0x1, FIR_VER, 0x1);
	}

	/* Send last when we have an underrun */
	WR_CONTROL(H3_AC->DAC_FIFOC, 0x1, SEND_LASAT, 0x1);
}

static uint32_t codec_get_mod_freq(uint32_t rate) {
	switch (rate) {
	case 176400:
	case 88200:
	case 44100:
	case 33075:
	case 22050:
	case 14700:
	case 11025:
	case 7350:
		return 22579200;

	case 192000:
	case 96000:
	case 48000:
	case 32000:
	case 24000:
	case 16000:
	case 12000:
	case 8000:
		return 24576000;

	default:
		return 24576000; // Default for 48000
	}
}

static uint32_t codec_get_hw_rate(uint32_t rate) {
	switch (rate) {
	case 192000:
	case 176400:
		return 6;

	case 96000:
	case 88200:
		return 7;

	case 48000:
	case 44100:
		return 0;

	case 32000:
	case 33075:
		return 1;

	case 24000:
	case 22050:
		return 2;

	case 16000:
	case 14700:
		return 3;

	case 12000:
	case 11025:
		return 4;

	case 8000:
	case 7350:
		return 5;

	default:
		return 0; // Default for 48000
	}
}

static void codec_hw_params(uint32_t rate, uint32_t channels) {
	clk_set_rate_codec_module();

	/* Set DAC sample rate */
	const uint32_t hw_rate = codec_get_hw_rate(rate);

	WR_CONTROL(H3_AC->DAC_FIFOC, 0x7, DAC_FS, hw_rate);

	DEBUG_PRINTF("hw_rate=%d", hw_rate);

	const uint32_t mod_freq = codec_get_mod_freq(rate);

	clk_set_rate_codec(mod_freq);

	DEBUG_PRINTF("mod_freq=%d", mod_freq);

	/* Set the number of channels we want to use */
	if (channels == 1) {
		WR_CONTROL(H3_AC->DAC_FIFOC, 0x1, DAC_MONO_EN, 0x1);
	} else {
		WR_CONTROL(H3_AC->DAC_FIFOC, 0x1, DAC_MONO_EN, 0x0);
	}

	/* Set the number of sample bits to 16 bits */
	WR_CONTROL(H3_AC->DAC_FIFOC ,0x1, TX_SAMPLE_BITS, 0x0);

	/* Set TX FIFO mode to repeat the MSB */
	WR_CONTROL(H3_AC->DAC_FIFOC ,0x1, FIFO_MODE, 0x1);

	/* DMA_SLAVE_BUSWIDTH_2_BYTES */

	codec_prepare(rate);
}

static void __attribute__((interrupt("FIQ"))) codec_fiq_handler(void) {
	dmb();
	clean_data_cache();
	isb();

#ifdef LOGIC_ANALYZER
	h3_gpio_set(6);
#endif


#if (CONFIG_TX_DESCR_NUM != 2)
 #error
#endif
	int16_t *txbuffs = &p_coherent_region->txbuffer[0][0];

	if ((H3_DMA_CHL0->CUR_SRC & (2 * CONFIG_BUFSIZE)) == ((uint32_t) txbuffs & (2 * CONFIG_BUFSIZE))) {
		txbuffs = &p_coherent_region->txbuffer[1][0];
	}

	int16_t *src;

	if (circular_buffer_index_head != circular_buffer_index_tail) {
		src = &circular_buffer[circular_buffer_index_tail][0];
		circular_buffer_index_tail = (circular_buffer_index_tail + 1) & CIRCULAR_BUFFER_INDEX_MASK;
#ifndef NDEBUG
		circular_buffer_full = false;
#endif
	} else {
		src = &circular_buffer[1 - circular_buffer_index_head][0];
	}

	uint32_t i;

	for (i = 0; i < circular_buffer_size; i++) {
		*txbuffs = *src;
		txbuffs++;
		src++;
	}

	dmb();

	H3_DMA->IRQ_PEND0 |= H3_DMA->IRQ_PEND0;

	H3_GIC_CPUIF->EOI = H3_DMA_IRQn;
	gic_unpend(H3_DMA_IRQn);
	isb();

#ifdef LOGIC_ANALYZER
	h3_gpio_clr(6);
#endif

	dmb();
}

void h3_codec_begin(void) {
	__disable_fiq();

#ifdef LOGIC_ANALYZER
	h3_gpio_fsel(1, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(1);
	h3_gpio_fsel(6, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(6);
#endif

	/*
	 * DMA setup
	 */

	uint32_t i,j;

	for (i = 0; i < CONFIG_TX_DESCR_NUM; i++) {
		for (j = 0; j < CONFIG_BUFSIZE; j++) {
			p_coherent_region->txbuffer[i][j] = 0;
		}
	}

	for (i = 0; i < CONFIG_TX_DESCR_NUM; i++) {
		struct sunxi_dma_lli *lli = &p_coherent_region->lli[i];

		lli->cfg = DMA_CHAN_CFG_DST_IO_MODE | DMA_CHAN_CFG_SRC_LINEAR_MODE
					| DMA_CHAN_CFG_SRC_DRQ(DRQSRC_SDRAM) | DMA_CHAN_CFG_SRC_WIDTH(1) | DMA_CHAN_CFG_SRC_BURST(1)
					| DMA_CHAN_CFG_DST_DRQ(DRQDST_AUDIO_CODEC) | DMA_CHAN_CFG_DST_WIDTH(1) | DMA_CHAN_CFG_DST_BURST(1);

		lli->src = (uint32_t) &p_coherent_region->txbuffer[i][0];
		lli->dst = (uint32_t) &H3_AC->DAC_TXDATA;
		lli->len = CONFIG_BUFSIZE;
		lli->para = DMA_NORMAL_WAIT;

		if (i > 0) {
			struct sunxi_dma_lli *lli_prev = &p_coherent_region->lli[i - 1];
			lli_prev->p_lli_next = (uint32_t) lli;
		}
#ifndef NDEBUG
		h3_dma_dump_lli(lli);
#endif
	}

	struct sunxi_dma_lli *lli_last = &p_coherent_region->lli[CONFIG_TX_DESCR_NUM - 1];
	lli_last->p_lli_next =  (uint32_t) &p_coherent_region->lli[0];

	arm_install_handler((unsigned) codec_fiq_handler, ARM_VECTOR(ARM_VECTOR_FIQ));

	gic_fiq_config(H3_DMA_IRQn, GIC_CORE0);

	H3_CCU->BUS_SOFT_RESET0 |= CCU_BUS_SOFT_RESET0_DMA;
	H3_CCU->BUS_CLK_GATING0 |= CCU_BUS_CLK_GATING0_DMA;

	H3_DMA->IRQ_PEND0 |= H3_DMA->IRQ_PEND0;
	H3_DMA->IRQ_PEND1 |= H3_DMA->IRQ_PEND1;

	H3_DMA->IRQ_EN0 = DMA_IRQ_EN0_DMA0_PKG_IRQ_EN;

	H3_DMA_CHL0->DESC_ADDR = (uint32_t) &p_coherent_region->lli[0];

	isb();
	__enable_fiq();

	/**
	 * Codec setup
	 */

	codec_init(31);

	codec_hw_params(48000, 1);

	/*
	 * Stop issuing DRQ when we have room for less than 16 samples
	 * in our TX FIFO
	 */
	WR_CONTROL(H3_AC->DAC_FIFOC, 0x3, DAC_DRQ_CLR_CNT, 0x3);

	circular_buffer_index_head = 0;
	circular_buffer_index_tail = 0;
#ifndef NDEBUG
	circular_buffer_full = false;
#endif
}

void __attribute__((cold)) h3_codec_start(void) {
	H3_AC->DAC_DAP_CTR = 0;

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

	printf("H3_AC->DAC_DPC=%p ", H3_AC->DAC_DPC);
	debug_print_bits(H3_AC->DAC_DPC);

	printf("H3_AC->DAC_FIFOC=%p ", H3_AC->DAC_FIFOC);
	debug_print_bits(H3_AC->DAC_FIFOC);

	printf("H3_AC->DAC_FIFOS=%p ", H3_AC->DAC_FIFOS);
	debug_print_bits(H3_AC->DAC_FIFOS);

	printf("H3_AC->DAC_DAP_CTR=%p ", H3_AC->DAC_DAP_CTR);
	debug_print_bits(H3_AC->DAC_DAP_CTR);

	printf("H3_AC->DAC_DRC_CTRL=%p ", H3_AC->DAC_DRC_CTRL);
	debug_print_bits(H3_AC->DAC_DRC_CTRL);

	printf("================\n");

	printf("DAC_PA_SRC=%p ", read_prcm_wvalue(DAC_PA_SRC));
	debug_print_bits(read_prcm_wvalue(DAC_PA_SRC));

	printf("MIC2G_LINEOUT_CTR=%p ", read_prcm_wvalue(MIC2G_LINEOUT_CTR));
	debug_print_bits(read_prcm_wvalue(MIC2G_LINEOUT_CTR));

	printf("================\n");
#endif

	H3_DMA_CHL0->EN = DMA_CHAN_ENABLE_START;

#ifndef NDEBUG
	h3_dma_dump_chl(H3_DMA_CHL0_BASE);
#endif
}

void h3_codec_set_buffer_length(uint32_t length) {
	assert((length * 2) < CONFIG_BUFSIZE);

	__disable_fiq();

	H3_DMA_CHL0->EN = DMA_CHAN_ENABLE_STOP;

	circular_buffer_size = length;

	uint32_t i;

	for (i = 0; i < CONFIG_TX_DESCR_NUM; i++) {
		struct sunxi_dma_lli *lli = &p_coherent_region->lli[i];
		lli->len = length * 2;;

	}

	__enable_fiq();
}

void h3_codec_push_data(const int16_t *src) {
#ifndef NDEBUG
	if (circular_buffer_full) {
		printf("f");
	}
#endif
	uint32_t i;

	int16_t *dst = &circular_buffer[circular_buffer_index_head][0];

	for (i = 0; i < circular_buffer_size; i++) {
		*dst = *src;
		dst++;
		src++;
	}

	circular_buffer_index_head = (circular_buffer_index_head + 1) & CIRCULAR_BUFFER_INDEX_MASK;

#ifndef NDEBUG
	circular_buffer_full = (circular_buffer_index_head == circular_buffer_index_tail);
#endif
}
