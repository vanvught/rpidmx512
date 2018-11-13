#!/bin/bash

array=( connect close socket setsockopt getsockopt recv recvfrom send sendto ctlsocket dis_connect listen)
for i in "${array[@]}"
do
	echo $i
	sed -i "socket.h.backup" "s/ $i(/ _$i(/g" socket.h
	sed -i "socket.c.backup" "s/ $i(/ _$i(/g" socket.c
done