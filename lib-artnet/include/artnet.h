/**
 * @file artnet.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 *
 * Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ARTNET_H_
#define ARTNET_H_

/**
 *
 */
enum TNodeStatus {
	ARTNET_OFF,					///<
	ARTNET_STANDBY,				///<
	ARTNET_ON					///<
};

/**
 * Table 4 – Style Codes
 * The Style code defines the general functionality of a Controller.
 * The Style code is returned in ArtPollReply.
 */
enum TNodeStyleCode {
	ARTNET_ST_NODE = 0x00,		///< A DMX to / from Art-Net device
	ARTNET_ST_SERVER = 0x01,	///< A lighting console.
	ARTNET_ST_MEDIA = 0x02,		///< A Media Server.
	ARTNET_ST_ROUTE = 0x03,		///< A network routing device.
	ARTNET_ST_BACKUP = 0x04,	///< A backup device.
	ARTNET_ST_CONFIG = 0x05,	///< A configuration or diagnostic tool.
	ARTNET_ST_VISUAL = 0x06		///< A visualiser.
};

/**
 * Table 5 – Priority Codes
 * Diagnostics Priority codes.
 * These are used in ArtPoll and ArtDiagData.
 */
enum TPriorityCodes {
	ARTNET_DP_LOW = 0x10,		///< Low priority message.
	ARTNET_DP_MED = 0x40,		///< Medium priority message.
	ARTNET_DP_HIGH = 0x80,		///< High priority message.
	ARTNET_DP_CRITICAL = 0xE0,	///< Critical priority message.
	ARTNET_DP_VOLATILE = 0xF0	///< Volatile message. Messages of this type are displayed on a single line in the DMX-Workshop diagnostics display. All other types are displayed in a list box.
};


enum TArtNetPortDataCode {
	ARTNET_PORT_DMX = 0x00,		///< Data is DMX-512
	ARTNET_PORT_MIDI = 0x01, 	///< Data is MIDI
	ARTNET_PORT_AVAB = 0x02,	///< Data is Avab
	ARTNET_PORT_CMX = 0x03,		///< Data is Colortran CMX
	ARTNET_PORT_ADB = 0x04,		///< Data is ABD 62.5
	ARTNET_PORT_ARTNET = 0x05	///< Data is ArtNet
};

/**
 * An enum for setting the behavior of a port.
 * Ports can either input data (DMX -> ArtNet) or
 * output (ArtNet -> DMX) data.
 */
enum TArtNetPortSettings {
	ARTNET_ENABLE_INPUT = 0x40, ///< Enables the input for this port
	ARTNET_ENABLE_OUTPUT = 0x80 ///< Enables the output for this port
};


/**
 * An enum for referring to a particular input or output port.
 */
enum TArtNetPortDir{
	ARTNET_INPUT_PORT = 1, 		///< The input port
	ARTNET_OUTPUT_PORT			///< The output port
};

/**
 * Table 3 – NodeReport Codes
 * The NodeReport code defines generic error, advisory and status messages for both Nodes and Controllers.
 * The NodeReport is returned in ArtPollReply.
 */
enum TArtNetNodeReportCode {
	ARTNET_RCDEBUG,			///<
	ARTNET_RCPOWEROK,		///<
	ARTNET_RCPOWERFAIL,		///<
	ARTNET_RCSOCKETWR1,		///<
	ARTNET_RCPARSEFAIL,		///<
	ARTNET_RCUDPFAIL,		///<
	ARTNET_RCSHNAMEOK,		///<
	ARTNET_RCLONAMEOK,		///<
	ARTNET_RCDMXERROR,		///<
	ARTNET_RCDMXUDPFULL,	///<
	ARTNET_RCDMXRXFULL,		///<
	ARTNET_RCSWITCHERR,		///<
	ARTNET_RCCONFIGERR,		///<
	ARTNET_RCDMXSHORT,		///<
	ARTNET_RCFIRMWAREFAIL,	///<
	ARTNET_RCUSERFAIL     	///<
};

/**
 * Merge is implemented in either LTP or HTP mode as specified by the ArtAddress packet.
 */
enum TMerge {
	ARTNET_MERGE_HTP,		///< Highest Takes Precedence (HTP)
	ARTNET_MERGE_LTP		///< Latest Takes Precedence (LTP)
};

/**
 * Node configuration commands
 */
enum TArtnetPortCommand {
	ARTNET_PC_NONE = 0x00,			///< No action
	ARTNET_PC_CANCEL = 0x01,		///< If Node is currently in merge mode, cancel merge mode upon receipt of next ArtDmx packet.
	ARTNET_PC_LED_NORMAL = 0x02,	///< The front panel indicators of the Node operate normally.
	ARTNET_PC_LED_MUTE = 0x03,		///< The front panel indicators of the Node are disabled and switched off.
	ARTNET_PC_LED_LOCATE = 0x04,	///< Rapid flashing of the Node’s front panel indicators. It is intended as an outlet locator for large installations.
	ARTNET_PC_RESET = 0x05,			///< Resets the Node’s Sip, Text, Test and data error flags.
	ARTNET_PC_MERGE_LTP_O = 0x10,	///< Set DMX Port 0 to Merge in LTP mode.
	ARTNET_PC_MERGE_LTP_1 = 0x11,	///< Set DMX Port 1 to Merge in LTP mode.
	ARTNET_PC_MERGE_LTP_2 = 0x12,	///< Set DMX Port 2 to Merge in LTP mode.
	ARTNET_PC_MERGE_LTP_3 = 0x13,	///< Set DMX Port 3 to Merge in LTP mode.
	ARTNET_PC_MERGE_HTP_0 = 0x50,	///< Set DMX Port 0 to Merge in HTP (default) mode.
	ARTNET_PC_MERGE_HTP_1 = 0x51,	///< Set DMX Port 1 to Merge in HTP (default) mode.
	ARTNET_PC_MERGE_HTP_2 = 0x52,	///< Set DMX Port 2 to Merge in HTP (default) mode.
	ARTNET_PC_MERGE_HTP_3 = 0x53,	///< Set DMX Port 3 to Merge in HTP (default) mode.
	ARTNET_PC_CLR_0 = 0x90,			///< Clear DMX Output buffer for Port 0
	ARTNET_PC_CLR_1 = 0x91,			///< Clear DMX Output buffer for Port 1
	ARTNET_PC_CLR_2 = 0x92,			///< Clear DMX Output buffer for Port 2
	ARTNET_PC_CLR_3 = 0x93			///< Clear DMX Output buffer for Port 3
};

#endif /* ARTNET_H_ */
