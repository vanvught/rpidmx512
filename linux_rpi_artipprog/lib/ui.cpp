/**
 * @file ui.cpp
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "ui.h"


#include "artnet.h"
#include "artnetipprog.h"

#include "network.h"

#include "input.h"
#include "inputset.h"

#include "display.h"
#include "displayset.h"

#define COLS	20
#define ROWS	4

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip_union;

Ui::Ui(const char *sVersion, const ArtNetController *pArtNetController, const InputSet *pInputSet): m_pDisplay(0), m_nStartIndex(1), m_State(STATE_INIT), m_nTimePrevious(0), m_nCursorRow(0), m_nCursorCol(0), m_ShowDetailIndex(0), m_nIPAddress(0), m_nSubmask(0), m_nDigitIndex(0), m_nDigit(0), m_nDigitValue(0) {
	m_pArtNetController = (ArtNetController *)pArtNetController;
	m_pInput = (InputSet *)pInputSet;

	m_nTimePrevious = time(NULL);

	memset(m_IPAddress, 0, sizeof m_IPAddress);
	memset(m_Submask, 0, sizeof m_Submask);
	memset(m_sDigit, 0, sizeof m_sDigit);

	m_pDisplay = new Display(COLS, ROWS);
	assert(m_pDisplay != 0);

	if (m_pDisplay->isDetected()) {
		m_pDisplay->Printf(1, "V%s ArtNet 3 IpProg", sVersion);
		m_pDisplay->Write(2, Network::Get()->GetHostName());
		m_pDisplay->Printf(3, IPSTR ,IP2STR(Network::Get()->GetIp()));
		m_pDisplay->Write(4, "Press <Enter>");
	}
}

Ui::~Ui(void) {
	if (m_pDisplay->isDetected()) {
		m_pDisplay->Cls();
	}
	delete m_pDisplay;
	m_pDisplay = 0;
}

void Ui::Show(void) {
	time_t now = time(NULL);

	if ((m_State != STATE_SHOW_CURSOR_OFF) || (now == m_nTimePrevious)) {
		return;
	}

	m_nTimePrevious = now;

	Update();
}

void Ui::Update(void) {
	uint8_t line = 1;
	uint8_t i;

	time_t now = time(NULL);

	if (m_pArtNetController->GetEntries() == 0) {
		m_pDisplay->Write(1, "Polling ...");
		return;
	}

	for (i = m_nStartIndex; (i <= m_pArtNetController->GetEntries()) && ((i - m_nStartIndex) <= (ROWS - 1)) ; i++) {
		struct TArtNetNodeEntry entry;
		m_pArtNetController->GetEntry(i, &entry);
		m_pDisplay->Printf(line++, "%2d:" IPSTR " %c%c%c", i, IP2STR(entry.IPAddress),
				(entry.Status2 & STATUS2_IP_DHCP) == STATUS2_IP_DHCP ? 'D' : 'S',
				(entry.IpProg.IPAddress == 0) ? ' ' : 'P',
				((now - entry.LastUpdate) > m_pArtNetController->GetPollInterval()) ? 'O' : 'A');
	}

	for (; line <= ROWS; line++) {
		m_pDisplay->ClearLine(line);
	}
}

void Ui::ShowDetails(void) {
	struct TArtNetNodeEntry entry;

	m_ShowDetailIndex = m_nCursorRow + m_nStartIndex;

	m_pArtNetController->GetEntry(m_ShowDetailIndex, &entry);

	m_pDisplay->Cls();

	ip_union.u32 = entry.IpProg.IPAddress;
	memcpy(&m_IPAddress, ip_union.u8, 4);
	m_pDisplay->Printf(1, IPSTR3 , IP2STR3(ip_union.u8));

	ip_union.u32 = entry.IpProg.SubMask;
	memcpy(&m_Submask, ip_union.u8, 4);
	m_pDisplay->Printf(2, IPSTR3 , IP2STR3(ip_union.u8));

	m_pDisplay->Write(3, (const char *)entry.LongName);
	m_pDisplay->Write(4, (const char *)entry.ShortName);

	m_pDisplay->SetCursorPos(0,0);

#ifndef NDEBUG
	time_t now = time(NULL);

	printf(IPSTR " %c%c\n", IP2STR(entry.IPAddress), (entry.Status2 & STATUS2_IP_DHCP) == STATUS2_IP_DHCP ? 'D' : 'S',  ((now - entry.LastUpdate) >  m_pArtNetController->GetPollInterval()) ? 'O' : 'A');
	printf(MACSTR "\n", MAC2STR(entry.Mac));
	puts((const char *)entry.LongName);
	puts((const char *)entry.ShortName);
#endif
}

void Ui::SetIpAddressDigit(void) {
	if (m_nCursorCol < 3) {
		m_nDigitIndex = 0;
		m_nDigit = m_nCursorCol;
	} else if (m_nCursorCol < 7) {
		m_nDigitIndex = 1;
		m_nDigit = m_nCursorCol - 4;
	}  else if (m_nCursorCol < 11) {
		m_nDigitIndex = 2;
		m_nDigit = m_nCursorCol - 8;
	} else {
		m_nDigitIndex = 3;
		m_nDigit = m_nCursorCol - 12;
	}

	if (m_nCursorRow == 0) {
		snprintf(m_sDigit, 4, "%.3d", m_IPAddress[m_nDigitIndex]);
#ifndef NDEBUG
		puts("Ui::SetIpAddressDigit, m_IPAddress");
#endif
	} else {
		snprintf(m_sDigit, 4, "%.3d", m_Submask[m_nDigitIndex]);
#ifndef NDEBUG
		puts("Ui::SetIpAddressDigit, m_Submask");
#endif
	}

	m_nDigitValue = m_sDigit[m_nDigit] - '0';

#ifndef NDEBUG
	printf("m_nDigitIndex = %d, [%s], m_nDigit = %d, m_nDigitValue = %d\n", m_nDigitIndex, m_sDigit, m_nDigit, m_nDigitValue);
#endif
}

void Ui::HandleIpAddressDown(void) {
	if (m_nDigit == 0) {
		if (m_nDigitValue == 0) {
			m_nDigitValue = 2;
		} else {
			m_nDigitValue--;
		}
	} else {
		if (m_nDigitValue == 0) {
			m_nDigitValue = 9;
		} else {
			m_nDigitValue--;
		}
	}

	UpdateIpAddressDigit();

}

void Ui::HandleIpAddressUp(void) {
	if (m_nDigit == 0) {
		if (m_nDigitValue == 2) {
			m_nDigitValue = 0;
		} else {
			m_nDigitValue++;
		}
	} else {
		if (m_nDigitValue == 9) {
			m_nDigitValue = 0;
		} else {
			m_nDigitValue++;
		}
	}

	UpdateIpAddressDigit();
}

void Ui::UpdateIpAddressDigit(void) {
#ifndef NDEBUG
	int i;
#endif

	m_sDigit[m_nDigit] = m_nDigitValue + '0';

	if (m_nCursorRow == 0) {
		m_IPAddress[m_nDigitIndex] = atoi(m_sDigit);
		memcpy(ip_union.u8, m_IPAddress, 4);
		m_nIPAddress = ip_union.u32;
#ifndef NDEBUG
		i = m_IPAddress[m_nDigitIndex];
#endif
	} else {
		m_Submask[m_nDigitIndex] = atoi(m_sDigit);
		memcpy(ip_union.u8, m_Submask, 4);
		m_nSubmask = ip_union.u32;
#ifndef NDEBUG
		i = m_Submask[m_nDigitIndex];
#endif
	}

	m_pDisplay->PutChar(m_nDigitValue + '0');
	m_pDisplay->SetCursorPos(m_nCursorCol, m_nCursorRow);

#ifndef NDEBUG
	printf("UpdateIpAddressDigit, m_nDigitIndex = %d, [%s]{%d}, m_nDigit = %d, m_nDigitValue = %d\n", m_nDigitIndex, m_sDigit, i, m_nDigit, m_nDigitValue);
	printf("\t" IPSTR "\n", IP2STR(m_nIPAddress));
	printf("\t" IPSTR "\n", IP2STR(m_nSubmask));
#endif
}

void Ui::HandleIpAddressChange(void) {
	struct TArtNetNodeEntry entry;

	m_pDisplay->SetCursor(SET_CURSOR_OFF);
	m_pArtNetController->GetEntry(m_ShowDetailIndex, &entry);

	m_ArtNetIpProg.Command = 0;

	if (m_nIPAddress != entry.IpProg.IPAddress) {
		m_ArtNetIpProg.Command |= IPPROG_COMMAND_PROGRAM_IPADDRESS;
		memcpy(&m_ArtNetIpProg.ProgIpHi, m_IPAddress, 4);
	}

	if (m_nSubmask != entry.IpProg.SubMask) {
		m_ArtNetIpProg.Command |= IPPROG_COMMAND_PROGRAM_SUBNETMASK;
		memcpy(&m_ArtNetIpProg.ProgSmHi, m_Submask, 4);
	}

	if (m_ArtNetIpProg.Command != 0) {
		m_pDisplay->Printf(1,"<ENTER> Send IpProg ");
		m_pDisplay->Printf(2,"<ESC>   Quit        ");
		m_pDisplay->ClearLine(3);
		m_pDisplay->ClearLine(4);
#ifndef NDEBUG
		printf("Ui::HandleIpAddressChange, ipprog Command = %.2x\n", m_ArtNetIpProg.Command);
#endif
	} else {
		m_State = STATE_SHOW_DETAILS;
	}
}

void Ui::SendIpProg(void) {
	struct TArtNetNodeEntry entry;

	m_pArtNetController->GetEntry(m_ShowDetailIndex, &entry);
#ifndef NDEBUG
	puts("Ui::SendIpProg, SendIpProg");
#endif
	m_pArtNetController->SendIpProg(entry.IPAddress, &m_ArtNetIpProg);

	m_State = STATE_SHOW_CURSOR_OFF;
	m_nCursorCol = 0;
	m_pDisplay->SetCursor(SET_CURSOR_OFF);
}

void Ui::KeyNumeric(const int c) {
	if (m_State == STATE_SHOW_CURSOR_ON) {
		// TODO STATE_SHOW_CURSOR_ON
	} else if ((m_State == STATE_IPADDRESS_EDIT) || (m_State == STATE_SUBMASK_EDIT) ){
		m_nDigitValue = c - INPUT_KEY_NUMERIC_0;
		UpdateIpAddressDigit();
	}
}

void Ui::KeyUp(void) {
	switch (m_State) {
	case STATE_SHOW_CURSOR_OFF:
		if (m_nStartIndex > 1) {
			m_nStartIndex--;
		}
		Update();
		break;
	case STATE_SHOW_CURSOR_ON:
		if (m_nCursorRow > 0) {
			m_nCursorRow--;
		}
		m_pDisplay->SetCursorPos(m_nCursorCol, m_nCursorRow);
		break;
	case STATE_IPADDRESS_EDIT:
		/* no break */
	case STATE_SUBMASK_EDIT:
		HandleIpAddressUp();
		break;
	default:
		break;
	}
}

