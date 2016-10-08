## Updating the firmware using F-OTA

**More to come ....**

## Initial uploading of the firmware ##

There are hardware 2 options :

**1.** `Windows` `Linux` `MacOs` Using a USB-UART converter. For example the "[USB to UART](http://www.bitwizard.nl/shop/usb-boards/USB-to-UART)" board.

<img src="http://www.bitwizard.nl/shop/image/cache/data/shop_pics/ftdi-serial/2.0/dsc04554_small-74x74.jpg" /> 

**2.** `Linux` Using the Raspberry Pi
Connect the Wifi UART directly to the P1 header. Or use the "[Raspberry Pi Serial BoB](http://www.bitwizard.nl/shop/raspberry-pi/Raspberry-Pi-Serial-BoB)" 

<img src="http://www.bitwizard.nl/shop/image/cache/data/shop_pics/rpi_serial/rpi2/new/dsc04836_small-74x74.jpg" />

Make sure that the signal levels are 3.3 Volt.

| Wifi UART | USB UART  | RPi P1    |Serial BoB |
| :-------  | :-------- | :-------  |:--------- |
| 1 GND     | 1 GND     | 6 GND     | 1 GND     |
| 2 Rx (R)  | 3 Tx (T)  | 14 Tx (T) | 3 Tx (T)  |
| 3 Tx (T)  | 2 Rx (R)  | 15 Rx (R) | 2 Rx (R)  |
| 4 VCC     | 4 VCC     | 1 3V3     | 4 VCC     |

Do not connect the VCC, yet.


**More to come ....**