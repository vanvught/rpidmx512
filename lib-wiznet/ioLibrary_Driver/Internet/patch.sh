#!/bin/bash

array=( connect close socket setsockopt getsockopt recv recvfrom send sendto ctlsocket dis_connect listen)
for i in "${array[@]}"
do
	echo $i
	sed -i "dhcp.c.backup" "s/$i(/_$i(/g" DHCP/dhcp.c
done