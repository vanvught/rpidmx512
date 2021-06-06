#!/bin/bash

SECONDS=0
NPROC=1

if [ "$(uname)" == "Darwin" ]; then
     NPROC=$(sysctl -a | grep machdep.cpu.core_count | cut -d ':' -f 2)     
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
     NPROC=$(nproc)
fi

DIR=../linux_*

for f in $DIR
do
	echo -e "\033[32m[$f]\033[0m"
	if [ -d $f ]; then
		if [[ $f != *"lib"* ]]; then
			if [[ $f = *"rpi"* ]]; then
				if which /opt/vc/bin/vcgencmd ; then		
					cd "$f"
					if [ -f Makefile ]
						then
							make clean $1 $2 && make -j $NPROC $1 $2 
					fi
					cd -
				else
					echo -e "\033[33mSkipping...\033[0m"
					continue
				fi
			fi
			cd "$f"
			if [ -f Makefile ]
				then
					make clean $1 $2 && make -j $NPROC $1 $2 
			fi
			cd -
		fi
	fi
done

for f in $DIR
do
	echo -e "\033[32m[$f]\033[0m"
	if [ -d $f ]; then
		if [[ $f != *"lib"* ]]; then
			if [[ $f = *"rpi"* ]]; then
				if which /opt/vc/bin/vcgencmd ; then		
					cd "$f"
					if [ -f Makefile ]
						then
							ls -al linux_rpi*
					fi
					cd -
				else
					echo -e "\033[33mSkipping...\033[0m"
					continue
				fi
			fi
			cd "$f"
			if [ -f Makefile ]
				then
					ls -al linux_*
			fi
			cd -
		fi
	fi
done

echo $SECONDS
