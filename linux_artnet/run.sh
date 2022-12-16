#!/bin/bash

rm -rf ../lib-artnet/build_linux/
rm -rf ../lib-artnet/lib_linux/

rm -rf ../lib-dmxmonitor/build_linux/
rm -rf ../lib-dmxmonitor/lib_linux/

rm -rf build_linux

make
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error"
	exit $retVal
fi

sudo valgrind  --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./linux_artnet eno1
