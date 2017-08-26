# Linux Open Source Art-Net 3 Node #
## Real-time DMX Monitor [Plug & Play] ##

This is an example for testing the [lib-artnet](https://github.com/vanvught/rpidmx512/tree/master/lib-artnet) C++ library.

Usage :

		./linux_artnet interface_name|ip_address [max_dmx_channels]

Sample output :

    ./linux_artnet eno1 16
	[V1.0] Linux #112-Ubuntu SMP Mon Jul 31 19:38:41 UTC 2017 Compiled on Aug  9 2017 at 20:14:43
	ArtNet 3 Node
	Running at : 192.168.2.120
	Netmask : 255.255.255.0
	Hostname : nuc-i5
	DHCP : No
	
	Node configuration
	 Firmware     : 1.12
	 Short name   : AvV Art-Net Node
	 Long name    : Linux Open Source Art-Net 3 Node
	 Net          : 0
	 Sub-Net      : 0
	 Universe     : 1
	 Active ports : 1
	
	10-08-2017 11:36:55.406500 Start
	10-08-2017 11:37:55.120387 DMX 512:16 2c  0  0 2c  0  0 2c  0  0 2c  0  0 2c  0  0 2c
	10-08-2017 11:37:55.337778 DMX 512:16 2d  0  0 2d  0  0 2d  0  0 2d  0  0 2d  0  0 2d
	10-08-2017 11:37:55.550698 DMX 512:16 2e  0  0 2e  0  0 2e  0  0 2e  0  0 2e  0  0 2e
	10-08-2017 11:37:57.828983 DMX 512:16  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
	10-08-2017 11:37:57.988581 DMX 512:16 2f  0  0 2f  0  0 2f  0  0 2f  0  0 2f  0  0 2f
	10-08-2017 11:37:58.204533 DMX 512:16 30  0  0 30  0  0 30  0  0 30  0  0 30  0  0 30
	10-08-2017 11:37:58.421922 DMX 512:16 31  0  0 31  0  0 31  0  0 31  0  0 31  0  0 31
	10-08-2017 11:37:58.638532 DMX 512:16 32  0  0 32  0  0 32  0  0 32  0  0 32  0  0 32
	10-08-2017 11:37:58.859499 DMX 512:16 33  0  0 33  0  0 33  0  0 33  0  0 33  0  0 33


<img src="https://raw.githubusercontent.com/vanvught/rpidmx512/master/linux_artnet/DMX-Workshop.PNG" />

[http://www.raspberrypi-dmx.org/raspberry-pi-art-net-dmx-out](http://www.raspberrypi-dmx.org/raspberry-pi-art-net-dmx-out)