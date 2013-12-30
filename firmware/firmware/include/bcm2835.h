#ifndef BCM2835_H_
#define BCM2835_H_

#define HIGH 0x1
#define LOW  0x0

#define BCM2835_SPI0_CS_LEN_LONG             0x02000000 ///< Enable Long data word in Lossi mode if DMA_LEN is set
#define BCM2835_SPI0_CS_DMA_LEN              0x01000000 ///< Enable DMA mode in Lossi mode
#define BCM2835_SPI0_CS_CSPOL2               0x00800000 ///< Chip Select 2 Polarity
#define BCM2835_SPI0_CS_CSPOL1               0x00400000 ///< Chip Select 1 Polarity
#define BCM2835_SPI0_CS_CSPOL0               0x00200000 ///< Chip Select 0 Polarity
#define BCM2835_SPI0_CS_RXF                  0x00100000 ///< RXF - RX FIFO Full
#define BCM2835_SPI0_CS_RXR                  0x00080000 ///< RXR RX FIFO needs Reading ( full)
#define BCM2835_SPI0_CS_TXD                  0x00040000 ///< TXD TX FIFO can accept Data
#define BCM2835_SPI0_CS_RXD                  0x00020000 ///< RXD RX FIFO contains Data
#define BCM2835_SPI0_CS_DONE                 0x00010000 ///< Done transfer Done
#define BCM2835_SPI0_CS_TE_EN                0x00008000 ///< Unused
#define BCM2835_SPI0_CS_LMONO                0x00004000 ///< Unused
#define BCM2835_SPI0_CS_LEN                  0x00002000 ///< LEN LoSSI enable
#define BCM2835_SPI0_CS_REN                  0x00001000 ///< REN Read Enable
#define BCM2835_SPI0_CS_ADCS                 0x00000800 ///< ADCS Automatically Deassert Chip Select
#define BCM2835_SPI0_CS_INTR                 0x00000400 ///< INTR Interrupt on RXR
#define BCM2835_SPI0_CS_INTD                 0x00000200 ///< INTD Interrupt on Done
#define BCM2835_SPI0_CS_DMAEN                0x00000100 ///< DMAEN DMA Enable
#define BCM2835_SPI0_CS_TA                   0x00000080 ///< Transfer Active
#define BCM2835_SPI0_CS_CSPOL                0x00000040 ///< Chip Select Polarity
#define BCM2835_SPI0_CS_CLEAR                0x00000030 ///< Clear FIFO Clear RX and TX
#define BCM2835_SPI0_CS_CLEAR_RX             0x00000020 ///< Clear FIFO Clear RX
#define BCM2835_SPI0_CS_CLEAR_TX             0x00000010 ///< Clear FIFO Clear TX
#define BCM2835_SPI0_CS_CPOL                 0x00000008 ///< Clock Polarity
#define BCM2835_SPI0_CS_CPHA                 0x00000004 ///< Clock Phase
#define BCM2835_SPI0_CS_CS                   0x00000003 ///< Chip Select

#define BCM2835_BSC_C_I2CEN 		0x00008000 ///< I2C Enable, 0 = disabled, 1 = enabled
#define BCM2835_BSC_C_INTR 			0x00000400 ///< Interrupt on RX
#define BCM2835_BSC_C_INTT 			0x00000200 ///< Interrupt on TX
#define BCM2835_BSC_C_INTD 			0x00000100 ///< Interrupt on DONE
#define BCM2835_BSC_C_ST 			0x00000080 ///< Start transfer, 1 = Start a new transfer
#define BCM2835_BSC_C_CLEAR_1 		0x00000020 ///< Clear FIFO Clear
#define BCM2835_BSC_C_CLEAR_2 		0x00000010 ///< Clear FIFO Clear
#define BCM2835_BSC_C_READ 			0x00000001 ///<	Read transfer

