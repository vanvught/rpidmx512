#include <stdio.h>
#include <stdint.h>
#include "bcm2835.h"
#include "bcm2835_gpio.h"

// TODO Place the jumper on the left side for 3V3
/*
 * Raspberry Pi
 * P5:Broadcom			FT245RL
 *
 *  3:GPIO02	<--->	D0
 *  5:GPIO03	<--->	D1
 *  7:GPIO04	<--->	D2
 * 26:GPIO07	<--->	D3
 * 24:GPIO08	<--->	D4
 * 21:GPIO09	<--->	D5
 * 19:GPIO10	<--->	D6
 * 23:GPIO11	<--->	D7
 *
 * 15:GPIO22	---->	WR
 * 16:GPIO23	---->	RD#
 *
 * 18:GPIO24	<----	TXE#
 * 22:GPIO25	<----	RXF#
 */

#define WR	22
#define _RD	23

/**
 *
 */
static void data_gpio_fsel_output()
{
	// Data output
	uint32_t value = BCM2835_GPIO->GPFSEL0;
	value &= ~(7 << 6);
	value |= BCM2835_GPIO_FSEL_OUTP << 6;
	value &= ~(7 << 9);
	value |= BCM2835_GPIO_FSEL_OUTP << 9;
	value &= ~(7 << 12);
	value |= BCM2835_GPIO_FSEL_OUTP << 12;
	value &= ~(7 << 21);
	value |= BCM2835_GPIO_FSEL_OUTP << 21;
	value &= ~(7 << 24);
	value |= BCM2835_GPIO_FSEL_OUTP << 24;
	value &= ~(7 << 27);
	value |= BCM2835_GPIO_FSEL_OUTP << 27;
	BCM2835_GPIO->GPFSEL0 = value;
	value = BCM2835_GPIO->GPFSEL1;
	value &= ~(7 << 0);
	value |= BCM2835_GPIO_FSEL_OUTP << 0;
	value &= ~(7 << 3);
	value |= BCM2835_GPIO_FSEL_OUTP << 3;
	BCM2835_GPIO->GPFSEL1 = value;
}

/**
 *
 */
static void data_gpio_fsel_input()
{
	// Data input
	uint32_t value = BCM2835_GPIO->GPFSEL0;
	value &= ~(7 << 6);
	value |= BCM2835_GPIO_FSEL_INPT << 6;
	value &= ~(7 << 9);
	value |= BCM2835_GPIO_FSEL_INPT << 9;
	value &= ~(7 << 12);
	value |= BCM2835_GPIO_FSEL_INPT << 12;
	value &= ~(7 << 21);
	value |= BCM2835_GPIO_FSEL_INPT << 21;
	value &= ~(7 << 24);
	value |= BCM2835_GPIO_FSEL_INPT << 24;
	value &= ~(7 << 27);
	value |= BCM2835_GPIO_FSEL_INPT << 27;
	BCM2835_GPIO->GPFSEL0 = value;
	value = BCM2835_GPIO->GPFSEL1;
	value &= ~(7 << 0);
	value |= BCM2835_GPIO_FSEL_INPT << 0;
	value &= ~(7 << 3);
	value |= BCM2835_GPIO_FSEL_INPT << 3;
	BCM2835_GPIO->GPFSEL1 = value;
}

/**
 *
 */
void FT245RL_init(void)
{
	// RD#, WR output
	uint32_t value = BCM2835_GPIO->GPFSEL2;
	value &= ~(7 << 6);
	value |= BCM2835_GPIO_FSEL_OUTP << 6;
	value &= ~(7 << 9);
	value |= BCM2835_GPIO_FSEL_OUTP << 9;
	// TXE#, RXF# input
	value &= ~(7 << 12);
	value |= BCM2835_GPIO_FSEL_INPT << 12;
	value &= ~(7 << 15);
	value |= BCM2835_GPIO_FSEL_INPT << 15;
	BCM2835_GPIO->GPFSEL2 = value;
	// RD#	high
	bcm2835_gpio_set(_RD);
	// WR	low
	bcm2835_gpio_clr(WR);
}

/**
 *
 * @param data
 */
void FT245RL_write_data(uint8_t data)
{
	data_gpio_fsel_output();
	// Raise WR to start the write.
	bcm2835_gpio_set(WR);
	asm volatile("nop"::);
	// Put the data on the bus.
	uint32_t out_gpio = ((data & ~0b00000111) << 4)	| ((data & 0b00000111) << 2);
	BCM2835_GPIO->GPSET0 = out_gpio;
	BCM2835_GPIO->GPCLR0 = out_gpio ^ 0b111110011100;
	asm volatile("nop"::);
	// Drop WR to tell the FT245 to read the data.
	bcm2835_gpio_clr(WR);
}

/**
 *
 * @return
 */
uint8_t FT245RL_read_data()
{
	data_gpio_fsel_input();
	bcm2835_gpio_clr(_RD);
	// Wait for the FT245 to respond with data.
	asm volatile("nop"::);
	// Read the data from the data port.
	uint32_t in_gpio = (BCM2835_GPIO->GPLEV0 & 0b111110011100) >> 2;
	uint8_t data = (uint8_t) ((in_gpio >> 2) & 0xF8) | (uint8_t) (in_gpio & 0x0F);
	// Bring RD# back up so the FT245 can let go of the data.
	bcm2835_gpio_set(_RD);
	// Wait to prevent false 'no data' readings.
	asm volatile("nop"::);
	return data;
}

/**
 * Read RXF#
 *
 * @return
 */
uint8_t FT245RL_data_available(void)
{
	return (!(BCM2835_GPIO->GPLEV0 & (1 << 25)));
}
