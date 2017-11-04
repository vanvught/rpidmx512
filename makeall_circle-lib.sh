#!/bin/bash

DIR=lib-*

for f in $DIR
do
	echo "[$f]"
	cd "$f"
	if [ -f Makefile.Circle ]
		then
			make -f Makefile.Circle $1 $2 || exit
	fi
	cd ..
done
