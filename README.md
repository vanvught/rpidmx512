# Orange Pi baremetal Open Source
## DMX512 / RDM / Art-Net 4 / sACN E1.31 / OSC / SMPTE / Pixel controller

* **Ethernet**
  * **Art-Net 4**
      * DMX Node / **RDM** Controller / **Pixel Controller (WS28xx/SK6812/APA102/UCSx903)** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_dmx.zip?raw=true)}
      * **Pixel** Controller (WS28xx/SK6812/APA102/UCSx903) **4 Ports** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_pixel_multi.zip?raw=true)}  {*Orange Pi Zero*} 
      * DMX Node / RDM Controller **2 Ports** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_dmx_multi.zip?raw=true)} {*Orange Pi Zero*}
      * DMX Node / RDM Controller **4 Ports** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_dmx_multi.zip?raw=true)} {*Orange Pi One*}
      * **Real-time Monitor** 1 Universe {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_monitor.zip?raw=true)} {*Orange Pi One - HDMI output*}
  * **sACN E1.31** 
      * DMX Bridge / **Pixel Controller (WS28xx/SK6812/AP102/UCSx903)** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_dmx.zip?raw=true)}
      * **Pixel** Controller(WS28xx/SK6812/APA102/UCSx903) **4 Ports** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_pixel_multi.zip?raw=true)} {*Orange Pi Zero*}
      * DMX Bridge **2 Ports** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_dmx_multi.zip?raw=true)} {*Orange Pi Zero*}
      * DMX Bridge **4 Ports** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_dmx_multi.zip?raw=true)} {*Orange Pi One*}
      * **Real-time Monitor** 1 Universe {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_monitor.zip?raw=true)} {*Orange Pi One - HDMI output*}
  * **OSC** 
      * DMX Bridge / **Pixel Controller (WS28xx/SK6812/AP102/UCSx903)** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_osc_dmx.zip?raw=true)}
      * **Client** with support for buttons {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_osc_client.zip?raw=true)}
      * **Real-time Monitor** 1 Universe {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_osc_monitor.zip?raw=true)} {*Orange Pi One - HDMI output*}
* **RDM**
  * Controller with USB [Compatible with **Enttec USB Pro protocol**] {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_dmx_usb_pro.zip?raw=true)}  {*Orange Pi Zero*}
  * Responder / **DMX Pixel Controller (WS28xx/SK6812/AP102/UCSx903)** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/h3_rdm_responder.zip?raw=true)}
* **DMX**
  * **Real-time Monitor** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_dmx_monitor.zip?raw=true)} {*Orange Pi One - HDMI output*}
* **SMPTE LTC**
  * **SMPTE Timecode** LTC Reader / Writer / Generator {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_ltc_smpte.zip?raw=true)}  {*Orange Pi Zero*}
* **MIDI**
  *  **Real-time Monitor** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_midi_monitor.zip?raw=true)}  {*Orange Pi One*}
* **Wifi**
  * **Art-Net 3** DMX Node / RDM Controller / Pixel Controller (WS28xx/SK6812/AP102/UCSx903) {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/h3_wifi_artnet_dmx.zip?raw=true)} {*Orange Pi Zero*}
  * **sACN E1.31** DMX Bridge  / Pixel Controller (WS28xx/SK6812/AP102/UCSx903) {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/h3_wifi_e131_dmx.zip?raw=true)} {*Orange Pi Zero*}
  * **OSC** DMX Bridge / Pixel Controller (WS28xx/SK6812/AP102/UCSx903) {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/h3_wifi_osc_dmx.zip?raw=true)} {*Orange Pi Zero*}

Image's download [https://github.com/vanvught/h3dmx512-zip](https://github.com/vanvught/h3dmx512-zip "https://github.com/vanvught/h3dmx512-zip")

U-Boot Orange Pi Zero: [uboot-orangpi_zero.img.zip](https://github.com/vanvught/h3dmx512-zip/blob/master/uboot-orangpi_zero.img.zip?raw=true)

U-Boot Orange Pi One: [uboot-orangpi_one.img.zip](https://github.com/vanvught/h3dmx512-zip/blob/master/uboot-orangpi_one.img.zip?raw=true)

> Special thanks to [@trebisky](https://github.com/trebisky/orangepi) (Thomas J. Trebisky), who helped me in understanding the H3 SoC.

# Raspberry Pi Baremetal Open Source
## DMX512 / RDM / Art-Net 3 / sACN E1.31 / Pixel controller / MIDI

Raspberry Pi **Open Source** solutions:

* **Ethernet**
  * Art-Net 3 DMX Node / **Pixel (WS28xx/SK6812/AP102/UCSx903) controller** {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_circle_artnet_dmx.zip?raw=true)}
* **RDM**
  * RDM Controller with USB [Compatible with **Enttec USB Pro protocol**] {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_dmx_usb_pro.zip?raw=true)}
* **Wifi**
  * **Art-Net 3** DMX Node / RDM Controller / DMX Real-time Monitor / Pixel Controller (WS28xx/SK6812/AP102/UCSx903) {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_wifi_artnet_dmx.zip?raw=true)}
  * sACN **E1.31** DMX Bridge / DMX Real-time monitor / Pixel Controller (WS28xx/SK6812/AP102/UCSx903) {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_wifi_e131_dmx.zip?raw=true)}
  * **OSC** DMX Bridge / DMX Real-time monitor / Pixel Controller (WS28xx/SK6812/AP102/UCSx903) {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_wifi_osc_dmx.zip?raw=true)}


All implementations are fully according to the standards. And successfully used in live lighting shows.

Detailed information can be found here : [http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

The kernel7.img is running on both Raspberry Pi 2 and Raspberry Pi 3(B+).

> Special thanks to [@rsta2](https://github.com/rsta2/circle) (Rene Stange), who helped me get the multi-core support working. 

# Raspberry Pi Zero Raspbian Open Source
## Art-Net
- ArtIpProg Widget Raspberry Pi Zero

# Linux / Cygwin / Mac OS X
## Art-Net 4 / sACN E1.31 / OSC
- **Art-Net 4** Real-time DMX Monitor
- sACN **E.131** Real-time DMX Monitor
- **OSC** Real-time DMX Monitor

<br>

[PayPal.Me Donate](https://paypal.me/AvanVught?locale.x=nl_NL)
