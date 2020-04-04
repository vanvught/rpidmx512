# Orange Pi baremetal Open Source
## DMX512 / RDM / Art-Net 4 / sACN E1.31 / OSC / SMPTE / Pixel controller

* **Ethernet**
  * **Art-Net 4**
      * Pixel controller **WS28xx/SK6812/APA102/UCSx903/P9813** [Orange Pi Zero]
         * 1x 4 Universes {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_pixel.zip?raw=true)}
         * 4x 4 Universes {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_pixel_multi.zip?raw=true)}
         * 8x 4 Universes {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_pixel_multi.zip?raw=true)}
      * DMX Input/Output Node / **RDM** Controller
         *  1 Port DMX {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_dmx.zip?raw=true)} {*Orange Pi Zero*}
         *  2 Ports {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_dmx_multi.zip?raw=true)} {*Orange Pi Zero*}
         *  4 Ports {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_dmx_multi.zip?raw=true)} {*Orange Pi One*}
      * **Real-time Monitor** 1 Universe {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_monitor.zip?raw=true)} {*Orange Pi One - HDMI output*}
     * Stepper controller **L6470 RDM**
         * Sparkfun AutoDriver chaining {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_rdm_l6470.zip?raw=true)} {*Orange Pi Zero*}
         * Roboteurs SlushEngine Model X LT {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_rdm_l6470.zip?raw=true)} {*Orange Pi One*}
  * **sACN E1.31** 
      * Pixel Controller **WS28xx/SK6812/APA102/UCSx903/P9813** [Orange Pi Zero]
         *  1x 4 Universes {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_dmx.zip?raw=true)}
         *  4x 4 Universes {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_pixel_multi.zip?raw=true)} 
         *  8x 4 Universes {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_pixel_multi.zip?raw=true)}
      * DMX Input/Output Bridge
         *  1 Port DMX {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_dmx.zip?raw=true)} {*Orange Pi Zero*}
         *  2 Ports {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_dmx_multi.zip?raw=true)} {*Orange Pi Zero*}
         *  4 Ports {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_dmx_multi.zip?raw=true)} {*Orange Pi One*}
      * **Real-time Monitor** 1 Universe {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_monitor.zip?raw=true)} {*Orange Pi One - HDMI output*}
      * **Art-Net** converter 4/32 Universes {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_artnet.zip?raw=true)} {*Orange Pi Zero*}
  * **OSC** 
      * DMX Bridge / **Pixel Controller (WS28xx/SK6812/APA102/UCSx903/P9813)** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_osc_dmx.zip?raw=true)}
      * **Client** with support for buttons {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_osc_client.zip?raw=true)}
      * **Real-time Monitor** 1 Universe {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_osc_monitor.zip?raw=true)} {*Orange Pi One - HDMI output*}
  * **Showfile**
      *  **Player** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_showfile.zip?raw=true)}
* **RDM** 
  * Controller with USB [Compatible with **Enttec USB Pro protocol**] {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_dmx_usb_pro.zip?raw=true)} {*Orange Pi Zero*}
  * Responder / **DMX Pixel Controller (WS28xx/SK6812/APA102/UCSx903/P9813)** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_rdm_responder.zip?raw=true)} {*Orange Pi Zero*}
  * Stepper controller L6470
     * Sparkfun AutoDriver chaining {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_rdm_responder_l6470.zip?raw=true)} {*Orange Pi Zero*}
     * Roboteurs SlushEngine Model X LT {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_rdm_responder_l6470.zip?raw=true)} {*Orange Pi One*}
* **DMX**
  * **Real-time Monitor** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_dmx_monitor.zip?raw=true)} {*Orange Pi One - HDMI output*}
* **SMPTE LTC**
  * **LTC SMPTE Timecode** Reader / Writer / Generator {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_ltc_smpte.zip?raw=true)}  {*Orange Pi Zero*}
* **MIDI**
  *  **Real-time Monitor** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_midi_monitor.zip?raw=true)}  {*Orange Pi One - HDMI output*}


Current limitation Ethernet versions: All nodes must be in the same network

All implementations are fully according to the standards.
<br>

* **Wifi**
  * **Art-Net 3** DMX Node / RDM Controller / Pixel Controller (WS28xx/SK6812/APA102/UCSx903) {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_wifi_artnet_dmx.zip?raw=true)} {*Orange Pi Zero*}
  * **sACN E1.31** DMX Bridge  / Pixel Controller (WS28xx/SK6812/APA102/UCSx903) {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_wifi_e131_dmx.zip?raw=true)} {*Orange Pi Zero*}
  * **OSC** DMX Bridge / Pixel Controller (WS28xx/SK6812/APA102/UCSx903) {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_wifi_osc_dmx.zip?raw=true)} {*Orange Pi Zero*}
 

Detailed information can be found here : [http://www.orangepi-dmx.org](http://www.orangepi-dmx.org)

Image's download [https://github.com/vanvught/h3dmx512-zip](https://github.com/vanvught/h3dmx512-zip "https://github.com/vanvught/h3dmx512-zip")

U-Boot Orange Pi Zero: [uboot-orangpi_zero.img.zip](https://github.com/vanvught/h3dmx512-zip/blob/master/uboot-orangpi_zero.img.zip?raw=true)

U-Boot Orange Pi One: [uboot-orangpi_one.img.zip](https://github.com/vanvught/h3dmx512-zip/blob/master/uboot-orangpi_one.img.zip?raw=true)

> Special thanks to [@trebisky](https://github.com/trebisky/orangepi) (Thomas J. Trebisky), who helped me in understanding the H3 SoC.

#  Mac OS X / Linux / Cygwin [debugging purpose only]
## Art-Net 4 / sACN E1.31 / OSC
- **Art-Net 4** Real-time DMX Monitor
- sACN **E.131** Real-time DMX Monitor
- **OSC** Real-time DMX Monitor

# Raspberry Pi Baremetal Open Source
## DMX512 / RDM / Art-Net 3 / sACN E1.31 / Pixel controller

Raspberry Pi **Open Source** solutions:

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

<br>

[PayPal.Me Donate](https://paypal.me/AvanVught?locale.x=nl_NL)
