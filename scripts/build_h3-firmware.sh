#!/bin/bash

array=('PLATFORM=ORANGE_PI' 'PLATFORM=ORANGE_PI NO_EXT_LED=1' 'PLATFORM=ORANGE_PI_ONE CONSOLE=CONSOLE_FB' 'PLATFORM=ORANGE_PI_ONE NO_EXT_LED=1')

#array=('PLATFORM=ORANGE_PI')
#array=('PLATFORM=ORANGE_PI_ONE CONSOLE=CONSOLE_FB')
#array=('PLATFORM=ORANGE_PI_ONE NO_EXT_LED=1')

do_build() 
{
./makeall_firmware_h3.sh $1 $2 clean
./makeall_firmware_h3.sh $1 $2
}

for i in "${array[@]}"
do
	echo $i
	do_build $i
done

cd ..

find . -name "*.uImage" | xargs ls -al

cd -
