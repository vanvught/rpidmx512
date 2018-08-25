#!/bin/bash

DIR=lib-*

for f in $DIR
do
	echo -e "\e[32m[$f]\e[0m"
	cd "$f"
	if [ -f Makefile.Linux ]; then
		if [ $(grep -c bcm Makefile.Linux) -ne 0 ] || [ $(grep -c i2c  Makefile.Linux) -ne 0 ] ; then
			if which /opt/vc/bin/vcgencmd ; then
				make -f Makefile.Linux $1 $2 || exit
			else
				echo -e "\e[33mSkipping...\e[0m"
				cd ..
				continue
			fi
		fi
		
		if [[ $f = *"bcm"* ]] ; then
			if which /opt/vc/bin/vcgencmd ; then
				make -f Makefile.Linux $1 $2 || exit
			else
				echo -e "\e[33mSkipping...\e[0m"
				cd ..
				continue
			fi
		fi		
		
		
		make -f Makefile.Linux $1 $2 || exit	
	fi
	cd ..
done
