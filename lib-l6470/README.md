## Open source Raspberry Pi C++ library for STMicro L6470 stepper motor driver ##

Supported devices :

- SlushEngine: Model X LT Stepper Motor Driver ([https://roboteurs.com/products/slushengine](https://roboteurs.com/products/slushengine))
    - Compatible with the [SlushEngine Python](https://github.com/Roboteurs/slushengine/tree/master/Slush) software library API
- SparkFun AutoDriver - Stepper Motor Driver ([https://www.sparkfun.com/products/13752](https://www.sparkfun.com/products/13752))
  - Compatible with the [Arduino library API](https://learn.sparkfun.com/tutorials/getting-started-with-the-autodriver---v13?_ga=2.159479294.2137032848.1507653699-1754619782.1457877282#arduino-library---configuration)

Supported platforms :

- Linux Raspbian
    -  Prerequisite: [C library for Broadcom BCM 2835 as used in Raspberry Pi](http://www.airspayce.com/mikem/bcm2835/)
- Bare-metal
    - Prerequisite: [https://github.com/vanvught/rpidmx512/tree/master/lib-bcm2835](https://github.com/vanvught/rpidmx512/tree/master/lib-bcm2835)

Compile and build the library on Linux Raspbian

	pi@raspberrypi-3:/development/workspace/lib-l6470 $ make -f Makefile.Linux "DEF=-DRASPPI"
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

Additional member function for the `L6470` object:

    Dump(void)
Example output at start-up:
> Registers:<br>
> 01:ABS_POS- Current position: 0<br>
> 02:EL_POS - Electrical position: 0<br>
> 03:MARK   - Mark position: 0<br>
> 04:SPEED  - Current speed: 0 (raw)<br>
> 05:ACC- Acceleration: 1004.09 steps/s2<br>
> 06:DEC- Deceleration: 1004.09 steps/s2<br>
> 07:MAX_SPEED  - Maximum speed: 991.82 steps/s<br>
> 08:MIN_SPEED  - Minimum speed: 0.00 steps/s<br>
> 09:KVAL_HOLD  - Holding KVAL: 41<br>
> 0A:KVAL_RUN   - Constant speed KVAL: 41<br>
> 0B:KVAL_ACC   - Acceleration starting KVAL: 41<br>
> 0C:KVAL_Dec   - Deceleration starting KVAL: 41<br>
> 0D:INT_SPEED  - Intersect speed: 0x0408 (raw)<br>
> 0E:ST_SLP - Start slope: 0.038% s/step (0x19)<br>
> 0F:FN_SLP_ACC - Acceleration final slope: 0.062% s/step (0x29)<br>
> 10:FN_SLP_DEC - Deceleration final slope: 0.062% s/step (0x29)<br>
> 11:K_THERM- Thermal compensation factor: 1.000 (0x00)<br>
> 12:ADC_OUT- ADC output: 0x01<br>
> 13:OCD_TH - OCD threshold: 3375 mA (0x08)<br>
> 14:STALL_TH   - STALL threshold: 2031 mA (0x40)<br>
> 15:FS_SPD - Full-step speed: 602.72 steps/s<br>
> 16:STEP_MODE  - Step mode: 128 microsteps<br>
> 17:ALARM_EN   - Alarm enable: 0xFF<br>
> 18:CONFIG - IC configuration: 0x2E88<br>

Additional parameter for the `SlushMotor` object:

    SlushMotor Motor(0, false); // Use /BUSY pin instead of SPI STATUS register

![](https://cdn.shopify.com/s/files/1/0742/2899/products/SlushEngineLT-white_grande.png?v=1487710515)
![](https://cdn.sparkfun.com//assets/parts/1/1/2/8/8/13752-01a.jpg)

[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

