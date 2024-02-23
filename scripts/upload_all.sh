#!/bin/bash
echo $1

DIR=../opi_emac*
UIMAGE=orangepi_zero.uImage.gz

array=('rconfig.txt' 'display.txt' 'network.txt' 'artnet.txt' 'e131.txt' 'devices.txt' 'params.txt' 'rdm_device.txt' 'ltc.txt' 'osc.txt' 'sensors.txt' 'subdev.txt' 'serial.txt' 'show.txt' 'tcnet.txt' 'show.txt')

for f in $DIR
do
	echo -e "\e[32m[$f]\e[0m"
	if [ -d $f ]; then
		cd $f
		if [ -f $UIMAGE ]; then
			../scripts/do-tftp.sh $1
			
			ON_LINE=$(echo '?list#' | nc -u -p 10501 -w 1 $1 10501) || true
					
			while  [ "$ON_LINE" == "" ]
			do
				ON_LINE=$(echo '?list#' | nc -u -p 10501 -w 1 $1 10501)  || true
			done
			
			echo -e "\e[36m[$ON_LINE]\e[0m"
 
			wget -q -O - http://$1/json/directory
			
			for i in "${array[@]}"
			do
				echo $i
				TXT_FILE=$(echo ?get#$i | nc -u -p 10501 -w 1 $1 10501)  || true
				echo -e "\e[32m$TXT_FILE\e[0m"
			done
		fi
		cd - >/dev/null
	fi
done