#define BCM2835_BSC_S_CLKT 			0x00000200 ///< Clock stretch timeout
#define BCM2835_BSC_S_ERR 			0x00000100 ///< ACK error
#define BCM2835_BSC_S_RXF 			0x00000080 ///< RXF FIFO full, 0 = FIFO is not full, 1 = FIFO is full
#define BCM2835_BSC_S_TXE 			0x00000040 ///< TXE FIFO full, 0 = FIFO is not full, 1 = FIFO is full
#define BCM2835_BSC_S_RXD 			0x00000020 ///< RXD FIFO contains data
#define BCM2835_BSC_S_TXD 			0x00000010 ///< TXD FIFO can accept data
#define BCM2835_BSC_S_RXR 			0x00000008 ///< RXR FIFO needs reading (full)
#define BCM2835_BSC_S_TXW 			0x00000004 ///< TXW FIFO needs writing (full)
#define BCM2835_BSC_S_DONE 			0x00000002 ///< Transfer DONE
#define BCM2835_BSC_S_TA 			0x00000001 ///< Transfer Active

#define BCM2835_BSC_FIFO_SIZE   				16 ///< BSC FIFO size

#define RPI_GPIO_P1_03         0  ///< Version 1, Pin P1-03
#define RPI_GPIO_P1_05         1  ///< Version 1, Pin P1-05
#define RPI_GPIO_P1_07         4  ///< Version 1, Pin P1-07
#define RPI_GPIO_P1_08        14  ///< Version 1, Pin P1-08, defaults to alt function 0 UART0_TXD
#define RPI_GPIO_P1_10        15  ///< Version 1, Pin P1-10, defaults to alt function 0 UART0_RXD
#define RPI_GPIO_P1_11        17  ///< Version 1, Pin P1-11
#define RPI_GPIO_P1_12        18  ///< Version 1, Pin P1-12
#define RPI_GPIO_P1_13        21  ///< Version 1, Pin P1-13
#define RPI_GPIO_P1_15        22  ///< Version 1, Pin P1-15
#define RPI_GPIO_P1_16        23  ///< Version 1, Pin P1-16
#define RPI_GPIO_P1_18        24  ///< Version 1, Pin P1-18
#define RPI_GPIO_P1_19        10  ///< Version 1, Pin P1-19, MOSI when SPI0 in use
#define RPI_GPIO_P1_21         9  ///< Version 1, Pin P1-21, MISO when SPI0 in use
#define RPI_GPIO_P1_22        25  ///< Version 1, Pin P1-22
#define RPI_GPIO_P1_23        11  ///< Version 1, Pin P1-23, CLK when SPI0 in use
#define RPI_GPIO_P1_24         8  ///< Version 1, Pin P1-24, CE0 when SPI0 in use
#define RPI_GPIO_P1_26         7  ///< Version 1, Pin P1-26, CE1 when SPI0 in use

 // RPi Version 2
#define RPI_V2_GPIO_P1_03      2  ///< Version 2, Pin P1-03
#define RPI_V2_GPIO_P1_05      3  ///< Version 2, Pin P1-05
#define RPI_V2_GPIO_P1_07      4  ///< Version 2, Pin P1-07
#define RPI_V2_GPIO_P1_08     14  ///< Version 2, Pin P1-08, defaults to alt function 0 UART0_TXD
#define RPI_V2_GPIO_P1_10     15  ///< Version 2, Pin P1-10, defaults to alt function 0 UART0_RXD
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

#ifdef __ASSEMBLY__
#define BCM2835_SPI0_FIFO              0x0004 ///< SPI Master TX and RX FIFOs
#define BCM2835_GPSET0                 0x001c ///< GPIO Pin Output Set 0
#define BCM2835_GPCLR0                 0x0028 ///< GPIO Pin Output Clear 0

#define BCM2835_ST_CS 					0x0000 ///< System Timer Control/Status
#define BCM2835_ST_CLO 					0x0004 ///< System Timer Counter Lower 32 bits
#define BCM2835_ST_CHI 					0x0008 ///< System Timer Counter Upper 32 bits
#define BCM2835_ST_C1					0x0010 ///< System Timer
#else
#include <stdint.h>

