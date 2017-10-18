/**
 * @file l6470constants.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

/*
 * http://www.st.com/content/ccc/resource/technical/document/datasheet/a5/86/06/1c/fa/b2/43/db/CD00255075.pdf/files/CD00255075.pdf/jcr:content/translations/en.CD00255075.pdf
 * March 2015 DocID16737 Rev 7
 */

#ifndef L6470CONSTANTS_H_
#define L6470CONSTANTS_H_

/**
 * 9.1.8 MIN_SPEED
 */

///< When the LSPD_OPT bit is set high, the low speed optimization feature is enabled and the
///< MIN_SPEED value indicates the speed threshold below which the compensation works.
///< In this case the minimum speed of the speed profile is set to zero.
#define L6470_LSPD_OPT			(1<<12)
#define L6470_MIN_SPEED_MASK	0x0FFF

/**
 * 9.1.17 OCD_TH
 */
enum TL6470OvercurrentThreshold {
	L6470_OCD_TH_375mA = 0x00,
	L6470_OCD_TH_750mA = 0x01,
	L6470_OCD_TH_1125mA = 0x02,
	L6470_OCD_TH_1500mA = 0x03,
	L6470_OCD_TH_1875mA = 0x04,
	L6470_OCD_TH_2250mA = 0x05,
	L6470_OCD_TH_2625mA = 0x06,
	L6470_OCD_TH_3000mA = 0x07,
	L6470_OCD_TH_3375mA = 0x08,
	L6470_OCD_TH_3750mA = 0x09,
	L6470_OCD_TH_4125mA = 0x0A,
	L6470_OCD_TH_4500mA = 0x0B,
	L6470_OCD_TH_4875mA = 0x0C,
	L6470_OCD_TH_5250mA = 0x0D,
	L6470_OCD_TH_5625mA = 0x0E,
	L6470_OCD_TH_6000mA = 0x0F
};

/**
 * 9.1.18 STALL_TH
 */

/**
 * 9.1.19 STEP_MODE
 */

///< When the SYNC_EN bit is set low, BUSY/SYNC output is forced low during command execution.
///< When the SYNC_EN bit is set high, BUSY/SYNC output provides a clock signal according to the SYNC_SEL parameter.
#define L6470_SYNC_EN	(1<<7)

#define L6470_SYNC_SEL_MASK 0x70

enum TL6470SyncSel {
	L6470_SYNC_SEL_1_2 = 0x00,	///< fFS/2
	L6470_SYNC_SEL_1 = 0x10,	///< fFS
	L6470_SYNC_SEL_2 = 0x20,	///< 2fFS
	L6470_SYNC_SEL_4 = 0x30,	///< 4fFS
	L6470_SYNC_SEL_8 = 0x40,	///< 8fFS
	L6470_SYNC_SEL_16 = 0x50,	///< 16fFS
	L6470_SYNC_SEL_32 = 0x60,	///< 32fFS
	L6470_SYNC_SEL_64 = 0x70	///< 64fFS
};

#define L6470_STEP_SEL_MASK 0x07

enum TL6470StepSel {
	L6470_STEP_SEL_1 = 0x00,	///< Full-step
	L6470_STEP_SEL_1_2 = 0x01,	///< Half-step
	L6470_STEP_SEL_1_4 = 0x02,	///< 1/4 mirco-step
	L6470_STEP_SEL_1_8 = 0x03,	///< 1/8 mirco-step
	L6470_STEP_SEL_1_16 = 0x04,	///< 1/16 mirco-step
	L6470_STEP_SEL_1_32 = 0x05,	///< 1/32 mirco-step
	L6470_STEP_SEL_1_64 = 0x06,	///< 1/64 mirco-step
	L6470_STEP_SEL_1_128 = 0x07	///< 1/128 mirco-step
};

/**
 * 9.1.21 CONFIG
 */

/**
 * The OSC_SEL and EXT_CLK bits set the system clock source
 */
#define L6470_CONFIG_OSC_SEL_MASK		0x000F		///< Bit 0 - Bit 3

