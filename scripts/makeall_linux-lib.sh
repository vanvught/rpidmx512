#!/bin/bash

DIR=../lib-*

for f in $DIR
do
	echo -e "\033[32m[$f]\033[0m"
	cd "$f"
	
	if [ -f Makefile.Linux ]; then
		
		if [ $(grep -c bcm Makefile.Linux) -ne 0 ] || [ $(grep -c i2c  Makefile.Linux) -ne 0 ] ; then
			if which /opt/vc/bin/vcgencmd ; then
				make -f Makefile.Linux $1 $2 || exit
			else
				echo -e "\033[33mSkipping...\033[0m"
				cd -
				continue
			fi
		fi
		
		if [[ $f = *"bcm"* ]] ; then
			if which /opt/vc/bin/vcgencmd ; then
				make -f Makefile.Linux $1 $2 || exit
			else
				echo -e "\033[33mSkipping...\033[0m"
				cd -
				continue
			fi
		fi		
		
		make -f Makefile.Linux $1 $2 || exit	
	fi
	
	cd -

done
