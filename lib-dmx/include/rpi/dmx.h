/**
 * @file dmx.h
 *
 */
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

#ifndef RPI_DMX_H_
#define RPI_DMX_H_

class Dmx: public DmxSet {
public:
	Dmx(uint8_t nGpioPin = GPIO_DMX_DATA_DIRECTION, bool DoInit = true);
	void Init();

	void SetPortDirection(uint32_t nPort, dmx::PortDirection portDirection, bool bEnableData = false) override;
	dmx::PortDirection GetPortDirection();

	// RDM
	void RdmSendRaw(uint32_t nPort, const uint8_t *pRdmData, uint16_t nLength) override;
	const uint8_t *RdmReceive(uint32_t nPort) override;
	const uint8_t *RdmReceiveTimeOut(uint32_t nPort, uint32_t nTimeOut) override;
	uint32_t RdmGetDateReceivedEnd() override;

	// DMX
	void ClearData();
	void SetSendData(const uint8_t *pData, uint16_t nLength);
	void SetSendDataWithoutSC(const uint8_t *pData, uint16_t nLength);

	const uint8_t* GetDmxCurrentData();
	const uint8_t* GetDmxAvailable();
	const uint8_t* GetDmxChanged();
	uint32_t GetUpdatesPerSecond();

	void SetDmxBreakTime(uint32_t nBreakTime);
	uint32_t GetDmxBreakTime();

	void SetDmxMabTime(uint32_t nMabTime);
	uint32_t GetDmxMabTime();

	void SetDmxPeriodTime(uint32_t nPeriodTime);
	uint32_t GetDmxPeriodTime();

	uint32_t GetSendDataLength() ;

	const volatile struct dmxsingle::TotalStatistics *GetTotalStatistics();

	static Dmx* Get() {
		return s_pThis;
	}

private:
	void UartInit();
	void SetSendDataLength(uint16_t send_data_length);
	void UartEnableFifo();
	void UartDisableFifo();
	void StopData();
	void StartData();

private:
	bool m_IsInitDone {false};
	uint32_t m_nDmxTransmitPeriodRequested { dmx::transmit::PERIOD_DEFAULT };
	uint8_t m_nDataDirectionGpio { GPIO_DMX_DATA_DIRECTION };

	static Dmx *s_pThis;
};

#endif /* RPI_DMX_H_ */
