## Open source Raspberry Pi C library for I2C ##

This library provides an abstraction layer for the I2C interface. 

<table>
<tr>
<td colspan="2" style="text-align:center;">lib-i2c <a href="https://github.com/vanvught/rpidmx512/tree/master/lib-i2c">https://github.com/vanvught/rpidmx512/tree/master/lib-i2c</a> (-li2c)</td>
</tr>
<tr>
<td>bare-metal<br><a href="https://github.com/vanvught/rpidmx512/tree/master/lib-bcm2835">https://github.com/vanvught/rpidmx512/tree/master/lib-bcm2835</a></br></td>
<td>Raspbian Linux<br><a href="http://www.airspayce.com/mikem/bcm2835/">http://www.airspayce.com/mikem/bcm2835/</a></br></td>
</tr>
</table>
 
**General** functions :

	void i2c_begin(void)
	void i2c_set_address(uint8_t address)
	void i2c_set_clockdivider(uint16_t divider)
	bool i2c_is_connected(uint8_t address)

**I2C read** functions :

	uint8_t i2c_read_uint8(void)
	uint16_t i2c_read_uint16(void)
	uint16_t i2c_read_reg_uint16(uint8_t reg)
	uint16_t i2c_read_reg_uint16_delayus(uint8_t reg, uint32_t delayus)

**I2C write** functions :

	void i2c_write_nb(const char *data, uint32_t length)
	void i2c_write(uint8_t data)
	void i2c_write_uint16(uint16_t data);
	void i2c_write_reg_uint8(uint8_t reg, uint8_t data)
	void i2c_write_reg_uint16(uint8_t reg, uint16_t data);
	void i2c_write_reg_uint16_mask(uint8_t reg, uint16_t data, uint16_t mask);

Compile and build the library on Linux Raspbian
	
	pi@raspberrypi-3:/development/workspace/lib-i2c $ make -f Makefile.Linux "DEF=-DRASPPI"
	gcc -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 src/i2c_write.c -c -o build_linux/i2c_write.o
	gcc -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 src/i2c_is_connected.c -c -o build_linux/i2c_is_connected.o
	gcc -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 src/i2c_begin.c -c -o build_linux/i2c_begin.o
	gcc -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 src/i2c_lookup_device.c -c -o build_linux/i2c_lookup_device.o
	gcc -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 src/i2c_set.c -c -o build_linux/i2c_set.o
	gcc -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 src/i2c_read.c -c -o build_linux/i2c_read.o
	ar -r lib_linux/libi2c.a  build_linux/i2c_write.o build_linux/i2c_is_connected.o build_linux/i2c_begin.o build_linux/i2c_lookup_device.o build_linux/i2c_set.o build_linux/i2c_read.o
	ar: creating lib_linux/libi2c.a
	objdump -D lib_linux/libi2c.a  | c++filt > lib_linux.list

[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

