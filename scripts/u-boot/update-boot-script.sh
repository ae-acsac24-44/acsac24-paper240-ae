#!/bin/bash

sudo mkimage -A arm -T script -C none -n "Columbia Utah Boot Script" \
	-d boot.script  boot.scr
sudo cp boot.scr /boot/.
