#!/bin/bash

DIR=../lib-*

for f in $DIR
do
	echo -e "\e[32m[$f]\e[0m"
	cd "$f"
	
	if [ -f Makefile.BCM ]; then
		make -f Makefile.BCM -j 4 $1 $2
		retVal=$?
		if [ $retVal -ne 0 ]; then
    		echo "Error : " "$f"
			exit $retVal
		fi
	fi
	
	cd -
done
