/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

/*----------------------------------------------------------------------*/
/* FatFs sample project for generic microcontrollers (C)ChaN, 2012      */
/*----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <bcm2835.h>
#include <bcm2835_vc.h>
#include <bcm2835_wdog.h>
#include <bcm2835_uart.h>
#include "ff.h"

FATFS Fatfs;		/* File system object */
FIL Fil;			/* File object */
BYTE Buff[128];		/* File read buffer */

void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {}
void __attribute__((interrupt("FIQ"))) c_fiq_handler(void) {}

#ifdef ENABLE_FRAMEBUFFER
extern void bcm2835_console_begin(void);
#endif

void die(FRESULT rc) {
	printf("Failed with rc=%u.\n", rc);
	while (1)
		;
}

int notmain (void)
{
	FRESULT rc;				/* Result code */
	// DIR dir;				/* Directory object */
	// FILINFO fno;			/* File information object */
	// UINT br;

	bcm2835_uart_begin();

#ifdef ENABLE_FRAMEBUFFER
	bcm2835_console_begin();
#endif

    printf("Compiled on %s at %s\n", __DATE__, __TIME__);

	printf("SD Card power state: %ld\n", bcm2835_vc_get_power_state(BCM2835_VCMSG_POWER_ID_SDCARD));

	f_mount(0, &Fatfs);		/* Register volume work area (never fails) */

#if 0
	int i;
	printf("\nOpen an existing file (devices.txt).\n");
	rc = f_open(&Fil, "devices.txt", FA_READ);
	if (rc)
		die(rc);

	printf("\nType the file content.\n");
	for (;;) {
		rc = f_read(&Fil, Buff, sizeof Buff, &br); /* Read a chunk of file */
		if (rc || !br)
			break; /* Error or end of file */
		for (i = 0; i < br; i++) /* Type the data */
			putchar(Buff[i]);
	}
	if (rc)
		die(rc);

	printf("\nClose the file.\n");
	rc = f_close(&Fil);
	if (rc)
		die(rc);
#endif

#if 1
	printf("\nOpen an existing file (devices.txt).\n");
	rc = f_open(&Fil, "devices.txt", FA_READ);
	if (rc)
		die(rc);

	printf("\nType the file content.\n");
	int i = 1;
	for (;;) {
		if (f_gets(Buff, sizeof Buff, &Fil) == NULL)
			break; /* Error or end of file */
		printf("%d\t:%s\n", i, Buff);
		i++;
	}
	if (rc)
		die(rc);

	printf("\nClose the file.\n");
	rc = f_close(&Fil);
	if (rc)
		die(rc);
#endif

#if 0
	printf("\nCreate a new file (hello.txt).\n");
	rc = f_open(&Fil, "hello.txt", FA_WRITE | FA_CREATE_ALWAYS);
	if (rc) die(rc);

	printf("\nWrite a text data. (Hello world!)\n");
	rc = f_write(&Fil, "Hello world!\n", 14, &bw);
	if (rc) die(rc);
	printf("%u bytes written.\n", bw);

	printf("\nClose the file.\n");
	rc = f_close(&Fil);
	if (rc) die(rc);
#endif

#if 0
	printf("\nOpen root directory.\n");
	rc = f_opendir(&dir, "");
	if (rc) die(rc);

	char filename_buf[32];

	printf("\nDirectory listing...\n");
	for (;;) {
		fno.lfname = filename_buf;
		fno.lfsize = sizeof(filename_buf);
		rc = f_readdir(&dir, &fno);		/* Read a directory item */
		if (rc || !fno.fname[0]) break;	/* Error or end of dir */
		if (fno.fattrib & AM_DIR)
			printf("   <dir>  %s\n", fno.fname);
		else {
			char * fn = *fno.lfname ? fno.lfname : fno.fname;
			printf("%8lu  %s\n", fno.fsize, (char *) fn);
		}
	}
	if (rc) die(rc);
#endif

	printf("SD Card power state: %ld\n", bcm2835_vc_get_power_state(BCM2835_VCMSG_POWER_ID_SDCARD));

	printf("\nTest completed.\n");

	watchdog_init();

	return 0;
}
