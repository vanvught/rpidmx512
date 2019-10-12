#!/bin/bash

# build the firmware in the current directory 
# and add the last line to the first line to upload it via do-tftp.sh

# example usage: 
#    cd /development/workspace/opi_emac_ltc_smpte/
#    ../scripts/build_this.sh

make --quiet -f Makefile.H3 clean && make --quiet -f Makefile.H3 # && ../scripts/do-tftp.sh 192.168.1.98