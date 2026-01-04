# Library LTC
## Open Source C++ library for SMPTE TimeCode LTC Protocol

Supported board :

-	Orange Pi Zero

Supported input :

- LTC SMPTE
- TCNet
- Art-Net
- rtpMIDI
- MIDI
- System Time

Supported output :

* LTC SMPTE
* Art-Net
* MIDI
* rtpMIDI
* Displays :   
   * OLED display 128 x 64 I2C (auto-detect)
      * SSD1306 I2C
      * SH1106 I2C 
   * LCD I2C (auto-detect)
      * 16x2 
      * 16x4
   * MAX7219 (SPI)
      *  8x 7-segment 
      *  Matrix
   * WS28xx (SPI)
      *  7-segment  
      *  Matrix 64 x 8 (1 line)
   * HUB75 (GPIO)
       * Matrix 64 x 32 (4 lines)
      
Prototype open hardware is available here : [https://github.com/vanvught/h3dmx512-zip/tree/master/eagle/Orange-Pi-Zero](https://github.com/vanvught/h3dmx512-zip/tree/master/eagle/Orange-Pi-Zero)

[http://www.orangepi-dmx.org/orange-pi-smpte-timecode-ltc-reader-converter](http://www.orangepi-dmx.org/orange-pi-smpte-timecode-ltc-reader-converter)

## Timers

<table>
	<tr>
		<th>Class</th>
		<th>Timer 0</th>
		<th>Timer 1</th>
		<th>Timer ARM Physical</th>
		<th>Timer ARM Virtual</th>
	</tr>
	<tr>
		<td>ArtNetReader</td>
		<td></td>
		<td></td>
		<td>gv_ltc_nUpdatesPerSecond</td>
		<td></td>
	</tr>	
	<tr>
		<td>LtcEtcReader</td>
		<td></td>
		<td></td>
		<td>gv_ltc_nUpdatesPerSecond</td>
		<td></td>
	</tr>
	<tr>
		<td>LtcGenerator</td>
		<td>gv_ltc_bTimeCodeAvailable = true</td>
		<td></td>
		<td></td>
		<td></td>
	</tr>
	<tr>
		<td>LtcMidiSystemRealtime</td>
		<td></td>
		<td></td>
		<td></td>
		<td>BPM</td>
	</tr>
	<tr>
		<td>LtcOutputs</td>
		<td></td>
		<td>sv_isMidiQuarterFrameMessage = true</td>
		<td></td>
		<td></td>
	</tr>
	<tr>
		<td>LtcReader</td>
		<td></td>
		<td></td>
		<td>gv_ltc_nUpdatesPerSecond</td>
		<td></td>
	</tr>
		<td>MidiReader</td>
		<td>class Midi</td>
		<td>gv_ltc_bTimeCodeAvailable = true</td>
		<td></td>
		<td></td>
	</tr>
	<tr>
		<td>RtpMidiReader</td>
		<td>gv_ltc_bTimeCodeAvailable = true</td>
		<td></td>
		<td>gv_ltc_nUpdatesPerSecond</td>
		<td></td>
	</tr>
	<tr>
		<td>SystimeReader</td>
		<td>gv_ltc_bTimeCodeAvailable = true</td>
		<td></td>
		<td></td>
		<td></td>
	</tr>
	</tr>
		<td>TCNetReader</td>
		<td></td>
		<td></td>
		<td>gv_ltc_nUpdatesPerSecond</td>
		<td></td>
	</tr>	
</table>