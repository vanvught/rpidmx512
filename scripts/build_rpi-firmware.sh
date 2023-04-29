#!/bin/bash

SECONDS=0

DIR=../rpi_dmx_usb_pro

for f in $DIR
do
	echo -e "\e[32m[$f]\e[0m"
	if [ -d $f ]; then
		cd "$f"
			
		if [ -f Makefile.BCM ]; then
			make -f Makefile.BCM clean $1 $2
			retVal=$?
			if [ $retVal -ne 0 ]; then
    			echo "Error : " "$f"
				exit $retVal
			fi
			make -f Makefile.BCM $1 $2
			retVal=$?
			if [ $retVal -ne 0 ]; then
    			echo "Error : " "$f"
				exit $retVal
			fi
		fi
			
		cd -
	fi
done

cd ..

find . -name "kernel*.img" | xargs ls -al | grep rpi

cd -

echo $SECONDS