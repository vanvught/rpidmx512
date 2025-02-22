#!/bin/bash

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

cd ..

find . -name "*.uImage" | xargs ls -al | wc -l
find . -name "*.uImage" | sort | xargs -I{} bash -c "do_check {}"
find . -name "*.uImage" | xargs ls -al | wc -l

cd -

