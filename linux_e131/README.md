# Linux Open Source sACN E1.31 Bridge #
## Real-time DMX Monitor [Plug & Play] ##

This is an example for testing the [lib-e131](https://github.com/vanvught/rpidmx512/tree/master/lib-e131) C++ library.

Usage :

		./linux_e131 interface_name|ip_address

Sample output :
	
	pi@nuc-i5:~/workspace/linux_e131$ ./linux_e131 eno1
	cat: /sys/firmware/devicetree/base/model: No such file or directory
	[V1.5] Linux #168-Ubuntu SMP Wed Jan 16 21:00:45 UTC 2019 Compiled on Feb 25 2019 at 20:20:39
	sACN E1.31 Real-time DMX Monitor {4 Universes}
	Network configuration
	 Hostname   : nuc-i5
	 Interface  : 192.168.2.120
	 Netmask    : 255.255.255.0
	 MacAddress : f4:4d:30:64:d9:e5
	 DHCP       : No
	Bridge configuration
	 Firmware : 1.7
	  Port A Universe 1 [HTP]
	  Port B Universe 2 [HTP]
	  Port C Universe 3 [HTP]
	  Port D Universe 4 [HTP]
	25-02-2019 20:31:37.056172 DMX:A 512:32:1 00 4f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	25-02-2019 20:31:37.056287 Start:A
	25-02-2019 20:31:39.556223 Stop:A
	25-02-2019 20:31:40.097945 DMX:A 512:32:1 00 4f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	25-02-2019 20:31:40.098014 Start:A
	25-02-2019 20:31:40.217098 DMX:A 512:32:1 02 4f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	25-02-2019 20:31:40.555995 DMX:A 512:32:1 04 4f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	25-02-2019 20:31:40.832571 DMX:A 512:32:1 04 51 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	25-02-2019 20:31:40.851549 DMX:A 512:32:1 04 53 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	25-02-2019 20:31:41.313122 DMX:A 512:32:1 04 53 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	25-02-2019 20:31:41.331176 DMX:A 512:32:1 04 53 04 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	25-02-2019 20:31:41.339280 DMX:A 512:32:1 04 53 06 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	25-02-2019 20:31:41.347255 DMX:A 512:32:1 04 53 08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	25-02-2019 20:31:43.904225 Stop:A




[http://www.orangepi-dmx.org/raspberry-pi-e131-bridge](http://www.orangepi-dmx.org/raspberry-pi-e131-bridge "http://www.orangepi-dmx.org/raspberry-pi-e131-bridge")