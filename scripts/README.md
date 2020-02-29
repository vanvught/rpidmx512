
Build all libraries and firmware
================================

- build_h3-firmware.sh - build for Allwinner SoC's H2+/H3 (OrangePi Zero and OrangePi One)
- build_linux.sh - build for host PC (testing and debugging)
- build_firmware.sh - build for Broadcom SoC's (Raspberry Pi boards, minimum support)

Assistants - Please read / modify these script files
====================================================

    build_this.sh
 
Used to build one H3 firmware from it's directory and optionally upload it, edit to specify IP address. </br>For example : ./opi_emac_ltc_smpte$ ../scripts/build_this.sh
 	
	do-tftp.sh

Upload to device via tftp, edit to select upload to either OPi Zero or OPi One

Used by build scripts
=====================
- libs-clean.sh
- makeall_firmware_h3-lib.sh
- makeall_firmware_h3.sh
- makeall_linux-lib.sh
- makeall_linux.sh
- makeall_firmware-lib.sh
- makeall_firmware.sh
