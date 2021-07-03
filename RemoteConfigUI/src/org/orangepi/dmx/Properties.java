/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

public class Properties {
	static String getString(String line) {
		final int index = line.indexOf('=') + 1;
		if ((index > 0) && (index < line.length())) {
			return line.substring(index);
		}
		return "";
	}
	
	static int getInt(String line) {
		final int index = line.indexOf('=') + 1;
		if ((index > 0) && (index < line.length())) {
			final int value = Integer.parseInt(line.substring(index));
			return value;
		}
		return 0;
	}
	
	static Boolean getBool(String line) {
		if (getInt(line) != 0) {
			return true;
		}
		return false;
	}
	
	static String removeComments(String s) {
		final String[] lines = s.split("\n");
		
		StringBuffer out = new StringBuffer(lines[0] + "\n");
		
		for (int i = 1; i < lines.length; i++) {
			final String line = lines[i];
			if (line.startsWith("#")) {
				continue;
			}
			out.append(lines[i] + "\n");
		}
		
		return out.toString();
	}
}