#define PUT32(x,y)	*(volatile uint32_t *)(x) = (y)
#define GET32(x)	*(volatile uint32_t *)(x)

typedef enum
{
    BCM2835_GPIO_FSEL_INPT  = 0b000,   ///< Input
    BCM2835_GPIO_FSEL_OUTP  = 0b001,   ///< Output
    BCM2835_GPIO_FSEL_ALT0  = 0b100,   ///< Alternate function 0
    BCM2835_GPIO_FSEL_ALT1  = 0b101,   ///< Alternate function 1
    BCM2835_GPIO_FSEL_ALT2  = 0b110,   ///< Alternate function 2
    BCM2835_GPIO_FSEL_ALT3  = 0b111,   ///< Alternate function 3
    BCM2835_GPIO_FSEL_ALT4  = 0b011,   ///< Alternate function 4
    BCM2835_GPIO_FSEL_ALT5  = 0b010,   ///< Alternate function 5
    BCM2835_GPIO_FSEL_MASK  = 0b111    ///< Function select bits mask
} bcm2835FunctionSelect;

typedef enum
{
    BCM2835_GPIO_PUD_OFF     = 0b00,   ///< Off ? disable pull-up/down
    BCM2835_GPIO_PUD_DOWN    = 0b01,   ///< Enable Pull Down control
    BCM2835_GPIO_PUD_UP      = 0b10    ///< Enable Pull Up control
} bcm2835PUDControl;

typedef enum
{
    BCM2835_SPI_BIT_ORDER_LSBFIRST = 0,  ///< LSB First
    BCM2835_SPI_BIT_ORDER_MSBFIRST = 1   ///< MSB First
}bcm2835SPIBitOrder;

typedef enum
{
    BCM2835_SPI_MODE0 = 0,  ///< CPOL = 0, CPHA = 0
    BCM2835_SPI_MODE1 = 1,  ///< CPOL = 0, CPHA = 1
    BCM2835_SPI_MODE2 = 2,  ///< CPOL = 1, CPHA = 0
    BCM2835_SPI_MODE3 = 3,  ///< CPOL = 1, CPHA = 1
}bcm2835SPIMode;

typedef enum
{
    BCM2835_SPI_CS0 = 0,     ///< Chip Select 0
    BCM2835_SPI_CS1 = 1,     ///< Chip Select 1
    BCM2835_SPI_CS2 = 2,     ///< Chip Select 2 (ie pins CS1 and CS2 are asserted)
    BCM2835_SPI_CS_NONE = 3, ///< No CS, control it yourself
} bcm2835SPIChipSelect;

