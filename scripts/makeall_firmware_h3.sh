#!/bin/bash

DIR=../opi_*

for f in $DIR
do
	echo -e "\e[32m[$f]\e[0m"
	if [ -d $f ]; then
		cd "$f"
		
		if [ $(grep -c NO_EMAC Makefile.H3) -ne 0 ] && [[ $1 = *"ORANGE_PI_ONE"* ]]; then
			echo -e "\e[33mSkipping...\e[0m"
		else
			make -f Makefile.H3 $1 $2 || exit
		fi
			
		cd -
	fi
done

DIR=../rpi_*

for f in $DIR
do
	echo -e "\e[32m[$f]\e[0m"
	if [ -d $f ]; then
		
		if [[ $f != *"circle"* ]]; then
			cd "$f"
			
			if [ -f Makefile.H3 ]; then
				if [ $(grep -c NO_EMAC Makefile.H3) -ne 0 ] && [[ $1 = *"ORANGE_PI_ONE"* ]]; then
					echo -e "\e[33mSkipping...\e[0m"
				else
					make -f Makefile.H3 $1 $2 || exit
				fi
			fi
			
			cd -
		fi
	fi
done
