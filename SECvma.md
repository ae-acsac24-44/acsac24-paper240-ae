# SECvma Environmental Setup

This guide details installing SECvma on a CloudLab m400 Arm server and running virtual machines.

## Install Dependencies
### Dependencies for Kernel Compilation
`sudo apt-get update && sudo apt install build-essential libncurses-dev bison flex libssl-dev libelf-dev`
### Dependencies for DTB Compilation
`sudo apt install device-tree-compiler`
### Dependencies for QEMU
`sudo apt install qemu-utils libglib2.0-dev libpixman-1-dev`
### Dependencies for u-boot
`sudo apt install u-boot-tools`


## Install SECvma
Clone the artifact repository:
```
git clone https://github.com/ae-acsac24-44/acsac24-paper240-ae.git
cd acsac24-paper240-ae/
git submodule update --init linux
```

### 1. SECvma Kernel Configuration & Compilation
#### Default SECvma Configuration (without CVM support):
To configure and compile the kernel with SECvma (_OPT-SECvma huge MEM mappings_), run the following commands:
```
cd linux
make secvma_defconfig
make -j8
make modules_install
make install
```

#### SECvma Configuration (with CVM support)
**To support CVM, you need to use this configuration.**
> The kernel source of SECvma is configured for optimized performance _(OPT-SECvma (huge MEM mappings)_ in Section 6 of the paper). However, as detailed in Section 7 of the paper, SECvma uses _OPT-SECvma (4KB MEM mappings)_ when running confidential VMs.

To configure and compile the kernel with SECvma (_OPT-SECvma 4KB  MEM mappings_), run the following commands:
```
cd linux
git checkout secvma-4kb-mem
make -j8
make modules_install
make install
```
A diff [patch](scripts/patch/secvma-4kb.patch) is available between these two configurations for reference.

### 2. Kernel Installation
You can use the provided automated script for installing the newly compiled SECvma binary to the boot partition so u-boot can load and boot SECvma on the hardware:
```
cd ../scripts/tools/
./install-kernel.sh
reboot
```

Note: If you wish to switch back to vanilla Linux, run `./restore-kernel.sh` in `tools/` and reboot your system. Alternatively, refer to the scripts under `u-boot/` for more information about the boot process and configuration options.

## Running VM on SECvma
The kernel source of SECvma is configured for optimized performance _(OPT-SECvma (huge MEM mappings)_ in Section 6 of the paper). However, as detailed in Section 7 of the paper, SECvma uses _OPT-SECvma (4KB MEM mappings)_ when running confidential VMs.

### VM Configuration
SECvma retains most of the SeKVM modules related to unmodified VMs. You can spin up and tear down VMs with SECvma like with SeKVM.

Note that to run VM, **_OPT-SECvma (4KB MEM mappings)_** should be configured and rebooted, 

1. Create the VM image:
    ```
    cd acsac24-paper240-ae/scripts/tools/
    ./create-images.sh $OUTPUT
    ``` 
    Replace `$OUTPUT` with the desired VM image location, such as `/mydata/cloud.img`. If not specified, the image will be created at `/mydata/cloud.img`.

2. Compile QEMU source from their work to support SeKVM:
      ```
      cd ../../
      git submodule update --init qemu
      ```
      Wait for the clone to complete, then do the following. Before compilation, you need to apply the [patch](/scripts/patch/qemu.patch) we provided, as some submodule links are outdated:

    ```
    cd qemu
    ./configure --target-list=aarch64-softmmu --disable-werror
    git apply ../scripts/patch/qemu.patch
    git submodule sync
    make -j8
    ```
3. Run `net.sh` to create a virtual bridge to a network interface connecting the client machine:
      ```
      cd ../scripts/tools/
      ./net.sh
      ```
      You only need to run it **once** whenever you (re)boot the host machine. You do not need to run it every time you boot the VM

4. Run the VM on SECvma:
      ```
      cd ../scripts/tests/
      ./run-guest-sekvm.sh
      ```
      
## Configure Cloudlab profile to enable mass local storage (Optional)
By default, m400 on cloudlab only provides 16GB disk mounted at `/root` which is infeasible for compiling the kernel and running a VM. You have to enable the internal SSD by editing the profile. You may refer to the [cloudlab manual](http://docs.cloudlab.us/advanced-storage.html) and the `m400-v4.18-ubuntu-20.04` profile.