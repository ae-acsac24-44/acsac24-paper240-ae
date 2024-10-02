# SECvma: Virtualization-based Linux Kernel Protection for Arm

This artifact includes SECvma, our Linux kernel protection framework for Arm-based platforms, built on the verified SeKVM hypervisor. It also provides instructions for running the performance evaluations of SECvma and presents the modifications to SeKVM modules.

# Table of Contents
  - [1. SECvma Architecture](#1-secvma-architecture)
    - [1.1 Kernel Memory Protection](#11-kernel-memory-protection)
    - [1.2 Dynamic Module Loader](#12-dynamic-module-loader)
    - [1.3 Systems Registers Protection](#13-systems-registers-protection)
  - [2. Environment Configuration](#2-environment-configuration)
    - [2.1 Overview](#21-overview)
    - [2.2 Server Configuration](#22-server-configuration)
      - [2.2.1 Bare-Metal Linux v4.18](#221-bare-metal-linux-v418)
      - [2.2.2 VM on mainline KVM](#222-vm-on-mainline-kvm)
      - [2.2.3 Bare-Metal SECvma](#223-bare-metal-secvma)
      - [2.2.4 VM on SECvma](#224-vm-on-secvma)
    - [2.3 VM Configuration](#23-vm-configuration)
    - [2.4 Client Configuration](#24-client-configuration)
      
  - [3. Performance Evaluation](#3-performance-evaluation)
    - [3.1 Overview](#31-overview)
    - [3.2 Running Application Benchmarks and Collect Results](#32-running-application-benchmarks-and-collect-results)
      - [3.2.1 Running All Benchmarks](#321-running-all-benchmarks)
      - [3.2.2 Running a Specific Benchmark](#322-running-a-specific-benchmark)
      - [3.2.3 Collecting Raw Results (Optional)](#323-collecting-raw-results-optional)
      - [3.2.4 Normalized the Results against Baseline](#324-normalized-the-results-against-baseline)
  - [4. Module Evaluation](#4-module-evaluation)
    - [4.1 Overview](#41-overview)
    - [4.2 Install and Uninstall Module and Collect the Result](#42-install-and-uninstall-module-and-collect-the-result)
    - [4.3 Signing a Kernel Module](#43-signing-a-kernel-module)
    - [4.4 Install the Signature in SECvma (Optional)](#44-install-the-signature-in-secvma-optional)
  - [5. Software/Hardware Requirements](#5-softwarehardware-requirements)

## 1. SECvma Architecture
SECvma provides Linux kernel protection with three security components, as detailed in Section 4 of the paper.

### 1.1 Kernel Memory Protection


|  File | Description |
| -------- | ------- |
| **Memory Usage Tracking** | (Refer to Section 4.1.1 in the paper)
| [el1_kpt] | Initializes kernel memory page usage. The `init_el1_pts()` function marks each section of the kernel image with its corresponding usage and traces page tables accordingly |
| [MemAux] | Sets up stage 2 permissions based on the intended types. The `map_page_host()` function parses types of faulting memory (e.g., code, data) and uses `mmap_s2pt()` to perform mapping with corresponding permissions in the stage 2 page table. |
|||
| **Kernel Page Table Protection** | (Refer to Section 4.1.2 in the paper) |
| [HostKpt] | Handles host permission faults. Responsibilities include mapping/unmapping kernel page tables, managing user page table faults and permission faults from the page pool  |
| [HostKptOps], [HostKpt*] | Implements detailed management of kernel page tables, including creating, updating, and deleting mappings invoked by `HostKpt` |
|||
| **User page table protection** | (Refer to Section 4.1.3 in the paper) |
| [HostPtSwitch] | Updates user page table usage. The hypercalls `el2_do_alloc/free_el0_pgd()` allocate/free the pgd and update usage for the user page table |
|||
| **DMA Protection** | (Refer to Section 4.1.4 in the paper) |
| [MemAux], [Smmu*] |  Manages device memory pages. The `update_smmu_page()` function in `MemAux` checks the usage of device memory pages to prevent protected kernel memory from being mapped to the SMMU |


### 1.2 Dynamic Module Loader
|  File | Description |
| -------- | ------- |
| [HostModule] |  Handles module hypercalls and authenticates modules (6). The kernel triggers the `el2_load_module()` hypercall handler and manages module loading step-by-step. The `verify_mod()` function authenticates the module using the crypto library. |
| [HostModuleOps] | Providing APIs for initializing information, (4) remap mod, (5) setup auth stack, (7) install to mod_mem, (8) mod sym linking, (9) config mod perm
| [HostModule*] |  Detailed implementations of the module loader functions invoked by `HostModule` or `HostModuleOps` |

### 1.3 Systems Registers Protection
|  File | Description |
| -------- | ------- |
| [HostSysRegsHandler] |  Manages write faults to system registers. It validates and updates the values written to registers, such as `handle_ttbr0` for updating the `TTBR0_EL1` register. |
| [HostPtSwitch] | 	Validates and updates the TTBR register during context switches. |

## 2. Environment Configuration
This section introduces the registration process to access two physical Arm machines, along with instructions for configuring the necessary environments on both the server and client machines.

### 2.1 Overview

We leverage [Cloudlab.us](https://www.cloudlab.us/) which provides machines and preconfigured profiles. Machines will be available upon request for artifact evaluation. See [Instructions for Cloudlab](#22-instructions-for-cloudlab). For our profile, we include two physical Arm machines (server/client) connected by _private_ network.
  
1. **Joining Cloudlab**
   
	Please sign up in [cloud.us](https://www.cloudlab.us/signup.php) to be able to access machines. Join the existing project: **NTUCSIE**, and we will receive a notification automatically and we will let you in.
    - To ensure anonymity, please use "ACSAC24 AE #nonce" for full name, a one-time email address, and random information in other fields.

2. **Cloudlab profiles**

	Start a new experiment by selecting `Start Experiment`. Use the `m400-v4.18-ubuntu-20.04` profile for running experiments. Please be patient and wait for the machines to setup and boot.


In this artifact, we present four configurations for evaluating performance on a server machine: 
- Bare-metal Linux v4.18
- Bare-metal SECvma
- VM on mainline KVM
- VM on SECvma

For VMs, additional [VM configurations](#23-vm-configuration) will be necessary. Before [running application benchmarks](#32-running-application-benchmarks-and-collect-results) ensure you have followed [client configuration](#24-client-configuration) to set up your client machine. Note that you are only required set up your client **once**. 

**Note: Linux v4.18 is pre-installed on both the server and client machines; we recommend starting with application performance measurement on bare-metal Linux v4.18, followed by VM on mainline KVM.**

### 2.2 Server Configuration
We provide automated scripts to set up different profiles on the server. Please follow one of the profiles to configure your server.

Before you begin, use `sudo su` to switch to the root user and clone the artifact repository to the dedicated SSD mounted at `/mydata`:
```
sudo su
cd /mydata
git clone https://github.com/ae-acsac24-44/acsac24-paper240-ae.git
cd acsac24-paper240-ae
```

- Alternatively, if you prefer to manually set up **Bare-metal SECvma** and **VM on SECvma** on Arm machines, you can refer to the installation instructions in [SECvma.md](SECvma.md).

#### 2.2.1 Bare-Metal Linux v4.18
Linux v4.18 is pre-installed. Proceed to [Client Configuration](#24-client-configuration).

#### 2.2.2 VM on mainline KVM
To run virtual machines on KVM, please follow these steps:

1. Prepare the VM image and configure QEMU by running:
    ```
    cd /mydata/acsac24-paper240-ae/scripts/tools/
    ./kvm-setup.sh
    ```
    
2. Proceed to [VM Configuration](#23-vm-configuration).


#### 2.2.3 Bare-Metal SECvma
To configure SECvma, please follow these steps:

1. Configure and compile SECvma by running:
    ```
    cd /mydata/acsac24-paper240-ae/scripts/tools/
    ./secvma-setup.sh
    ```
    Please wait for a few minutes for the linux source to be cloned and compiled.

2. Reboot the system by running:
    ```
    reboot
    ```
    Each time after reboot, run `sudo su`.
    
3. Proceed to [Client Configuration](#24-client-configuration).

#### 2.2.4 VM on SECvma
As detailed in Section 7 in the paper, SECvma uses _OPT-SECvma (4KB MEM mappings)_ when running confidential VMs. To run virtual machines on SECvma, please follow these steps:

1. Re-configure SECvma with _4KB MEM mappings_ by running:
    ```
    cd /mydata/acsac24-paper240-ae/scripts/tools/
    ./secvma-4kb-setup.sh
    ```
2. Reboot the system by running:
    ```
    reboot
    ```
    Each time after reboot, run `sudo su`.
    
3. Setup QEMU with SeKVM support. Clone and configure the QEMU source from their work by running:
    ```
    cd /mydata/acsac24-paper240-ae/scripts/tools/
    ./sekvm-setup.sh
    ```
4. Proceed to [VM Configuration](#23-vm-configuration).

### 2.3 VM Configuration
1. Running a virtual machine:
     - To run a **VM on mainline KVM**:
        ```
        cd /mydata/acsac24-paper240-ae/scripts/tests/
        ./run-guest-kvm.sh
        ```
        Note: KVM can only run before SECvma is installed.
    - To run a **VM on SECvma**:
        ```
        cd /mydata/acsac24-paper240-ae/scripts/tests/
        ./run-guest-sekvm.sh
        ```
        Note: Ensure that SECvma with VM support is installed (See [VM on SECvma](#224-vm-on-secvma)).
2. Log in with the `root` user; no password is required. If you experience a delay, press `Ctrl+C` to login.
    ```
    ...
    Ubuntu 20.04.6 LTS ubuntu ttyAMA0

    ubuntu login: root
    ```
3. Once log in, you will see the `./vm-install.sh` script is already installed in the root directory of your VM. Execute the `./vm-install.sh` script on the VM to configure SSH and install required applications for performance evaluation in the VM.
    ```
    [VM ~] # ./vm-install.sh
    ```
    Press `Enter` when prompted to generate the SSH key. If the key is already created, this step will be skipped. 

4. To terminate a virtual machine after the measurement. Run `halt -p` command iteratively inside virtual machines  running **on the server** until you get the server shell.
    ```
    [VM ~] # halt -p
    ```
    
### 2.4 Client Configuration
You can run the experiments to measure application workloads on the server machine (i.e., **bare-metal** machine and **virtual machines**). In contrast, the client machine sends workloads to the server machine over the network.

Run the following command **on the client machine** to automatically install all the applications for the performance evaluation on the client machine.
```
cd /mydata/acsac24-paper240-ae/scripts/client
./install.sh
```
This is required for the client machine and only needs to be done **once**.


## 3. Performance Evaluation
This section provides instructions for running application benchmarks, as detailed in Section 6 of the paper.

### 3.1 Overview
We offer scripts for testing both **bare metal** and **virtual machine** using either the vanilla kernel or SECvma. The table below summarizes the server configurations along with the corresponding metrics used in paper:
|  [Server Configuration](#22-server-configuration) | Metric Used in Paper (Figure 6 & 7) |
| -------- | ------- |
| [Bare-Metal linux v4.18](#221-bare-metal-linux-v418) | Baseline for normalization of bare-metal results |
| [VM on mainline KVM](#222-vm-on-mainline-kvm) | Baseline for normalization of VM results |
| [Bare-Metal SECvma](#223-bare-metal-secvma)| _OPT-SECvma (huge MEM mappings)_|
| [VM on SECvma](#224-vm-on-secvma) | _OPT-SECvma (4KB MEM mappings)_ |

**Note: Before running the applications benchmarks, please ensure that both your client and server machines (with your preferred configuration, See [Client Configuration](#24-client-configuration), [Server Configuration](#22-server-configuration))  are configured accordingly. If you are evaluating a VM, make sure the VM is spinning up, see [VM Configuration](#23-vm-configuration)**.

### 3.2 Running Application Benchmarks and Collect Results

After all required packages are installed on the client, you can then run the benchmark from the client machine using the respective script.

1. Running the following command in your **client** machine:

    ```
    cd /mydata/acsac24-paper240-ae/scripts/client/{bm|vm}
    ```

    Run the scripts located in the `bm/` directory on the client machine for bare-metal benchmarks, and use the `vm/` directory for VM benchmarks.

2. Check the server's IP address with:
    ```
    ip a
    ```
    **Please run application benchmarks over the private network using the IP address range `10.10.1.*`**.

3. We provide an automated script `run-all.sh` allows you to run all or specific benchmarks with optional settings.
    ```
    ./run-all.sh $SERVER_IP [-t $BENCHMARK] [-n REPTS] [-o $OUTPUT]
    ```
    - `$SERVER_IP`: Replace `$SERVER_IP` with the server's IP address (typically `10.10.1.1` in bare-metal or `10.10.1.100` for a virtual machine). 
    - `-t $BENCHMARK`: Specify the benchmark to run (e.g., `hackbench`, `benchmark`, `kernbench`, `netperf`, `apache`, `memcached`). All benchmarks will be run by default.
    - `-n $REPTS`: Define the number of repetitions for network benchmarks (i.e., `netperf`, `apache`, `memcached`).
    - `-o OUTPUT`: Specify the output filename. The aggregated results are stored in the `results/` directory.  If not specified, results are saved as `benchmark_results.$NUM` where `$NUM` auto-increments with each run.

#### 3.2.1 Running All Benchmarks
To run all benchmarks,
```
./run-all.sh $SERVER_IP [-n REPTS] [-o $OUTPUT]
```
The command executes all benchmarks (see Table 4 in the paper) and prints the raw data on the terminal. The output includes results for each benchmark, along with an overall average. It takes approximately 10-15 minutes to complete.

By default, network applications are conducted only a few times, which may introduce variability in the results due to fluctuating network conditions. The paper uses `-n 50` repetitions for network benchmarks to achieve consistent evaluation results, which will take longer to complete.

To reproduce the evaluation result presented in the paper, you can follow this example:
```
./run-all.sh 10.10.1.1 -n 50 -o test-all
...
cat results/test-all

Hackbench Result (sec):
11.121
11.391
11.249
11.166
Average: 11.2317
... // Other benchmarks
```
The evaluation will take approximately **1-2 hours** to complete, and it may take even longer when running on VMs.


#### 3.2.2 Running a Specific Benchmark
To run a specific application benchmark, use the following command:
```
./run-all.sh $SERVER_IP -t $BENCHMARK [-n $REPTS] [-o $OUTPUT]
```
Replace `$BENCHMARK` with the desired application workload (e.g., `apache`, `netperf`, `memcached`, `hackbench`, `kernbench`).


#### 3.2.3 Collecting Raw Results (Optional)
After running the benchmarks, to view the raw data from each application, use the following command:
```
cat log/$BENCHMARK.$NUM
```
This shows the unprocessed output from the benchmarks, stored in `log/$BENCHMARK.$NUM`. If an output filename was specified with `-o`, the individual results will be stored as `log/$BENCHMARK.$OUTPUT`

#### 3.2.4 Normalized the Results against Baseline
To aggregate the normalized results, run the following command:
```
./norm.sh results/$BASELINE results/$RESULT
```
Replace `$BASELINE` with the baseline result file, e.g., `benchmark_results.0`, and `$RESULT` with the result you want to normalize against the baseline, e.g., `benchmark_results.1`.

The normalized result will append to `results/$RESULT` and displayed on the terminal.

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

### 4.2 Install and Uninstall Module and Collect the Result
To install and uninstall the `xfs.ko` and `crypto_virtio` kernel modules, and collect the timing results, run the following script on your **server machine**:
```
cd /mydata/acsac24-paper240-ae/scripts/tests/
./run-mod.sh
```
The script will measure and aggregate the time to install and remove the `xfs.ko` and `crypto_virtio` kernel modules. The results will be printed directly on the terminal.


### 4.3 Signing a Kernel Module
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
Replace `$PATH_TO_MODULE` with the path to your module, e.g., `/lib/modules/$(uname -r)/kernel/fs/xfs/xfs.ko`.

### 4.4 Install the signature in SECvma (Optional)
To install a signature in SECvma, follow these steps:
1. Copy the **signature** generated by the [signing tool](#43-signing-a-kernel-module).
2. Edit the SECvma source file at [`linux/arch/arm64/kint/HostModule.c`](https://github.com/ae-acsac24-44/acsac24-ae-linux/tree/master/arch/arm64/kint/HostModule.c).
3. Find the list of pre-installed signatures in the source file and add your signature in the format `{name, signature}`. For example:
   ```
   const struct sig_info sig[5] = {
	      ...
	      { "xfs", "bdcc8409cac0b4719a11063366c44c0ec03..." }
   };
    ```
4. Recompile SECvma and reboot the system by following the steps in the [Bare-Metal SECvma](#223-bare-metal-secvma).
5. After rebooting, you can install the newly added kernel module.

## 5. Software/Hardware Requirements
The SECvma prototype included in this artifact currently supports Arm's m400 hardware. Our evaluation was conducted on an HP Moonshot m400 server running Ubuntu 20.04 with Linux v4.18, with an 8-core 64-bit Armv8-A 2.4 GHz Applied Micro Atlas SoC, 64 GB of RAM, a 120 GB SATA3 SSD, and a Dual-port Mellanox ConnectX-3 10GbE NIC. 

However, it can be extended to support other Arm hardware with virtualization extensions, as well as other Linux distributions. To achieve this, developers will need to make two key adaptations:
1. First, they must modify SECvma's boot process to accommodate different hardware specifications. This involves altering Linux's boot sequence to establish an EL2 runtime environment for SECvma's TCB (KPCore). For example, SECvma allocates memory pools for page tables during the boot process. Developers may need to adjust these allocations based on specific hardware requirements, such as available RAM size. 
2. Second, SECvma may require further modifications to meet various hardware specifications. For instance, while the current prototype supports Arm's Generic Interrupt Controller version 2 (GICv2), additional development would be necessary to incorporate support for GICv3 or other hardware-specific features.


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
