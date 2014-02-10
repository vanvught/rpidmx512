#ifndef BW_DIO_H_
#define BW_DIO_H_

#define BW_DIO_DEFAULT_SLAVE_ADDRESS		0x84

#define BW_DIO_ID_STRING_LENGTH				20

typedef enum
{
	BW_DIO_PIN_IO0	= 0b00000001,
	BW_DIO_PIN_IO1	= 0b00000010,
	BW_DIO_PIN_IO2	= 0b00000100,
	BW_DIO_PIN_IO3	= 0b00001000,
	BW_DIO_PIN_IO4	= 0b00010000,
	BW_DIO_PIN_IO5	= 0b00100000,
	BW_DIO_PIN_IO6	= 0b01000000
} bw_spi_dio_Pin;

typedef enum
{
	BW_DIO_FSEL_INPT = 0b00,
	BW_DIO_FSEL_OUTP = 0b01
} bw_spi_dio_FunctionSelect;

#endif /* BW_DIO_H_ */
