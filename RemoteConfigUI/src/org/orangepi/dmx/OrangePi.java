/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

package org.orangepi.dmx;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;

public class OrangePi {
	private static final int BUFFERSIZE = 1024;
	private static final int PORT = 0x2905;
	
	private static final String RCONFIG_TXT = "rconfig.txt";
	private static final String DISPLAY_TXT = "display.txt";
	private static final String NETWORK_TXT = "network.txt";
	private static final String[] TYPES_TXT = {"artnet.txt", "e131.txt", "osc.txt", "ltc.txt", "oscclnt.txt", "", "show.txt"};
	private static final String[] TYPEVALUES = {"Art-Net", "sACN E1.31", "OSC Server", "LTC", "OSC Client", "RDMNet LLRP Only", "Showfile"};
	private static final String[] MODES_TXT = {"params.txt", "devices.txt", "mon.txt", "artnet.txt", "serial.txt", "rgbpanel.txt", ""};
	private static final String LDISPLAY_TXT = "ldisplay.txt";
	private static final String TCNET_TXT = "tcnet.txt";
	private static final String GPS_TXT = "gps.txt";
	private static final String MOTORS_TXT[] = {"motor0.txt", "motor1.txt", "motor2.txt", "motor3.txt", "motor4.txt", "motor5.txt", "motor6.txt", "motor7.txt" }; 
	private static final String RDM_TXT = "rdm_device.txt";
	private static final String SPARKFUN_TXT = "sparkfun.txt";
	
	private InetAddress localAddress;
	private DatagramSocket socketReceive;
	
	private Boolean isValid = false;
	
	private InetAddress address;
	
	private String nodeId = "";
	private String nodeDisplayName = "";
	
	private String nodeRemoteConfig = null;
	private String nodeDisplay = null;
	private String nodeNetwork = null;
	private String nodeType = null;
	private String nodeMode = null;
	private String nodeLtcDisplay = null;
	private String nodeTCNet = null;
	private String nodeGPS = null;
	private String nodeMotors[] = {null, null, null, null, null, null, null, null};
	private String nodeRDM = null;
	private String nodeSparkFun = null;
	private String nodeShow = null;
	
	private String sbRemoteConfig = null;
	private String sbDisplay = null;
	private String sbNetwork = null;
	private String sbType = null;
	private String sbMode = null;
	private String sbLtcDisplay = null;
	private String sbTCNet = null;
	private String sbGPS = null;
	private String sbMotors[] = {null, null, null, null, null, null, null, null};
	private String sbRDM = null;
	private String sbSparkFun = null;
	
	public OrangePi(String arg, InetAddress localAddress, DatagramSocket socketReceive) {
		super();
		this.localAddress = localAddress;
		this.socketReceive = socketReceive;
		
		System.out.println(arg);
	
		String[] values = arg.split(",");
		
		if (values.length >= 4) {
			String[] Mode = values[2].split("\n");
			isValid = isMapTypeValues(values[1]);	
			
			System.out.println("Mode[0]=" + Mode[0]);
			
			if (isValid) {
				if (Mode[0].equals("DMX") || Mode[0].equals("RDM")) {
					nodeMode = MODES_TXT[0];
					if (Mode[0].equals("RDM")) {
						nodeRDM = RDM_TXT;
					}
				} else if (Mode[0].equals("Pixel")) {
					nodeMode = MODES_TXT[1];
				} else if (Mode[0].equals("Monitor")) {
					nodeMode = MODES_TXT[2];
				} else if (Mode[0].equals("TimeCode")) {
					nodeLtcDisplay = LDISPLAY_TXT;
					nodeTCNet = TCNET_TXT;
					nodeGPS = GPS_TXT;
				} else if (Mode[0].equals("OSC")) {
				} else if (Mode[0].equals("Config")) {
					//
				} else if (Mode[0].equals("Stepper")) {
					nodeMode = MODES_TXT[1];
					nodeSparkFun = SPARKFUN_TXT;
					for (int i = 0; i < nodeMotors.length; i++) {
						nodeMotors[i] = MOTORS_TXT[i];
					}
					nodeRDM = RDM_TXT;
				} else if (Mode[0].equals("Player")) {
					// 
				} else if (Mode[0].equals("Art-Net")) {
					//
				} else if (Mode[0].equals("Serial")) {
					nodeMode = MODES_TXT[4];
				} else if (Mode[0].equals("RGB Panel")) {
					nodeMode = MODES_TXT[5];	
				} else {
					isValid = false;
				}
			}
			
			if (isValid) {
				nodeRemoteConfig = RCONFIG_TXT;
				nodeNetwork = NETWORK_TXT;
				
				try {
					address = InetAddress.getByName(values[0]);
				} catch (UnknownHostException e) {
					isValid = false;
					e.printStackTrace();
				}
			}
			
			if (isValid) {
				nodeId = values[0] + " " + values[1] + " " + values[2]  + " " + (values[3].equals("0") ? "" : values[3]);
				if (values.length == 5) {
					nodeDisplayName = values[4];
				} else {
					nodeDisplayName = "";
				}
				
				System.out.println("{" + nodeId + "} {" + nodeDisplayName + "}");
			} else {
				System.out.println("Invalid respone");
			}
		}
	}
	
