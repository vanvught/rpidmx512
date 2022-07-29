package org.raspberrypi.dmx;
/**
 * @file DiscoverySniffer.java
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

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Locale;

public class DiscoverySniffer {
	public static String getLocalTime() {
		Calendar calendar = Calendar.getInstance();
		SimpleDateFormat dateFormat = new SimpleDateFormat("EEE, dd MMM yyyy HH:mm:ss z", Locale.US);
		return dateFormat.format(calendar.getTime());
	}

	public static void main(String[] args) {
		InetAddress ia = null;
		MulticastSocket ms = null;
		byte[] buffer = new byte[1500];
		DatagramPacket dp = new DatagramPacket(buffer, buffer.length);
		int port = 5568;

		try {
			ia = InetAddress.getByName("239.255.250.214");
			ms = new MulticastSocket(port);
			ms.joinGroup(ia);
			while (true) {
				ms.receive(dp);
				int dataLength = dp.getLength();
				System.out.printf("%s %s [%d]\n", getLocalTime(), dp.getAddress(), dataLength);
				if (dataLength < 120) {
					System.err.println("Invalid Discovery Packet\n");
					continue;
				}
				System.out.printf("Preamble size : %02x%02x[%d]\n", dp.getData()[0], dp.getData()[1], dp.getData()[1]);
				System.out.printf("Post size : %02x%02x\n", dp.getData()[2], dp.getData()[3]);
				System.out.print("ACN Packet id : ");
				for (int i = 4; i < 15; i++) {
					System.out.printf("%c", dp.getData()[i]);
				}
				System.out.printf("\nFlags and Length : %02x%02x[%d]\n", dp.getData()[16], dp.getData()[17], dp.getData()[17]);
				System.out.printf("Vector : %02x%02x%02x%02x\n", dp.getData()[18], dp.getData()[19], dp.getData()[20], dp.getData()[21]);
				System.out.print("CID : ");
				for (int i = 22; i < 38; i++) {
					if (((i - 22) == 4) || ((i - 22) == 6) || ((i - 22) == 8) || ((i - 22) == 10)) {
						System.out.print('-');
					}
					System.out.printf("%02x", dp.getData()[i]);
				}
				// Framing
				System.out.printf("\nFlags and Length : %02x%02x[%d]\n", dp.getData()[38], dp.getData()[39], dp.getData()[39]);
				System.out.printf("Vector : %02x%02x%02x%02x\n", dp.getData()[40], dp.getData()[41], dp.getData()[42], dp.getData()[43]);
				System.out.print("Source Name : ");
				for (int i = 44; i < 107; i++) {
					int c = dp.getData()[i];
					if ((c >= 32 && c <= 126) || c == '/') {
						System.out.printf("%c", c);
					} else {
						System.out.printf(".", c);
					}
				}
				// Discovery
				System.out.printf("\nFlags and Length : %02x%02x[%d]\n", dp.getData()[112], dp.getData()[113], dp.getData()[113]);
				System.out.printf("Vector : %02x%02x%02x%02x\n", dp.getData()[114], dp.getData()[115], dp.getData()[116], dp.getData()[117]);
				System.out.printf("Page : %02x\n", dp.getData()[118]);
				System.out.printf("Last Page : %02x\n", dp.getData()[119]);
				int UniverseList = (dp.getData()[113] - 8) / 2;
				System.out.printf("Universe list entries : %d\n", UniverseList);
				for (int i = 0; i < UniverseList; i++) {
					System.out.printf("%02x%02x ", dp.getData()[120 + (2 * i)], dp.getData()[121 + (2 * i)]);
				}
				System.out.println("\n");
			}
		} catch (UnknownHostException e) {
			System.err.println(e);
		} catch (SocketException se) {
			System.err.println(se);
		} catch (IOException ie) {
			System.err.println(ie);
		} finally {
			ms.close();
		}
	}
}