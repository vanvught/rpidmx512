/**
 * @file pca9685dmxparamsconst.h
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PCA9685DMXPARAMSCONST_H_
#define PCA9685DMXPARAMSCONST_H_

struct PCA9685DmxParamsConst {
	static inline const char FILE_NAME[]= "pca9685.txt";
	static inline const char I2C_ADDRESS[] = "i2c_address";
	static inline const char MODE[] = "mode";
	static inline const char CHANNEL_COUNT[] = "channel_count";
	static inline const char USE_8BIT[] = "use_8bit";
	static inline const char LED_PWM_FREQUENCY[] = "led_pwm_frequency";
	static inline const char LED_OUTPUT_INVERT[] = "led_output_invert";
	static inline const char LED_OUTPUT_OPENDRAIN[] = "led_output_opendrain";
	static inline const char SERVO_LEFT_US[] = "servo_left_us";
	static inline const char SERVO_CENTER_US[] = "servo_center_us";
	static inline const char SERVO_RIGHT_US[] = "servo_right_us";
};

#endif /* _PCA9685DMXPARAMSCONST_H_ */
