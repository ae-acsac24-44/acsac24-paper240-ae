#!/bin/bash

KERNEL_VERSION=$(uname -r)

KERNEL_MODULE_PATH="/lib/modules/$KERNEL_VERSION/kernel"
CRYPTO_ENGINE_KO="$KERNEL_MODULE_PATH/crypto/crypto_engine.ko"
VIRTIO_CRYPTO_KO="$KERNEL_MODULE_PATH/drivers/crypto/virtio/virtio_crypto.ko"
LIBCRC32C_KO="$KERNEL_MODULE_PATH/lib/libcrc32c.ko"
XFS_KO="$KERNEL_MODULE_PATH/fs/xfs/xfs.ko"

install_module() {
    local module=$1
    if [[ $2 == 1 ]]; then
		echo "----------------------------------------------------------"
    	echo "Inserting module: $module"
    	TIME_OUTPUT=$( { time insmod "$module"; } 2>&1 )
    	REAL_TIME=$(echo "$TIME_OUTPUT" | grep 'real' | awk '{print $2}')
    	echo "insmod performance: $REAL_TIME"
		echo ""
    fi
}

remove_module() {
    local module=$1
    if [[ $2 == 1 ]]; then
		echo "----------------------------------------------------------"
    	echo "Removing module: $module"
    	TIME_OUTPUT=$( { time rmmod "$module"; } 2>&1 )
    	REAL_TIME=$(echo "$TIME_OUTPUT" | grep 'real' | awk '{print $2}')
		echo "rmmod performance: $REAL_TIME"
		echo ""
    fi
}

if [[ -f "$CRYPTO_ENGINE_KO" && -f "$VIRTIO_CRYPTO_KO" && -f "$LIBCRC32C_KO" && -f "$XFS_KO" ]]; then
    echo "Installing crypto_virtio modules..."
    cd "$KERNEL_MODULE_PATH"
    install_module "$CRYPTO_ENGINE_KO" 0
    install_module "$VIRTIO_CRYPTO_KO" 1
    echo "Removing crypto_virtio modules..."
    remove_module "$VIRTIO_CRYPTO_KO" 1
    remove_module "$CRYPTO_ENGINE_KO" 0

    echo "Installing xfs modules..."
    install_module "$LIBCRC32C_KO" 0
    install_module "$XFS_KO" 1
    echo "Removing xfs modules..."
    remove_module "$XFS_KO" 1
    remove_module "$LIBCRC32C_KO" 0
else
    echo "One or more kernel modules not found in $KERNEL_MODULE_PATH."
    exit 1
fi