enum TL6470ConfigOsc {
	L6470_CONFIG_OSC_INT_16MHZ = 0x0000,				///< Internal 16MHz, no output
	L6470_CONFIG_OSC_EXT_8MHZ_XTAL_DRIVE = 0x0004,		///< External 8MHz crystal
	L6470_CONFIG_OSC_EXT_16MHZ_XTAL_DRIVE = 0x0005,		///< External 16MHz crystal
	L6470_CONFIG_OSC_EXT_24MHZ_XTAL_DRIVE = 0x0006,		///< External 24MHz crystal
	L6470_CONFIG_OSC_EXT_32MHZ_XTAL_DRIVE = 0x0007,		///< External 32MHz crystal
	L6470_CONFIG_OSC_INT_16MHZ_OSCOUT_2MHZ = 0x0008,	///< Default; internal 16MHz, 2MHz output
	L6470_CONFIG_OSC_INT_16MHZ_OSCOUT_4MHZ = 0x0009,	///< Internal 16MHz, 4MHz output
	L6470_CONFIG_OSC_INT_16MHZ_OSCOUT_8MHZ = 0x000A,	///< Internal 16MHz, 8MHz output
	L6470_CONFIG_OSC_INT_16MHZ_OSCOUT_16MHZ = 0x000B,	///< Internal 16MHz, 16MHz output
	L6470_CONFIG_OSC_EXT_8MHZ_OSCOUT_INVERT = 0x000C,	///< External 8MHz crystal, output inverted
	L6470_CONFIG_OSC_EXT_16MHZ_OSCOUT_INVERT = 0x000D,	///< External 16MHz crystal, output inverted
	L6470_CONFIG_OSC_EXT_24MHZ_OSCOUT_INVERT = 0x000E,	///< External 24MHz crystal, output inverted
	L6470_CONFIG_OSC_EXT_32MHZ_OSCOUT_INVERT = 0x000F	///< External 32MHz crystal, output inverted
};

/**
 * The SW_MODE bit sets the external switch to act as HardStop interrupt or not
 */
#define L6470_CONFIG_SW_MODE_MASK		0x0010		///< Bit 4

enum TL6470ConfigSw {
	TL6470_CONFIG_SW_MODE_HARD_STOP = 0x0000,
	TL6470_CONFIG_SW_MODE_USER = 0x0010
};

/**
 * Motor supply voltage compensation enable
 */
#define L6470_CONFIG_EN_VSCOMP_MASK		0x0020		///< Bit 5

enum TL6470ConfigVsComp {
	TL6470_CONFIG_VS_COMP_DISABLE = 0x0000,
	TL6470_CONFIG_VS_COMP_ENABLE = 0x0020
};

#define L6470_CONFIG_RESERVED_MASK		0x0040		///< Bit 6

/**
 * The OC_SD bit sets whether an overcurrent event causes or not the bridges to turn off;
 * the OCD flag in the STATUS register is forced low anyway
 */
#define L6470_CONFIG_OC_SD_MASK			0x0080		///< Bit 7

enum TL6470ConfigOcSd {
	L6470_CONFIG_OC_SD_DISABLE = 0x0000,	///< Bridges do NOT shutdown on OC detect
	L6470_CONFIG_OC_SD_ENABLE = 0x0080 		///< Bridges shutdown on OC detect
};

/**
 * The POW_SR bits set the slew rate value of power bridge output
 */
#define L6470_CONFIG_POW_SR_MASK		0x0300		///< Bit 8, Bit 9

enum TL6470ConfigPowSr {
	L6470_CONFIG_POW_SR_320V_us = 0x0000,
	L6470_CONFIG_POW_SR_75V_us = 0x0100,
	L6470_CONFIG_POW_SR_110V_us = 0x0200,
	L6470_CONFIG_POW_SR_260V_us = 0x0300
};

/**
 * The F_PWM_DEC bits set the multiplication factor of PWM frequency generation
 */
#define L6470_CONFIG_F_PWM_DEC_MASK		0x1C00		///< Bit 10 - Bit 12

enum TL6470ConfigFPwmDec {
	L6470_CONFIG_PWM_DEC_MUL_0_625 = 0x00 << 10,
	L6470_CONFIG_PWM_DEC_MUL_0_75 = 0x01 << 10,
	L6470_CONFIG_PWM_DEC_MUL_0_875 = 0x02 << 10,
	L6470_CONFIG_PWM_DEC_MUL_1 = 0x03 << 10,
	L6470_CONFIG_PWM_DEC_MUL_1_25 = 0x04 << 10,
	L6470_CONFIG_PWM_DEC_MUL_1_5 = 0x05 << 10,
	L6470_CONFIG_PWM_DEC_MUL_1_75 = 0x06 << 10,
	L6470_CONFIG_PWM_DEC_MUL_2 = 0x07 << 10
};

