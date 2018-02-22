#!/bin/bash

./makeall_linux-lib.sh clean 
./makeall_linux-lib.sh 'DEF=-DRASPPI'

./makeall_linux.sh clean 
./makeall_linux.sh 'DEF=-DRASPPI'

