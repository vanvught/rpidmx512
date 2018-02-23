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

Compile and build on Linux Raspbian
	
	pi@raspberrypi-3:/development/workspace/lib-i2c/tools $ make
	cd ./../../lib-i2c && make -f Makefile.Linux "DEF=-DRASPPI"
	make[1]: Entering directory '/development/workspace/lib-i2c'
	gcc -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 src/i2c_write.c -c -o build_linux/i2c_write.o
	gcc -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 src/i2c_is_connected.c -c -o build_linux/i2c_is_connected.o
	gcc -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 src/i2c_begin.c -c -o build_linux/i2c_begin.o
	gcc -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 src/i2c_lookup_device.c -c -o build_linux/i2c_lookup_device.o
	gcc -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 src/i2c_set.c -c -o build_linux/i2c_set.o
	gcc -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 src/i2c_read.c -c -o build_linux/i2c_read.o
	ar -r lib_linux/libi2c.a  build_linux/i2c_write.o build_linux/i2c_is_connected.o build_linux/i2c_begin.o build_linux/i2c_lookup_device.o build_linux/i2c_set.o build_linux/i2c_read.o
	ar: creating lib_linux/libi2c.a
	objdump -D lib_linux/libi2c.a  | c++filt > lib_linux.list
	make[1]: Leaving directory '/development/workspace/lib-i2c'
	gcc i2cdetect.c -I./../../lib-i2c/include -I./../../lib-bcm2835_raspbian/include -Wall -Werror -O3 -DNDEBUG -o i2cdetect -L./../../lib-i2c/lib_linux -L./../../lib-bcm2835_raspbian/lib_linux -li2c -lbcm2835_raspbian
	objdump -D i2cdetect | c++filt > i2cdetect.lst


[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

