## Interface : Raspberry PI <-> ESP8266 ##

**Wifi** functions :

**.** Start the ESP8266 in AP mode. This function must be called first.

    wifi_init(const char *password)

**.** Change the ESP8266 to Station Mode with DHCP.

    void wifi_station(const char *ssid, const char *password)

**.** Change the ESP8266 to Station Mode with an fixed IP-address.

    void wifi_station_ip(const char *ssid, const char *password, const struct ip_info *info)


**UDP** functions :

**.** Start UDP.

    udp_begin(const uint16_t port)

**.** Receive UDP message

    uint16_t udp_recvfrom(const uint8_t *buffer, const uint16_t length, uint32_t *ip_address, uint16_t *port)
**.** Send UDP message

    void udp_sendto(const uint8_t *buffer, const uint16_t length, const uint32_t ip_address, const uint16_t port)


**FOTA** functions :

**.** Start F-OTA update

    void esp8266_fota_start(const uint32_t server_ip_address)

**.** Get F-OTA status message

    void esp8266_fota_status(char *status, uint16_t *len)

**Miscellaneous** functions :

**.** Get the build date stamp of the firmware
s 
    const char *wifi_get_firmware_version(void)

**ESP8266 SDK** functions :

- `const char *system_get_sdk_version(void)`
- `bool wifi_get_macaddr(const uint8_t *)`
- `bool wifi_get_ip_info(const struct ip_info *)`
- `const char *wifi_station_get_hostname(void)`
- `_wifi_mode wifi_get_opmode(void)`
- `const _wifi_station_status wifi_station_get_connect_status(void)`