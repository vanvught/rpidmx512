#!/bin/bash

# build the firmware in the current directory 
# and add the last line to the first line to upload it via do-tftp.sh

# example usage: 
#    cd /development/workspace/opi_emac_ltc_smpte/
#    ../scripts/build_this.sh

make --quiet -f Makefile.H3 clean && make --quiet -f Makefile.H3 

# due to a uboot related issue, firmware sizes are restricted
# this change here allows upto 0x26000 bytes (155648 bytes)
# https://github.com/vanvught/rpidmx512/issues/68#issuecomment-541406598

# compress the image
filename=./orangepi_zero.uImage.gz

if [ -f "$filename" ]; then
    echo
    #gzip -n --best -c orangepi_zero.img > orangepi_zero.img.gz && mkimage -n 'http://www.orangepi-dmx.org' -A arm -O u-boot -T standalone -C gzip -a 0x40000000 -d orangepi_zero.img.gz orangepi_zero.uImage.gz
    
    maxsize=155648
    filesize=$(stat -c%s "$filename")
    echo
    echo "Size of $filename = $filesize bytes."

    if (( filesize > maxsize )); then
        echo "Firmware is too big for SPI Flash, maximum $maxsize."
    else
        ../scripts/do-tftp.sh 192.168.1.82 
    fi

fi