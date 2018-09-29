#!/bin/bash

array=( 'PLATFORM=ORANGE_PI' 'PLATFORM=NANO_PI' 'PLATFORM=ORANGE_PI_ONE')

do_build() 
{
./makeall_firmware_h3-lib.sh clean $1
./makeall_firmware_h3-lib.sh $1

./makeall_firmware_h3.sh clean $1
./makeall_firmware_h3.sh $1
}

for i in "${array[@]}"
do
	echo $i
	do_build $i
done

find . -name "*.uImage" | xargs ls -al
