/**
 * @file WebServer.java
 */
/* This code is inspired by:
 * https://github.com/sysprogs/BSPTools/blob/master/DebugPackages/ESP8266DebugPackage/ESPImageTool/OTAServer.cs
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

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.Charset;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Locale;

/**
 * 
 */
public class WebServer {
	final static int port = 8888;

	/**
	 * Returns the time in local time zone.
	 * 
	 * @return
	 */
	public String getServerTime() {
		Calendar calendar = Calendar.getInstance();
		SimpleDateFormat dateFormat = new SimpleDateFormat("EEE, dd MMM yyyy HH:mm:ss z", Locale.US);
		// dateFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
		return dateFormat.format(calendar.getTime());
	}

	/**
	 * The real HTTP Server
	 * 
	 * @param dir
	 * @throws Exception
	 */
	public void start(String dir) throws Exception {
		String[] filesList = new String[2];

		filesList[0] = dir + "user1.bin";
		filesList[1] = dir + "user2.bin";

		System.out.println("Processing files : [" + filesList[0] + "] , [" + filesList[1] + "]");

		ServerSocket serverSocket = null;

		try {

			serverSocket = new ServerSocket(port);

			Socket clientSocket = null;

			byte[] buffer = new byte[1024];

			while (true) {
				System.out.println("Waiting for connection.....");
				clientSocket = serverSocket.accept();
				clientSocket.setSoTimeout(100);

				System.out.println(getServerTime() + " - Incoming connection from " + clientSocket.getRemoteSocketAddress());

				InputStream in = clientSocket.getInputStream();
				OutputStream out = new BufferedOutputStream(clientSocket.getOutputStream());

				StringBuilder requestBuilder = new StringBuilder();

				while (!requestBuilder.toString().contains("\r\n\r")) {
					int done = in.read(buffer);
					requestBuilder.append(new String(buffer, 0, done, Charset.defaultCharset()));
				}

				String request = requestBuilder.toString();
				String[] parts = request.split(" ");

				if (parts.length < 3)
					throw new Exception("Invalid HTTP request: " + request);

				String url = parts[1];
				System.out.println("Received request " + request);

				int fileListIndex = 0;

				if (url.toLowerCase().contains("user1.bin")) {
					fileListIndex = 0;
				} else if (url.toLowerCase().contains("user2.bin")) {
					fileListIndex = 1;
				} else {
					System.out.flush();
					System.err.println("Invalid request; user1.bin or user2.bin is not requested");
					break;
				}

				File file = new File(filesList[fileListIndex]);

				// The reply is the same for the HEAD and GET request
				String reply = String.format("HTTP/1.0 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %d\r\n\r\n", file.length());

				try {
					out.write(reply.getBytes("ASCII"));

					if (parts[0].equals("GET")) {
						// The files are checked in main.
						// A FileNotFoundException is never thrown.
						FileInputStream ios = new FileInputStream(file);

						int read = 0;
						int totalDone = 0;

						while ((read = ios.read(buffer)) != -1) {
							out.write(buffer, 0, read);
							totalDone += read;
							int percent = (int) ((totalDone * 100) / file.length());
							System.out.print("\r" + percent + "%");
						}

						ios.close();

						System.out.println("\r\nFile sent successfully\n" + totalDone);
						// After a successful sent, the server will be ended.
						break;

					}

				} catch (IOException ioExecption) {
					System.out.println("\nRemote side closed connection...restart\n");
				} finally {
					try {
						out.flush();
						out.close();
						in.close();
					} catch (Exception e) {
					}
				}

				// There is no check for HTTP Header "Connection:"
				// Always closing the session.
				clientSocket.close();
			}
		} catch (IOException ioExecption) {
			System.err.println("IOException: " + ioExecption);
			ioExecption.printStackTrace();
		} catch (SecurityException securityException) {
			System.err.println("SecurityException: " + securityException);
			securityException.printStackTrace();
		} finally {
			try {
				System.out.println("Closing the server");
				serverSocket.close();
			} catch (IOException e) {
				System.err.println("Error: " + e);
				e.printStackTrace();
			}
		}
	}
}
