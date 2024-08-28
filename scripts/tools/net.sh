#!/bin/bash

ifconfig br0 > /dev/null 2>&1

if [[ $? == 0 ]]; then
	echo "br0 has been created."
	exit
fi

ifconfig enp1s0d1 > /dev/null 2>&1
err=$?
if [[ $err != 0 ]]; then
	echo "enp1s0d1 not found - are you using the right topology?" >&2
	exit 1
fi

#IP=`ifconfig enp1s0d1 | grep 'inet addr:' | awk '{ print $2 }' | sed 's/.*://'`
IP=`ifconfig enp1s0d1 | grep 'inet ' | awk '{ print $2 }' | sed 's/.*://'`
ifconfig enp1s0d1 0.0.0.0
brctl addbr br0
brctl addif br0 enp1s0d1
echo $IP
ifconfig br0 $IP netmask 255.255.255.0
