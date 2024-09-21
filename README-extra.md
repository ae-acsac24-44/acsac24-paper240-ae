# Extended Context

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

## 3. Performance Evaluation
#### 3.2.1 Running All Benchmarks
To run all application benchmarks, use the following command:
```
./run-all.sh $SERVER_IP
```
Replace `$SERVER_IP` with the server's IP address. The script executes all benchmarks (see Table 4 in the paper) and prints the raw data on the terminal. It takes approximately 10-15 minutes to complete.

Results are stored in `results/benchmark_results.$NUM` where each run increments `$NUM` increments. For example, `benchmark_results.0` is the first run. The latest result corresponds to the highest `$NUM`. The output includes the results for each benchmark and an overall average. For example, 
```
cat results/benchmark_results.0

Hackbench Result (sec):
11.121
11.391
11.249
11.166
Average: 11.2317
... // Other benchmarks
```

##### 3.2.1.1 Repetitions for Network Benchmarks (Optional)
By default, network applications, such as `netperf`, `apache`,`memcached` are conducted only a few times, which may introduce variability in the results due to fluctuating network conditions.

Alternatively, you can specify the number of repetitions. For example, use the following command:
```
./run-all.sh $SERVER_IP all 50
```
The evaluation in the paper uses `50` repetitions for network benchmarks, which will take longer to complete. Replace`50` with the desired number of repetitions and `all` with the specific benchmark you want to run (see [Running a Specific Benchmark](#322-running-a-specific-benchmark)).


#### 3.2.2 Running a Specific Benchmark
To run a specific application benchmark, use the following command:
```
./run-all.sh $SERVER_IP $BENCHMARK [$REPTS]
```
Replace `$BENCHMARK` with the desired application workload (e.g., `apache`, `netperf`, `memcached`, `hackbench`, `kernbench`).

The results are stored in `results/benchmark_results.$NUM`.

#### 3.2.3 Collecting Raw Results (Optional)
After running the benchmarks, to view the raw data from each application, use the following command:
```
cat log/$BENCHMARK.$NUM
```
This shows the unprocessed output from the benchmarks, stored in `log/$BENCHMARK.$NUM`

#### 3.2.4 Normalized the Results against Baseline
To compare results, run the following command:
```
./norm.sh results/$BASELINE results/$RESULT
```
Replace `$BASELINE` with the baseline result file, e.g., `benchmark_results.0`, and `$RESULT` with the result you want to normalize against the baseline, e.g., `benchmark_results.1`.

The normalized result will append to `results/$RESULT` and display on the terminal.

## 5. Software/Hardware Requirements
The SECvma prototype included in this artifact currently supports Arm's m400 hardware. However, it can be extended to support other Arm hardware with virtualization extensions. To achieve this, developers will need to make two key adaptations:
First, they must modify SECvma's boot process to accommodate different hardware specifications. This involves altering Linux's boot sequence to establish an EL2 runtime environment for SECvma's TCB (KPCore). For example, SECvma allocates memory pools for page tables during the boot process. Developers may need to adjust these allocations based on specific hardware requirements, such as available RAM size. Second, SECvma may require further modifications to meet various hardware specifications. For instance, while the current prototype supports Arm's Generic Interrupt Controller version 2 (GICv2), additional development would be necessary to incorporate support for GICv3 or other hardware-specific features.

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
