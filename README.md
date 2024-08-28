# SECvma: Virtualization-based Linux Kernel Protection for Arm

This artifact includes SECvma, our Linux kernel protection framework for Arm-based platforms, built on the verified SeKVM hypervisor. It also provides instructions for running the performance evaluations of SECvma and presents the modifications to SeKVM modules.

# Table of Contents
  - [1. SECvma Architecture](#1-secvma-architecture)
    - [1.1 Kernel Memory Protection](#11-kernel-memory-protection)
    - [1.2 Dynamic Module Loader](#12-dynamic-module-loader)
    - [1.3 Systems Registers Protection](#13-systems-registers-protection)
  - [2. Environment Configuration](#2-environment-configuration)
    - [2.1 Prerequisites](#21-prerequisites)
    - [2.2 Instructions for Cloudlab](#22-instructions-for-cloudlab)
      - [2.2.1 Joining Cloudlab](#221-joining-cloudlab)
      - [2.2.2 Cloudlab profiles](#222-cloudlab-profiles)
    - [2.3 Overview](#23-overview)
    - [2.4 Basic Preparation](#24-basic-preparation)
      - [2.4.1 Clone the Artifact Repo](#241-clone-the-artifact-repo)
    - [2.5 Client Configuration](#25-client-configuration)
    - [2.6 Server Configuration](#26-server-configuration)
      - [2.6.1 Bare-Metal Linux v4.18](#261-bare-metal-linux-v418)
      - [2.6.2 VM on mainline KVM](#262-vm-on-mainline-kvm)
      - [2.6.3 Bare-Metal SECvma](#263-bare-metal-secvma)
      - [2.6.4 VM on SECvma](#264-vm-on-secvma)
    - [2.7 SECvma Configuration](#27-secvma-configuration)
      - [2.7.1 SECvma Configuration & Compilation](#271-secvma-configuration--compilation)
      - [2.7.2 SECvma Installation](#272-secvma-installation)
      - [2.7.3 OPT-SECvma (4KB MEM mappings) configuration (Optional)](#273-opt-secvma-4kb-mem-mappings-configuration-optional)
    - [2.8 VM Configuration](#28-vm-configuration)
      - [2.8.1 Create the VM Image](#281-create-the-vm-image)
      - [2.8.2 QEMU Configuration](#282-qemu-configuration)
        - [2.8.2.1 QEMU compilation for KVM](#2821-qemu-compilation-for-kvm)
        - [2.8.2.2 QEMU compilation for SECvma](#2822-qemu-compilation-for-secvma)
      - [2.8.3 Running a Virtual Machine](#283-running-a-virtual-machine)
      - [2.8.4 Environment Configuration for Virtual Machine](#284-environment-configuration-for-virtual-machine)
      - [2.8.5 Terminating a Virtual Machine](#285-terminating-a-virtual-machine)
  - [3. Performance Evaluation](#3-performance-evaluation)
    - [3.1 Overview](#31-overview)
    - [3.2 Running Application Benchmarks and Collect Results](#32-running-application-benchmarks-and-collect-results)
      - [3.2.1 Apache/Netperf/Memcached](#321-apachenetperfmemcached)
      - [3.2.2 Hackbench/Kernbench](#322-hackbenchkernbench)
  - [4. Module Evaluation](#4-module-evaluation)
    - [4.1 Overview](#41-overview)
    - [4.2 Install crypto_virtio Module and Collect the Result](#42-install-crypto_virtio-module-and-collect-the-result)
    - [4.3 Install xfs Module and Collect the Result](#43-install-xfs-module-and-collect-the-result)
    - [4.4 Uninstall a Module and Collect the Result](#44-uninstall-a-module-and-collect-the-result)
    - [4.5 Signing a Kernel Module](#45-signing-a-kernel-module)
    - [4.6 Install the Signature in SECvma (Optional)](#46-install-the-signature-in-secvma-optional)

## 1. SECvma Architecture
SECvma provides Linux kernel protection with three security components, as detailed in Section 4 of the paper.

### 1.1 Kernel Memory Protection


|  File | Description |
| -------- | ------- |
| **Memory Usage Tracking** | (Refer to Section 4.1.1 in the paper)
| [el1_kpt] |  Initializes kernel memory pages' usage |
| [MemAux] | Sets up Stage 2 permissions based on intended types  |
|||
| **Kernel Page Table Protection** | (Refer to Section 4.1.2 in the paper) |
| [HostKpt] | Handles host permission faults |
| [HostKptOps], [HostKpt*] | Manages kernel page tables, e.g., creating, updating, and deleting mappings |
|||
| **User page table protection** | (Refer to Section 4.1.3 in the paper) |
| [HostPtSwitch] | Updates user page table usage |
|||
| **DMA Protection** | (Refer to Section 4.1.4 in the paper) |
| [MemAux], [Smmu*] |  Manages device memory pages |


### 1.2 Dynamic Module Loader
|  File | Description |
| -------- | ------- |
| [HostModule] |  Handles module hypercalls and authenticates modules (6) |
| [HostModuleOps] | The APIs including initializing information, (4) remap mod, (5) setup auth stack, (7) install to mod_mem, (8) mod sym linking, (9) config mod perm
| [HostModule*] |  Detailed implementations of the module loader functions |

### 1.3 Systems Registers Protection
|  File | Description |
| -------- | ------- |
| [HostSysRegsHandler] |  Handles write faults to system registers|
| [HostPtSwitch] | 	Manages TTBR register updates during context switches |

## 2. Environment Configuration
This section introduces the registration process to access two physical Arm machines, along with instructions for configuring the necessary environments on both the server and client machines.

### 2.1 Prerequisites

* We leverage [Cloudlab.us](https://www.cloudlab.us/) which provides machines and preconfigured profiles. Machines will be available upon request for artifact evaluation. See [Instructions for Cloudlab](#22-instructions-for-cloudlab). For our profile, we include two physical Arm machines (server/client) connected by _private_ network.
  
### 2.2 Instructions for Cloudlab

#### 2.2.1 Joining Cloudlab
Please sign up in cloud.us: https://www.cloudlab.us/signup.php to be able to access machines. Join the existing project: NTUCSIE, and we will receive a notification automatically and we will let you in.
To ensure anonymity, please use "ACSAC24 AE #nonce" for full name, a one-time email address, and random information in other fields.
#### 2.2.2 Cloudlab profiles
Start a new experiment by selecting `Start Experiment`. Use the `m400-v4.18-ubuntu-20.04` profile for running experiments. Please be patient and wait for the machines to setup and boot.

Once your machines are ready, you can login either via ssh or the UI interface. You have root access without password. You need to switch to root by:
```
sudo su
```

### 2.3 Overview
In this artifact, we present four configurations for evaluating performance on a server machine: 
- Bare-metal Linux v4.18
- Bare-metal SECvma
- VM on mainline KVM
- VM on SECvma

Before proceeding, you need to do the [basic preparation](#24-basic-preparation) to run various scripts and compile the source code. Since Linux v4.18 is pre-installed on both the server and client machines, we recommend starting with application performance measurement on bare-metal Linux v4.18, followed by VM on mainline KVM.

### 2.4 Basic Preparation
Note that all the commands other than this `git clone` command need to be executed in the directory in which this repo is cloned.
```
sudo su
```

#### 2.4.1 Clone the Artifact Repo
Clone this repository on both machines as a **root** user to a dedicated SSD mounted to `mydata`. 
```
cd /mydata
git clone https://github.com/ae-acsac24-44/acsac24-paper240-ae.git
cd acsac24-paper240-ae
```

### 2.5 Client Configuration
You can run the experiments to measure application workloads on the server machine (i.e., **bare-metal** machine and **virtual machines**). In contrast, the client machine sends workloads to the server machine over the network.

Run the following command **on the client machine** to automatically install all the applications for the performance evaluation on the client machine.
```
cd /mydata/acsac24-paper240-ae/scripts/client
./install.sh
```
This is required for the client machine and only needs to be done **once**.

### 2.6 Server Configuration
Before proceeding, please make sure both [Basic Preparation](#24-basic-preparation) and [Client Configuration](#25-client-configuration) are completed. The following provides an overview of instructions for configuring your server for different setups, please follow one of the setups and proceed to [Performance Evaluation](#3-performance-evaluation).

#### 2.6.1 Bare-Metal Linux v4.18
No additional setup is needed.

#### 2.6.2 VM on mainline KVM
Compile QEMU and launch the VM as outlined in [VM Configuration](#28-vm-configuration).

#### 2.6.3 Bare-Metal SECvma
Configure your server for SECvma by following the [SECvma Configuration](#27-secvma-configuration).

#### 2.6.4 VM on SECvma
To run VM benchmarks on SECvma:
1. Switch to the [OPT-SECvma (4KB MEM Mappings)](#273-opt-secvma-4kb-mem-mappings-configuration-optional) configuration.
2. Recompile the kernel and reboot your server.
3. Compile QEMU with SECvma support as detailed in [QEMU Compilation for SECvma](#2822-qemu-compilation-for-secvma).
4. Launch the VM using the image created for mainline KVM if available.

### 2.7 SECvma Configuration
First, sync from the remote to fetch the kernel source.
```
cd /mydata/acsac24-paper240-ae/
git submodule update --init linux
```
Please wait for a few minutes for the clone to complete. You will then see the source code in the `linux/` directory.

#### 2.7.1 SECvma Configuration & Compilation
The following commands will configure and compile SECvma.
```
cd linux
make secvma_defconfig
make -j8
make modules_install
make install
```

#### 2.7.2 SECvma Installation
The following commands will install the newly compiled SECvma binary to your boot partition, so u-boot can load and boot SECvma on the hardware.
```
cd /mydata/acsac24-paper240-ae/scripts/tools/
./install-kernel.sh
reboot
```
Each time after reboot, run `sudo su`.

**Note: If you wish to switch back to vanilla Linux v4.18, run `./restore-kernel.sh` in `tools/` and reboot your system.**

#### 2.7.3 OPT-SECvma (4KB MEM mappings) configuration (Optional)
By default, the kernel source of SECvma is configured for optimized performance (_OPT-SECvma (huge MEM mappings)_ in Section 6 of the paper). However, as detailed in Section 7, SECvma uses _OPT-SECvma (4KB MEM mappings)_ when running confidential VMs. 

To apply this configuration, run the following command to switch to the target branch in the source artifact and recompile the kernel.
```
cd linux
git checkout secvma-4kb-mem
make -j8
make modules_install
make install
```
We have provided a diff [patch](scripts/patch/secvma-4kb.patch) between these two configurations.

To return to the original branch (_OPT-SECvma (huge MEM mappings)_), use the following command:
```
git checkout master
```

### 2.8 VM Configuration
SECvma retains most of the SeKVM modules related to VMs unmodified. To run VMs with SECvma, you can spin up and tear down VMs in the same way as with SeKVM. The following instructions will show the ways to run VMs on **KVM** and **SECvma**.

#### 2.8.1 Create the VM Image
To create the image, on the server, run
```
cd /mydata/acsac24-paper240-ae/scripts/tools/
./create-images.sh
```
The image will be created under `/mydata/cloud.img`. 


#### 2.8.2 QEMU Configuration
##### 2.8.2.1 QEMU compilation for KVM
You will be able to run KVM on the server after the compilation is finished. See here for [instructions to run VMs on KVM](#283-running-a-virtual-machine). Note that KVM can only run before SECvma is installed.
```
cd /srv/vm/qemu
./configure --target-list=aarch64-softmmu --disable-werror
make -j8
```

#### 2.8.2.2 QEMU compilation for SECvma
To run VMs with SECvma, as with SeKVM, you will first have to compile QEMU from the source code from their work to support SeKVM.

Download the QEMU source from the repo below and configure the environment.
```
cd /mydata/acsac24-paper240-ae/
git submodule update --init qemu
```
Wait for a moment for the clone to complete, then do the following. Before compilation, you need to apply the [patch](/scripts/patch/qemu.patch) we provided, as some submodule links are outdated.

```
cd qemu
./configure --target-list=aarch64-softmmu --disable-werror
git apply ../scripts/patch/qemu.patch
git submodule sync
make -j8
```

#### 2.8.3 Running a Virtual Machine
To run virtual machines, please go to the directory `acsac24-paper240-ae/scripts/tests/`.
```
cd /mydata/acsac24-paper240-ae/scripts/tests/
```

Run `net.sh` to create a virtual bridge to a network interface connecting the client machine. You only need to run it **once** whenever you (re)boot the host machine. You do not need to run it every time you boot the VM.
```
../tools/net.sh
```

You can run the VM on SECvma using the following command.
```
./run-guest-sekvm.sh
```
You can replace `run-guest-sekvm.sh` with `run-guest-kvm.sh` to run the VM in KVM, if SECvma has not been installed and booted.

Finally, log in with the `root` user; no password is required. If you experience a delay, press `Ctrl+C` to login.
```
...
Ubuntu 20.04.6 LTS ubuntu ttyAMA0

ubuntu login: root
```

#### 2.8.4 Environment Configuration for Virtual Machine
Run the following commands for **VM performance workload**. After spinning up the VM on the server, you will see the `./vm-install.sh` file installed in the root directory of your VM. Execute the script to configure SSH and install required applications for performance evaluation in the VM.
```
chmod 700 vm-install.sh
./vm-install.sh
```
Press `Enter` when prompted to generate the SSH key.

#### 2.8.5 Terminating a Virtual Machine

After the experiment, you need to terminate a virtual machine. Run `halt -p` command iteratively inside virtual machines  running **on the server** until you get the server shell.
```
[VM ~] # halt -p
```

## 3. Performance Evaluation
This section provides instructions for running application benchmarks as detailed in Section 6 of the paper.
### 3.1 Overview
 We offer scripts for testing both **bare metal** and **virtual machine** using either the vanilla kernel or SECvma. 

**Note: Please ensure that both your client and server machines (with your preferred configuration, See [Client Configuration](#25-client-configuration), [Server Configuration](#26-server-configuration))  are configured accordingly before running application benchmarks.**

### 3.2 Running Application Benchmarks and Collect Results

After all required packages are installed on the client, you can then run the benchmark from the client machine using the respective script. Note that for VM benchmarks, ensure you have completed the installation in [Environment Configuration for VM](#284-environment-configuration-for-virtual-machine).

Running the following command in your **client** machine:

```
cd /mydata/acsac24-paper240-ae/scripts/client/{bm|vm}
```

Run the scripts located in the `bm/` directory on the client machine for bare-metal benchmarks, and use the `vm/` directory for VM benchmarks.

You can check the server's IP address (typically `10.10.1.1` in bare-metal or `10.10.1.100` for a virtual machine) using the following command:
```
ip a
```
**Please run application benchmarks over the private network that using the IP address range `10.10.1.*`**.

#### 3.2.1 Apache/Netperf/Memcached
For instance, to run apache/netperf/memcached, do the following:

```
./prep-{apache|netperf|memcached}.sh $SERVER_IP
./{apache|netperf|memcached}.sh $SERVER_IP
```

Place the application workload (e.g., `apache`, `netperf`, `memcached`) that you want to run, and replace `$SERVER_IP` with the IP address of the server.

The results will be stored at `scripts/client/{bm|vm}/{apache|netperf|memcached}.txt`.

#### 3.2.2 Hackbench/Kernbench

To run `hackbench/kernbench`:
```
./{hack|kern}.sh $SERVER_IP
```

After hackbench/kernbench is done, you can download the result from the server:
```
./grab-{hack|kern}.sh $SERVER_IP
```
The result will be stored at `scripts/client/{bm|vm}/{hack|kern}bench.txt`.

**Note: You can rename the result when you are trying to change different [server configuration](#26-server-configuration) and make a comparison.**

## 4. Module Evaluation
In this section, we provide instructions for evaluating kernel modules in **server machine** as detailed in Section 6 of the paper.

### 4.1 Overview
Currently, the following module signatures are installed in SECvma and can be installed securely:
| Name | Dependency |
| ----| ----|
| ipod | |
| crypto_virtio | crypto_engine |
| xfs | libcrc32c |

The `ipod` module is installed by default after booting.

### 4.2 Install `crypto_virtio` Module and Collect the Result
Running the following command in your **server machine**:
```
cd /lib/modules/4.18.0+/kernel
insmod ./crypto/crypto_engine.ko
time insmod ./drivers/crypto/virtio/virtio_crypto.ko
```
### 4.3 Install `xfs` Module and Collect the Result
Running the following command in your **server machine**:
```
cd /lib/modules/4.18.0+/kernel
insmod ./lib/libcrc32c.ko
time insmod ./fs/xfs/xfs.ko
```

### 4.4 Uninstall a Module and Collect the Result
To uninstall a module, use the following command:
```
time rmmod $MOD_NAME
```
Replace `$MOD_NAME` with the name of the loaded module.

### 4.5 Signing a Kernel Module
To sign a kernel module, use the provided signing tool.

Run the following commands to compile the tool:
```
cd /mydata/acsac24-paper240-ae/scripts/module/
make
```
After compiling, use the tool to generate and print the module's signature:
```
cd ../tools/
./sign_mod $PATH_TO_MODULE
```
Replace `$PATH_TO_MODULE` with the path to your module, e.g., `/lib/modules/4.18.0+/kernel/fs/xfs/xfs.ko`.

### 4.6 Install the signature in SECvma (Optional)
To install a signature in SECvma, follow these steps:
1. Copy the **signature** generated by the [signing tool](#4.5-signing-a-kernel-module).
2. Edit the SECvma source file at [`linux/arch/arm64/kint/HostModule.c`](https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/kint/HostModule.c).
3. Find the list of pre-installed signatures in the source file and add your signature in the format `{name, signature}`. For example:
   ```
   const struct sig_info sig[5] = {
	      ...
	      { "xfs", "bdcc8409cac0b4719a11063366c44c0ec03..." }
   };
    ```
4. Recompile SECvma and reboot the system by following the steps in the [SECvma Configuration](#27-secvma-configuration).
5. After rebooting, you can install the newly added kernel module.

#### 
[el1_kpt]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/kint/el1_kpt.c
[MemAux]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/sekvm/MemAux.c
[TrapDispatcher]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/sekvm/TrapDispatcher.c
[FaultHandler]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/sekvm/FaultHandler.c
[PTWalk]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/sekvm/PTWalk.c
[NPTWalk]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/sekvm/NPTWalk.c
[NPTOps]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/sekvm/NPTOps.c
[HostKpt]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/kint/HostKpt.c
[HostKptOps]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/kint/HostKptOps.c
[HostKpt*]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/kint/
[HostPtSwitch]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/sekvm/HostPtSwitch.c
[Smmu*]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/sekvm/
[HostModule]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/kint/HostModule.c
[HostModuleOps]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/kint/HostModuleOps.c
[HostModule*]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/kint/
[HostSysRegsHandler]: https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/sekvm/HostSysRegsHandler.c
