#!/bin/bash

# build the firmware in the current directory 
# and add the last line to the first line to upload it via do-tftp.sh

# example usage: 
#    cd /development/workspace/opi_emac_ltc_smpte/
#    ../scripts/build_this.sh

make --quiet -f Makefile.H3 clean && make --quiet -f Makefile.H3 

# compressed image
filename=./orangepi_zero.uImage.gz

if [ -f "$filename" ]; then
    echo
    
    maxsize=139264
    filesize=$(stat -c%s "$filename")
    echo
    echo "Size of $filename = $filesize bytes."

    if (( filesize > maxsize )); then
        echo "Firmware is too big for SPI Flash, maximum $maxsize."
    else
        ../scripts/do-tftp.sh 192.168.1.56 
    fi
fi
