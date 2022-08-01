#!/bin/bash

make
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error"
	exit $retVal
fi

sudo valgrind  --leak-check=full --show-leak-kinds=all --track-origins=yes -v ./linux_pp eno1
