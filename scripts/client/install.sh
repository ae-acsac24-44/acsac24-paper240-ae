#!/bin/bash

sudo apt-get update
sleep 1

echo "Install ab"
which ab > /dev/null
if [[ $? != 0 ]]; then
	sudo apt-get install -y apache2-utils
fi

echo "Install netperf"
which netperf > /dev/null
if [[ $? != 0 ]];then
	wget http://ports.ubuntu.com/pool/multiverse/n/netperf/netperf_2.6.0-2.1_arm64.deb
	dpkg -i netperf_2.6.0-2.1_arm64.deb
	rm netperf_2.6.0-2.1_arm64.deb
fi

echo "Install memtier_benchmark"
which memtier_benchmark > /dev/null 2>&1
if [[ $? != 0 ]]; then
        apt-get install -y build-essential autoconf automake libpcre3-dev libevent-dev pkg-config zlib1g-dev
        cd /tmp
        git clone https://github.com/RedisLabs/memtier_benchmark.git
        cd memtier_benchmark
        git checkout aabf9659830ad7a4d126d1fff75ac024dad49d3a
        autoreconf -ivf
        ./configure
        make -j 8
        make install
fi

#echo "Install env for YCSB"
#sudo apt-get install -y default-jre default-jdk maven
#sleep 1
#curl -O --location https://github.com/brianfrankcooper/YCSB/releases/download/0.17.0/ycsb-0.17.0.tar.gz
#tar xfvz ycsb-0.17.0.tar.gz
