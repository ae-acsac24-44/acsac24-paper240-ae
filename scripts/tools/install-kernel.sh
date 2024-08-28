cd ../u-boot
cp /boot/uImage /boot/uImage.bak
cp /boot/uInitrd /boot/uInitrd.bak
cp /boot/boot.scr /boot/boot.scr.bak

./update-kernel.sh
./update-boot-script.sh
./update-initrd.sh initrd.img-4.18.0+
