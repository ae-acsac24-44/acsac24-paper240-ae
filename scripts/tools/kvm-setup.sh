#!/bin/bash

if [ ! -d "/mydata/acsac24-paper240-ae/" ]; then
    echo "Artifact not found"
    exit 1
fi

if [ ! -f "/mydata/cloud.img" ]; then
    echo "Creating Image..."
    cd /mydata/acsac24-paper240-ae/scripts/tools/
    ./create-images.sh
fi

echo "Compiling qemu..."
cd /srv/vm/qemu
./configure --target-list=aarch64-softmmu --disable-werror
make -j8

if ! ifconfig br0 > /dev/null 2>&1; then
    cd /mydata/acsac24-paper240-ae/scripts/tools/
    ./net.sh
fi
