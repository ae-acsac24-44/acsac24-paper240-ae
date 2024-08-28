#please use this after make install
mkimage -A arm -T ramdisk -C none -n uInitrd -d /boot/initrd.img-4.18.0+ /boot/uInitrd
