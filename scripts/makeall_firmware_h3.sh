#!/bin/bash
echo $1 $2 $3

DIR=../opi_*

for f in $DIR
do
	do_build=0
	
	echo -e "\e[32m[$f]\e[0m"
	if [ -d $f ]; then
		cd "$f"

		echo $1 $2 $3
		
		if [[ $f = '../opi_dmx_usb_pro' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 = *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_artnet_dmx' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_artnet_pixel' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_e131_dmx' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_e131_pixel' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_osc_dmx' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_osc_pixel' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = '../opi_rdm_responder' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_llrp_tftpd' ]] && [[ $1 = *"ORANGE_PI_ONE"* ]] && [[ $2 = *"CONSOLE=CONSOLE_FB"* ]]; then
			do_build=1
		elif [[ $f = "../opi_"*"monitor" ]] && [[ $1 = *"ORANGE_PI_ONE"* ]] && [[ $2 = *"CONSOLE=CONSOLE_FB"* ]]; then
			do_build=1
		elif [[ $f = "../opi_emac_"*"dmx_multi" ]] && [[ $1 = *"ORANGE_PI_ONE"* ]] && [[ $2 = *"CONSOLE=CONSOLE_FB"* ]]; then
			do_build=1
		elif [[ $f = "../opi_emac_"* ]] && [[ $f != "../opi_"*"monitor" ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = "../opi_"*"l6470" ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = "../opi_"*"l6470" ]] && [[ $1 = *"ORANGE_PI_ONE"* ]] && [[ $2 = *"NO_EXT_LED=1"* ]]; then
			do_build=1
		fi
				
		if [[ $do_build -eq 1 ]]; then
			1>&2 echo $f $1 $2 $3
			echo $f $1 $2 $3
			board=0
			if [[ $1 = *"ORANGE_PI_ONE"* ]]; then
				board=1
			fi
			echo $f $1 $2 $3 > build$board.txt
			make -f Makefile.H3 $1 $2 $3 clean || exit
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
			
			echo $1 $2 $3
			
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
					make -f Makefile.H3 $1 $2 $3 clean || exit
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