	public String getTxt(String txt) {
		if (isRemoteConfigTxt(txt)) {
			if (sbRemoteConfig == null) {
				sbRemoteConfig = doGet(txt);
			}
			return sbRemoteConfig.toString();
		} else if (isDisplayTxt(txt)) {
			if (sbDisplay == null) {
				sbDisplay = doGet(txt);
			}
			return sbDisplay.toString();
		} else if (isNetworkTxt(txt)) {
			if (sbNetwork == null) {
				sbNetwork = doGet(txt);
			}
			return sbNetwork.toString();
		} else if (isTypeTxt(txt)) {
			if (sbType == null) {
				sbType = doGet(txt);
			}
			return sbType.toString();
		} else if (isModeTxt(txt)) {
			if (sbMode == null) {
				sbMode = doGet(txt);
			}
			return sbMode.toString();
		} else if (isLtcDisplayTxt(txt)) {
			if (sbLtcDisplay == null) {
				sbLtcDisplay = doGet(txt);
			}
			return sbLtcDisplay.toString();
		} else if (isTCNetTxt(txt)) {
			if (sbTCNet == null) {
				sbTCNet = doGet(txt);
			}
			return sbTCNet.toString();
		} else if (isGPSTxt(txt)) {
			if (sbGPS == null) {
				sbGPS = doGet(txt);
			}
			return sbGPS.toString();	
		} else if (isMotorTxt(txt)) {
			int nIndex = txt.charAt(5) - '0';
			if (sbMotors[nIndex] == null) {
				sbMotors[nIndex] = doGet(txt);
			}
			return sbMotors[nIndex].toString();
		} else if (isRdmTxt(txt)) {
			if (sbRDM == null) {
				sbRDM = doGet(txt);
			}
			return sbRDM.toString();
		} else if (isSparkFunTxt(txt)) {
			if (sbSparkFun == null) {
				sbSparkFun = doGet(txt);
			}
			return sbSparkFun.toString();
		}

		return null;
	}
	
