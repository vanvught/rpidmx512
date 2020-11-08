/**
 * @file main.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdio.h>
#include <stdint.h>

#include "hardware.h"
#include "networkh3emac.h"
#include "ledblink.h"

#include "h3/showsystime.h"

#include "ntpclient.h"

#include "display.h"

#include "networkconst.h"

#include "firmwareversion.h"
#include "software_version.h"

//#include "device/fb.h"


#include "fontem.h"
#include "font-7seg-24.h"

#include "drawing.h"
#include "ui.h"



// takes in a pointer to a Window class, used to get the content screen area from.  
void draw_content_test(void * win_ref) {
    if (!win_ref) 
		return;

	bool no_change = true;

	Window * win = static_cast<Window *>(win_ref);
	
	Drawing * draw = Drawing::Get();
	
	// get the on screen content rectangle we should in.
	uint32_t x = win->m_nContentX;
	uint32_t y = win->m_nContentY;

	if (no_change && (!win->m_bForceRepaint))
	 return;  // nothing to do...

    y += 3;
    draw->text(x+3, y, "Content:", UI_TITLEBAR_TXT_CLR);
    y += 16 + 2; // font height + 2;
    draw->text(x+3, y, "       Some Stuff", UI_TITLEBAR_TXT_CLR);
    y += 16 + 2; // fontheight + 2;
    draw->text(x+3, y, "       Some Things", UI_TITLEBAR_TXT_CLR);   
}

extern "C" {



void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	ShowSystime showSystime;

	Drawing draw;
	Window window1;
	Window window2;
	

	// background
	draw.fillRect(0,0,fb_width,fb_height,0x000000FF);
	
	console_puts("Drawing Test ");
	console_set_fg_color(CONSOLE_GREEN);
	{
	char buf [8];
	sprintf(buf, "%d", fb_width);
	console_puts(buf);
	console_puts(" x ");
	sprintf(buf, "%d", fb_height);
	console_puts(buf);	
	}
	console_set_fg_color(CONSOLE_WHITE);
	console_putc('\n');

	hw.SetLed(HARDWARE_LED_ON);
	nw.Init();
	nw.Print();
	
	NtpClient ntpClient;
	ntpClient.Init();
	ntpClient.Print();

	console_set_top_row(20);
 
	window1.setupWindow(500, 300, 200, 100, "Window Title", 0x00FFFFFF, 0, draw_content_test, window1.Get() );
	window2.setupWindow(200, 50, 300, 250, "DMX Universe", 0x00FFCE00, 0, draw_content_test, window2.Get());

	

	hw.WatchdogInit();

	uint32_t istime = hw.Millis();

	uint32_t startt, stopt;
	char buf[24];
	char buf2[24];	
	sprintf(&buf[0],"");
	sprintf(&buf2[0],"");

	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		ntpClient.Run();
		lb.Run();
		showSystime.Run();


		if (hw.Millis() > istime + 33){			
		startt = hw.Millis();
		//font_7seg_puts(window2.m_nContentX+10, window2.m_nContentY+100, &buf[0], 0x00000000);
		sprintf(&buf[0],"%d",startt);
		draw.fillRect(window2.m_nContentX+10, window2.m_nContentY+100, 200, 50 ,0);
		font_7seg_puts(window2.m_nContentX+10, window2.m_nContentY+100, &buf[0], 0x0000FF00);
		stopt = hw.Millis();
		font_7seg_puts(window2.m_nContentX+10, window2.m_nContentY+100+50, &buf2[0], 0x00000000);		
		sprintf(&buf2[0],"%d",stopt-startt);
		font_7seg_puts(window2.m_nContentX+10, window2.m_nContentY+100+50, &buf2[0], 0x0000FFFF);
		istime = hw.Millis();
		}
	}
}

}
