#ifndef BCM2835_H_
#define BCM2835_H_

#include <stdint.h>

#ifndef HIGH
/*! This means pin HIGH, true, 3.3 volts on a pin. */
#define HIGH 0x1
/*! This means pin LOW, false, 0 volts on a pin. */
#define LOW  0x0
#endif

#if RASPPI == 3
#define BCM2835_CORE_CLK_HZ		300000000	///< 300 MHz
#else
#define BCM2835_CORE_CLK_HZ		250000000	///< 250 MHz
#endif

enum TRPiGPIOPin {
    RPI_V2_GPIO_P1_03     =  2,  /*!< Version 2, Pin P1-03 */
    RPI_V2_GPIO_P1_05     =  3,  /*!< Version 2, Pin P1-05 */
    RPI_V2_GPIO_P1_07     =  4,  /*!< Version 2, Pin P1-07 */
    RPI_V2_GPIO_P1_08     = 14,  /*!< Version 2, Pin P1-08, defaults to alt function 0 UART0_TXD */
    RPI_V2_GPIO_P1_10     = 15,  /*!< Version 2, Pin P1-10, defaults to alt function 0 UART0_RXD */
    RPI_V2_GPIO_P1_11     = 17,  /*!< Version 2, Pin P1-11 */
    RPI_V2_GPIO_P1_12     = 18,  /*!< Version 2, Pin P1-12, can be PWM channel 0 in ALT FUN 5 */
    RPI_V2_GPIO_P1_13     = 27,  /*!< Version 2, Pin P1-13 */
    RPI_V2_GPIO_P1_15     = 22,  /*!< Version 2, Pin P1-15 */
    RPI_V2_GPIO_P1_16     = 23,  /*!< Version 2, Pin P1-16 */
    RPI_V2_GPIO_P1_18     = 24,  /*!< Version 2, Pin P1-18 */
    RPI_V2_GPIO_P1_19     = 10,  /*!< Version 2, Pin P1-19, MOSI when SPI0 in use */
    RPI_V2_GPIO_P1_21     =  9,  /*!< Version 2, Pin P1-21, MISO when SPI0 in use */
    RPI_V2_GPIO_P1_22     = 25,  /*!< Version 2, Pin P1-22 */
    RPI_V2_GPIO_P1_23     = 11,  /*!< Version 2, Pin P1-23, CLK when SPI0 in use */
    RPI_V2_GPIO_P1_24     =  8,  /*!< Version 2, Pin P1-24, CE0 when SPI0 in use */
    RPI_V2_GPIO_P1_26     =  7,  /*!< Version 2, Pin P1-26, CE1 when SPI0 in use */
    RPI_V2_GPIO_P1_29     =  5,  /*!< Version 2, Pin P1-29 */
    RPI_V2_GPIO_P1_31     =  6,  /*!< Version 2, Pin P1-31 */
    RPI_V2_GPIO_P1_32     = 12,  /*!< Version 2, Pin P1-32 */
    RPI_V2_GPIO_P1_33     = 13,  /*!< Version 2, Pin P1-33 */
    RPI_V2_GPIO_P1_35     = 19,  /*!< Version 2, Pin P1-35, can be PWM channel 1 in ALT FUN 5  */
    RPI_V2_GPIO_P1_36     = 16,  /*!< Version 2, Pin P1-36 */
    RPI_V2_GPIO_P1_37     = 26,  /*!< Version 2, Pin P1-37 */
    RPI_V2_GPIO_P1_38     = 20,  /*!< Version 2, Pin P1-38 */
    RPI_V2_GPIO_P1_40     = 21,  /*!< Version 2, Pin P1-40 */
};

enum TBcm2835GioFsel {
	BCM2835_GPIO_FSEL_INPT = 0x00,	/*!< Input 0b000 */
	BCM2835_GPIO_FSEL_OUTP = 0x01,	/*!< Output 0b001 */
	BCM2835_GPIO_FSEL_ALT0 = 0x04,	/*!< Alternate function 0 0b100 */
	BCM2835_GPIO_FSEL_ALT1 = 0x05,	/*!< Alternate function 1 0b101 */
	BCM2835_GPIO_FSEL_ALT2 = 0x06,	/*!< Alternate function 2 0b110, */
	BCM2835_GPIO_FSEL_ALT3 = 0x07,	/*!< Alternate function 3 0b111 */
	BCM2835_GPIO_FSEL_ALT4 = 0x03,	/*!< Alternate function 4 0b011 */
	BCM2835_GPIO_FSEL_ALT5 = 0x02,	/*!< Alternate function 5 0b010 */
	BCM2835_GPIO_FSEL_MASK = 0x07	/*!< Function select bits mask 0b111 */
};

enum TBcm2835SPIMode {
	BCM2835_SPI_MODE0 = 0,	/*!< CPOL = 0, CPHA = 0 */
	BCM2835_SPI_MODE1 = 1,	/*!< CPOL = 0, CPHA = 1 */
	BCM2835_SPI_MODE2 = 2,	/*!< CPOL = 1, CPHA = 0 */
	BCM2835_SPI_MODE3 = 3 	/*!< CPOL = 1, CPHA = 1 */
};

