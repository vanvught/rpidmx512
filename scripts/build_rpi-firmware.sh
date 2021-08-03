#!/bin/bash

./makeall_firmware_bcm-lib.sh clean 
./makeall_firmware_bcm-lib.sh

./makeall_firmware_bcm.sh clean 
./makeall_firmware_bcm.sh 

cd ..

find . -name "kernel*.img" | xargs ls -al | grep rpi

cd -

