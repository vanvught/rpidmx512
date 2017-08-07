#!/bin/bash
DIR=linux_*
for f in $DIR
do
	echo "[$f]"
	if [ -d $f ]; then
		if [[ $f != *"circle"* ]]; then
			cd "$f"
			if [ -f Makefile ]
				then
					make $1 $2 || exit
			fi
			cd ..
		fi
	fi
done
