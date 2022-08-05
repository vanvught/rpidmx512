#!/bin/bash

rm -rf build_linux

make
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error"
	exit $retVal
fi

sudo valgrind  --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./osc eno1
