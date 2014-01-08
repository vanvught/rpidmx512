#ifndef MCP23S08_H_
#define MCP23S08_H_

#include <stdint.h>

#define MCP23S08_OK						0
#define MCP23S08_ERROR					1

#define MCP23S08_DEFAULT_SLAVE_ADDRESS	0x00

#define MCP23S08_IODIR					0x00
#define MCP23S08_IPOL					0x01
#define MCP23S08_GPINTEN				0x02
#define MCP23S08_DEFVAL					0x03
#define MCP23S08_INTCON					0x04
#define MCP23S08_IOCON					0x05
#define MCP23S08_GPPU					0x06
#define MCP23S08_INTF					0x07
#define MCP23S08_INTCAP					0x08
#define MCP23S08_GPIO					0x09
#define MCP23S08_OLAT					0x0A

#define MCP23S08_CMD_WRITE				0x40
#define MCP23S08_CMD_READ				0x41

typedef enum
{
	MCP23S08_PIN_GP0		= 0b00000001,
	MCP23S08_PIN_GP1		= 0b00000010,
	MCP23S08_PIN_GP2		= 0b00000100,
	MCP23S08_PIN_GP3		= 0b00001000,
	MCP23S08_PIN_GP4		= 0b00010000,
	MCP23S08_PIN_GP5		= 0b00100000,
	MCP23S08_PIN_GP6		= 0b01000000,
	MCP23S08_PIN_GP7		= 0b10000000
} mcp23s08Pin;

typedef enum
{
	MCP23S08_FSEL_OUTP  	= 0b000,   ///< Output
	MCP23S08_FSEL_INPT  	= 0b001,   ///< Input
} mcp23s08FunctionSelect;

extern int mcp23s08_start(char slave_address);
extern void mcp23s08_end (void);

extern void mcp23s08_gpio_fsel(uint8_t pin, uint8_t mode);
extern void mcp23s08_gpio_set(uint8_t pin);
extern void mcp23s08_gpio_clr(uint8_t pin);

#endif /* MCP23S08_H_ */
