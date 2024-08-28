#!/bin/bash

cd ../../linux

mkimage -A arm -O linux -C none -T kernel -a 0x00080000 -e 0x00080000 -n Linux \
	-d arch/arm64/boot/Image \
	arch/arm64/boot/uImage

sudo cp arch/arm64/boot/uImage /boot/uImage
