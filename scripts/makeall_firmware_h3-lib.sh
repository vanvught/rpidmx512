#!/bin/bash

SECONDS=0
NPROC=1

if [ "$(uname)" == "Darwin" ]; then
     NPROC=$(sysctl -a | grep machdep.cpu.core_count | cut -d ':' -f 2)     
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
     NPROC=$(nproc)
fi

echo $1 $2 $3

DIR=../lib-*

for f in $DIR
do
	echo -e "\e[32m[$f]\e[0m"
	cd "$f"
	
	if [ -f Makefile.H3 ]; then
		make -f Makefile.H3 -j $NPROC $1 $2 $3 
	fi
	
	cd -

done

echo $SECONDS