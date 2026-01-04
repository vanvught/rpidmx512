# Library Network
## Network implementation (UDP/IP & TCP/IP)

Included is also a DHCP, NTP client, mDNS and TFTP implementation.
PTP support for GigaDevice ARM only

Supported platforms:

- MCU's: GD32F107R / GD32F207R / GD32F407R / GD32F450V / GD32F470V / GD32H759I {GigaDevice ARM}
- Board's: Orange Pi Zero / One (baremetal) {Allwinner SoC H2+/H3}
- OS's: Mac OS / Linux

Higly inspired by [lwIP](https://savannah.nongnu.org/projects/lwip/).

Two-layer organization:
- netif = Network Interface Layer (L2 / link-layer interface)
Manages the network interface itself: MAC address, hostname, DHCP/AutoIP triggers, counters, DNS servers.
- net = Network Stack Layer (L3/L4 logic)
Performs actual IP addressing, masks, gateway, ACD conflict detection, DHCP/AutoIP logic, shutdown, etc.

## netif
Based on `netif.h` and `netif.cpp`:

Interface Identity (Layer 2)
- MAC address copy
- Interface name "eth0"
- Interface index
- Hostname
- Domain name
- DNS server list (nameservers)

These belong to the network interface, not routing or IP logic.

[http://www.gd32-dmx.org](http://www.gd32-dmx.org)

[http://www.orangepi-dmx.org](http://www.orangepi-dmx.org)


