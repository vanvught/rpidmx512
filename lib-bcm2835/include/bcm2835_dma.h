/**
 * @file bcm2835_dma.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef BCM2835_DMA_H_
#define BCM2835_DMA_H_

#include <stdint.h>

#define BCM2835_DMA_CHANNELS						7			///< only channels 0-6 are supported so far

#define BCM2835_DMA_CS_RESET						(1 << 31)	///<
#define BCM2835_DMA_CS_ABORT						(1 << 30)	///<
#define BCM2835_DMA_CS_DISDEBUG						(1 << 29)	///<
#define BCM2835_DMA_CS_WAIT_FOR_OUTSTANDING_WRITES	(1 << 28)	///<
#define BCM2835_DMA_S_PRIORITY_SHIFT				16			///<
#define BCM2835_DMA_CS_ERROR						(1 << 8)	///<
#define BCM2835_DMA_CS_PAUSED						(1 << 4)	///<
#define BCM2835_DMA_CS_DREQ							(1 << 3)	///<
#define BCM2835_DMA_CS_INT							(1 << 2)	///<
#define BCM2835_DMA_CS_END							(1 << 1)	///<
#define BCM2835_DMA_CS_ACTIVE						(1 << 0)	///<

#define BCM2835_DMA_TI_PERMAP_SHIFT			16			///<
#define BCM2835_DMA_TI_BURST_LENGTH_SHIFT	12			///<
#define BCM2835_DMA_TI_SRC_IGNORE			(1 << 11)	///<
#define BCM2835_DMA_TI_SRC_DREQ				(1 << 10)	///<
#define BCM2835_DMA_TI_SRC_WIDTH			(1 << 9)	///<
#define BCM2835_DMA_TI_SRC_INC				(1 << 8)	///<
#define BCM2835_DMA_TI_DEST_DREQ			(1 << 6)	///<
#define BCM2835_DMA_TI_DEST_WIDTH			(1 << 5)	///<
#define BCM2835_DMA_TI_DEST_INC				(1 << 4)	///<
#define BCM2835_DMA_TI_WAIT_RESP			(1 << 3)	///<
#define BCM2835_DMA_TI_TDMODE				(1 << 1)	///<
#define BCM2835_DMA_TI_INTEN				(1 << 0)	///<

#define BCM2835_DMA_TXFR_LEN_XLENGTH_SHIFT	0			///<
#define BCM2835_DMA_TXFR_LEN_YLENGTH_SHIFT	16			///<

#define BCM2835_DMA_STRIDE_SRC_SHIFT		0			///<
#define BCM2835_DMA_STRIDE_DEST_SHIFT		16			///<

#define BCM2835_DMA_DEBUG_LITE				(1 << 28)	///<

#define BCM2835_DMA_DEFAULT_PRIORITY		0			///<
#define BCM2835_DMA_DEFAULT_BURST_LENGTH	0			///<
#define BCM2835_DMA_TXFR_LEN_MAX			0x3FFFFFFF	///<

struct dma_control_block {
	uint32_t transfer_information;			///< Transfer Information
	uint32_t source_address;				///< Source Address
	uint32_t destination_address;			///< Destination Address
	uint32_t transfer_length;				///< Transfer Length
	uint32_t mode_stride;					///< 2D Mode Stride
	uint32_t next_control_block_address;	///< Next Control Block Address
	uint32_t reserved[2];					///< Reserved – set to zero
};

#endif /* BCM2835_DMA_H_ */
