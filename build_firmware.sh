#!/bin/bash

./makeall_firmware-lib.sh clean 
./makeall_firmware-lib.sh

./makeall_firmware.sh clean 
./makeall_firmware.sh 

./create_sdcard_zip_file.sh
