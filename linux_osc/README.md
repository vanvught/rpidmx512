# Linux Open Source OSC Bridge #
## Real-time DMX Monitor [Plug & Play] ##

This is an example for testing the [lib-osc](https://github.com/vanvught/rpidmx512/tree/master/lib-osc) C++ library.

Usage :

		./linux_osc interface_name|ip_address

Sample output :
	
	~/workspace/linux_osc$ ./linux_osc eno1
	cat: /sys/firmware/devicetree/base/model: No such file or directory
	[V1.6] Linux #168-Ubuntu SMP Wed Jan 16 21:00:45 UTC 2019 Compiled on Feb 25 2019 at 20:20:40
	OSC Real-time DMX Monitor
	Network configuration
	 Hostname   : nuc-i5
	 Interface  : 192.168.2.120
	 Netmask    : 255.255.255.0
	 MacAddress : f4:4d:30:64:d9:e5
	 DHCP       : No
	OSC Server configuration:
	 Incoming Port        : 8000
	 Outgoing Port        : 9000
	 DMX Path             : [/dmx1][/dmx1/*]
	  Blackout Path       : [/dmx1/blackout]
	 Partial Transmission : No
	25-02-2019 20:34:50.989529 Start:A
	Run() src/oscserver.cpp, line 293: ping received
	0000  2f 64 6d 78 31 2f 31 00  2c 66 00 00 3e 97 58 8e  /dmx1/1. ,f..>.X.
	Run() src/oscserver.cpp, line 323: [16] path : /dmx1/1
	Run() src/oscserver.cpp, line 392: f received 0.295597
	Run() src/oscserver.cpp, line 398: Channel = 1, Data = 4b
	25-02-2019 20:35:22.317546 DMX:A 512:32:1 4b 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	0000  2f 64 6d 78 31 2f 31 00  2c 66 00 00 3e a1 cf b2  /dmx1/1. ,f..>...
	Run() src/oscserver.cpp, line 323: [16] path : /dmx1/1
	Run() src/oscserver.cpp, line 392: f received 0.316038
	Run() src/oscserver.cpp, line 398: Channel = 1, Data = 50
	25-02-2019 20:35:22.323397 DMX:A 512:32:1 50 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	0000  2f 64 6d 78 31 2f 31 00  2c 66 00 00 3e b4 53 ba  /dmx1/1. ,f..>.S.
	Run() src/oscserver.cpp, line 323: [16] path : /dmx1/1
	Run() src/oscserver.cpp, line 392: f received 0.352201
	Run() src/oscserver.cpp, line 398: Channel = 1, Data = 59
	25-02-2019 20:35:22.336333 DMX:A 512:32:1 59 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	0000  2f 64 6d 78 31 2f 31 00  2c 66 00 00 3e cb ac 46  /dmx1/1. ,f..>..F
	Run() src/oscserver.cpp, line 323: [16] path : /dmx1/1
	Run() src/oscserver.cpp, line 392: f received 0.397799
	Run() src/oscserver.cpp, line 398: Channel = 1, Data = 65
	25-02-2019 20:35:22.353377 DMX:A 512:32:1 65 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	0000  2f 64 6d 78 31 2f 31 00  2c 66 00 00 3e ea 43 a0  /dmx1/1. ,f..>.C.
	Run() src/oscserver.cpp, line 323: [16] path : /dmx1/1
	Run() src/oscserver.cpp, line 392: f received 0.457547
	Run() src/oscserver.cpp, line 398: Channel = 1, Data = 74
	25-02-2019 20:35:22.371461 DMX:A 512:32:1 74 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00




[http://www.orangepi-dmx.org/pi-osc-bridge-dmx-pixel](http://www.orangepi-dmx.org/pi-osc-bridge-dmx-pixel "http://www.orangepi-dmx.org/pi-osc-bridge-dmx-pixel")