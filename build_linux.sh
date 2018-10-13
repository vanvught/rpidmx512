#!/bin/bash

if which /opt/vc/bin/vcgencmd ; then
	./makeall_linux-lib.sh 'DEF=-DRASPPI' clean 
	./makeall_linux-lib.sh 'DEF=-DRASPPI'
else
	./makeall_linux-lib.sh clean 
	./makeall_linux-lib.sh
fi

if which /opt/vc/bin/vcgencmd ; then
	./makeall_linux.sh 'DEF=-DRASPPI' clean 
	./makeall_linux.sh 'DEF=-DRASPPI'
else
	./makeall_linux.sh clean 
	./makeall_linux.sh 
fi
