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



static void event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        printf("Clicked\n");
    }
    else if(event == LV_EVENT_VALUE_CHANGED) {
        printf("Toggled\n");
    }
}


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
 

	lv_port_disp_init();

//    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

    /*Initialize the (dummy) input device driver*/
//    lv_indev_drv_t indev_drv;
//    lv_indev_drv_init(&indev_drv);
//    indev_drv.type = LV_INDEV_TYPE_POINTER;
//    indev_drv.read_cb = my_touchpad_read;
//    lv_indev_drv_register(&indev_drv);

	/* Try an example from the lv_examples repository
		* https://github.com/lvgl/lv_examples*/
//	void lv_ex_btn_1(void)

//Create a label. Style is NULL by default for labels    
    lv_obj_t * label1 = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label1, "Function 1");
    lv_obj_t * label2 = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label2, "Section A");

    // Alignment
    // NULL = align on parent (the screen)
    lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_obj_align(label2, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

{
    lv_obj_t * label;
    lv_obj_t * btn1 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn1, event_handler);
    lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, 0, -40);
    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, "Button");
    lv_obj_t * btn2 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn2, event_handler);
    lv_obj_align(btn2, NULL, LV_ALIGN_CENTER, 0, 40);
    //lv_btn_set_toggle(btn2, true);
    lv_btn_toggle(btn2);
    lv_btn_set_fit2(btn2, LV_FIT_NONE, LV_FIT_TIGHT);
    label = lv_label_create(btn2, NULL);
    lv_label_set_text(label, "Toggled");
}



	hw.WatchdogInit();

	uint32_t nLastPollMillis = 0; 
	uint32_t nCurrentMillis = Hardware::Get()->Millis(); 
		
	for (;;) {
		hw.WatchdogFeed();
		nw.Run();
		ntpClient.Run();
		lb.Run();
		showSystime.Run();
		
		nCurrentMillis = Hardware::Get()->Millis();
		if (__builtin_expect((nCurrentMillis - nLastPollMillis >= 5),0){
		lv_tick_inc(nCurrentMillis - nLastPollMillis);
		nLastPollMillis = nCurrentMillis;
        lv_task_handler();
		}

	}
}

}
