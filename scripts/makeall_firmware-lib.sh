#!/bin/bash

DIR=../lib-*

for f in $DIR
do
	echo -e "\e[32m[$f]\e[0m"
	cd "$f"
	
	if [ -f Makefile ]; then
		make -j 4 $1 $2 || exit
	fi
	
	cd -

done