typedef enum
{
    BCM2835_SPI_CLOCK_DIVIDER_65536 = 0,       ///< 65536 = 262.144us = 3.814697260kHz
    BCM2835_SPI_CLOCK_DIVIDER_32768 = 32768,   ///< 32768 = 131.072us = 7.629394531kHz
    BCM2835_SPI_CLOCK_DIVIDER_16384 = 16384,   ///< 16384 = 65.536us = 15.25878906kHz
    BCM2835_SPI_CLOCK_DIVIDER_8192  = 8192,    ///< 8192 = 32.768us = 30/51757813kHz
    BCM2835_SPI_CLOCK_DIVIDER_4096  = 4096,    ///< 4096 = 16.384us = 61.03515625kHz
    BCM2835_SPI_CLOCK_DIVIDER_2048  = 2048,    ///< 2048 = 8.192us = 122.0703125kHz
    BCM2835_SPI_CLOCK_DIVIDER_1024  = 1024,    ///< 1024 = 4.096us = 244.140625kHz
    BCM2835_SPI_CLOCK_DIVIDER_512   = 512,     ///< 512 = 2.048us = 488.28125kHz
    BCM2835_SPI_CLOCK_DIVIDER_256   = 256,     ///< 256 = 1.024us = 976.5625MHz
    BCM2835_SPI_CLOCK_DIVIDER_128   = 128,     ///< 128 = 512ns = = 1.953125MHz
    BCM2835_SPI_CLOCK_DIVIDER_64    = 64,      ///< 64 = 256ns = 3.90625MHz
    BCM2835_SPI_CLOCK_DIVIDER_32    = 32,      ///< 32 = 128ns = 7.8125MHz
    BCM2835_SPI_CLOCK_DIVIDER_16    = 16,      ///< 16 = 64ns = 15.625MHz
    BCM2835_SPI_CLOCK_DIVIDER_8     = 8,       ///< 8 = 32ns = 31.25MHz
    BCM2835_SPI_CLOCK_DIVIDER_4     = 4,       ///< 4 = 16ns = 62.5MHz
    BCM2835_SPI_CLOCK_DIVIDER_2     = 2,       ///< 2 = 8ns = 125MHz, fastest you can get
    BCM2835_SPI_CLOCK_DIVIDER_1     = 1,       ///< 0 = 262.144us = 3.814697260kHz, same as 0/65536
} bcm2835SPIClockDivider;

typedef enum
{
    BCM2835_I2C_CLOCK_DIVIDER_2500   = 2500,      ///< 2500 = 10us = 100 kHz
    BCM2835_I2C_CLOCK_DIVIDER_626    = 626,       ///< 622 = 2.504us = 399.3610 kHz
    BCM2835_I2C_CLOCK_DIVIDER_150    = 150,       ///< 150 = 60ns = 1.666 MHz (default at reset)
    BCM2835_I2C_CLOCK_DIVIDER_148    = 148,       ///< 148 = 59ns = 1.689 MHz
} bcm2835I2CClockDivider;

typedef enum
{
    BCM2835_I2C_REASON_OK   		 = 0x00,      ///< Success
    BCM2835_I2C_REASON_ERROR_NACK    = 0x01,      ///< Received a NACK
    BCM2835_I2C_REASON_ERROR_CLKT    = 0x02,      ///< Received Clock Stretch Timeout
    BCM2835_I2C_REASON_ERROR_DATA    = 0x04,      ///< Not all data is sent / received
} bcm2835I2CReasonCodes;


extern int bcm2835_init(void);
extern int bcm2835_close(void);

extern void inline bcm2835_delayMicroseconds(unsigned long long);

//extern void bcm2835_peri_set_bits(uint32_t, uint32_t, uint32_t);

extern void bcm2835_gpio_fsel(uint8_t, uint8_t);
extern void inline bcm2835_gpio_set(uint8_t);
extern void inline bcm2835_gpio_clr(uint8_t);
extern void inline bcm2835_gpio_write(uint8_t, uint8_t);
extern uint8_t inline bcm2835_gpio_lev(uint8_t pin);

extern void bcm2835_spi_begin(void);
extern void bcm2835_spi_end(void);
extern void bcm2835_spi_setBitOrder(uint8_t);
extern void inline bcm2835_spi_setClockDivider(uint16_t);
extern void bcm2835_spi_setDataMode(uint8_t);
extern void inline bcm2835_spi_chipSelect(uint8_t);
extern void bcm2835_spi_setChipSelectPolarity(uint8_t, uint8_t);
extern void bcm2835_spi_transfernb(char*, char*, uint32_t);
extern void bcm2835_spi_transfern(char* buf, uint32_t len);
extern void bcm2835_spi_writenb(char* tbuf, uint32_t len);
extern void inline bcm2835_spi_write(uint16_t data);

extern void bcm2835_i2c_begin(void);
extern void bcm2835_i2c_end(void);
extern void bcm2835_i2c_setSlaveAddress(uint8_t);
extern void bcm2835_i2c_setClockDivider(uint16_t );
extern uint8_t bcm2835_i2c_write(const char *, uint32_t);
extern uint8_t bcm2835_i2c_read(char*, uint32_t);