/**
 * The F_PWM_INT bits set the integer division factor of PWM frequency generation
 */
#define L6470_CONFIG_F_PWM_INT_MASK		0xE000		///< Bit 13 - Bit 15

enum TL6470ConfigFPwmInt {
	L6470_CONFIG_PWM_INT_DIV_1 = 0x00 << 13,
	L6470_CONFIG_PWM_INT_DIV_2 = 0x01 << 13,
	L6470_CONFIG_PWM_INT_DIV_3 = 0x02 << 13,
	L6470_CONFIG_PWM_INT_DIV_4 = 0x03 << 13,
	L6470_CONFIG_PWM_INT_DIV_5 = 0x04 << 13,
	L6470_CONFIG_PWM_INT_DIV_6 = 0x05 << 13,
	L6470_CONFIG_PWM_INT_DIV_7 = 0x06 << 13
};

/**
 * 9.1.22 STATUS
 */

#define L6470_STATUS_HIZ			0x0001 ///< high when bridges are in HiZ mode
#define L6470_STATUS_BUSY			0x0002 ///< mirrors BUSY pin
#define L6470_STATUS_SW_F			0x0004 ///< low when switch open, high when closed

/**
 * MOT_STATUS indicates the current motor status
 */
#define L6470_STATUS_MOT_STATUS_MASK	0x0060		///< Bit 5, Bit 6

enum TL6470StatusMotStatus {
	L6470_STATUS_MOT_STATUS_STOPPED = 0x0000 << 5,		///< Motor stopped
	L6470_STATUS_MOT_STATUS_ACCELERATION = 0x0001 << 5,	///< Motor accelerating
	L6470_STATUS_MOT_STATUS_DECELERATION = 0x0002 << 5,	///< Motor decelerating
	L6470_STATUS_MOT_STATUS_CONST_SPD = 0x0003 << 5		///< Motor at constant speed
};

#define L6470_STATUS_SW_EVN			0x0008 ///< active high, set on switch falling edge, cleared by reading STATUS
#define L6470_STATUS_DIR			0x0010 ///< Indicates current motor direction. High is FWD, Low is REV.
#define L6470_STATUS_NOTPERF_CMD	0x0080 ///< Last command not performed.
#define L6470_STATUS_WRONG_CMD		0x0100 ///< Last command not valid.
#define L6470_STATUS_UVLO			0x0200 ///< Undervoltage lockout is active
#define L6470_STATUS_TH_WRN			0x0400 ///< Thermal warning
#define L6470_STATUS_TH_SD			0x0800 ///< Thermal shutdown
#define L6470_STATUS_OCD			0x1000 ///< Overcurrent detected
#define L6470_STATUS_STEP_LOSS_A	0x2000 ///< Stall detected on A bridge
#define L6470_STATUS_STEP_LOSS_B	0x4000 ///< Stall detected on B bridge
#define L6470_STATUS_SCK_MOD		0x8000 ///< Step clock mode is active

/**
 * 9.2 Application commands
 */

#define L6470_CMD_NOP			0x00
#define L6470_CMD_SET_PARAM		0x00
#define L6470_CMD_GET_PARAM		0x20
#define L6470_CMD_RUN			0x50
#define L6470_CMD_STEP_CLOCK	0x58
#define L6470_CMD_MOVE			0x40
#define L6470_CMD_GOTO			0x60
#define L6470_CMD_GOTO_DIR		0x68
#define L6470_CMD_GO_UNTIL		0x82
#define L6470_CMD_RELEASE_SW	0x92
#define L6470_CMD_GO_HOME		0x70
#define L6470_CMD_GO_MARK		0x78
#define L6470_CMD_RESET_POS		0xD8
#define L6470_CMD_RESET_DEVICE	0xC0
#define L6470_CMD_SOFT_STOP		0xB0
#define L6470_CMD_HARD_STOP		0xB8
#define L6470_CMD_SOFT_HIZ		0xA0
#define L6470_CMD_HARD_HIZ		0xA8
#define L6470_CMD_GET_STATUS	0xD0

#endif /* L6470CONSTANTS_H_ */
