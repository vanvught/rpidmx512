#!/bin/bash

./makeall_circle-lib.sh RASPPI=1 clean 
./makeall_circle-lib.sh RASPPI=1 

./makeall_circle.sh RASPPI=1 clean 
./makeall_circle.sh RASPPI=1 

./makeall_circle-lib.sh RASPPI=2 clean 
./makeall_circle-lib.sh RASPPI=2 

./makeall_circle.sh RASPPI=2 clean 
./makeall_circle.sh RASPPI=2 

cd ..

find . -name "kernel*.img" | xargs ls -al | grep rpi

cd -
