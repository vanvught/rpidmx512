## Open source Raspberry Pi C++ library for input devices ##

Supported devices :

- Adafruit 128x64 OLED Bonnet for Raspberry Pi [Raspbian] ([https://www.adafruit.com/product/3531](https://www.adafruit.com/product/3531))
- BitWizard User Interface 6-buttons [Raspbian] ([http://bitwizard.nl/shop/expansion-boards/Raspberry-Pi-User-Interface-with-16-x-2-LCD](http://bitwizard.nl/shop/expansion-boards/Raspberry-Pi-User-Interface-with-16-x-2-LCD))
- Keyboard [Linux]
- Infrared remote control [lirc-Linux] ([https://opencircuit.nl/Product/10028/Infrarood-ontvanger-en-afstandsbediening-kit](https://opencircuit.nl/Product/10028/Infrarood-ontvanger-en-afstandsbediening-kit))
 
<table>
<tr>
<td colspan="3" style="text-align:center;">lib-input <a href="https://github.com/vanvught/rpidmx512/tree/master/lib-input">https://github.com/vanvught/rpidmx512/tree/master/lib-input</a> (-linput)<br>class InputSet</br></td>
</tr>
<tr>
<td>&lt;Keyboard&gt; class KbLinux: public InputSet</td>
<td colspan="2">&lt;Buttons&gt; class ButtonsAdafruit: public InputSet</td>
</tr>
<tr>
<td>&lt;Infrared&gt; class IrLinux: public InputSet</td>
<td colspan="2">&lt;Buttons&gt; class ButtonsBw: public InputSet</td>
</tr>
</table>

Methods :

	bool IsAvailable(void)
	int GetChar(void)
	

![](https://cdn-shop.adafruit.com/970x728/3531-02.jpg)
![](http://www.bitwizard.nl/shop/image/cache/data/shop_pics/rpi_case/dsc05266-600x600.jpg)
![](https://opencircuit.nl/ContentImage/100034/crop/400-286/Infrarood-ontvanger-en-afstandsbediening-kit.jpg)


[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

