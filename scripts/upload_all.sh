#!/bin/bash
echo $1

DIR=../opi_emac*
UIMAGE=orangepi_zero.uImage.gz

for f in $DIR
do
	echo -e "\e[32m[$f]\e[0m"
	if [ -d $f ]; then
		cd $f
		if [ -f $UIMAGE ]; then
			../scripts/do-tftp.sh $1
		fi
		cd - >/dev/null
		
		ON_LINE=$(echo '?list#' | nc -u -p 10501 -w 1 $1 10501) || true
		
		echo -e "\e[33m[$ON_LINE]\e[0m"
		
		while  [ "$ON_LINE" == "" ]
		do
			ON_LINE=$(echo '?list#' | nc -u -p 10501 -w 1 $1 10501)  || true
		done
		
		echo -e "\e[36m[$ON_LINE]\e[0m"
		
		sleep 1
	fi
done