	private void sendUdpPacket(byte[]  buffer) {
		DatagramPacket packet = new DatagramPacket(buffer, buffer.length, address, PORT);
		
		try {
			DatagramSocket socketSend = new DatagramSocket(null);
			socketSend.setReuseAddress(true);
			SocketAddress sockaddr = new InetSocketAddress(localAddress, PORT);
			socketSend.bind(sockaddr);
			socketSend.send(packet);
			socketSend.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
		
	private String doGet(String message) {
		String p = new String("?get#" + message);
		System.out.println(address + ":" + PORT + " " + p);

		byte[] bufferSendReceive = p.getBytes();
		
		sendUdpPacket(bufferSendReceive);
			
		bufferSendReceive = new byte[BUFFERSIZE];
		DatagramPacket packetReceive = new DatagramPacket(bufferSendReceive, bufferSendReceive.length);
		
		try {
			while (true) {
				socketReceive.receive(packetReceive);
				System.out.println("Message received");
				return new String(packetReceive.getData()).trim();
			}
		} catch (SocketTimeoutException e) {
			System.out.println("Timeout reached!");
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		return new String("#" + message + "\n");
	}
	
	public Boolean doSave(String data) throws IOException {
		System.out.println("doSave [" + data + "]");
		
		if (!data.startsWith("#")) {
			System.err.println("Does not start with #");
			return false;
		}
		
		int nIndex = data.indexOf(".txt");
		
		if (nIndex < 0) {
			System.err.println("Does not start with #????.txt");
			return false;
		}
		
		String txt = data.substring(1, nIndex + 4);
		
		Boolean bDoSave = false;

		if (isRemoteConfigTxt(txt)) {
			sbRemoteConfig = null;
			bDoSave = true;
		} else if (isDisplayTxt(txt)) {
			sbDisplay = null;
			bDoSave = true;
		} else if (isNetworkTxt(txt)) {
			sbNetwork = null;
			bDoSave = true;
		} else if (isModeTxt(txt)) {
			sbMode = null;
			bDoSave = true;
		} else if (isTypeTxt(txt)) {
			sbType = null;
			bDoSave = true;
		} else if (isLtcDisplayTxt(txt)) {
			sbLtcDisplay = null;
			bDoSave = true;
		} else if (isTCNetTxt(txt)) {
			sbTCNet = null;
			bDoSave = true;
		} else if (isGPSTxt(txt)) {
			sbGPS = null;
			bDoSave = true;	
		} else if (isMotorTxt(txt)) {
			int nMotorIndex = txt.charAt(5) - '0';
			sbMotors[nMotorIndex] = null;
			bDoSave = true;
		} else if (isRdmTxt(txt)) {
			sbRDM = null;
			bDoSave = true;
		} else if (isSparkFunTxt(txt)) {
			sbSparkFun = null;
			bDoSave = true;
		} 
		
		if (bDoSave) {
			byte[] buffer = data.trim().getBytes();
			sendUdpPacket(buffer);
		}
		
		return bDoSave;
	}
	
	public void doReboot() throws IOException {
		String p = new String("?reboot##");
		System.out.println(address + ":" + PORT + " " + p);

		byte[] buffer = p.getBytes();
		
		sendUdpPacket(buffer);
	}
	
	public void doSetDisplay(Boolean bOnOff) {
		String p = new String("!display#");
		if (bOnOff) {
			p = p + '1';
		} else {
			p = p + '0';
		}
		
		System.out.println(address + ":" + PORT + " " + p);
		
		byte[] buffer = p.getBytes();
		
		sendUdpPacket(buffer);
	}
	
	private String doRequest(String p) {
		System.out.println(address + ":" + PORT + " " + p);

		byte[] bufferSendReceive = p.getBytes();
		
		sendUdpPacket(bufferSendReceive);
			
		bufferSendReceive = new byte[BUFFERSIZE];
		DatagramPacket packetReceive = new DatagramPacket(bufferSendReceive, bufferSendReceive.length);
		
		try {
			while (true) {
				socketReceive.receive(packetReceive);
				System.out.println("Message received");
				return new String(packetReceive.getData()).trim();
			}
		} catch (SocketTimeoutException e) {
			System.out.println("Timeout reached!");
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		return new String("#ERROR - time out");
	}
	
	public String doGetDisplay() {
		return doRequest("?display#");
	}
	
	public String doUptime() {
		return doRequest("?uptime#");

	}
	
	public String doVersion() {
		return doRequest("?version#");

	}
	
	public String doGetTFTP() {
		return doRequest("?tftp#");
	}
	
	public void doSetTFTP(Boolean bOnOff) {
		String p = new String("!tftp#");
		if (bOnOff) {
			p = p + '1';
		} else {
			p = p + '0';
		}
		
		System.out.println(address + ":" + PORT + " " + p);
		
		byte[] buffer = p.getBytes();
		
		sendUdpPacket(buffer);
	}
			
	private Boolean isMapTypeValues(String type) {
		for (int i = 0; i < TYPEVALUES.length; i++) {
			if (type.equals(TYPEVALUES[i])) {
				nodeType = TYPES_TXT[i];
				if ((i == 0) || (i == 1)) {
					nodeDisplay = DISPLAY_TXT;
				}
				return true;
			}
		}
		return false;
	}
	
	private Boolean isRemoteConfigTxt(String config) {
		if (config.equals(RCONFIG_TXT)) {
			return true;
		}
		return false;
	}
	
	private Boolean isDisplayTxt(String config) {
		if (config.equals(DISPLAY_TXT)) {
			return true;
		}
		return false;
	}
	
	private Boolean isNetworkTxt(String network) {
		if (network.equals(NETWORK_TXT)) {
			return true;
		}
		return false;
	}
	
	private Boolean isTypeTxt(String type) {
		for (int i = 0; i < TYPES_TXT.length; i++) {
			if (type.equals(TYPES_TXT[i])) {
				return true;
			}
		}
		return false;
	}	
	
	private Boolean isModeTxt(String mode) {
		for (int i = 0; i < MODES_TXT.length; i++) {
			if (mode.equals(MODES_TXT[i])) {
				return true;
			}
		}
		return false;
	}
	
	private Boolean isLtcDisplayTxt(String ldisplay) {
		if (ldisplay.equals(LDISPLAY_TXT)) {
			return true;
		}
		return false;
	}
	
	private Boolean isTCNetTxt(String tcnet) {
		if (tcnet.equals(TCNET_TXT)) {
			return true;
		}
		return false;
	}
	
	private Boolean isGPSTxt(String gps) {
		if (gps.equals(GPS_TXT)) {
			return true;
		}
		return false;
	}
	
	private Boolean isMotorTxt(String motor) {
		for (int i = 0; i < MOTORS_TXT.length; i++) {
			if (motor.equals(MOTORS_TXT[i])) {
				return true;
			}
		}
		return false;
	}
	
	private Boolean isRdmTxt(String rdm) {
		if (rdm.equals(RDM_TXT)) {
			return true;
		}
		return false;
	}
	
	private Boolean isSparkFunTxt(String sparkFun) {
		if (sparkFun.equals(SPARKFUN_TXT)) {
			return true;
		}
		return false;
	}
	
	public Boolean getIsValid() {
		return isValid;
	}
	
	public String getNodeId() {
		return nodeId;
	}
	
	public String getNodeDisplayName() {
		return nodeDisplayName;
	}
	
	public String getNodeRemoteConfig() {
		return nodeRemoteConfig;
	}
	
	public String getNodeDisplay() {
		return nodeDisplay;
	}
		
	public String getNodeNetwork() {
		return nodeNetwork;
	}

	public String getNodeType() {
		if (nodeType.trim().length() == 0) {
			return null;
		}
		return nodeType;
	}

	public String getNodeMode() {
		return nodeMode;
	}
	
	public String getNodeLtcDisplay() {
		return nodeLtcDisplay;
	}
	
	public String getNodeTCNet() {
		return nodeTCNet;
	}
	
	public String getNodeGPS() {
		return nodeGPS;
	}
	
	public String getNodeMotor(int MotorIndex) {
		if (MotorIndex >= nodeMotors.length) {
			return null;
		}
		return nodeMotors[MotorIndex];
	}
	
	public String getNodeRDM() {
		return nodeRDM;
	}
	
	public String getNodeSparkFun() {
		return nodeSparkFun;
	}

	public String getNodeShow() {
		return nodeShow;
	}
	
	public InetAddress getAddress() {
		return address;
	}
		
	@Override
	public String toString() {
		if (nodeDisplayName.length() != 0) {
			return nodeDisplayName;
		}
		
		return nodeId;
	}
}
