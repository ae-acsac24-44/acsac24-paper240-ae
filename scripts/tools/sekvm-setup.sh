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

echo "Cloning the qemu source..."
cd /mydata/acsac24-paper240-ae/
git submodule update --init qemu

echo "Compiling qemu for SECvma..."
cd qemu
./configure --target-list=aarch64-softmmu --disable-werror
git apply ../scripts/patch/qemu.patch
git submodule sync
make -j8

if ! ifconfig br0 > /dev/null 2>&1; then
    cd /mydata/acsac24-paper240-ae/scripts/tools/
    ./net.sh
fi