uint64_t bcm2835_st_read(void);
void bcm2835_st_delay(uint64_t offset_micros, uint64_t micros);

extern void bcm2835_uart_begin(void);
extern void inline bcm2835_uart_send(uint32_t);

extern void bcm2835_spi_write(uint16_t data);
extern void dac_write_all(uint16_t x, uint16_t y, uint8_t intensity, uint8_t red, uint8_t green, uint8_t blue);

// TODO
// https://github.com/raspberrypi/linux/blob/rpi-3.6.y/arch/arm/mach-bcm2708/include/mach/platform.h
#define ARM_IRQ1_BASE 		0
#define INTERRUPT_TIMER1 	(ARM_IRQ1_BASE + 1)
#define INTERRUPT_TIMER3 	(ARM_IRQ1_BASE + 3)
#define INTERRUPT_AUX 		(ARM_IRQ1_BASE + 29)

typedef enum
{
  BCM2835_TIMER1_IRQn		= 1 << INTERRUPT_TIMER1,
  BCM2835_TIMER3_IRQn		= 1 << INTERRUPT_TIMER3,
  BCM2835_UART1_IRQn		= 1 << INTERRUPT_AUX,
} BCM2835_IRQn_TypeDef;

#ifdef __cplusplus
#define     __I     volatile                /*!< defines 'read only' permissions      */
#else
#define     __I     volatile const          /*!< defines 'read only' permissions      */
#endif
#define     __O     volatile                  /*!< defines 'write only' permissions     */
#define     __IO    volatile                  /*!< defines 'read / write' permissions   */

typedef struct {
	__IO uint32_t CS;		// 0x00
	__IO uint32_t CLO;		// 0x04
	__IO uint32_t CHI;		// 0x08
	__IO uint32_t C0;		// 0x0C used by GPU
	__IO uint32_t C1;		// 0x10
	__IO uint32_t C2;		// 0x14	used by GPU
	__IO uint32_t C3;		// 0x18
} BCM2835_ST_TypeDef;

typedef struct {
	__I uint32_t IRQ;      	// 0x00
	__IO uint32_t ENABLE;   // 0x04
	__IO uint32_t PAD[14];  // 0x08
	__IO uint32_t IO;       // 0x40
	__IO uint32_t IER;      // 0x44
	__IO uint32_t IIR;      // 0x48
	__IO uint32_t LCR;      // 0x4C
	__IO uint32_t MCR;      // 0x50
	__IO uint32_t LSR;      // 0x54
	__IO uint32_t MSR;      // 0x58
	__IO uint32_t SCR;      // 0x5C
	__IO uint32_t CNTL;     // 0x60
	__I uint32_t STAT;     	// 0x64
	__IO uint32_t BAUD;     // 0x68
} BCM2835_UART_TypeDef;

typedef struct {
	__IO uint32_t GPFSEL0; 	// 0x00
	__IO uint32_t GPFSEL1; 	// 0x04
	__IO uint32_t GPFSEL2; 	// 0x08
	__IO uint32_t GPFSEL3; 	// 0x0C
	__IO uint32_t GPFSEL4; 	// 0x10
	__IO uint32_t GPFSEL5; 	// 0x14
	__IO uint32_t RES1;   	// 0x18
	__O uint32_t GPSET0; 	// 0x1C
	__O uint32_t GPSET1; 	// 0x20
	__IO uint32_t RES2;		// 0x24
	__O uint32_t GPCLR0; 	// 0x28
	__O uint32_t GPCLR1; 	// 0x2C
	__IO uint32_t RES3;   	// 0x30
	__I uint32_t GPLEV0;	// 0x34
	__I uint32_t GPLEV1;	// 0x38
	__IO uint32_t RES4;
	__IO uint32_t EDS[2];
	__IO uint32_t RES5;
	__IO uint32_t REN[2];
	__IO uint32_t RES6;
	__IO uint32_t FEN[2];
	__IO uint32_t RES7;
	__IO uint32_t HEN[2];
	__IO uint32_t RES8;
	__IO uint32_t LEN[2];   // 0x70
	__IO uint32_t RES9;
	__IO uint32_t AREN[2];
	__IO uint32_t RES10;
	__IO uint32_t AFEN[2];  // 0x88
	__IO uint32_t RES11;
	__IO uint32_t GPPUD; 	// 0x94
	__IO uint32_t GPPUDCLK0;// 0x98
	__IO uint32_t GPPUDCLK1;// 0x9C
} BCM2835_GPIO_TypeDef;

