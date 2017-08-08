#!/bin/bash

cd lib-artnet
make -f Makefile.Circle $1 $2 || exit

cd ../lib-ledblink
make -f Makefile.Circle $1 $2 || exit

cd ../lib-lightset
make -f Makefile.Circle $1 $2 || exit

cd ../lib-network
make -f Makefile.Circle $1 $2 || exit

cd ../lib-osc
make -f Makefile.Circle $1 $2 || exit

cd ..

DIR=rpi_circle_lib*
for f in $DIR
do
	echo "[$f]"
	cd "$f"
	if [ -f Makefile ]
		then
			make $1 $2 || exit
	fi
	cd ..
done
