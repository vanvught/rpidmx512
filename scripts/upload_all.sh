#!/bin/bash
echo $1

DIR=../opi_emac*
UIMAGE=orangepi_zero.uImage.gz

array=(
	'remote' 
	'global' 
	'network' 
)

for f in $DIR
do
	echo -e "\e[32m[$f]\e[0m"
	if [ -d $f ]; then
		cd $f
		if [ -f $UIMAGE ]; then
			do_tftp-zero.py $1
			
			ON_LINE=$(echo '?list#' | nc -u -p 10501 -w 1 $1 10501) || true
					
			while  [ "$ON_LINE" == "" ]
			do
				ON_LINE=$(echo '?list#' | nc -u -p 10501 -w 1 $1 10501)  || true
			done
			
			echo -e "\e[36m[$ON_LINE]\e[0m"
 
			wget -O - http://$1/json/config/directory
			
			for i in "${array[@]}"
			do
				echo $i
				wget -O - http://$1/json/config/$i
				echo -e "\e[32m$TXT_FILE\e[0m"
			done
		fi
		cd - >/dev/null
	fi
done
