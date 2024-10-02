#! /bin/bash

# create temp directory for mounting
TMP_DIR=$(mktemp -d --tmpdir=.)

# temp file for modifying /etc/passwd in the image
TMP_FILE=$(mktemp --tmpdir=.)

IMG_NAME=${1-/mydata/cloud.img}

SERVER=ubuntu-20.04-server-cloudimg-arm64-root.tar.xz
if [ ! -e $SERVER ];then
# Download server image
	wget https://cloud-images.ubuntu.com/releases/focal/release/ubuntu-20.04-server-cloudimg-arm64-root.tar.xz
fi

qemu-img create -f raw ${IMG_NAME} 10g
mkfs.ext4 ${IMG_NAME}
sudo mount ${IMG_NAME} ${TMP_DIR}
sudo tar xvf $SERVER -C ${TMP_DIR}
sudo sync
sudo touch ${TMP_DIR}/etc/cloud/cloud-init.disabled
# copy /etc/passwd to TMP_FILE but without the first line
sudo /bin/bash -c "tail -n +2 ${TMP_DIR}/etc/passwd > ${TMP_FILE}"
# remove the original /etc/passwd
sudo rm ${TMP_DIR}/etc/passwd
# add the first line (no root password) to TMP_FILE
sudo /bin/bash -c "echo root::0:0:root:/root:/bin/bash > ${TMP_DIR}/etc/passwd"
# copy the TMP_FILE to create the new /etc/passwd
sudo /bin/bash -c "cat ${TMP_FILE} >> ${TMP_DIR}/etc/passwd"
sudo /bin/bash -c "cp -r /usr/src/kvmperf ${TMP_DIR}/root"
sudo /bin/bash -c "cp vm-install.sh ${TMP_DIR}/root"
sudo /bin/bash -c "mkdir -p ${TMP_DIR}/root/.ssh/;cat /root/.ssh/id_rsa.pub >> ${TMP_DIR}/root/.ssh/authorized_keys"
sudo /bin/bash -c "cp 50-cloud-init.yaml ${TMP_DIR}/etc/netplan/"
sudo sync
sudo umount ${TMP_DIR}

# remove the temp file
rm ${TMP_FILE}
# remove the temp directory
rm -rf ${TMP_DIR}
