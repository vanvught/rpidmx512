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

#include "drawing.h"

#include "../lib/lvgl/lvgl.h"
#include "lv_port_disp.h"

extern "C" {



void notmain(void) {
	Hardware hw;
	NetworkH3emac nw;
	LedBlink lb;
	
	FirmwareVersion fw(SOFTWARE_VERSION, __DATE__, __TIME__);
	ShowSystime showSystime;

	Drawing draw;

	// background
	draw.fillRect(0,0,fb_width,fb_height,0x000000FF);
	
	console_puts("Drawing Test ");
	console_set_fg_color(CONSOLE_GREEN);
	console_puts("Check 123");
	console_set_fg_color(CONSOLE_WHITE);
	console_putc('\n');


	hw.SetLed(HARDWARE_LED_ON);
	nw.Init();
	nw.Print();
	
	NtpClient ntpClient;
	ntpClient.Init();
	ntpClient.Print();

	console_set_top_row(20);

	draw.rect(500,20,200,100, 0x0000FFFF); // yellow rectangle
	draw.line(0,0,fb_width,fb_height,0x00FFFFFF); // top left to bottom right
	draw.line(fb_width,0,0,fb_height,0x00FFFFFF); // top right to bottom left
	draw.circle(200,200,100,0x0000FF00); // green circle outline
	draw.fillCircle(250,200,50,0x00FF0000); // red, filled circle
	draw.triangle(300,300,350,200,400,300, 0x00FF00FF); // draw a triangle outline
 

	hw.WatchdogInit();

	
	
	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		ntpClient.Run();
		lb.Run();
		showSystime.Run();
			
	



	}
}

}
