#!/bin/bash
DIR=lib-*
for f in $DIR
do
	echo "[$f]"
	cd "$f"
	if [ -f Makefile.Linux ]
		then
			make -f Makefile.Linux $1 $2 || exit
	fi
	cd ..
done
