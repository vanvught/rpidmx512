/**
 * @file l6470.h
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

#ifndef L6470_H_
#define L6470_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * Direction options: functions that accept dir as an argument can be
 * passed one of these constants. These functions are:
 *
 * run()
 * stepClock()
 * move()
 * goToDir()
 * goUntil()
 * releaseSw()
 */
enum TL6470Direction {
	L6470_DIR_FWD = 0x01, L6470_DIR_REV = 0x00
};

/**
 * Action options: functions that accept action as an argument can be
 * passed one of these constants. The contents of ABSPOS will either be
 * reset or copied to MARK, depending on the value sent. These functions are:
 *
 * goUntil()
 * releaseSw()
 */
enum TL6470Action {
	L6470_ABSPOS_RESET = 0x00, L6470_ABSPOS_COPY = 0x08
};

/**
 * 9.1 Registers and flags description
 *
 * See the Param_Handler() function for more info about these.
 */
enum TL6470ParamRegisters {
	L6470_PARAM_ABS_POS = 0x01,		///< len=22
	L6470_PARAM_EL_POS = 0x02,		///< len=9
	L6470_PARAM_MARK = 0x03,		///< len=22
	L6470_PARAM_SPEED = 0x04,		///< len=20
	L6470_PARAM_ACC = 0x05,			///< len=12
	L6470_PARAM_DECEL = 0x06,		///< len=12
	L6470_PARAM_MAX_SPEED = 0x07,	///< len=10
	L6470_PARAM_MIN_SPEED = 0x08,	///< len=13
	L6470_PARAM_KVAL_HOLD = 0x09,	///< len=8
	L6470_PARAM_KVAL_RUN = 0x0A,	///< len=8
	L6470_PARAM_KVAL_ACC = 0x0B,	///< len=8
	L6470_PARAM_KVAL_DEC = 0x0C,	///< len=8
	L6470_PARAM_INT_SPD = 0x0D,		///< len=14
	L6470_PARAM_ST_SLP = 0x0E,		///< len=8
	L6470_PARAM_FN_SLP_ACC = 0x0F,	///< len=8
	L6470_PARAM_FN_SLP_DEC = 0x10,	///< len=8
	L6470_PARAM_K_THERM = 0x11,		///< len=8
	L6470_PARAM_ADC_OUT = 0x12,		///< len=8
	L6470_PARAM_OCD_TH = 0x13,		///< len=8
	L6470_PARAM_STALL_TH = 0x14,	///< len=8
	L6470_PARAM_FS_SPD = 0x15,		///< len=10
	L6470_PARAM_STEP_MODE = 0x16,	///< len=8
	L6470_PARAM_ALARM_EN = 0x17,	///< len=8
	L6470_PARAM_CONFIG = 0x18,		///< len=16
	L6470_PARAM_STATUS = 0x19		///< len=16
};

class L6470 {

public:
	virtual ~L6470(void);
	virtual int busyCheck()=0;

public:
	int getStatus(void);

	void setParam(TL6470ParamRegisters, unsigned long);
	long getParam(TL6470ParamRegisters);

	void setLoSpdOpt(bool);
	void configSyncPin(uint8_t, uint8_t);
	void configStepMode(uint8_t);
	void setMaxSpeed(float);
	void setMinSpeed(float);
	void setFullSpeed(float);
	void setAcc(float);
	void setDec(float);
	void setOCThreshold(uint8_t);
	void setPWMFreq(int, int);
	void setSlewRate(int);
	void setOCShutdown(int);
	void setVoltageComp(int);
	void setSwitchMode(int);
	void setOscMode(int);
	void setAccKVAL(uint8_t);
	void setDecKVAL(uint8_t);
	void setRunKVAL(uint8_t);
	void setHoldKVAL(uint8_t);

	inline void setCurrent(uint8_t hold, uint8_t run, uint8_t acc, uint8_t dec) {
		setHoldKVAL(hold);
		setRunKVAL(run);
		setAccKVAL(acc);
		setDecKVAL(dec);
	}

	void setMicroSteps(unsigned int);

	bool getLoSpdOpt(void);
	uint8_t getStepMode(void);
	float getMaxSpeed(void);
	float getMinSpeed(void);
	float getFullSpeed(void);
	float getAcc(void);
	float getDec(void);
	uint8_t getOCThreshold(void);
	int getPWMFreqDivisor(void);
	int getPWMFreqMultiplier(void);
	int getSlewRate(void);
	int getOCShutdown(void);
	int getVoltageComp(void);
	int getSwitchMode(void);
	int getOscMode(void);
	uint8_t getAccKVAL(void);
	uint8_t getDecKVAL(void);
	uint8_t getRunKVAL(void);
	uint8_t getHoldKVAL(void);

	long getPos(void);
	long getMark(void);
	void run(TL6470Direction, float);
	void stepClock(TL6470Direction);
	void move(TL6470Direction, unsigned long);
	void goTo(long pos);
	void goToDir(TL6470Direction, long);
	void goUntil(TL6470Action, TL6470Direction, float);
	void releaseSw(TL6470Action, TL6470Direction);
	void goHome(void);
	void goMark(void);
	void setMark(long);
	void setPos(long);
	void resetPos(void);
	void resetDev(void);
	void softStop(void);
	void hardStop(void);
	void softHiZ(void);
	void hardHiZ(void);

	inline void move(long nStep) {
		if (nStep >= 0) {
			move(L6470_DIR_FWD, nStep);
		} else {
			move(L6470_DIR_REV, -nStep);
		}
	}

	inline void run(int dir, float stepsPerSec) {
		run((TL6470Direction) dir, stepsPerSec);
	}

	inline void goUntilPress(int action, int dir, float stepsPerSec) {
		goUntil((TL6470Action) action, (TL6470Direction) dir, stepsPerSec);
	}

	/**
	 * Just for administration purposes
	 */
	inline const int GetMotorNumber(void) {
		return m_nMotorNumber;
	}

	void Dump(void);

private:
	virtual uint8_t SPIXfer(uint8_t)=0;

private:
	long paramHandler(uint8_t, unsigned long);
	long xferParam(unsigned long, uint8_t);

	unsigned long accCalc(float);
	unsigned long decCalc(float);
	unsigned long minSpdCalc(float);
	unsigned long maxSpdCalc(float);
	unsigned long FSCalc(float);
	unsigned long intSpdCalc(float);
	unsigned long spdCalc(float);

	float accParse(unsigned long);
	float decParse(unsigned long);
	float minSpdParse(unsigned long);
	float maxSpdParse(unsigned long);
	float FSParse(unsigned long);
	float intSpdParse(unsigned long);
	float spdParse(unsigned long);

protected:
	int m_nMotorNumber;	///< Just for administration purposes
};

#endif /* L6470_H_ */
