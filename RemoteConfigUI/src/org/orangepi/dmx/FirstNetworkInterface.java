/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;

public class FirstNetworkInterface {

	public static NetworkInterface get() {

		try {
			Enumeration<NetworkInterface> networks = NetworkInterface.getNetworkInterfaces();

			while (networks.hasMoreElements()) {

				NetworkInterface ni = networks.nextElement();

				System.out.println(ni.getDisplayName());

				Enumeration<InetAddress> addresses = ni.getInetAddresses();

				while (addresses.hasMoreElements()) {
					InetAddress ia = addresses.nextElement();

					System.out.println("\t" + ia);

					if (!ia.isAnyLocalAddress() && !ia.isLinkLocalAddress() && !ia.isLoopbackAddress()
							&& ia.isSiteLocalAddress() && !ni.isPointToPoint()) {
						return ni;
					}
				}
			}
		} catch (Exception ex) {
			System.out.println(ex);
		}

		return null;
	}
}
