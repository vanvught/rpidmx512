## Open source Raspberry Pi C++ library for 12-Channel 16-bit PWM LED Driver TLC59711 ##

Successfully tested with :

- Adafruit 12-Channel 16-bit PWM LED Driver - SPI Interface - TLC59711 ([https://www.adafruit.com/product/1455](https://www.adafruit.com/product/1455 "12-Channel 16-bit PWM LED Driver - SPI Interface - TLC59711"))


Supported platforms :

- Linux Raspbian
    -  Prerequisite: [C library for Broadcom BCM 2835 as used in Raspberry Pi](http://www.airspayce.com/mikem/bcm2835/)
- Bare-metal
    - Prerequisite: [https://github.com/vanvught/rpidmx512/tree/master/lib-bcm2835](https://github.com/vanvught/rpidmx512/tree/master/lib-bcm2835)

Compile and build the library on Linux Raspbian

	pi@raspberrypi-3:/development/workspace/lib-tlc59711 $ make -f Makefile.Linux "DEF=-DRASPPI"
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/tlc59711.cpp -c -o build_linux/tlc59711.o
	ar -r lib_linux/libtlc59711.a   build_linux/tlc59711.o
	ar: creating lib_linux/libtlc59711.a
	objdump -D lib_linux/libtlc59711.a  | c++filt > lib_linux.list

Compile and build the examples on Linux Raspbian

	pi@raspberrypi-3:/development/workspace/lib-tlc59711/examples $ make
	cd ./../../lib-tlc59711 && make -f Makefile.Linux "DEF=-DRASPPI"
	make[1]: Entering directory '/development/workspace/lib-tlc59711'
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/tlc59711.cpp -c -o build_linux/tlc59711.o
	ar -r lib_linux/libtlc59711.a   build_linux/tlc59711.o
	ar: creating lib_linux/libtlc59711.a
	objdump -D lib_linux/libtlc59711.a  | c++filt > lib_linux.list
	make[1]: Leaving directory '/development/workspace/lib-tlc59711'
	g++ pwmled.cpp -I./../../lib-tlc59711/include -Wall -Werror -O3 -fno-rtti -std=c++11 -DNDEBUG -o pwmled -L./../../lib-tlc59711/lib_linux -ltlc59711  -lbcm2835
	objdump -D pwmled | c++filt > pwmled.lst


![](https://cdn-shop.adafruit.com/1200x900/1455-00.jpg)

[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

