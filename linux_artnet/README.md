# Linux Open Source Art-Net 4 Node
## Real-time DMX Monitor / RDM Responder

This is an example for testing the [lib-artnet](https://github.com/vanvught/rpidmx512/tree/master/lib-artnet) C++ library.

Usage :

		./linux_artnet interface_name|ip_address

Sample output :
	
	~/workspace/linux_artnet$ ./linux_artnet eno1
	cat: /sys/firmware/devicetree/base/model: No such file or directory
	[V1.8] Linux #168-Ubuntu SMP Wed Jan 16 21:00:45 UTC 2019 Compiled on Feb 25 2019 at 20:20:39
	Art-Net 4 Node - Real-time DMX Monitor {4 Universes}
	Network configuration
	 Hostname   : nuc-i5
	 Interface  : 192.168.2.120
	 Netmask    : 255.255.255.0
	 MacAddress : f4:4d:30:64:d9:e5
	 DHCP       : No
	Node 4 configuration
	 Firmware   : 1.29
	 Short name : AvV Art-Net Node
	 Long name  : NUC6i5SYB Art-Net 4 www.orangepi-dmx.org
	 Net        : 0
	 Sub-Net    : 0
	  Port A Universe 1 [HTP] {Art-Net}
	  Port B Universe 2 [HTP] {Art-Net}
	  Port C Universe 3 [HTP] {sACN}
	  Port D Universe 4 [HTP] {sACN}
	Bridge configuration
	 Firmware : 1.7
	  Port C Universe 3 [HTP]
	  Port D Universe 4 [HTP]
	25-02-2019 20:27:12.388241 DMX:A 512:32:1   0  79   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
	25-02-2019 20:27:12.388366 Start:A
	25-02-2019 20:27:15.424249 DMX:A 512:32:1   0  79   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
	25-02-2019 20:27:18.466205 DMX:A 512:32:1   0  79   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
	25-02-2019 20:27:18.596248 DMX:A 512:32:1   0  79   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
	25-02-2019 20:27:18.628281 DMX:A 512:32:1   0  79   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
	25-02-2019 20:27:18.632251 DMX:A 512:32:1   0  79   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
	25-02-2019 20:27:21.042960 DMX:D 512:32:1 216   0   0 216   0   0 216   0   0 216   0   0 216   0   0 216   0   0 216   0   0 216   0   0 216   0   0 216   0   0 216   0
	25-02-2019 20:27:21.043027 Start:D
	25-02-2019 20:27:21.068281 DMX:D 512:32:1 217   0   0 217   0   0 217   0   0 217   0   0 217   0   0 217   0   0 217   0   0 217   0   0 217   0   0 217   0   0 217   0
	25-02-2019 20:27:21.104250 DMX:D 512:32:1 218   0   0 218   0   0 218   0   0 218   0   0 218   0   0 218   0   0 218   0   0 218   0   0 218   0   0 218   0   0 218   0
	25-02-2019 20:27:21.140247 DMX:D 512:32:1 219   0   0 219   0   0 219   0   0 219   0   0 219   0   0 219   0   0 219   0   0 219   0   0 219   0   0 219   0   0 219   0
	25-02-2019 20:27:21.168252 DMX:D 512:32:1 220   0   0 220   0   0 220   0   0 220   0   0 220   0   0 220   0   0 220   0   0 220   0   0 220   0   0 220   0   0 220   0
	25-02-2019 20:27:21.196164 DMX:D 512:32:1 221   0   0 221   0   0 221   0   0 221   0   0 221   0   0 221   0   0 221   0   0 221   0   0 221   0   0 221   0   0 221   0
	25-02-2019 20:27:21.228283 DMX:D 512:32:1 222   0   0 222   0   0 222   0   0 222   0   0 222   0   0 222   0   0 222   0   0 222   0   0 222   0   0 222   0   0 222   0
	25-02-2019 20:27:21.264251 DMX:D 512:32:1 223   0   0 223   0   0 223   0   0 223   0   0 223   0   0 223   0   0 223   0   0 223   0   0 223   0   0 223   0   0 223   0
	25-02-2019 20:27:21.292248 DMX:D 512:32:1 224   0   0 224   0   0 224   0   0 224   0   0 224   0   0 224   0   0 224   0   0 224   0   0 224   0   0 224   0   0 224   0
	25-02-2019 20:27:21.328216 DMX:D 512:32:1 225   0   0 225   0   0 225   0   0 225   0   0 225   0   0 225   0   0 225   0   0 225   0   0 225   0   0 225   0   0 225   0
	25-02-2019 20:27:21.356246 DMX:D 512:32:1 226   0   0 226   0   0 226   0   0 226   0   0 226   0   0 226   0   0 226   0   0 226   0   0 226   0   0 226   0   0 226   0
	25-02-2019 20:27:21.384244 DMX:D 512:32:1 227   0   0 227   0   0 227   0   0 227   0   0 227   0   0 227   0   0 227   0   0 227   0   0 227   0   0 227   0   0 227   0
	25-02-2019 20:27:21.420245 DMX:D 512:32:1 228   0   0 228   0   0 228   0   0 228   0   0 228   0   0 228   0   0 228   0   0 228   0   0 228   0   0 228   0   0 228   0
	25-02-2019 20:27:21.448243 DMX:D 512:32:1 229   0   0 229   0   0 229   0   0 229   0   0 229   0   0 229   0   0 229   0   0 229   0   0 229   0   0 229   0   0 229   0
	25-02-2019 20:27:23.952229 Stop:A
	25-02-2019 20:27:23.952286 Stop:D
	25-02-2019 20:27:24.549831 DMX:A 512:32:1   0  79   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
	25-02-2019 20:27:27.593760 DMX:A 512:32:1   0  79   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
	

</br>
<img src="https://raw.githubusercontent.com/vanvught/rpidmx512/master/linux_artnet/DMX-Workshop.PNG" />

[http://www.orangepi-dmx.org/raspberry-pi-art-net-dmx-out](http://www.orangepi-dmx.org/raspberry-pi-art-net-dmx-out "http://www.orangepi-dmx.org/raspberry-pi-art-net-dmx-out")