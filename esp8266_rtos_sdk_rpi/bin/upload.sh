#!/bin/sh
esptool.py -b 115200 write_flash 0 boot_v1.5.bin 0x1000 upgrade/user1.bin 0x81000 upgrade/user2.bin