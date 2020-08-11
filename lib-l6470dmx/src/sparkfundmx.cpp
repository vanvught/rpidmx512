/**
 * @file sparkfundmx.cpp
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <algorithm>
#include <stdint.h>
#include <stdio.h>
#include <cassert>

#include "sparkfundmx.h"
#include "sparkfundmx_internal.h"
#include "sparkfundmxparams.h"
#include "sparkfundmxparamsconst.h"

#include "lightset.h"
#include "lightsetdisplay.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "l6470params.h"
#include "l6470dmxmodes.h"

#include "motorparams.h"
#include "modeparams.h"

#include "hal_spi.h"
#include "hal_gpio.h"

#include "hardware.h"

#include "debug.h"

SparkFunDmx::SparkFunDmx():
	m_nDmxStartAddress(DMX_ADDRESS_INVALID),
	m_nDmxFootprint(0),
	m_pModeStore(0)
{
	DEBUG_ENTRY;

	m_nGlobalSpiCs = SPI_CS0;
	m_nGlobalResetPin = GPIO_RESET_OUT;
	m_nGlobalBusyPin = GPIO_BUSY_IN;

	m_bIsGlobalSpiCsSet = false;
	m_bIsGlobalResetSet = false;
	m_bIsGlobalBusyPinSet = false;

	m_nLocalPosition = 0;
	m_nLocalSpiCs = SPI_CS0;
	m_nLocalResetPin = GPIO_RESET_OUT;
	m_nLocalBusyPin = GPIO_BUSY_IN;

	m_bIsLocalPositionSet = false;
	m_bIsLocalSpiCsSet = false;
	m_bIsLocalResetSet = false;
	m_bIsLocalBusyPinSet = false;

	m_nDmxMode = L6470DMXMODE_UNDEFINED;
	m_nDmxStartAddressMode = 0;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		m_pAutoDriver[i] = 0;
		m_pMotorParams[i] = 0;
		m_pModeParams[i] = 0;
		m_pL6470DmxModes[i] = 0;
		m_pSlotInfo[i] = 0;
	}

	DEBUG_EXIT;
}

SparkFunDmx::~SparkFunDmx() {
	DEBUG_ENTRY;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pAutoDriver[i] != 0) {
			delete m_pAutoDriver[i];
			m_pAutoDriver[i] = 0;
		}

		if (m_pMotorParams[i] != 0) {
			delete m_pMotorParams[i];
			m_pMotorParams[i] = 0;
		}

		if (m_pModeParams[i] != 0) {
			delete m_pModeParams[i];
			m_pModeParams[i] = 0;
		}

		if (m_pL6470DmxModes[i] != 0) {
			delete m_pL6470DmxModes[i];
			m_pL6470DmxModes[i] = 0;
		}

		if (m_pSlotInfo[i] != 0) {
			delete m_pSlotInfo[i];
			m_pSlotInfo[i] = 0;
		}
	}

	DEBUG_EXIT;
}

void SparkFunDmx::Start(__attribute__((unused)) uint8_t nPort) {
	DEBUG_ENTRY;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->Start();
		}
	}

	DEBUG_EXIT;
}

void SparkFunDmx::Stop(__attribute__((unused)) uint8_t nPort) {
	DEBUG_ENTRY;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			m_pL6470DmxModes[i]->Stop();
		}
	}

	DEBUG_EXIT;
}

void SparkFunDmx::ReadConfigFiles(struct TSparkFunStores *ptSparkFunStores) {
	DEBUG_ENTRY;
#if !defined (H3)
	m_bIsGlobalSpiCsSet = false;
#else
	m_bIsGlobalSpiCsSet = true;
	m_nGlobalSpiCs = SPI_CS0;
#endif
	m_bIsGlobalResetSet = false;
	m_bIsGlobalBusyPinSet = false;

	SparkFunDmxParams sparkFunDmxParams(ptSparkFunStores == 0 ? 0 : static_cast<SparkFunDmxParamsStore*>(ptSparkFunStores->pSparkFunDmxParamsStore));

	if (sparkFunDmxParams.Load()) {
		sparkFunDmxParams.SetGlobal(this);
		sparkFunDmxParams.Dump();
	}

	if (m_bIsGlobalBusyPinSet) {
		FUNC_PREFIX(gpio_fsel(m_nGlobalBusyPin, GPIO_FSEL_INPUT));
	}

	FUNC_PREFIX(gpio_fsel(m_nGlobalResetPin, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_set(m_nGlobalResetPin));

	FUNC_PREFIX(gpio_clr(m_nGlobalResetPin));
	udelay(10000);
	FUNC_PREFIX(gpio_set(m_nGlobalResetPin));
	udelay(10000);

	FUNC_PREFIX(spi_begin());

	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
#ifndef NDEBUG
		printf("SparkFun motor%d.txt:\n", i);
#endif

		m_bIsLocalPositionSet = false;
		m_bIsLocalSpiCsSet = false;
		m_bIsLocalResetSet = false;
		m_bIsLocalBusyPinSet = false;

		if (sparkFunDmxParams.Load(i)) {
			sparkFunDmxParams.SetLocal(this);
			sparkFunDmxParams.Dump(i);

			if ((m_bIsLocalPositionSet) && (m_nLocalPosition < SPARKFUN_DMX_MAX_MOTORS)) {
				const uint8_t nSpiCs = m_bIsLocalSpiCsSet ? m_nLocalSpiCs : m_nGlobalSpiCs;
				const uint8_t nResetPin = m_bIsLocalResetSet ? m_nLocalResetPin : m_nGlobalResetPin;
				const uint8_t nBusyPin = m_bIsLocalBusyPinSet ? m_nLocalBusyPin : m_nGlobalBusyPin;
#ifndef NDEBUG
				printf("nSpiCs=%d [m_bIsLocalSpiCsSet=%d], nResetPin=%d [m_bIsLocalResetSet=%d], nBusyPin=%d [m_bIsLocalBusyPinSet=%d, m_bIsGlobalBusyPinSet=%d]\n",
						static_cast<int>(nSpiCs),
						static_cast<int>(m_bIsLocalSpiCsSet),
						static_cast<int>(nResetPin),
						static_cast<int>(m_bIsLocalResetSet),
						static_cast<int>(nBusyPin),
						static_cast<int>(m_bIsLocalBusyPinSet),
						static_cast<int>(m_bIsGlobalBusyPinSet));
#endif
				if (m_bIsGlobalBusyPinSet || m_bIsLocalBusyPinSet) {
					m_pAutoDriver[i] = new AutoDriver(m_nLocalPosition, nSpiCs, nResetPin, nBusyPin);
				} else {
					m_pAutoDriver[i] = new AutoDriver(m_nLocalPosition, nSpiCs, nResetPin);
				}
			}
		} else {
#ifndef NDEBUG
			printf("Configuration file : motor%c.txt not found\n", i + '0');
#endif
		}
	}

#ifndef NDEBUG
	printf("NumBoards : %d\n", static_cast<int>(AutoDriver::getNumBoards()));
#endif

	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
#ifndef NDEBUG
			printf("motor%d.txt:\n", i);
#endif
			if (m_pAutoDriver[i] != 0) {
				m_pModeParams[i] = new ModeParams(ptSparkFunStores == 0 ? 0 : ptSparkFunStores->pModeParamsStore);
				assert(m_pModeParams[i] != 0);
				m_pModeParams[i]->Load(i);
				m_pModeParams[i]->Dump();

				m_nDmxMode = m_pModeParams[i]->GetDmxMode();
				m_nDmxStartAddressMode = m_pModeParams[i]->GetDmxStartAddress();
#ifndef NDEBUG
				printf("\t-----------------------------\n");
				printf("\tm_nDmxMode=%d (DMX footprint=%d)\n", m_nDmxMode, L6470DmxModes::GetDmxFootPrintMode(m_nDmxMode));
				printf("\tm_nDmxStartAddressMode=%d\n", m_nDmxStartAddressMode);
				printf("\t=============================\n");
#endif

				if ((m_nDmxStartAddressMode <= DMX_UNIVERSE_SIZE) && (L6470DmxModes::GetDmxFootPrintMode(m_nDmxMode) != 0)) {
					if (m_pAutoDriver[i]->IsConnected()) {
						printf("Motor %d is connected\n", i);

						m_pAutoDriver[i]->setMotorNumber(i);
						m_pAutoDriver[i]->Dump();

						m_pMotorParams[i] = new MotorParams(ptSparkFunStores == 0 ? 0 : ptSparkFunStores->pMotorParamsStore);
						assert(m_pMotorParams[i] != 0);
						m_pMotorParams[i]->Load(i);
						m_pMotorParams[i]->Dump();
						m_pMotorParams[i]->Set(m_pAutoDriver[i]);

						L6470Params l6470Params(ptSparkFunStores == 0 ? 0 : ptSparkFunStores->pL6470ParamsStore);
						l6470Params.Load(i);
						l6470Params.Dump();
						l6470Params.Set(m_pAutoDriver[i]);

						m_pAutoDriver[i]->Dump();

						m_pL6470DmxModes[i] = new L6470DmxModes(static_cast<TL6470DmxModes>(m_nDmxMode), m_nDmxStartAddressMode, m_pAutoDriver[i], m_pMotorParams[i], m_pModeParams[i]);
						assert(m_pL6470DmxModes[i] != 0);

						if (m_nDmxStartAddress == DMX_ADDRESS_INVALID) {
							m_nDmxStartAddress = m_pL6470DmxModes[i]->GetDmxStartAddress();
							m_nDmxFootprint = m_pL6470DmxModes[i]->GetDmxFootPrint();
						} else {
							const uint16_t nDmxChannelLastCurrent = m_nDmxStartAddress + m_nDmxFootprint;
							m_nDmxStartAddress = std::min(m_nDmxStartAddress, m_pL6470DmxModes[i]->GetDmxStartAddress());

							const uint16_t nDmxChannelLastNew = m_nDmxStartAddressMode + m_pL6470DmxModes[i]->GetDmxFootPrint();
							m_nDmxFootprint = std::max(nDmxChannelLastCurrent, nDmxChannelLastNew) - m_nDmxStartAddress;
						}
#ifndef NDEBUG
						printf("DMX Mode: %d, DMX Start Address: %d\n", m_pL6470DmxModes[i]->GetMode(), m_pL6470DmxModes[i]->GetDmxStartAddress());
						printf("DMX Start Address:%d, DMX Footprint:%d\n", static_cast<int>(m_nDmxStartAddress), static_cast<int>(m_nDmxFootprint));
#endif
						const uint32_t nMaxSlots = std::min(MODE_PARAMS_MAX_DMX_FOOTPRINT, m_pL6470DmxModes[i]->GetDmxFootPrint());
#ifndef NDEBUG
						printf("SlotInfo slots: %d\n", static_cast<int>(nMaxSlots));
#endif
						m_pSlotInfo[i] = new struct TLightSetSlotInfo[nMaxSlots];
						assert(m_pSlotInfo[i] != 0);

						for (uint32_t j = 0; j < nMaxSlots; j++) {
							m_pModeParams[i]->GetSlotInfo(j, m_pSlotInfo[i][j]);
#ifndef NDEBUG
							printf(" Slot:%d %2x:%4x\n", j, m_pSlotInfo[i][j].nType, m_pSlotInfo[i][j].nCategory);
#endif
						}
					} else {
						delete m_pAutoDriver[i];
						m_pAutoDriver[i] = 0;
						printf("Motor %d - Communication issues! Check SPI configuration and cables\n", i);
					}
				} else {
					delete m_pAutoDriver[i];
					m_pAutoDriver[i] = 0;
					printf("Motor %d - Configuration error! Check Mode parameters\n", i);
				}
#ifndef NDEBUG
				printf("Motor %d --------- end ---------\n", i);
#endif
			} else {
				printf("Skipping Motor %d\n", i);
			}
	}

	printf("InitSwitch()\n");
	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			printf(" Motor %d\n", i);
			m_pL6470DmxModes[i]->InitSwitch();
		}
	}

	printf("busyCheck()\n");
	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pAutoDriver[i] != 0) {
			printf(" Motor %d\n", i);
			const uint32_t nMillis = Hardware::Get()->Millis();
			while (m_pAutoDriver[i]->busyCheck()) {
				if ((Hardware::Get()->Millis() - nMillis) > 1000) {
					printf("  Time-out!\n");
					break;
				}
			}
		}
	}

	printf("InitPos()\n");
	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			printf(" Motor %d\n", i);
			m_pL6470DmxModes[i]->InitPos();
		}
	}

	DEBUG_EXIT;
}

void SparkFunDmx::SetData(__attribute__((unused)) uint8_t nPortId, const uint8_t *pData, uint16_t nLength) {
	DEBUG_ENTRY;

	assert(pData != 0);
	assert(nLength <= DMX_UNIVERSE_SIZE);

	bool bIsDmxDataChanged[SPARKFUN_DMX_MAX_MOTORS];

	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			bIsDmxDataChanged[i] = m_pL6470DmxModes[i]->IsDmxDataChanged(pData, nLength);

			if(bIsDmxDataChanged[i]) {
				m_pL6470DmxModes[i]->HandleBusy();
			}
		} else {
			bIsDmxDataChanged[i] = false;
		}
#ifndef NDEBUG
		printf("bIsDmxDataChanged[%d]=%d\n", i, bIsDmxDataChanged[i]);
#endif
	}

	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (bIsDmxDataChanged[i]) {
			while (m_pL6470DmxModes[i]->BusyCheck())
				;
		}
	}

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (bIsDmxDataChanged[i]) {
			m_pL6470DmxModes[i]->DmxData(pData, nLength);
		}
	}

	DEBUG_EXIT;
}

bool SparkFunDmx::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	DEBUG_ENTRY;

	if (nDmxStartAddress == m_nDmxStartAddress) {
		return true;
	}

	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != 0) {
			const uint16_t nCurrentDmxStartAddress = m_pL6470DmxModes[i]->GetDmxStartAddress();
			const uint16_t nNewDmxStartAddress =  (nCurrentDmxStartAddress - m_nDmxStartAddress) + nDmxStartAddress;
#ifndef NDEBUG
			printf("\tMotor=%d, Current DMX Start Address=%d, New DMX Start Address=%d\n", i, nCurrentDmxStartAddress, nNewDmxStartAddress);
#endif
			m_pL6470DmxModes[i]->SetDmxStartAddress(nNewDmxStartAddress);

			if (m_pModeStore != 0) {
				m_pModeStore->SaveDmxStartAddress(i, nNewDmxStartAddress);
			}
		}
	}

	m_nDmxStartAddress = nDmxStartAddress;

	if (m_pLightSetDisplay != 0) {
		m_pLightSetDisplay->ShowDmxStartAddress();
	}

	DEBUG_EXIT;
	return true;
}

bool SparkFunDmx::GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo& tSlotInfo) {
	DEBUG2_ENTRY;

	if (nSlotOffset > m_nDmxFootprint) {
		DEBUG2_EXIT
		return false;
	}

	const uint16_t nDmxAddress = m_nDmxStartAddress + nSlotOffset;

	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if ((m_pL6470DmxModes[i] != 0) && (m_pSlotInfo[i] != 0)) {
			const int16_t nOffset = nDmxAddress - m_pL6470DmxModes[i]->GetDmxStartAddress();

			if ((nDmxAddress >= m_pL6470DmxModes[i]->GetDmxStartAddress()) && (nOffset < m_pL6470DmxModes[i]->GetDmxFootPrint())) {

				tSlotInfo.nType = m_pSlotInfo[i][nOffset].nType;
				tSlotInfo.nCategory = m_pSlotInfo[i][nOffset].nCategory;

				DEBUG2_EXIT
				return true;
			}
		}
	}

	return false;
}
