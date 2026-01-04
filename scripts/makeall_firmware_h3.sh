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
		elif [[ $f = "../opi_emac_node_dmx_multi" ]] && [[ $1 = *"ORANGE_PI_ONE"* ]] && [[ $2 = *"CONSOLE=CONSOLE_FB"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_node_pixel_dmx' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_node_pixel_dmx_multi' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_artnet_dmx' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = "../opi_emac_artnet_dmx_multi" ]] && [[ $1 = *"ORANGE_PI_ONE"* ]] && [[ $2 = *"CONSOLE=CONSOLE_FB"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_artnet_pixel_dmx' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_artnet_pixel_dmx_multi' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_e131_dmx' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = "../opi_emac_e131_dmx_multi" ]] && [[ $1 = *"ORANGE_PI_ONE"* ]] && [[ $2 = *"CONSOLE=CONSOLE_FB"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_e131_pixel_dmx' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = '../opi_emac_e131_pixel_dmx_multi' ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
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
		elif [[ $f = "../opi_emac_"* ]] && [[ $f != "../opi_"*"monitor" ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		elif [[ $f = "../opi_"*"l6470" ]] && [[ $1 != *"ORANGE_PI_ONE"* ]] && [[ $2 != *"NO_EXT_LED=1"* ]]; then
			do_build=1
		fi
		
		if [[ $f = "../opi_"*"debug"* ]]; then
			do_build=0
		fi
				
		if [[ $do_build -eq 1 ]]; then
			1>&2 echo $f $1 $2 $3
			echo $f $1 $2 $3
			board=0
			if [[ $1 = *"ORANGE_PI_ONE"* ]]; then
				board=1
			fi
			echo $f $1 $2 $3 > build$board.txt
			make -f Makefile.H3 $1 $2 $3 clean -j
			make -f Makefile.H3 $1 $2 $3
			retVal=$?
			if [ $retVal -ne 0 ]; then
    			echo "Error : " "$f"
				exit $retVal
			fi
		fi
			
		cd -
	fi
done
