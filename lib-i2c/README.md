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
	void i2c_set_address(const uint8_t address)
	void i2c_set_clockdivider(const uint16_t divider)
	bool i2c_is_connected(const uint8_t address)

**I2C read** functions :

	uint8_t i2c_read_uint8(void)
	uint16_t i2c_read_uint16(void)
	uint16_t i2c_read_reg_uint16(const uint8_t reg)
	uint16_t i2c_read_reg_uint16_delayus(const uint8_t reg, const uint32_t delayus)

**I2C write** functions :

	void i2c_write_nb(const char *data, const uint32_t length)
	void i2c_write(const uint8_t data)
	void i2c_write_reg_uint8(const uint8_t reg, const uint8_t data)


[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