typedef struct {
	__IO uint32_t CS;		// 0x00
	__IO uint32_t FIFO;		// 0x04
	__IO uint32_t CLK;		// 0x08
	__IO uint32_t DLEN;		// 0x0C
	__IO uint32_t LTOH;		// 0x10
	__IO uint32_t DC;		// 0x14
} BCM2835_SPI_TypeDef;

typedef struct {
	__IO uint32_t C;		// 0x00
	__IO uint32_t S;		// 0x04
	__IO uint32_t DLEN;		// 0x08
	__IO uint32_t A;		// 0x0C
	__IO uint32_t FIFO;		// 0x10
	__IO uint32_t DIV;		// 0x14
	__IO uint32_t DEL;		// 0x18
	__IO uint32_t CLKT;		// 0x1C
} BCM2835_BSC_TypeDef;

typedef struct {
  __I uint32_t IRQ_BASIC_PENDING;// 0x00
  __I uint32_t IRQ_PENDING1;	// 0x04
  __I uint32_t IRQ_PENDING2;	// 0x08
  __IO uint32_t FIQ_CONTROL;	// 0x0C
  __IO uint32_t IRQ_ENABLE1;	// 0x10
  __IO uint32_t IRQ_ENABLE2;	// 0x14
  __IO uint32_t IRQ_BASIC_ENABLE;// 0x18
  __IO uint32_t IRQ_DISABLE1;	// 0x1C
  __IO uint32_t IRQ_DISABLE2;	// 0x20
  __IO uint32_t IRQ_BASIC_DISABLE;// 0x24
} BCM2835_IRQ_TypeDef;

#endif

#define BCM2835_PERI_BASE      		0x20000000
#define BCM2835_ST_BASE				(BCM2835_PERI_BASE + 0x3000)
#define BCM2835_IRQ_BASE			(BCM2835_PERI_BASE + 0xB200)
#define BCM2835_GPIO_BASE      		(BCM2835_PERI_BASE + 0x200000)
#define BCM2835_SPI0_BASE          	(BCM2835_PERI_BASE + 0x204000)
#define BCM2835_UART1_BASE			(BCM2835_PERI_BASE + 0x215000)
#define BCM2835_BSC1_BASE			(BCM2835_PERI_BASE + 0x804000)
#define BCM2835_BSC2_BASE			(BCM2835_PERI_BASE + 0x805000)

#define BCM2835_ST					((BCM2835_ST_TypeDef *)   BCM2835_ST_BASE)
#define BCM2835_IRQ					((BCM2835_IRQ_TypeDef *)  BCM2835_IRQ_BASE)
#define BCM2835_GPIO 				((BCM2835_GPIO_TypeDef *) BCM2835_GPIO_BASE)
#define BCM2835_SPI0 				((BCM2835_SPI_TypeDef *)  BCM2835_SPI0_BASE)
#define BCM2835_UART1 				((BCM2835_UART_TypeDef *) BCM2835_UART1_BASE)
#define BCM2835_BSC1 				((BCM2835_BSC_TypeDef *)  BCM2835_BSC1_BASE)
#define BCM2835_BSC2 				((BCM2835_BSC_TypeDef *)  BCM2835_BSC2_BASE)

#endif /* BCM2835_H_ */
