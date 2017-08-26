## Open source Raspberry Pi C library for UDP/IP ##

This library provides an abstraction layer for the UDP/IP network. 

<table>
<tr>
<td colspan="3" style="text-align:center;">lib-network <a href="https://github.com/vanvught/rpidmx512/tree/master/lib-network">https://github.com/vanvught/rpidmx512/tree/master/lib-network</a> (-lnetwork)</td>
</tr>
<tr>

<td>Raspberry Pi bare-metal<br><a href="https://github.com/vanvught/rpidmx512/tree/master/lib-esp8266">https://github.com/vanvught/rpidmx512/tree/master/lib-esp8266</a></br></td>

<td>Linux<br>Standard library</br></td>

<td>Raspberry Pi Circle<br><a href="https://github.com/rsta2/circle">https://github.com/rsta2/circle</a></br></td>

</tr>
</table>

**.** Start UDP

	void network_begin(const uint16_t port)
	
**.** Get MAC Address

	bool network_get_macaddr(const uint8_t *macaddr)

**.** Get IP Address (Linux: The IP Address of the selected interface)

	uint32_t network_get_ip(void)

**.** Get Subnet Mask

	uint32_t network_get_netmask(void)

**.** Get Broadcast IP Address

	const uint32_t network_get_bcast(void)

**.** Get Gateway IP Address

	uint32_t network_get_gw(void)

**.** Get Hostname

	const char *network_get_hostname(void)

**.** Is DHCP used

	bool network_is_dhcp_used(void) 

**.** Receive UDP message

	uint16_t network_recvfrom(const uint8_t *packet, const uint16_t size, uint32_t *from_ip, uint16_t *from_port)

**.** Send UDP message

	void network_sendto(const uint8_t *packet, const uint16_t size, const uint32_t to_ip, const uint16_t remote_port)

**.** Join group

	void network_joingroup(const uint32_t ip)

**.** Set IP Address for selected interace (Linux only)

	void network_set_ip(const uint32_t ip)

[http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

