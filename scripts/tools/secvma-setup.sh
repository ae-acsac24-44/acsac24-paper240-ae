#!/bin/bash

if [ ! -d "/mydata/acsac24-paper240-ae/" ]; then
    echo "Artifact not found"
    exit 1
fi

echo "Cloning the linux source..."
cd /mydata/acsac24-paper240-ae/
git submodule update --init linux

echo "Compiling linux..."
cd linux
make secvma_defconfig
make -j8
make modules_install
make install

echo "Installing kernel..."
cd /mydata/acsac24-paper240-ae/scripts/tools/
./install-kernel.sh
