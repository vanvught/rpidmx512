/**
 * @file bcm2835.h
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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
#ifndef BCM2835_H_
#define BCM2835_H_

#define HIGH 0x1
#define LOW  0x0

// RPi Version 2
#define RPI_V2_GPIO_P1_03      2  ///< Version 2, Pin P1-03
#define RPI_V2_GPIO_P1_05      3  ///< Version 2, Pin P1-05
#define RPI_V2_GPIO_P1_07      4  ///< Version 2, Pin P1-07
#define RPI_V2_GPIO_P1_08     14  ///< Version 2, Pin P1-08, defaults to alt function 0 PL011_TXD
#define RPI_V2_GPIO_P1_10     15  ///< Version 2, Pin P1-10, defaults to alt function 0 PL011_RXD
#define RPI_V2_GPIO_P1_11     17  ///< Version 2, Pin P1-11
#define RPI_V2_GPIO_P1_12     18  ///< Version 2, Pin P1-12
#define RPI_V2_GPIO_P1_13     27  ///< Version 2, Pin P1-13
#define RPI_V2_GPIO_P1_15     22  ///< Version 2, Pin P1-15
#define RPI_V2_GPIO_P1_16     23  ///< Version 2, Pin P1-16
#define RPI_V2_GPIO_P1_18     24  ///< Version 2, Pin P1-18
#define RPI_V2_GPIO_P1_19     10  ///< Version 2, Pin P1-19, MOSI when SPI0 in use
#define RPI_V2_GPIO_P1_21      9  ///< Version 2, Pin P1-21, MISO when SPI0 in use
#define RPI_V2_GPIO_P1_22     25  ///< Version 2, Pin P1-22
#define RPI_V2_GPIO_P1_23     11  ///< Version 2, Pin P1-23, CLK when SPI0 in use
#define RPI_V2_GPIO_P1_24      8  ///< Version 2, Pin P1-24, CE0 when SPI0 in use
#define RPI_V2_GPIO_P1_26      7  ///< Version 2, Pin P1-26, CE1 when SPI0 in use

// System Timer
#define BCM2835_ST_CS_M0		((uint32_t)(1 << 0))	///<
#define BCM2835_ST_CS_M1		((uint32_t)(1 << 1))	///<
#define BCM2835_ST_CS_M2		((uint32_t)(1 << 2))	///<
#define BCM2835_ST_CS_M3		((uint32_t)(1 << 3))	///<

#define BCM2835_PERI_BASE			0x20000000
#define BCM2835_ST_BASE				(BCM2835_PERI_BASE + 0x3000)
#define BCM2835_IRQ_BASE			(BCM2835_PERI_BASE + 0xB200)
#define BCM2835_MAILBOX_BASE 		(BCM2835_PERI_BASE + 0xB880)
#define BCM2835_PM_WDOG_BASE		(BCM2835_PERI_BASE + 0x100000)
#define BCM2835_GPIO_BASE			(BCM2835_PERI_BASE + 0x200000)
#define BCM2835_SPI0_BASE			(BCM2835_PERI_BASE + 0x204000)
#define BCM2835_PL011_BASE			(BCM2835_PERI_BASE + 0x201000)
#define BCM2835_UART1_BASE			(BCM2835_PERI_BASE + 0x215000)
#define BCM2835_BSC1_BASE			(BCM2835_PERI_BASE + 0x804000)
#define BCM2835_BSC2_BASE			(BCM2835_PERI_BASE + 0x805000)

#ifdef __ASSEMBLY__
#else
#include <stdint.h>

// ST
extern void bcm2835_st_delay(const uint64_t offset_micros, const uint64_t micros);
// DELAY
extern void udelay(const uint64_t);

// https://github.com/raspberrypi/linux/blob/rpi-3.6.y/arch/arm/mach-bcm2708/include/mach/platform.h
#define ARM_IRQ1_BASE		0
#define INTERRUPT_TIMER1	(ARM_IRQ1_BASE + 1)
#define INTERRUPT_TIMER3	(ARM_IRQ1_BASE + 3)
#define INTERRUPT_AUX		(ARM_IRQ1_BASE + 29)

#define ARM_IRQ2_BASE		32
#define INTERRUPT_VC_UART	(ARM_IRQ2_BASE + 25)

typedef enum {
	// ARM_IRQ1_BASE
	BCM2835_TIMER1_IRQn = 1 << (INTERRUPT_TIMER1 - ARM_IRQ1_BASE),
	BCM2835_TIMER3_IRQn = 1	<< (INTERRUPT_TIMER3 - ARM_IRQ1_BASE),
	BCM2835_UART1_IRQn  = 1 << (INTERRUPT_AUX - ARM_IRQ1_BASE),
	// ARM_IRQ2_BASE
	BCM2835_VC_UART_IRQn  = 1 << (INTERRUPT_VC_UART - ARM_IRQ2_BASE)
} BCM2835_IRQn_TypeDef;

#define BCM2835_FIQ_ENABLE	(1 << 7)///< 0x80

#ifdef __cplusplus
#define		__I		volatile		///< defines 'read only' permissions
#else
#define		__I		volatile const	///< defines 'read only' permissions
#endif
#define		__O		volatile		///< defines 'write only' permissions
#define		__IO	volatile		///< defines 'read / write' permissions

typedef struct {
	__IO uint32_t CS;			///< 0x00	System Timer Control/Status 
	__IO uint32_t CLO;			///< 0x04	System Timer Counter Lower 32 bits
	__IO uint32_t CHI;			///< 0x08	System Timer Counter Higher 32 bits
	__I uint32_t  C0;			///< 0x0C	System Timer Compare 0.  DO NOT USE; is used by GPU.
	__IO uint32_t C1;			///< 0x10	System Timer Compare 1
	__I uint32_t  C2;			///< 0x14	System Timer Compare 2.  DO NOT USE; is used by GPU.
	__IO uint32_t C3;			///< 0x18	System Timer Compare 3
} BCM2835_ST_TypeDef;

typedef struct {
	__I uint32_t  IRQ;			///< 0x00
	__IO uint32_t ENABLE;		///< 0x04
	__IO uint32_t PAD[14];		///< 0x08
	__IO uint32_t IO;			///< 0x40
	__IO uint32_t IER;			///< 0x44
	__IO uint32_t IIR;			///< 0x48
	__IO uint32_t LCR;			///< 0x4C
	__IO uint32_t MCR;			///< 0x50
	__IO uint32_t LSR;			///< 0x54
	__IO uint32_t MSR;			///< 0x58
	__IO uint32_t SCR;			///< 0x5C
	__IO uint32_t CNTL;			///< 0x60
	__I uint32_t  STAT;			///< 0x64
	__IO uint32_t BAUD;			///< 0x68
} BCM2835_UART_TypeDef;

typedef struct {
	__IO uint32_t DR;			///< 0x00
	__IO uint32_t RSRECR;		///< 0x04
	__IO uint32_t PAD[4];		///< 0x08
	__IO uint32_t FR;			///< 0x18
	__IO uint32_t RES1;			///< 0x1C
	__IO uint32_t ILPR;			///< 0x20
	__IO uint32_t IBRD;			///< 0x24
	__IO uint32_t FBRD;			///< 0x28
	__IO uint32_t LCRH;			///< 0x2C
	__IO uint32_t CR;			///< 0x30
	__IO uint32_t IFLS;			///< 0x34
	__IO uint32_t IMSC;			///< 0x38
	__IO uint32_t RIS;			///< 0x3C
	__I uint32_t  MIS;			///< 0x40
	__IO uint32_t ICR;			///< 0x44
	__IO uint32_t DMACR;		///< 0x48
} BCM2835_PL011_TypeDef;

/// Defines for GPIO\n
/// The BCM2835 has 54 GPIO pins.\n
/// BCM2835 data sheet, Page 90 onwards.\n
typedef struct {
	__IO uint32_t GPFSEL0;		///< 0x00, GPIO Function Select 0
	__IO uint32_t GPFSEL1;		///< 0x04, GPIO Function Select 1
	__IO uint32_t GPFSEL2;		///< 0x08, GPIO Function Select 2
	__IO uint32_t GPFSEL3;		///< 0x0C, GPIO Function Select 3
	__IO uint32_t GPFSEL4;		///< 0x10, GPIO Function Select 4
	__IO uint32_t GPFSEL5;		///< 0x14, GPIO Function Select 5
	__IO uint32_t RES1;			///< 0x18, Reserved
	__O uint32_t GPSET0;		///< 0x1C, GPIO Pin Output Set 0
	__O uint32_t GPSET1;		///< 0x20, GPIO Pin Output Set 1
	__IO uint32_t RES2;			///< 0x24, Reserved
	__O uint32_t GPCLR0;		///< 0x28, GPIO Pin Output Clear 0
	__O uint32_t GPCLR1;		///< 0x2C, GPIO Pin Output Clear 1
	__IO uint32_t RES3;			///< 0x30, Reserved
	__I uint32_t GPLEV0;		///< 0x34, GPIO Pin Level 0
	__I uint32_t GPLEV1;		///< 0x38, GPIO Pin Level 1
	__IO uint32_t RES4;			///< 0x3C, Reserved
	__IO uint32_t GPEDS0;		///< 0x40, GPIO Pin Event Detect Status 0
	__IO uint32_t GPEDS1;		///< 0x44, GPIO Pin Event Detect Status 1
	__IO uint32_t RES5;			///< 0x48, Reserved
	__IO uint32_t GPREN0;		///< 0x4C, GPIO Pin Rising Edge Detect Enable 0
	__IO uint32_t GPREN1;		///< 0x50, GPIO Pin Rising Edge Detect Enable 1
	__IO uint32_t RES6;			///< 0x54, Reserved
	__IO uint32_t GPFEN0;		///< 0x58, GPIO Pin Falling Edge Detect Enable 0
	__IO uint32_t GPFEN1;		///< 0x5C, GPIO Pin Falling Edge Detect Enable 1
	__IO uint32_t RES7;			///< 0x60, Reserved
	__IO uint32_t GPHEN0;		///< 0x64, GPIO Pin High Detect Enable 0
	__IO uint32_t GPHEN1;		///< 0x68, GPIO Pin High Detect Enable 1
	__IO uint32_t RES8;			///< 0x6C, Reserved
	__IO uint32_t GPLEN0;		///< 0x70, GPIO Pin Low Detect Enable 0
	__IO uint32_t GPLEN1;		///< 0x74, GPIO Pin Low Detect Enable 1
	__IO uint32_t RES9;			///< 0x78, Reserved
	__IO uint32_t AREN[2];		///< 0x7C
	__IO uint32_t RES10;		///< 0x84, Reserved
	__IO uint32_t AFEN[2];		///< 0x88
	__IO uint32_t RES11;		///< 0x90
	__IO uint32_t GPPUD; 		///< 0x94, GPIO Pin Pull-up/down Enable
	__IO uint32_t GPPUDCLK0;	///< 0x98, GPIO Pin Pull-up/down Enable Clock 0
	__IO uint32_t GPPUDCLK1;	///< 0x9C, GPIO Pin Pull-up/down Enable Clock 1
} BCM2835_GPIO_TypeDef;

typedef struct {
	__IO uint32_t CS;			///< 0x00
	__IO uint32_t FIFO;			///< 0x04
	__IO uint32_t CLK;			///< 0x08
	__IO uint32_t DLEN;			///< 0x0C
	__IO uint32_t LTOH;			///< 0x10
	__IO uint32_t DC;			///< 0x14
} BCM2835_SPI_TypeDef;

/// Defines for I2C\n
/// GPIO register offsets from BCM2835_BSC*_BASE.\n
/// Offsets into the BSC Peripheral block in bytes per 3.1 BSC Register Map
typedef struct {
	__IO uint32_t C;		///< 0x00, BSC Master Control
	__IO uint32_t S;		///< 0x04, BSC Master Status
	__IO uint32_t DLEN;		///< 0x08, BSC Master Data Length
	__IO uint32_t A;		///< 0x0C, BSC Master Slave Address
	__IO uint32_t FIFO;		///< 0x10, BSC Master Data FIFO
	__IO uint32_t DIV;		///< 0x14, BSC Master Clock Divider
	__IO uint32_t DEL;		///< 0x18, BSC Master Data Delay
	__IO uint32_t CLKT;		///< 0x1C, BSC Master Clock Stretch Timeout
} BCM2835_BSC_TypeDef;

typedef struct {
	__I uint32_t IRQ_BASIC_PENDING;		///< 0x00
	__I uint32_t IRQ_PENDING1;			///< 0x04
	__I uint32_t IRQ_PENDING2;			///< 0x08
	__IO uint32_t FIQ_CONTROL;			///< 0x0C
	__IO uint32_t IRQ_ENABLE1;			///< 0x10
	__IO uint32_t IRQ_ENABLE2;			///< 0x14
	__IO uint32_t IRQ_BASIC_ENABLE;		///< 0x18
	__IO uint32_t IRQ_DISABLE1;			///< 0x1C
	__IO uint32_t IRQ_DISABLE2;			///< 0x20
	__IO uint32_t IRQ_BASIC_DISABLE;	///< 0x24
} BCM2835_IRQ_TypeDef;

typedef struct {
	__I uint32_t READ;		///< 0x00
	__I uint32_t RES1;		///< 0x04
	__I uint32_t RES2;		///< 0x08
	__I uint32_t RES3;		///< 0x0C
	__I uint32_t PEEK;		///< 0x10
	__I uint32_t SENDER;	///< 0x14
	__IO uint32_t STATUS;	///< 0x18
	__I uint32_t CONFIG;	///< 0x1C
	__O uint32_t WRITE;		///< 0x20
} BCM2835_MAILBOX_TypeDef;

typedef struct {
	__I uint32_t UNKWOWN0[7];	///< 0x00
	__IO uint32_t RSTC;			///< 0x1C
	__I uint32_t UNKWOWN1;		///< 0x20
	__IO uint32_t WDOG;			///< 0x24
} BCM2835_PM_WDOG_TypeDef;

#define BCM2835_ST					((BCM2835_ST_TypeDef *)   BCM2835_ST_BASE)
#define BCM2835_IRQ					((BCM2835_IRQ_TypeDef *)  BCM2835_IRQ_BASE)
#define BCM2835_MAILBOX				((BCM2835_MAILBOX_TypeDef *) BCM2835_MAILBOX_BASE)
#define BCM2835_PM_WDOG				((BCM2835_PM_WDOG_TypeDef *) BCM2835_PM_WDOG_BASE)
#define BCM2835_GPIO				((BCM2835_GPIO_TypeDef *) BCM2835_GPIO_BASE)
#define BCM2835_SPI0				((BCM2835_SPI_TypeDef *)  BCM2835_SPI0_BASE)
#define BCM2835_PL011				((BCM2835_PL011_TypeDef *) BCM2835_PL011_BASE)
#define BCM2835_UART1				((BCM2835_UART_TypeDef *) BCM2835_UART1_BASE)
#define BCM2835_BSC1				((BCM2835_BSC_TypeDef *)  BCM2835_BSC1_BASE)
#define BCM2835_BSC2				((BCM2835_BSC_TypeDef *)  BCM2835_BSC2_BASE)

#define dmb() asm volatile ("mcr p15, #0, %[zero], c7, c10, #5" : : [zero] "r" (0) )
#define dsb() asm volatile ("mcr p15, #0, %[zero], c7, c10, #4" : : [zero] "r" (0) )

inline static int bcm2835_init(void) {
	return 1;
}

inline static int bcm2835_close(void) {
	return 1;
}

#define bcm2835_st_read()			*(volatile uint64_t *)(BCM2835_ST_BASE + 0x04)

#endif

#endif /* BCM2835_H_ */
