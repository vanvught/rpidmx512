#!/bin/bash

SECONDS=0

cd ..

find . -name "*.uImage" | xargs ls -al | wc -l
find . -name "*.uImage" | xargs rm

cd -

array=('PLATFORM=ORANGE_PI' 'PLATFORM=ORANGE_PI NO_EXT_LED=1' 'PLATFORM=ORANGE_PI_ONE CONSOLE=CONSOLE_FB')

#array=('PLATFORM=ORANGE_PI')
#array=('PLATFORM=ORANGE_PI NO_EXT_LED=1')
#array=('PLATFORM=ORANGE_PI_ONE CONSOLE=CONSOLE_FB')
#array=('PLATFORM=ORANGE_PI_ONE NO_EXT_LED=1')

do_check()
{
	maxsize=139264
	filesize=$(stat -c%s "$1")
    if (( filesize > maxsize )); then
    	echo -e "\e[33m[$1 is too big for SPI Flash -> $filesize]\e[0m"
        rm -rf $1
        ls -al "$1.gz"
        filesize=$(stat -c%s "$1.gz")
        if (( filesize > maxsize )); then
    		echo -e "\e[31m[Error: $1.gz is too big for SPI Flash -> $filesize]\e[0m"
    	fi
	else
		ls -al "$1"
	fi	
}

export -f do_check

for i in "${array[@]}"
do
	echo $i
	./makeall_firmware_h3.sh $i
	retVal=$?
	if [ $retVal -ne 0 ]; then
    	echo "Error"
		exit $retVal
	fi
done

cd ..

find . -name sofware_version_id.h -delete
find . -name "*.uImage" | xargs ls -al | wc -l
find . -name "*.uImage" | sort | xargs -I{} bash -c "do_check {}"
find . -name "*.uImage" | xargs ls -al | wc -l

cd -
echo $SECONDS
