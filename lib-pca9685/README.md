## Open source Raspberry Pi C++ library for 16-Channel 12-bit PWM/Servo Driver PCA9685 ##

Successfully tested with :

- Adafruit 16-Channel PWM / Servo HAT for Raspberry Pi - Mini Kit ([https://www.adafruit.com/product/2327](https://www.adafruit.com/product/2327 "Adafruit 16-Channel PWM / Servo HAT for Raspberry Pi - Mini Kit"))
- Adafruit 16-Channel 12-bit PWM/Servo Driver - I2C interface - PCA9685 ([https://www.adafruit.com/product/815](https://www.adafruit.com/product/815 "Adafruit 16-Channel 12-bit PWM/Servo Driver - I2C interface - PCA9685"))

Supported platforms :

- Linux Raspbian
    -  Prerequisite: [C library for Broadcom BCM 2835 as used in Raspberry Pi](http://www.airspayce.com/mikem/bcm2835/)
- Bare-metal
    - Prerequisite: [https://github.com/vanvught/rpidmx512/tree/master/lib-bcm2835](https://github.com/vanvught/rpidmx512/tree/master/lib-bcm2835)

Compile and build the library on Linux Raspbian

	pi@raspberrypi-3:/development/workspace/lib-pca9685 $ make -f Makefile.Linux "DEF=-DRASPPI"
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/pca9685servo.cpp -c -o build_linux/pca9685servo.o
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/pca9685pwmled.cpp -c -o build_linux/pca9685pwmled.o
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/pca9685.cpp -c -o build_linux/pca9685.o
	ar -r lib_linux/libpca9685.a   build_linux/pca9685servo.o build_linux/pca9685pwmled.o build_linux/pca9685.o
	ar: creating lib_linux/libpca9685.a
	objdump -D lib_linux/libpca9685.a  | c++filt > lib_linux.list

Compile and build the examples on Linux Raspbian

	pi@raspberrypi-3:/development/workspace/lib-pca9685/examples $ make
	cd ./../../lib-pca9685 && make -f Makefile.Linux "DEF=-DRASPPI"
	make[1]: Entering directory '/development/workspace/lib-pca9685'
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/pca9685servo.cpp -c -o build_linux/pca9685servo.o
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/pca9685pwmled.cpp -c -o build_linux/pca9685pwmled.o
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/pca9685.cpp -c -o build_linux/pca9685.o
	ar -r lib_linux/libpca9685.a   build_linux/pca9685servo.o build_linux/pca9685pwmled.o build_linux/pca9685.o
	ar: creating lib_linux/libpca9685.a
	objdump -D lib_linux/libpca9685.a  | c++filt > lib_linux.list
	make[1]: Leaving directory '/development/workspace/lib-pca9685'
	g++ simple.cpp -I./../../lib-pca9685/include -Wall -Werror -O3 -fno-rtti -std=c++11 -DNDEBUG -o simple -L./../../lib-pca9685/lib_linux -lpca9685  -lbcm2835
	objdump -D simple | c++filt > simple.lst
	g++ pwmled.cpp -I./../../lib-pca9685/include -Wall -Werror -O3 -fno-rtti -std=c++11 -DNDEBUG -o pwmled -L./../../lib-pca9685/lib_linux -lpca9685  -lbcm2835
	objdump -D pwmled | c++filt > pwmled.lst
	g++ servo.cpp -I./../../lib-pca9685/include -Wall -Werror -O3 -fno-rtti -std=c++11 -DNDEBUG -o servo -L./../../lib-pca9685/lib_linux -lpca9685  -lbcm2835
	objdump -D servo | c++filt > servo.lst


![](https://cdn-shop.adafruit.com/970x728/2327-12.jpg)
![](https://cdn-shop.adafruit.com/970x728/815-04.jpg)

[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

