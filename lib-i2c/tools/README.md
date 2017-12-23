## I2C Tools ##


i2cdetect is a program to scan an I2C bus for devices. It outputs a table with the list of detected devices. Additional to the standard Linux version, if known, the chip name is also printed.

**Warning**
This program can confuse your I2C bus, cause data loss and worse! 

	sudo ./i2cdetect

    [V1.0] Compiled on Dec 23 2017 at 17:50:46
    
    0x20 : 0x40 : MCP23017 {16-bit I/O port}
    0x3C : 0x78 : SSD1306 {128 x 64 Dot Matrix}
    0x48 : 0x90 : PCF8591 {8-bit A/D and D/A} | ADS1x1x {A/D} | BW:Motor
    0x57 : 0xAE : MCP7941X {EEPROM}
    0x6F : 0xDE : MCP7941X {SRAM RTCC}
    
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    00:  -- -- -- -- -- -- -- -- -- -- -- -- --
    10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    20: 20 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    30: -- -- -- -- -- -- -- -- -- -- -- -- 3c -- -- --
    40: -- -- -- -- -- -- -- -- 48 -- -- -- -- -- -- --
    50: -- -- -- -- -- -- -- 57 -- -- -- -- -- -- -- --
    60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 6f
    70: -- -- -- -- -- -- -- --


[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

