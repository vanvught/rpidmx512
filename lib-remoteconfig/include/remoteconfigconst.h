/**
 * @file remoteconfigconst.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef REMOTECONFIGCONST_H_
#define REMOTECONFIGCONST_H_

struct RemoteConfigConst {
	static inline const char PARAMS_FILE_NAME[] = "rconfig.txt";

	static inline const char PARAMS_DISABLE[] = "disable";

	static inline const char PARAMS_DISABLE_WRITE[] = "disable_write";
	static inline const char PARAMS_ENABLE_REBOOT[] = "enable_reboot";
	static inline const char PARAMS_ENABLE_UPTIME[] = "enable_uptime";
	static inline const char PARAMS_ENABLE_FACTORY[] = "enable_factory";

	static inline const char PARAMS_DISPLAY_NAME[] = "display_name";
};

#endif /* REMOTECONFIGCONST_H_ */