void Ui::KeyDown(void) {
	switch (m_State) {
	case STATE_SHOW_CURSOR_OFF:
		if (m_nStartIndex < m_pArtNetController->GetEntries()) {
			m_nStartIndex++;
		}
		Update();
		break;
	case STATE_SHOW_CURSOR_ON:
		if (m_nCursorRow < (m_pArtNetController->GetEntries() - m_nStartIndex)) {
			m_nCursorRow++;
		}
		m_pDisplay->SetCursorPos(m_nCursorCol, m_nCursorRow);
		break;
	case STATE_IPADDRESS_EDIT:
		/* no break */
	case STATE_SUBMASK_EDIT:
		HandleIpAddressDown();
		break;
	default:
		break;
	}
}

void Ui::KeyLeft(void) {
	switch (m_State) {
	case STATE_IN_IPADDRESS:
		/* no break */
	case STATE_IN_SUBMASK:

		if ((m_nCursorCol == 4) || (m_nCursorCol == 8) 	|| (m_nCursorCol == 12)) {
			m_nCursorCol -= 2;
		} else if (m_nCursorCol == 0) {
			m_nCursorCol = 14;
		} else if (m_nCursorCol > 0) {
			m_nCursorCol--;
		}

		m_pDisplay->SetCursorPos(m_nCursorCol, m_nCursorRow);
		break;
	default:
		break;
	}
}

