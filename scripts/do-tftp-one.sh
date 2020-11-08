#!/bin/bash

ON_LINE=
		
echo -e "\e[33m[$ON_LINE]\e[0m"
		
while  [ "$ON_LINE" == "" ]
	do
		echo '!tftp#1' | nc -u -p 10501 -w 1 $1 10501
		ON_LINE=$(echo '?tftp#' | nc -u -p 10501 -w 1 $1 10501)  || true
		echo -e "\e[33m[$ON_LINE]\e[0m"
	done

tftp $1 << -EOF
binary
put orangepi_one.uImage
quit
-EOF

ON_LINE=
		
echo -e "\e[33m[$ON_LINE]\e[0m"
		
while  [ "$ON_LINE" == "" ]
	do
		echo '!tftp#0' | nc -u -p 10501 -w 1 $1 10501
		ON_LINE=$(echo '?tftp#' | nc -u -p 10501 -w 1 $1 10501)  || true
		echo -e "\e[33m[$ON_LINE]\e[0m"
	done

echo '?reboot##' | nc -u -p 10501 -w 1 $1 10501
