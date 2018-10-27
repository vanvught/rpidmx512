# Raspberry Pi Baremetal #
## DMX512 / RDM / Art-Net 3 / sACN E1.31 / Pixel controller / MIDI / SMPTE / OSC

Raspberry Pi **Open Source** solutions:

- RDM Controller with USB [Compatible with **Enttec USB Pro protocol**] {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_dmx_usb_pro.zip?raw=true)}
- Art-Net 3 DMX Node / **Pixel (WS28xx/SK6812) controller** {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_circle_artnet_dmx.zip?raw=true)}
- **Art-Net 3 Wifi** DMX Node / RDM Controller / Monitor / Pixel (WS28xx / SK6812) controller {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_wifi_artnet_dmx.zip?raw=true)}
- sACN **E1.31 Wifi** DMX Bridge / DMX Real-time monitor {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_wifi_e131_dmx.zip?raw=true)}
- DMX Real-time Monitor {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_dmx_monitor.zip?raw=true)}
- RDM Responder / **DMX Pixel (WS28xx/SK6812) controller** {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_rdm_responder.zip?raw=true)}
- MIDI Sniffer {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_midi_sniffer.zip?raw=true)}
- MIDI-DMX Bridge {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_midi_dmx_bridge.zip?raw=true)}
- **SMPTE** Timecode LTC Reader {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_ltc_reader.zip?raw=true)}
- **OSC DMX** Bridge {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_circle_osc_dmx.zip?raw=true)}
- OSC Wifi DMX Bridge {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_wifi_osc_dmx.zip?raw=true)}
- OSC Pixel (WS28xx/SK6812) Conroller {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_circle_osc_ws28xx.zip?raw=true)}
- **OSC Wifi Pixel** (WS28xx/SK6812) Controller {[SDCard](https://github.com/vanvught/rpidmx512-zip/blob/master/rpi_wifi_osc_ws28xx.zip?raw=true)}


All implementations are fully according to the standards. And successfully used in live lighting shows.

Detailed information can be found here : [http://www.raspberrypi-dmx.org](http://www.raspberrypi-dmx.org)

The kernel7.img is running on both Raspberry Pi 2 and Raspberry Pi 3(B+).

> Special thanks to [@rsta2](https://github.com/rsta2/circle) (Rene Stange), who helped me get the multi-core support working. 

# Orange Pi baremetal
## DMX512 / RDM / Art-Net 3 / sACN E1.31 / Pixel controller

**Open Source** solutions:

- RDM Controller with USB [Compatible with **Enttec USB Pro protocol**] {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/h3_dmx_usb_pro.zip?raw=true)}  {*Orange Pi Zero only*}
- **Art-Net 3 Ethernet** DMX Node / **RDM** Controller / **Pixel (WS28xx/SK6812) controller** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_dmx.zip?raw=true)}
- **Art-Net 3 Ethernet** DMX Node / RDM Controller **4 Ports** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_artnet_dmx_multi.zip?raw=true)}
- **Art-Net 3 Wifi** DMX Node / RDM Controller / Pixel (WS28xx/SK6812) controller {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/h3_wifi_artnet_dmx.zip?raw=true)}
- sACN **E1.31 Ethernet** DMX Node / Pixel (WS28xx/SK6812) controller {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/opi_emac_e131_dmx.zip?raw=true)}
- sACN **E1.31 Wifi** DMX Bridge  / Pixel (WS28xx / SK6812) controller {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/h3_wifi_e131_dmx.zip?raw=true)}
- RDM Responder / **DMX Pixel (WS28xx) controller** {[zip](https://github.com/vanvught/h3dmx512-zip/blob/master/h3_rdm_responder.zip?raw=true)}

Image's download [https://github.com/vanvught/h3dmx512-zip](https://github.com/vanvught/h3dmx512-zip "https://github.com/vanvught/h3dmx512-zip")

U-Boot Orange Pi Zero: [uboot-orangpi_zero.img.zip](https://github.com/vanvught/h3dmx512-zip/blob/master/uboot-orangpi_zero.img.zip?raw=true)

U-Boot Orange Pi One: [uboot-orangpi_one.img.zip](https://github.com/vanvught/h3dmx512-zip/blob/master/uboot-orangpi_one.img.zip?raw=true)

> Special thanks to [@trebisky](https://github.com/trebisky/orangepi) (Thomas J. Trebisky), who helped me in understanding the H3 SoC.


# Raspberry Pi Raspbian #
## Art-Net 3 / L6470 / Stepper

Linux Raspbian **Open Source** solutions:

- ArtIpProg Widget Raspberry Pi Zero
- **Art-Net 3** L6470 Stepper Motor Controller {will be moved to Orange Pi Zero baremetal}

# Linux / Cygwin / Mac OS X #
## Art-Net 3 / sACN E1.31 / OSC ##

Linux **Open Source** solutions:

- **Art-Net 3** Real-time DMX Monitor
- sACN **E.131** Real-time DMX Monitor
- **OSC** Real-time Sniffer 