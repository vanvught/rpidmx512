#!/bin/bash
echo $1 $2 $3

DIR=../opi_*

for f in $DIR
do
	echo -e "\e[32m[$f]\e[0m"
	if [ -d $f ]; then
		cd "$f"
		
		if [ $(grep -c NO_EMAC Makefile.H3) -ne 0 ] && [[ $1 = *"ORANGE_PI_ONE"* ]]; then
			echo -e "\e[33mSkipping...\e[0m"
		elif [ $(grep -c NO_EMAC Makefile.H3) -ne 0 ] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			echo -e "\e[33mSkipping...\e[0m"
		elif [ $(grep -c LTC_READER Makefile.H3) -ne 0 ] && [[ $1 = *"ORANGE_PI_ONE"* ]]; then
			echo -e "\e[33mSkipping...\e[0m"
		elif [ $(grep -c PIXEL_MULTI Makefile.H3) -ne 0 ] && [[ $1 = *"ORANGE_PI_ONE"* ]]; then
			echo -e "\e[33mSkipping...\e[0m"
		elif [ $(grep -c ARTNET_NODE Makefile.H3) -ne 0 ] && [ $(grep -c STEPPER Makefile.H3) -eq 0 ] && [[ $2 = *"NO_EXT_LED=1"* ]]; then
			echo -e "\e[33mSkipping...\e[0m"
		elif [ $(grep -c E131_BRIDGE Makefile.H3) -ne 0 ] && [[ $2 = *"NO_EXT_LED=1"* ]]; then
			echo -e "\e[33mSkipping...\e[0m"
		elif [ $(grep -c OSC_SERVER Makefile.H3) -ne 0 ] && [[ $2 = *"NO_EXT_LED=1"* ]]; then
			echo -e "\e[33mSkipping...\e[0m"
		elif [ $(grep -c LTC_READER Makefile.H3) -ne 0 ] && [[ $2 = *"NO_EXT_LED=1"* ]]; then
			echo -e "\e[33mSkipping...\e[0m"
		elif [ $(grep -c STEPPER Makefile.H3) -ne 0 ] && [[ $2 = *"CONSOLE_FB"* ]]; then
			echo -e "\e[33mSkipping...\e[0m"
		else
			echo $1 $2 $3
			make -f Makefile.H3 $1 $2 $3 || exit
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
				if [ $(grep -c ESP8266 Makefile.H3) -ne 0 ] && [[ $1 = *"ORANGE_PI_ONE"* ]]; then
					echo -e "\e[33mSkipping...\e[0m"
				elif [ $(grep -c ESP8266 Makefile.H3) -ne 0 ] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
					echo -e "\e[33mSkipping...\e[0m"
				elif [ $(grep -c RDM_CONTROLLER Makefile.H3) -ne 0 ] && [[ $1 = *"ORANGE_PI_ONE"* ]]; then
					echo -e "\e[33mSkipping...\e[0m"					
				elif [ $(grep -c MONITOR_DMX Makefile.H3) -ne 0 ] && [[ $1 = "PLATFORM=ORANGE_PI" ]]; then
					echo -e "\e[33mSkipping...\e[0m"
				elif [ $(grep -c MONITOR_DMX Makefile.H3) -ne 0 ] && [[ $2 = *"NO_EXT_LED=1"* ]]; then
					echo -e "\e[33mSkipping...\e[0m"
				elif [ $(grep -c RDM_RESPONDER Makefile.H3) -ne 0 ] && [[ $1 = *"ORANGE_PI_ONE"* ]]; then
					echo -e "\e[33mSkipping...\e[0m"
				elif [ $(grep -c RDM_RESPONDER Makefile.H3) -ne 0 ] && [[ $2 = *"NO_EXT_LED=1"* ]]; then
					echo -e "\e[33mSkipping...\e[0m"
				else
					echo $1 $2 $3
					make -f Makefile.H3 $1 $2 $3 || exit
				fi
			else
				echo -e "\e[33mSkipping...\e[0m"
			fi
			
			cd -
		else
			echo -e "\e[33mSkipping...\e[0m"
		fi
	fi
done
