#!/bin/bash

make
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error"
	exit $retVal
fi

sudo valgrind  --track-origins=yes -v ./linux_e131 bond0
