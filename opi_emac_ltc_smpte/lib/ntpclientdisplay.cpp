/*
 * ntpclientdisplay.cpp
 */

#include "ntpclient.h"

#include "display.h"
#include "display7segment.h"

void NtpClientDisplay::ShowNtpClientStatus(ntpclient::Status nStatus) {
	if (nStatus == ntpclient::Status::IDLE) {
		Display::Get()->TextStatus("NTP Client", Display7SegmentMessage::INFO_NTP);
		return;
	}

	if (nStatus == ntpclient::Status::FAILED) {
		Display::Get()->TextStatus("Error: NTP", Display7SegmentMessage::ERROR_NTP);
		return;
	}
}
