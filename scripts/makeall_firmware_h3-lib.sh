#!/bin/bash

DIR=../lib-*

for f in $DIR
do
	echo -e "\e[32m[$f]\e[0m"
	cd "$f"
	
	if [ -f Makefile.H3 ]; then
		make -f Makefile.H3 $1 $2 || exit
	fi
	
	cd -

done
