# Appendix

This appendix provides an overview of the scripts included in the artifact, making it easier to modify and adapt them as needed for different configurations or environments.

## Application Benchmark scripts

### The `tree` structure
```
client
│   ├── bm
│   │   ├── apache.sh
│   │   ├── grab-*.sh
│   │   ├── hack.sh
│   │   ├── kern.sh
│   │   ├── memcached.sh
│   │   ├── netperf.sh
│   │   ├── norm.sh
│   │   ├── prep-*.sh
│   │   └── run-all.sh
│   ├── install.sh
│   └── vm
│       ├── <similar to bm/>
```

------
|  File | Description |
| -------- | ------- |
| `prep-{BENCHMARK}.sh`| Prepares the benchmark environment, including starting necessary services on the server.
| `apache.sh`, `memcached.sh`, `netperf.sh` |  Executes client-server workload benchmarks, with data being sent between the client and server. |
| `hack.sh`, `kern.sh` | Runs the `hackbench` and `kernbench` benchmarks on the server, using `grab-*.sh` to collect and transfer the results back to the client machine
| `run-all.sh` | An automated script that allows you to run all or specific benchmarks, with optional settings such as repetition count and output file naming. See [Running Application Benchmarks](https://github.com/ae-acsac24-44/acsac24-paper240-ae/blob/master/README.md#32-running-application-benchmarks-and-collect-results). |
| `norm.sh` | Normalized results against a baseline |

## Module Signing Scripts
### The `tree` structure
```
module
│   ├── hacl-20
│   │   ├── <the crypto library files>
│   ├── helper.c
│   ├── main.c
│   ├── Makefile
│   ├── moduleAux.c
│   ├── moduleOps.c
│   ├── module-plts.c
│   ├── sign_mod.c
│   └── sign_mod.h
```
------
|  File | Description |
| -------- | ------- |
| `main.c`| The entry point of the program that grabs the module file and invokes `simulate_load_module`.
| `sign_mod.c` | Implements `simulate_load_module`, which simulates the kernel module loading process with reduced steps similar to kernel module loading and also signing the module by invoking `sign_module`. This file also includes functions for initializing module file information. |
| `moduleOps.c` | Contains operations that prepare the sections to be verified (e.g., `init_info`), stack the buffer, and handle the signing of the module by invoking the crypto library.
| `moduleAux.c` | Provides detailed functions for preparing sections, invoked by `moduleOps` |
| `module-plts.c`|  Prepares additional information (e.g., plts) required during module installation. These functions are mostly architecture-independent, with support for Arm64.  |
| `helper.c`, `sign_mod.h` | The helper functions and the header file.|

## Patch scripts
### The `tree` structure
```
├── patch
│   ├── qemu.patch
│   ├── secvma-4kb.patch
│   └── secvma.patch
```
------
|  File | Description |
| -------- | ------- |
| `qemu.patch`| The patch for updating the outdated links for the QEMU source.
| `secvma.patch` | All changes applied to the SeKVM artifact |
| `secvma-4kb.patch` | The differences in configuration settings between _OPT-SECvma (Huge MEM mappings)_ and _OPT-SECvma (4KB MEM mappings)_|

## Tests scripts
### The `tree` structure
```
├── tests
│   ├── Image
│   ├── Image.sekvm
│   ├── run-guest-kvm.sh
│   ├── run-guest-sekvm.sh
│   └── run-mod.sh
```
------
|  File | Description |
| -------- | ------- |
| `run-guest-sekvm.sh`, `run-guest-kvm.sh`| Scripts to run a VM on either mainline KVM or SECvma. You can specify different VM setups, by using the `-h` option for help.
| `Image`, `Image.sekvm` | The kernel images required for running VM on KVM and SECvma |
| `run-mod.sh` | An automated script to install and remove kernel module support for SECvma. See [Install and Uninstall Module](https://github.com/ae-acsac24-44/acsac24-paper240-ae/blob/master/README.md#42-install-and-uninstall-module-and-collect-the-result). |

## Tools scripts
### The `tree` structure
```
├── tools
│   ├── 50-cloud-init.yaml
│   ├── create-images.sh
│   ├── install-kernel.sh
│   ├── kvm-setup.sh
│   ├── net.sh
│   ├── restore-kernel.sh
│   ├── secvma-4kb-setup.sh
│   ├── secvma-setup.sh
│   ├── sekvm-setup.sh
│   └── vm-install.sh
```
------
|  File | Description |
| -------- | ------- |
| `50-cloud-init.yaml`| Configures the network for the VM; you can change the assigned static IP.
| `create-images.sh` | Creates the disk image for the VM; you can change output file and disk size. |
| `net.sh` | Creates a virtual bridge to a network interface. |
| `vm-install.sh` | The automated script for initial setup and installation of required packages in the VM |
| `kvm-setup.sh`, `secvma-setup.sh`, `secvma-4kb-setup,sh`, `sekvm-setup.sh`| The automated scripts for different environment setup for the server. See [Server Configuration](https://github.com/ae-acsac24-44/acsac24-paper240-ae/blob/master/README.md#22-server-configuration) |
|`install-kernel.sh`, `restore-kernel.sh`| `install-kernel.sh` installs the compiled kernel binary to the boot partition and invokes u-boot scripts to load and boot binary on the hardware. `restore-kernel.sh` restores the backup boot image, ramdisk and boot script to default |

## Boot scripts
### The `tree` structure
```
└── u-boot
    ├── <device tree source, boot script>
    ├── update-initrd.sh
    └── update-kernel.sh
```
------
|  File | Description |
| -------- | ------- |
| `update-*.sh`| Generates the boot image, boot script, and ramdisk. |