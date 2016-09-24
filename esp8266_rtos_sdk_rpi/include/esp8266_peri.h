/*
 * https://github.com/esp8266/Arduino/blob/master/cores/esp8266/esp8266_peri.h
 */

#include <stdio.h>

#ifndef ESP8266_PERI_H_
#define ESP8266_PERI_H_

#define GPFFS0 4 //Function Select bit 0
#define GPFFS1 5 //Function Select bit 1
#define GPFFS2 8 //Function Select bit 2

#define GPC(p) ESP8266_REG(0x328 + ((p & 0xF) * 4))
#define GPCI   7  //INT_TYPE (3bits) 0:disable,1:rising,2:falling,3:change,4:low,5:high

#define ESP8266_REG(addr) *((volatile uint32_t *)(0x60000000+(addr)))
#define GPFFS(f) (((((f) & 4) != 0) << GPFFS2) | ((((f) & 2) != 0) << GPFFS1) | ((((f) & 1) != 0) << GPFFS0))
#define GPFFS_GPIO(p) (((p)==0||(p)==2||(p)==4||(p)==5)?0:((p)==16)?1:3)

#define GPOS   ESP8266_REG(0x304) //GPIO_OUT_SET WO
#define GPOC   ESP8266_REG(0x308) //GPIO_OUT_CLR WO

#define GPES   ESP8266_REG(0x310) //GPIO_ENABLE_SET WO
#define GPEC   ESP8266_REG(0x314) //GPIO_ENABLE_CLR WO

static uint8_t esp8266_gpioToFn[16] = { 0x34, 0x18, 0x38, 0x14, 0x3C, 0x40, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x04, 0x08, 0x0C, 0x10 };
#define GPF(p) ESP8266_REG(0x800 + esp8266_gpioToFn[(p & 0xF)])

#define GPI    ESP8266_REG(0x318) //GPIO_IN RO (Read Input Level)

#endif /* ESP8266_PERI_H_ */
