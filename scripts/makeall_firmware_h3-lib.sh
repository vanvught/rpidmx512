#!/bin/bash
echo $1 $2 $3

DIR=../lib-*

for f in $DIR
do
	echo -e "\e[32m[$f]\e[0m"
	cd "$f"
	
	if [ -f Makefile.H3 ]; then
#		make -f Makefile.H3 $1 $2 $3 clean || exit
		make -f Makefile.H3 -j3 $1 $2 $3 || exit
	fi
	
	cd -

done
