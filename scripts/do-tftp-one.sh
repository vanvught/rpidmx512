#!/bin/sh

echo '!tftp#1' | nc -u -p 10501 -w 1 $1 10501
echo '?tftp#' | nc -u -p 10501 -w 1 $1 10501

tftp $1 << -EOF
binary
put orangepi_one.uImage
quit
-EOF

echo '!tftp#0' | nc -u -p 10501 -w 1 $1 10501
echo '?tftp#' | nc -u -p 10501 -w 1 $1 10501

echo '?reboot##' | nc -u -p 10501 -w 1 $1 10501
