#!/bin/bash

cd ..

find . -name "*.uImage" | xargs ls -al | wc -l
find . -name "*.uImage" | xargs rm

cd -

array=('PLATFORM=ORANGE_PI' 'PLATFORM=ORANGE_PI NO_EXT_LED=1' 'PLATFORM=ORANGE_PI_ONE CONSOLE=CONSOLE_FB' 'PLATFORM=ORANGE_PI_ONE NO_EXT_LED=1')

#array=('PLATFORM=ORANGE_PI')
#array=('PLATFORM=ORANGE_PI NO_EXT_LED=1')
#array=('PLATFORM=ORANGE_PI_ONE CONSOLE=CONSOLE_FB')
#array=('PLATFORM=ORANGE_PI_ONE NO_EXT_LED=1')

do_build() 
{
	./makeall_firmware_h3.sh $1 $2
}

do_check()
{
	maxsize=139264
	filesize=$(stat -c%s "$1")
    if (( filesize > maxsize )); then
    	echo -e "\e[31m[$1 is too big for SPI Flash -> $filesize]\e[0m"
        rm -rf $1
        ls -al "$1.gz"
	else
		ls -al "$1"
	fi	
}

export -f do_check

for i in "${array[@]}"
do
	echo $i
		do_build $i
done

cd ..

find . -name sofware_version_id.h | xargs rm

#find . -name "*.uImage" | xargs ls -al
find . -name "*.uImage" | xargs ls -al | wc -l
find . -name "*.uImage" | sort | xargs -I{} bash -c "do_check {}"
find . -name "*.uImage" | xargs ls -al | wc -l

cd -