enum TBcm2835SPIChipSelect {
	BCM2835_SPI_CS0 = 0,	/*!< Chip Select 0 */
	BCM2835_SPI_CS1 = 1,	/*!< Chip Select 1 */
	BCM2835_SPI_CS2 = 2,	/*!< Chip Select 2 (ie pins CS1 and CS2 are asserted) */
	BCM2835_SPI_CS_NONE = 3	/*!< No CS, control it yourself */
};

enum TBbm2835SPIClockDivider {
	BCM2835_SPI_CLOCK_DIVIDER_65536 = 0,		/*!< 65536 = 3.814697260kHz on Rpi2, 6.1035156kHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_32768 = 32768,	/*!< 32768 = 7.629394531kHz on Rpi2, 12.20703125kHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_16384 = 16384,	/*!< 16384 = 15.25878906kHz on Rpi2, 24.4140625kHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_8192 = 8192,		/*!< 8192 = 30.51757813kHz on Rpi2, 48.828125kHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_4096 = 4096,		/*!< 4096 = 61.03515625kHz on Rpi2, 97.65625kHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_2048 = 2048,		/*!< 2048 = 122.0703125kHz on Rpi2, 195.3125kHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_1024 = 1024,		/*!< 1024 = 244.140625kHz on Rpi2, 390.625kHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_512 = 512,		/*!< 512 = 488.28125kHz on Rpi2, 781.25kHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_256 = 256,		/*!< 256 = 976.5625kHz on Rpi2, 1.5625MHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_128 = 128,		/*!< 128 = 1.953125MHz on Rpi2, 3.125MHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_64 = 64,			/*!< 64 = 3.90625MHz on Rpi2, 6.250MHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_32 = 32,			/*!< 32 = 7.8125MHz on Rpi2, 12.5MHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_16 = 16,			/*!< 16 = 15.625MHz on Rpi2, 25MHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_8 = 8,			/*!< 8 = 31.25MHz on Rpi2, 50MHz on RPI3 */
	BCM2835_SPI_CLOCK_DIVIDER_4 = 4,			/*!< 4 = 62.5MHz on Rpi2, 100MHz on RPI3. Dont expect this speed to work reliably. */
	BCM2835_SPI_CLOCK_DIVIDER_2 = 2,			/*!< 2 = 125MHz on Rpi2, 200MHz on RPI3, fastest you can get. Dont expect this speed to work reliably.*/
	BCM2835_SPI_CLOCK_DIVIDER_1 = 1				/*!< 1 = 3.814697260kHz on Rpi2, 6.1035156kHz on RPI3, same as 0/65536 */
};

enum TBcm2835I2CClockDivider {
	BCM2835_I2C_CLOCK_DIVIDER_2500 = 2500,	/*!< 2500 = 10us = 100 kHz */
	BCM2835_I2C_CLOCK_DIVIDER_626 = 626,	/*!< 622 = 2.504us = 399.3610 kHz */
	BCM2835_I2C_CLOCK_DIVIDER_150 = 150,	/*!< 150 = 60ns = 1.666 MHz (default at reset) */
	BCM2835_I2C_CLOCK_DIVIDER_148 = 148		/*!< 148 = 59ns = 1.689 MHz */
};

enum TBcm2835I2CReasonCodes {
	BCM2835_I2C_REASON_OK = 0x00,			/*!< Success */
	BCM2835_I2C_REASON_ERROR_NACK = 0x01,	/*!< Received a NACK */
	BCM2835_I2C_REASON_ERROR_CLKT = 0x02,	/*!< Received Clock Stretch Timeout */
	BCM2835_I2C_REASON_ERROR_DATA = 0x04	/*!< Not all data is sent / received */
};

#ifdef __cplusplus
extern "C" {
#endif

extern int bcm2835_init(void);

extern void bcm2835_gpio_fsel(uint8_t pin, uint8_t mode);
extern void bcm2835_gpio_set(uint8_t pin);
extern void bcm2835_gpio_clr(uint8_t pin);
extern uint8_t bcm2835_gpio_lev(uint8_t pin);

extern int bcm2835_i2c_begin(void);
extern void bcm2835_i2c_end(void);
extern void bcm2835_i2c_setSlaveAddress(uint8_t addr);
extern void bcm2835_i2c_setClockDivider(uint16_t divider);
extern uint8_t bcm2835_i2c_write(const char * buf, uint32_t len);
extern uint8_t bcm2835_i2c_read(char* buf, uint32_t len);

extern int bcm2835_spi_begin(void);
extern void bcm2835_spi_end(void);
extern void bcm2835_spi_setClockDivider(uint16_t divider);
extern void bcm2835_spi_setDataMode(uint8_t mode);
extern void bcm2835_spi_chipSelect(uint8_t cs);
extern void bcm2835_spi_transfernb(char* tbuf, char* rbuf, uint32_t len);
extern void bcm2835_spi_transfern(char* buf, uint32_t len);

extern void bcm2835_delayMicroseconds (uint64_t micros);

#ifdef __cplusplus
}
#endif

#endif /* BCM2835_H_ */
