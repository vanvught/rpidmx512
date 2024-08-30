/**
 * @file sparkfundmx.cpp
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdint>
#include <cstdio>
#include <cassert>

#include "sparkfundmx.h"
#include "sparkfundmx_internal.h"
#include "sparkfundmxparams.h"
#include "sparkfundmxparamsconst.h"

#include "lightset.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "l6470params.h"
#include "l6470dmxmodes.h"

#include "motorparams.h"
#include "modeparams.h"

#include "hal_api.h"
#include "hal_spi.h"
#include "hal_gpio.h"

#include "hardware.h"

#include "debug.h"

using namespace lightset;

SparkFunDmx::SparkFunDmx(): m_nDmxStartAddress(dmx::ADDRESS_INVALID) {
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
		m_pAutoDriver[i] = nullptr;
		m_pMotorParams[i] = nullptr;
		m_pModeParams[i] = nullptr;
		m_pL6470DmxModes[i] = nullptr;
		m_pSlotInfo[i] = nullptr;
	}

	DEBUG_EXIT;
}

SparkFunDmx::~SparkFunDmx() {
	DEBUG_ENTRY;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pAutoDriver[i] != nullptr) {
			delete m_pAutoDriver[i];
			m_pAutoDriver[i] = nullptr;
		}

		if (m_pMotorParams[i] != nullptr) {
			delete m_pMotorParams[i];
			m_pMotorParams[i] = nullptr;
		}

		if (m_pModeParams[i] != nullptr) {
			delete m_pModeParams[i];
			m_pModeParams[i] = nullptr;
		}

		if (m_pL6470DmxModes[i] != nullptr) {
			delete m_pL6470DmxModes[i];
			m_pL6470DmxModes[i] = nullptr;
		}

		if (m_pSlotInfo[i] != nullptr) {
			delete m_pSlotInfo[i];
			m_pSlotInfo[i] = nullptr;
		}
	}

	DEBUG_EXIT;
}

void SparkFunDmx::Start([[maybe_unused]] uint32_t nPortIndex) {
	DEBUG_ENTRY;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != nullptr) {
			m_pL6470DmxModes[i]->Start();
		}
	}

	DEBUG_EXIT;
}

void SparkFunDmx::Stop([[maybe_unused]] uint32_t nPortIndex) {
	DEBUG_ENTRY;

	for (int i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != nullptr) {
			m_pL6470DmxModes[i]->Stop();
		}
	}

	DEBUG_EXIT;
}

void SparkFunDmx::ReadConfigFiles() {
	DEBUG_ENTRY;
#if !defined (H3)
	m_bIsGlobalSpiCsSet = false;
#else
	m_bIsGlobalSpiCsSet = true;
	m_nGlobalSpiCs = SPI_CS0;
#endif
	m_bIsGlobalResetSet = false;
	m_bIsGlobalBusyPinSet = false;

	SparkFunDmxParams sparkFunDmxParams;

	sparkFunDmxParams.Load();
	sparkFunDmxParams.SetGlobal(this);

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
		printf("SparkFun motor%d.txt:\n", i);

		m_bIsLocalPositionSet = false;
		m_bIsLocalSpiCsSet = false;
		m_bIsLocalResetSet = false;
		m_bIsLocalBusyPinSet = false;

		sparkFunDmxParams.Load(i);
		sparkFunDmxParams.SetLocal(this);

		if ((m_bIsLocalPositionSet) && (m_nLocalPosition < SPARKFUN_DMX_MAX_MOTORS)) {
			const uint8_t nSpiCs = m_bIsLocalSpiCsSet ? m_nLocalSpiCs : m_nGlobalSpiCs;
			const uint8_t nResetPin = m_bIsLocalResetSet ? m_nLocalResetPin : m_nGlobalResetPin;
			const uint8_t nBusyPin = m_bIsLocalBusyPinSet ? m_nLocalBusyPin : m_nGlobalBusyPin;

			printf("nSpiCs=%d [m_bIsLocalSpiCsSet=%d], nResetPin=%d [m_bIsLocalResetSet=%d], nBusyPin=%d [m_bIsLocalBusyPinSet=%d, m_bIsGlobalBusyPinSet=%d]\n",
					static_cast<int>(nSpiCs),
					static_cast<int>(m_bIsLocalSpiCsSet),
					static_cast<int>(nResetPin),
					static_cast<int>(m_bIsLocalResetSet),
					static_cast<int>(nBusyPin),
					static_cast<int>(m_bIsLocalBusyPinSet),
					static_cast<int>(m_bIsGlobalBusyPinSet));

			if (m_bIsGlobalBusyPinSet || m_bIsLocalBusyPinSet) {
				m_pAutoDriver[i] = new AutoDriver(m_nLocalPosition, nSpiCs, nResetPin, nBusyPin);
			} else {
				m_pAutoDriver[i] = new AutoDriver(m_nLocalPosition, nSpiCs, nResetPin);
			}
		} else {
			printf("Local position is not set\n");
		}
	}

	printf("NumBoards : %d\n", static_cast<int>(AutoDriver::getNumBoards()));

	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
#ifndef NDEBUG
			printf("motor%d.txt:\n", i);
#endif
			if (m_pAutoDriver[i] != nullptr) {
				m_pModeParams[i] = new ModeParams;
				assert(m_pModeParams[i] != nullptr);
				m_pModeParams[i]->Load(i);

				m_nDmxMode = m_pModeParams[i]->GetDmxMode();
				m_nDmxStartAddressMode = m_pModeParams[i]->GetDmxStartAddress();
#ifndef NDEBUG
				printf("\t-----------------------------\n");
				printf("\tm_nDmxMode=%d (DMX footprint=%d)\n", m_nDmxMode, L6470DmxModes::GetDmxFootPrintMode(m_nDmxMode));
				printf("\tm_nDmxStartAddressMode=%d\n", m_nDmxStartAddressMode);
				printf("\t=============================\n");
#endif

				if ((m_nDmxStartAddressMode <= dmx::UNIVERSE_SIZE) && (L6470DmxModes::GetDmxFootPrintMode(m_nDmxMode) != 0)) {
					if (m_pAutoDriver[i]->IsConnected()) {
						printf("Motor %d is connected\n", i);

						m_pAutoDriver[i]->setMotorNumber(i);
						m_pAutoDriver[i]->Dump();

						m_pMotorParams[i] = new MotorParams;
						assert(m_pMotorParams[i] != nullptr);
						m_pMotorParams[i]->Load(i);
						m_pMotorParams[i]->Set(m_pAutoDriver[i]);

						L6470Params l6470Params;
						l6470Params.Load(i);
						l6470Params.Set(m_pAutoDriver[i]);

						m_pAutoDriver[i]->Dump();

						m_pL6470DmxModes[i] = new L6470DmxModes(static_cast<TL6470DmxModes>(m_nDmxMode), m_nDmxStartAddressMode, m_pAutoDriver[i], m_pMotorParams[i], m_pModeParams[i]);
						assert(m_pL6470DmxModes[i] != nullptr);

						if (m_nDmxStartAddress == dmx::ADDRESS_INVALID) {
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
						const uint32_t nMaxSlots = std::min(modeparams::MODE_PARAMS_MAX_DMX_FOOTPRINT, m_pL6470DmxModes[i]->GetDmxFootPrint());
#ifndef NDEBUG
						printf("SlotInfo slots: %d\n", static_cast<int>(nMaxSlots));
#endif
						m_pSlotInfo[i] = new SlotInfo[nMaxSlots];
						assert(m_pSlotInfo[i] != nullptr);

						for (uint32_t j = 0; j < nMaxSlots; j++) {
							m_pModeParams[i]->GetSlotInfo(j, m_pSlotInfo[i][j]);
#ifndef NDEBUG
							printf(" Slot:%d %2x:%4x\n", j, m_pSlotInfo[i][j].nType, m_pSlotInfo[i][j].nCategory);
#endif
						}
					} else {
						delete m_pAutoDriver[i];
						m_pAutoDriver[i] = nullptr;
						printf("Motor %d - Communication issues! Check SPI configuration and cables\n", i);
					}
				} else {
					delete m_pAutoDriver[i];
					m_pAutoDriver[i] = nullptr;
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
		if (m_pL6470DmxModes[i] != nullptr) {
			printf(" Motor %d\n", i);
			m_pL6470DmxModes[i]->InitSwitch();
		}
	}

	printf("busyCheck()\n");
	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pAutoDriver[i] != nullptr) {
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
		if (m_pL6470DmxModes[i] != nullptr) {
			printf(" Motor %d\n", i);
			m_pL6470DmxModes[i]->InitPos();
		}
	}

	DEBUG_EXIT;
}

void SparkFunDmx::SetData([[maybe_unused]] uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, [[maybe_unused]] const bool doUpdate) {
	DEBUG_ENTRY;
	assert(pData != nullptr);
	assert(nLength <= lightset::dmx::UNIVERSE_SIZE);

	bool bIsDmxDataChanged[SPARKFUN_DMX_MAX_MOTORS];

	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != nullptr) {
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

void SparkFunDmx::Sync([[maybe_unused]] uint32_t const nPortIndex) {
	//TODO Implement Sync
}

void SparkFunDmx::Sync() {
	//TODO Implement Sync
}

bool SparkFunDmx::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	DEBUG_ENTRY;

	if (nDmxStartAddress == m_nDmxStartAddress) {
		return true;
	}

	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if (m_pL6470DmxModes[i] != nullptr) {
			const auto nCurrentDmxStartAddress = m_pL6470DmxModes[i]->GetDmxStartAddress();
			const auto nNewDmxStartAddress = static_cast<uint16_t>((nCurrentDmxStartAddress - m_nDmxStartAddress) + nDmxStartAddress);
#ifndef NDEBUG
			printf("\tMotor=%d, Current DMX Start Address=%d, New DMX Start Address=%d\n", i, nCurrentDmxStartAddress, nNewDmxStartAddress);
#endif
			m_pL6470DmxModes[i]->SetDmxStartAddress(nNewDmxStartAddress);
			ModeStore::SaveDmxStartAddress(i, nNewDmxStartAddress);
		}
	}

	m_nDmxStartAddress = nDmxStartAddress;

	DEBUG_EXIT;
	return true;
}

bool SparkFunDmx::GetSlotInfo(uint16_t nSlotOffset, SlotInfo& tSlotInfo) {
	DEBUG_ENTRY;

	if (nSlotOffset > m_nDmxFootprint) {
		DEBUG_EXIT
		return false;
	}

	const uint16_t nDmxAddress = m_nDmxStartAddress + nSlotOffset;

	for (uint32_t i = 0; i < SPARKFUN_DMX_MAX_MOTORS; i++) {
		if ((m_pL6470DmxModes[i] != nullptr) && (m_pSlotInfo[i] != nullptr)) {
			const auto nOffset = static_cast<int16_t>(nDmxAddress - m_pL6470DmxModes[i]->GetDmxStartAddress());

			if ((nDmxAddress >= m_pL6470DmxModes[i]->GetDmxStartAddress()) && (nOffset < m_pL6470DmxModes[i]->GetDmxFootPrint())) {

				tSlotInfo.nType = m_pSlotInfo[i][nOffset].nType;
				tSlotInfo.nCategory = m_pSlotInfo[i][nOffset].nCategory;

				DEBUG_EXIT
				return true;
			}
		}
	}

	return false;
}
