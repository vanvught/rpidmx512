/**
 * @file h3_spi_internal.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef H3_SPI_INTERNAL_H_
#define H3_SPI_INTERNAL_H_

#define GC_EN			(1 << 0) 	///< ENable
#define GC_MODE_MASTER	(1 << 1) 	///< Mode Master,  1 = Master, 0 = Slave
#define GC_TP_EN		(1 << 7) 	///< 1 = Stop transmit when RXFIFO is full
#define GC_SRST			(1 << 31) 	///< Soft Reset

#define TC_CPHA			(1 << 0)	///< 1 = Phase 1
#define TC_CPOL			(1 << 1)	///< 1 = Active Low
#define TC_SPOL			(1 << 2)  	///< 1 = Active Low
#define TC_SSCTL		(1 << 3)	///<
	#define TC_SS_MASK_SHIFT	4	///< Chip select
	#define TC_SS_MASK			(0x3 << TC_SS_MASK_SHIFT)
#define TC_SS_OWNER		(1 << 6)	///< 1 = Software Controlled
#define TC_SS_LEVEL		(1 << 7)	///< 1 = CS High
#define TC_DHB			(1 << 8)	///<
#define TC_DDB			(1 << 9)	///< Dummy Burst Type
#define TC_RPSM			(1 << 10)	///< Rapid Mode Select
#define TC_SDC			(1 << 11)	///< Master Sample Data Control
#define TC_FBS			(1 << 12) 	///< First Transmit Bit Select (1 = LSB)
#define TC_SDM			(1 << 13) 	///< Master Sample Data Mode
#define TC_XCH			(1 << 31)	///< Initiate transfer

#define IE_RX_RDY		(1 << 0)	///< RX FIFO Ready Request Interrupt Enable
#define IE_RX_EMP		(1 << 1)	///< RX FIFO Empty Interrupt Enable
#define IE_RX_FULL		(1 << 2)  	///< RX FIFO Full Interrupt Enable
#define IE_TX_ERQ		(1 << 4)  	///< TX FIFO Empty Request Interrupt Enable
#define IE_TX_EMP		(1 << 5)  	///< TX FIFO Empty Interrupt Enable
#define IE_TX_FULL		(1 << 6)	///< TX FIFO Full Interrupt Enable
#define IE_RX_OVF		(1 << 8)	///< RX FIFO Overflow Interrupt Enable
#define IE_RX_UDR   	(1 << 9)	///< RX FIFO Under run Interrupt Enable
#define IE_TX_OVF		(1 << 10)	///< TX FOFO Overflow Interrupt Enable
#define IE_TX_UDR		(1 << 11) 	///< TX FIFO Under run Interrupt Enable
#define IE_TC			(1 << 12) 	///< Transfer Completed Interrupt Enable
#define IE_SS			(1 << 13) 	///< SSI Interrupt Enable

#define IS_RX_RDY		(1 << 0)	///< RX FIFO Ready
#define IS_RX_EMP		(1 << 1)	///< RX FIFO Empty
#define IS_RX_FULL		(1 << 2)	///< RX FIFO Full
#define IS_TX_RDY		(1 << 4)	///< TX FIFO Ready
#define IS_TX_EMP		(1 << 5)	///< TX FIFO Empty
#define IS_TX_FULL		(1 << 6)	///< TX FIFO Full
#define IS_RX_OVF		(1 << 8)	///< RX FIFO Overflow
#define IS_RX_UDR		(1 << 9)	///< RX FIFO Under run
#define IS_TX_OVF		(1 << 10)	///< TX FIFO Overflow
#define IS_TX_UDR		(1 << 11)	///< TX FIFO Under run
#define IS_TC			(1 << 12)	///< Transfer completed
#define IS_SSI			(1 << 13)	///< SS Invalid Interrupt

	#define FC_RX_LEVEL_SHIFT	(0xFF << 0)///< RX FIFO Ready Request Trigger Level
#define FC_RX_DRQEN		(1 << 8)	///< RX FIFO DMA Request Enable
#define FC_RX_DMA_MODE	(1 << 9)	///< RX DMA Mode Control
#define FC_RX_TESTEN	(1 << 14)	///< RX Test Mode Enable
#define FC_RX_RST		(1 << 15)	///< RX FIFO Reset
	#define FC_TX_LEVEL_SHIFT	(0xFF << 16)///< TX FIFO Empty Request Trigger Level Shift
#define FC_TX_DRQEN		(1 << 24)	///< TX FIFO DMA Request Enable
#define FC_TX_TESTEN	(1 << 30)	///< TX Test Mode Enable
#define FC_TX_RST		(1 << 31)	///< TX FIFO Reset

#define	CC_DRS			(1 << 12)	///< Clock divider select, 1 = Rate 2
	#define	CC_CDR1_MASK	0x0F00	///< Clock Divide Rate 1 SPI_CLK = AHB_CLK/2^n
	#define	CC_CDR1_SHIFT	8
	#define CC_CDR2_MASK	0x00FF	///< Clock Divide Rate 2 SPI_CLK = AHB_CLK/(2*(n+1))
	#define CC_CDR2_SHIFT	0

#define FS_RX_CNT		(0xFF << 0)	///< RX FIFO Counter
#define FS_RB_CNT		(0x7 << 12)	///< RX FIFO Read Buffer Counter
#define FS_RB_WR		(1 << 15)	///< RX FIFO Read Buffer Write Enable
#define FS_TX_CNT		(0xFF << 16)///< TX FIFO Counter
#define FS_TB_CNT		(0x7 << 28)	///< TX FIFO Write Buffer Counter
#define FS_TB_WR		(1 << 31)	///< TX FIFO Write Buffer Write Enable
	#define FS_RXCNT_BIT_POS	0
	#define FS_TXCNT_BIT_POS	16

#define	SPI_FIFO_SIZE	64

#endif /* H3_SPI_INTERNAL_H_ */
