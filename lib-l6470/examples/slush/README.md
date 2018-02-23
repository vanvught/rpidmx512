SlushEngine: Model X LT Stepper Motor Driver Examples
==========

[https://roboteurs.com/products/slushengine](https://roboteurs.com/products/slushengine)


Raspberry Pi 

- Prerequisite: [C library for Broadcom BCM 2835 as used in Raspberry Pi](http://www.airspayce.com/mikem/bcm2835/)

Compile and build the examples on Linux Raspbian

	pi@raspberrypi-3:/development/workspace/lib-l6470/examples/slush $ make
	cd ./../../../lib-l6470 && make -f Makefile.Linux "DEF=-DRASPPI"
	make[1]: Entering directory '/development/workspace/lib-l6470'
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/slushboard.cpp -c -o build_linux/slushboard.o
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/l6470support.cpp -c -o build_linux/l6470support.o
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/autodriver.cpp -c -o build_linux/autodriver.o
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/l6470config.cpp -c -o build_linux/l6470config.o
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/slushtemp.cpp -c -o build_linux/slushtemp.o
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/l6470dump.cpp -c -o build_linux/l6470dump.o
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/l6470commands.cpp -c -o build_linux/l6470commands.o
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/l6470.cpp -c -o build_linux/l6470.o
	g++ -DRASPPI -DNDEBUG  -I./include -I../lib-bcm2835_raspbian/include -Wall -Werror -O3 -fno-rtti -std=c++11 src/slushmotor.cpp -c -o build_linux/slushmotor.o
	ar -r lib_linux/libl6470.a   build_linux/slushboard.o build_linux/l6470support.o build_linux/autodriver.o build_linux/l6470config.o build_linux/slushtemp.o build_linux/l6470dump.o build_linux/l6470commands.o build_linux/l6470.o build_linux/slushmotor.o
	ar: creating lib_linux/libl6470.a
	objdump -D lib_linux/libl6470.a  | c++filt > lib_linux.list
	make[1]: Leaving directory '/development/workspace/lib-l6470'
	g++ input_output.cpp -I./../../../lib-l6470/include -Wall -Werror -O3 -fno-rtti -std=c++11 -DNDEBUG -o input_output -L./../../../lib-l6470/lib_linux -ll6470  -lbcm2835
	g++ limit_switch.cpp -I./../../../lib-l6470/include -Wall -Werror -O3 -fno-rtti -std=c++11 -DNDEBUG -o limit_switch -L./../../../lib-l6470/lib_linux -ll6470  -lbcm2835
	g++ precise_move.cpp -I./../../../lib-l6470/include -Wall -Werror -O3 -fno-rtti -std=c++11 -DNDEBUG -o precise_move -L./../../../lib-l6470/lib_linux -ll6470  -lbcm2835
	g++ simple_move.cpp -I./../../../lib-l6470/include -Wall -Werror -O3 -fno-rtti -std=c++11 -DNDEBUG -o simple_move -L./../../../lib-l6470/lib_linux -ll6470  -lbcm2835
	g++ get_set_param_test.cpp -I./../../../lib-l6470/include -Wall -Werror -O3 -fno-rtti -std=c++11 -DNDEBUG -o get_set_param_test -L./../../../lib-l6470/lib_linux -ll6470  -lbcm2835
	g++ get_temp.cpp -I./../../../lib-l6470/include -Wall -Werror -O3 -fno-rtti -std=c++11 -DNDEBUG -o get_temp -L./../../../lib-l6470/lib_linux -ll6470  -lbcm2835 -lm

![](https://cdn.shopify.com/s/files/1/0742/2899/products/SlushEngineLT-white_grande.png?v=1487710515)

[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)
