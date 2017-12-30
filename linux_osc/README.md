# Linux Open Source OSC #
## Real-time OSC Sniffer [Plug & Play] ##

This is an example for testing the [lib-osc](https://github.com/vanvught/rpidmx512/tree/master/lib-osc) C++ library.

Usage :

		./linux_osc interface_name|ip_address

Sample output :

    ./linux_osc eno1
	[V1.2] Linux #127-Ubuntu SMP Mon Dec 11 12:16:42 UTC 2017 Compiled on Dec 30 2017 at 15:50:08
	OSC, Incoming port: 8000, Outgoing port: 9000
	Running at : 192.168.2.120:8000
	Netmask : 255.255.255.0
	Hostname : nuc-i5
	DHCP : No
	30-12-2017 15:55:21.614966 path : /dmx1/1
	        arg 0, float, 199.669815
	30-12-2017 15:55:23.107585 path : /dmx1/2
	        arg 0, float, 173.207550
	30-12-2017 15:55:24.200199 path : /dmx1/blackout
	        arg 0, float, 1.000000
	30-12-2017 15:55:24.572153 path : /2
	        arg 0, float, 1.000000
	30-12-2017 15:55:39.849635 path : /1/multitoggle5/3/7
	        arg 0, float, 1.000000
	30-12-2017 15:55:40.381417 path : /1/multitoggle5/3/7
	        arg 0, float, 0.000000
	30-12-2017 15:55:40.381508 path : /1/multitoggle5/2/3
	        arg 0, float, 1.000000
	30-12-2017 15:55:41.515194 path : /1/multifader7/1
	        arg 0, float, 0.133721
	30-12-2017 15:55:41.849554 path : /1/multipush4/2/1
	        arg 0, float, 1.000000
	30-12-2017 15:55:41.870518 path : /1/multipush4/2/1
	        arg 0, float, 0.000000
	30-12-2017 15:55:42.344527 path : /1/multifader4/2
	        arg 0, float, 0.710227



[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)