void Ui::KeyRight(void) {
	switch (m_State) {
	case STATE_IN_IPADDRESS:
		/* no break */
	case STATE_IN_SUBMASK:

		if ((m_nCursorCol == 2) || (m_nCursorCol == 6) || (m_nCursorCol == 10)) {
			m_nCursorCol += 2;
		} else if (m_nCursorCol == 14) {
			m_nCursorCol = 0;
		} else if (m_nCursorCol < 14) {
			m_nCursorCol++;
		}

		m_pDisplay->SetCursorPos(m_nCursorCol, m_nCursorRow);
		break;
	default:
		break;
	}
}

void Ui::KeyEnter(void) {
	switch (m_State) {
	case STATE_INIT:
		m_State = STATE_SHOW_CURSOR_OFF;
		m_pDisplay->Cls();
		break;
	case STATE_SHOW_CURSOR_OFF:
		if (m_pArtNetController->GetEntries() == 0) {
			return;
		}
		m_State = STATE_SHOW_CURSOR_ON;
		if (m_nCursorRow > (m_pArtNetController->GetEntries() - m_nStartIndex)) {
			m_nCursorRow = (m_pArtNetController->GetEntries() - m_nStartIndex);
		}
		m_pDisplay->SetCursorPos(m_nCursorCol, m_nCursorRow);
		m_pDisplay->SetCursor(SET_CURSOR_ON);
		break;
	case STATE_SHOW_CURSOR_ON:
		m_State = STATE_SHOW_DETAILS;
		m_pDisplay->SetCursor(SET_CURSOR_OFF);
		ShowDetails();
		break;
	case STATE_SHOW_DETAILS:
		m_State = STATE_IN_IPADDRESS;
		m_nCursorRow = 0;
		m_pDisplay->SetCursor(SET_CURSOR_ON);
		break;
	case STATE_IN_IPADDRESS:
		m_State = STATE_IPADDRESS_EDIT;
		m_pDisplay->SetCursor((TCursorMode)(SET_CURSOR_ON | SET_CURSOR_BLINK_ON));
		SetIpAddressDigit();
		break;
	case STATE_IN_SUBMASK:
		m_State = STATE_SUBMASK_EDIT;
		m_pDisplay->SetCursor((TCursorMode)(SET_CURSOR_ON | SET_CURSOR_BLINK_ON));
		SetIpAddressDigit();
		break;
	case STATE_SEND_IPPROG:
		SendIpProg();
		break;
	default:
		break;
	}
}

