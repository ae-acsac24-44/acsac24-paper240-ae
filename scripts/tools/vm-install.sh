#!/bin/bash

dhclient
apt-get update
sleep 1

if [ ! -f ~/.ssh/id_rsa ];then
	# ssh configure
	dpkg-reconfigure openssh-server
	ssh-keygen
	echo "PermitRootLogin yes" >> /etc/ssh/sshd_config
	systemctl enable sshd
fi

systemctl restart sshd
sleep 1

echo "Installing package..."
apt install make flex bison

echo "Install apache server"
which apache2 > /dev/null
if [[ $? != 0 ]]; then
        apt-get install -y apache2
        update-rc.d apache2 disable
fi

which ab > /dev/null
if [[ $? != 0 ]]; then
        sudo apt-get install -y apache2-utils
fi

if [[ ! -d "/var/www/html/gcc" ]]; then
        cd /var/www/html
        wget http://gcc.gnu.org/onlinedocs/gcc-4.4.7/gcc-html.tar.gz
        tar xvf gcc-html.tar.gz
fi

echo "Install netperf server"
which netperf > /dev/null
if [[ $? != 0 ]];then
	wget http://ports.ubuntu.com/pool/multiverse/n/netperf/netperf_2.6.0-2.1_arm64.deb
	dpkg -i netperf_2.6.0-2.1_arm64.deb
	rm netperf_2.6.0-2.1_arm64.deb
	update-rc.d netperf disable
fi

echo "Install memcached server"

dpkg --get-selections | grep '\<memcached\>' > /dev/null 2>&1
if [[ ! $? == 0 ]]; then
        apt-get install -y memcached
        update-rc.d memcached disable
fi
