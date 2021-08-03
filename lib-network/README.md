# Library Network
## Network Abstraction Layer (UDP/IP) implementation

Included is also a mDNS and TFTP implementation.

Supported platforms:

- Baremetal with EMAC/PHY
- Linux
- Mac OS X

For the baremetal environment, the follownig C functions *MUST* be implemented:

Low-level

	int emac_start(bool);
	void enet_mac_address_get(uint32_t, uint8_t paddr[]);
	
	void net_init(const uint8_t *mac_address, struct ip_info *p_ip_info, const uint8_t *hostname, bool *use_dhcp, bool *is_zeroconf_used);
	void net_shutdown(void);
	void net_handle(void);	
	
	void net_set_hostname(const char *name);
	void net_set_ip(uint32_t ip);
	void net_set_gw(uint32_t gw);
	bool net_set_zeroconf(struct ip_info *p_ip_info);
	
	bool net_set_dhcp(struct ip_info *p_ip_info, bool *is_zeroconf_used);
	void net_dhcp_release(void);
	
High-level

	int udp_bind(uint16_t local_port);
	int udp_unbind(uint16_t local_port);
	uint16_t udp_recv(uint8_t idx, uint8_t *packet, uint16_t size, uint32_t *from_ip, uint16_t *from_port);
	int udp_send(uint8_t idx, const uint8_t *packet, uint16_t size, uint32_t to_ip, uint16_t remote_port);
	
	int igmp_join(uint32_t group_address);
	int igmp_leave(uint32_t group_address);

[http://www.orangepi-dmx.org](http://www.orangepi-dmx.org)
