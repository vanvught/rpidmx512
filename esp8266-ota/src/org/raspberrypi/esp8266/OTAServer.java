/**
 * @file OTAServer.java
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
package org.raspberrypi.esp8266;

import java.io.File;
import java.io.IOException;

/**
 * 
 */
public class OTAServer {

	public static void main(String[] args) throws Exception {
		String dir = "";
		
		if (args.length == 1) {
			dir = args[0];
		} else if (args.length > 1){
			System.out.println("Discarding parameters");
		}
		
		if (dir.length() > 0) {
			if (dir.charAt(dir.length() - 1) != '\\' || dir.charAt(dir.length() - 1) != '/') {
				dir = dir + '/';
			}
		}
		
		System.out.println("Reading the files from directory [" + dir + "]");
		
		checkFiles(dir);
		
        WebServer ws = new WebServer();
        ws.start(dir);
	}

	/**
	 * Simple check :
	 * 1. Both files must exist.
	 * 2. Both files are of the same size.
	 * 
	 * @param dir
	 * @throws IOException
	 */
	private static void checkFiles(String dir) throws IOException {
		File user1bin = null;
		File user2bin = null;

		String sUser1bin = dir + "user1.bin";
		String sUser2bin = dir + "user2.bin";

		System.out.println("Checking file : [" + sUser1bin + "]");

		user1bin = new File(sUser1bin);

		if (!user1bin.exists())
			throw new IOException("File 'user1.bin' does not exist");

		System.out.println("Checking file : [" + sUser2bin + "]");

		user2bin = new File(sUser2bin);

		if (!user2bin.exists())
			throw new IOException("File 'user2.bin' does not exist");
		
		if (user1bin.length() != user2bin.length())
			throw new IOException("Files are not the same size");
	}
}