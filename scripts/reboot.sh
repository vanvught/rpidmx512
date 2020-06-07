#!/bin/bash
while :
do
	ON_LINE=$(echo '?list#' | nc -u -p 10501 -w 1 $1 10501) || true
	
	echo -e "\e[33m[$ON_LINE]\e[0m"
	
	while  [ "$ON_LINE" == "" ]
	do
		ON_LINE=$(echo '?list#' | nc -u -p 10501 -w 1 $1 10501)  || true
	done
		
	echo -e "\e[36m[$ON_LINE]\e[0m"

	sleep 2

	echo '?reboot##' | nc -u -p 10501 -w 1 $1 10501
done