void Ui::KeyEsc(void) {
	switch (m_State) {
	case STATE_SHOW_DETAILS:
		/* no break */
	case STATE_SHOW_CURSOR_ON:
		/* no break */
	case STATE_SEND_IPPROG:
		m_State = STATE_SHOW_CURSOR_OFF;
		m_nCursorCol = 0;
		m_pDisplay->SetCursor(SET_CURSOR_OFF);
		break;
	case STATE_IN_IPADDRESS:
		/* no break */
	case STATE_IN_SUBMASK:
		m_State = STATE_SEND_IPPROG;
		HandleIpAddressChange();
		break;
	case STATE_IPADDRESS_EDIT:
		m_State = STATE_IN_IPADDRESS;
		m_pDisplay->SetCursor(SET_CURSOR_ON);
		break;
	case STATE_SUBMASK_EDIT:
		m_State = STATE_IN_SUBMASK;
		m_pDisplay->SetCursor(SET_CURSOR_ON);
		break;
	default:
		break;
	}
}

void Ui::Run(void) {
	if (m_pInput->IsAvailable()) {
		const int c = m_pInput->GetChar();

#ifndef NDEBUG
		printf("m_pInput->GetChar() = %d\n", c);

		DebugShowState();
#endif

		if (c >= INPUT_KEY_NUMERIC_0 && c <= INPUT_KEY_NUMERIC_9) {
			KeyNumeric(c);
		} else {
			switch (c) {
			case INPUT_KEY_ESC:
				KeyEsc();
				break;
			case INPUT_KEY_ENTER:
				KeyEnter();
				break;
			case INPUT_KEY_UP:
				KeyUp();
				break;
			case INPUT_KEY_DOWN:
				KeyDown();
				break;
			case INPUT_KEY_LEFT:
				KeyLeft();
				break;
			case INPUT_KEY_RIGHT:
				KeyRight();
				break;
			default:
#ifndef NDEBUG
				m_pArtNetController->Dump();
#endif
				break;
			}
		}
#ifndef NDEBUG
		DebugShowState();
#endif
	}

	Show();
}

#ifndef NDEBUG
void Ui::DebugShowState(void) {
	switch (m_State) {
	case STATE_INIT:
		puts("STATE_INIT");
		break;
	case STATE_SHOW_CURSOR_OFF:
		puts("STATE_SHOW_CURSOR_OFF");
		break;
	case STATE_SHOW_CURSOR_ON:
		puts("STATE_SHOW_CURSOR_ON");
		break;
	case STATE_SHOW_DETAILS:
		puts("STATE_SHOW_DETAILS");
		break;
	case STATE_IN_IPADDRESS:
		puts("STATE_IN_IPADDRESS");
		break;
	case STATE_IN_SUBMASK:
		puts("STATE_IN_SUBMASK");
		break;
	case STATE_IPADDRESS_EDIT:
		puts("STATE_IPADDRESS_EDIT");
		break;
	case STATE_SUBMASK_EDIT:
		puts("STATE_SUBMASK_EDIT");
		break;
	case STATE_SEND_IPPROG:
		puts("STATE_SEND_IPPROG");
		break;
	default:
		break;
	}
}
#endif
