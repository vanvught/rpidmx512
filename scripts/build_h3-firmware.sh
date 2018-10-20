#!/bin/bash

array=( 'PLATFORM=ORANGE_PI' 'PLATFORM=ORANGE_PI_ONE CONSOLE=CONSOLE_ILI9340')

do_build() 
{
./makeall_firmware_h3-lib.sh clean $1 $2
./makeall_firmware_h3-lib.sh $1 $2

./makeall_firmware_h3.sh clean $1 $2
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
