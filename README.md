<h1 align="center">
	<img width="300" src="https://raw.githubusercontent.com/EDI-Systems/M7M01_Eukaron/master/Document/Public/Demo/logo.png" alt="logo">
</h1>

# RME Concord Microkernel
<div align="center">

[![Github release](https://img.shields.io/github/release/EDI-Systems/M7M01_Eukaron.svg)](https://github.com/EDI-Systems/M7M01_Eukaron/releases/latest)
[![Github commits](https://img.shields.io/github/commits-since/EDI-Systems/M7M01_Eukaron/main@{30day}.svg)](https://github.com/EDI-Systems/M7M01_Eukaron/compare/main@{30day}...main)
[![Discord](https://img.shields.io/badge/chat-Discord-purple)](https://discord.gg/VxCFSFC6bW)

</div>
<div align="center">

![language](https://img.shields.io/badge/language-C-orange.svg)
[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/1836/badge)](https://www.bestpractices.dev/projects/1836) 
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/fb3f40e96daa42169333ba698b4fd083)](https://app.codacy.com/gh/EDI-Systems/M7M01_Eukaron/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

</div>

点击 **[这里](README_CN.md)** 查看中文版。

&ensp;&ensp;**RME** is a general-purpose operating system which focuses on many advanced features, including:
- [Capability](https://en.wikipedia.org/wiki/Capability-based_security)-based configurable protection domains;
- Massive [scalability](https://en.wikipedia.org/wiki/Scalability) and [parallelism](https://en.wikipedia.org/wiki/Parallel_computing);
- [Fault-tolerance](https://en.wikipedia.org/wiki/Fault_tolerance) and [attack resilience](https://en.wikipedia.org/wiki/Resilience_(network));
- User-level [hierachical scheduling](https://en.wikipedia.org/wiki/Hierarchical_control_system);
- [Full virtualization](https://en.wikipedia.org/wiki/Full_virtualization), [paravirtualization](https://en.wikipedia.org/wiki/Paravirtualization) and [container](https://en.wikipedia.org/wiki/Operating-system-level_virtualization);
- [Non-volatile memory (NVM)](https://en.wikipedia.org/wiki/Non-volatile_memory) based systems;
- [Network function virtualization (NFV)](https://en.wikipedia.org/wiki/Network_function_virtualization) applications;
- Real-time multi-core [mixed-criticality](https://en.wikipedia.org/wiki/Mixed_criticality) systems, and so on.

&ensp;&ensp;The manual of the operating system can be found **[here](/Document/Public)**.

&ensp;&ensp;Read **[Contributing](CONTRIBUTING.md)** and **[Code of Conduct](CODE_OF_CONDUCT.md)** if you want to contribute, and **[Pull Request Template](PULL_REQUEST_TEMPLATE.md)** when you make pull requests.
This software is an official work of EDI, and thus belongs to the **public domain**. All copyrights reserved by EDI are granted to all entities under all applicable laws to the maximum extent.

&ensp;&ensp;For vendor-supplied packages and hardware abstraction libraries, please refer to the **[M0P00_Library](https://github.com/EDI-Systems/M0P00_Library)** repo to download and use them properly.

## Why a New Microkernel?
&ensp;&ensp;Microkernels have been invented for at least 30 years, and numerous great designs that emphasis performance, parallelism, fault-tolerance, security and even formal correctness have appeared. However, **none of them were able to chart a concord abstraction over all computing devices**, which harms software portability and ecosystem coherence. More precisely, (1) few of them support cloud native environments with a high level of parallelism, (2) even fewer of them support microcontrollers with scarce resources, and (3) none of them support the two extremes and the continuum between them with a single kernel design. Moreover, the configurablity of existing systems were also restricted, as **very few of them allow you to pick the exact very lines of code that you need**. If the system was not designed with configurablity and reusability in mind, adding them as an afterthought probably won't end very well.

&ensp;&ensp;The first goal of RME is to enable **hyperadaptability**. It conjures an overarching abstraction over all types of hardwares, regardless of whether they are cloud servers, edge routers, terminal nodes, or even battery-powered devices. When creating such an abstraction, we aim to **discover and express true common ground between different hardware models rather than coercing one into another**. To put differently, the same abstraction and the resulting implementation should be practically usable on both microprocessors and microcontollers: when deployed on the former, it can keep up with the state-of-the-art cloud-native operating systems while providing more parallelism; when deployed on the latter, it can keep up with the state-of-the-art RTOSes while providing more security. This unravels previously unaware difficulties and research opportunities, as concerns from different application fields are intertwined with each other: (1) high parallelism, (2) hard real-time responsiveness, (3) extreme resource scalability, (4) capability-based security and (5) user-level resource management policy must be **simultaneously** taken care of.

&ensp;&ensp;The second goal of RME is to enable **hyperreusability**. All system components including drivers and middlewares are fully decoupled, and written in a way that could be configured and deployed as a standalone software package. This allows them **to be used with other operating systems, or even in cases where no operating system is present**. The components are coded in a special fashion so that **function pointers can be totally eliminated when only one instance of the library is present**, minimizing redundancy and maximizing resource efficiency. Ideally, if the components were configured correctly, the resulting image will not contain a single line of garbage code. Stripping away unused code also helps with system security, as the vulnerabilities from ununsed garbage code would not be carried into the resulting image.

&ensp;&ensp;The last goal of RME is to enable **hyperdeployability**. Many microkernels exist for a sole purpose - education or research, and their lack of software engineering concerns makes them impossible to use in commercial or even hobbyist projects. RME is different in that it **deliberately embraces the engineering difficulties**, and is willing to expend resources to **support as many production environments as possible**. This **particularly applies to the microcontrollers**, where the compilation toolchain is esoteric and the debugging facilities are restricted in nature. To this end, the **[RVM](https://github.com/EDI-Systems/M7M02_Ammonite)** microcontroller hypervisor takes care of every dirty work, and ready-to-use **Keil** or **Makefile** projects are just one click away. More toolchains such as **IAR** are planned as well. 

### Kernel Design

**Guidelines**
&ensp;&ensp;We believe that the following principles are self-evident without need of further explanation:
- Ideals are cheaper than ideas, ideas are cheaper than real deployable code.
- In theory, there is no difference between practice and theory; in practice there is.
- All performance issues are caused by an obscure division of rights and duties.
- Good overall system design are always preferred over peephole optimizations.
- True useful adaptability is better than false demonstrational portability.
- You have a wrong abstraction if you coerce some hardware model into it.

**Permission**

**Scheduler**

**Memory**

**Execution**

**Communication**

**Miscellaneous**

Composite, seL4

### User-level Library Design

### Microcontroller Components
&ensp;&ensp;All planned components are listed below. If a github link is provided, the component is available for now.  
- **[RVM](https://github.com/EDI-Systems/M7M02_Ammonite)**, a hypervisor capable of running multiple bare-metal applications or RTOSes simutaneously. Scales up to a maximum of 32 virtual machines on 128k RAM.
- **[RVM/RMP](https://github.com/EDI-Systems/M5P01_Prokaron)**, a port of the simplistic RMP RTOS to RVM.
- **[RVM/FreeRTOS](https://github.com/EDI-Systems/M7M02_Ammonite/Guest/A7M/FRT)**, a port of the FreeRTOS to RVM.
- **[RVM/RT-Thread](https://github.com/EDI-Systems/M7M02_Ammonite/Guest/A7M/RTT)**, a port of the RT-Thread to RVM.

### Microprocessor Components
- Work-in-progress. We use to have a bootable x64 port, but currently it is not working.

## Performance on all Supported Architectures

&ensp;&ensp;The timing performance of the kernel in **real action** is shown as follows. All compiler options are the highest optimization (usually -O3 with LTO when available) and optimized for time, and all values are **average case** in CPU cycles.
- Yield/S : Intra-process thread yield to itself.
- Yield/2 : Inter-process thread yield, one-way.
- Inv/2   : Inter-process invocation call/return pair. 
- Sig/1   : Intra-process signal endpoint send/receive latency. 
- Sig/2   : Inter-process signal endpoint send/receive latency. 
- Sig/S   : Intra-process signal endpoint send/receive pair.
- Sig/I   : Interrupt signal endpoint send/receive latency.

### Microcontroller

&ensp;&ensp;The **absolute minimum** value for RME on microcontrollers is about **64k ROM and 20k RAM**, which is reached on the STM32L071CB (Cortex-M0+) port. This is the absolute minimum proof-of-concept that can finish the benchmark (alongside a virtualized RMP benchmark as well). RME also requires that the microcontroller be equipped with a Memory Protection Unit (MPU), with which we can confine the processes to their own address spaces.

&ensp;&ensp;The use of RVM as a user-level library is required on microcontrollers, which supports automatic generation of projects against multiple architectures, toolchains, and IDEs. It also enables virtualization on even microcontrollers, which allows seamless integration with existing bare-metal code or RTOSes. Only single-core microcontrollers are supported; multi-core support for microcontrollers is currently out of scope.

|Chipname     |Platform    |Clock |Build |Yield/S|Yield/2|Inv/2|Sig/1|Sig/2|Sig/S|Sig/I|
|:-----------:|:----------:|:----:|:----:|:-----:|:-----:|:---:|:---:|:---:|:---:|:---:|
|STM32L071CB  |Cortex-M0+  |36M   |Keil  |492    |763    |956  |718  |810  |749  |522  |
|...          |...         |...   |GCC   |513    |799    |939  |736  |830  |776  |534  |
|STM32F405RG  |Cortex-M4F  |168M  |Keil  |324    |524    |692  |576  |731  |568  |420  |
|...          |...         |...   |GCC   |332    |520    |684  |608  |735  |540  |416  |
|STM32F767IG  |Cortex-M7F  |216M  |Keil  |264    |400    |600  |438  |484  |477  |282  |
|...          |...         |...   |GCC   |294    |456    |644  |406  |460  |429  |321  |
|CH32V307VC   |RV32IMAFC   |168M  |GCC   |358    |669    |706  |703  |723  |624  |523  |

&ensp;&ensp;**FAQ:** Can XXX (some microcontroller) be supported? **Answer**: If your microcontroller has **at least 64k ROM and 20k RAM**, and features a memory protection unit, just like the majority of Cortex-M microcontrollers, then yes! However, it's best to have **128k ROM and 128k RAM** to start with, as such systems will be useful in real projects. Some **sub-$1 chips such as STM32G0B1CBT6** are more than sufficient.

### Microprocessor

&ensp;&ensp;The **recommended** resource for RME on microprocessors is about **32M ROM and 32M RAM**, which is reached on the F1C100S (ARM926EJ-S) port. Although this is not the absolute minimum to run the benchmark which requires far less memory, this is indeed required for a meaningful and useful system. The microprocessor must be equipped with a Memory Management Unit (MMU), with which we can confine the processes to their own address spaces. For microprocessors, we only accept the GCC (and probably CLANG as well later) toolchain. Support for other toolchains are out of the scope. 

&ensp;&ensp;The use of RMC (concept design in progress) is required on microprocessors, which allows integration of feather-weight unix-like containers, unikernels and RTOSes on to the same platform. This is achieved without specific extensions like the hardware virtualization extension; and when there is, we strive to provide full virtualization environments where you can boot Windows and Linux. We would not delve into the drivers though, and will assume a pass-through model for all peripherals in these cases. This provides less flexibility, but makes it possible to use existing software investments (desktop environments, industry applications, and even 3D games) with zero hassle. The following metrics only make sense for RMC's unix containers:

- Pipe   : Inter-process pipe send/receive latency.
- Sem    : Inter-process semaphore send/receive latency.
- Msgq   : Inter-process message queue send/receive latency. 
- Signal : Inter-process unix signal trigger/activate latency. 
- Socket : Inter-process localhost socket send/receive latency.

|Chipname     |Platform    |Bits|Cores|Yield/S|Yield/2|Inv/2|Sig/1|Sig/2|Sig/S|Sig/I|Pipe |Sem  |Msgq |Signal|Socket|
|:-----------:|:----------:|:--:|:---:|:-----:|:-----:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:----:|:----:|
|F1C100S      |ARM926EJ-S  |32  |1    |TBD    |TBD    |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD   |TBD   |
|S3C2416      |...         |32  |1    |TBD    |TBD    |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD   |TBD   |
|XC7Z010      |Cortex-A9   |32  |2    |TBD    |TBD    |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD   |TBD   |
|XCZU2EG      |Cortex-A53  |64  |4    |TBD    |TBD    |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD   |TBD   |
|AWT-D1S      |RV64IMAFCV  |64  |1    |TBD    |TBD    |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD   |TBD   |
|LS1C300B     |GS232       |32  |1    |TBD    |TBD    |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD   |TBD   |
|LS2K300      |LA264       |64  |1    |TBD    |TBD    |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD   |TBD   |
|E5-2696 v2   |x86-64      |64  |12   |TBD    |TBD    |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD   |TBD   |
|TMS320C6678  |C66x        |64  |8    |TBD    |TBD    |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD  |TBD   |TBD   |

&ensp;&ensp;**FAQ:** Why is XXX (some popular board) not supported? **Answer:** Unlike microcontrollers, some microprocessor manufacturers put their datasheets behind a (very) high paywall. Nothing is open; and to support these boards, we would have to **reverse engineer** the details from their Linux drivers. A few manufacturers are notably notorious for this, and we'd rather leave them alone and focus on manufacturers that embrace openness. We do make exceptions for chips that are already thoroughly reverse engineered though.

## Getting Started

&ensp;&ensp;These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

&ensp;&ensp;You need to choose a hardware platform listed above to run the system. This project focuses on high-performance platforms that are at least 32-bit and does not support lower-end microcontrollers that are 8/16- bit or microcontrollers lacking memory protection unit, or legacy microprocessors prior to ARM9. For 8/16-bit microcontrollers, use [RMP](https://github.com/EDI-Systems/M5P01_Prokaron) Real-Time Kernel instead; RMP supports all Cortex-Ms and some Cortex-Rs, though without memory protection support.

&ensp;&ensp;If you do not have a standalone hardware platform, you can also use VMMs such as KVM, VMware and Virtual Box to try out the x86-64 ISO image. Do not use QEMU to test the projects because they do not behave correctly in many scenarios. You can use a x86-64 QEMU though because it is wery well tested and uses KVM as its underlying virtualization engine.

### Compilation
**Microcontroller**  
&ensp;&ensp;Please refer to the **[RVM](https://github.com/EDI-Systems/M7M02_Ammonite)** repo for details; RME includes neither projects nor compilation instructions for them.

**Microprocessor**  
&ensp;&ensp;Work-in-progress.

### Running the tests
**Mircocontoller**  
&ensp;&ensp;To run the sample programs, simply download them into the development board and start step-by-step debugging. All hardware the example will use is the serial port, and it is configured for you in the example.

**Microprocessor**  
&ensp;&ensp;Work-in-progress.

### Deployment
**Mircocontoller**  
&ensp;&ensp;Please refer to the **[RVM](https://github.com/EDI-Systems/M7M02_Ammonite)** project for details, and refrain from manually modifying configuration files and creating projects. These daunting tasks are better performed by the RVM project generator.

**Microprocessor**  
&ensp;&ensp;Deploy it as if you are deploying any other operating system (Linux, etc).

### Supported Toolchains

- GCC/Clang-LLVM
- Keil uVision (ARMCC/ARMCLANG)

&ensp;&ensp;Other toolchains are neither recommended nor supported at this point, though it might be possible to support them later on.

## Contributing

&ensp;&ensp;Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

&ensp;&ensp;We wish to thank the developers of [Composite](https://github.com/gwsystems/composite) system which is developed at George Washington University (and RME is heavily influenced by it), and we also wish to thank the developers of [Fiasco.OC](https://os.inf.tu-dresden.de/fiasco) system which is developed at TU Dresden. 

### EDI Project Information
- M7M01 R3T1

## Frequently Asked Questions

### What are Capabilities?
&ensp;&ensp;Capabilities are a kind of certificate that is initially introduced into multi-user computer systems to control access permissions. They are unforgeable tokens that point to some resource and carry permissions to allow operations on the object. In some sense, the Unix file descriptor can be treated as a type of capability; the Windows access permissions can also be treated as a type of capability. Generally speaking, capabilities are fat pointers that points to some resources.  We guarantee the safety of the system with the three rules:
- Capabilities cannot be modified at user-level;
- Capabilities can only be transfered between different processes with well-defined interfaces;
- Capabilities will only be given to processes that can operate on the corresponding resources.

### Why do We Need Capability-based Systems?
&ensp;&ensp;The idea of capability is nothing new. Thousands of years ago, kings and emperors have made dedicated tokens for their generals to command a specific branch or group of their army. Usually, these tokens will contain unforgeable (or at least, very difficult to fake) alphabets or characters indicating what powers the general should have, and which army can they command, thus safely handing the army commanding duty off to the generals. In the same sense, capability-based systems can provide a very fine grain of resource management in a very elegant way. By exporting policy through combinations of different capabilities to the user-level, capability-based systems reach a much greater level of flexibity when compared to traditional Unix systems. Additional benefits include increased isolation, fault confinement and ease of formal analysis.

### Wouldn't the Microkernel Design Harm System Execution Efficiency?
&ensp;&ensp;Short answer: **No**.  
&ensp;&ensp;Long answer: If **designed carefully and used correctly** (especially the communication mechanisms), it would instead **greatly boost performance** in multiple aspects, because the fast-paths are much more aggressively optimized now. For example, on some architectures, the context switch performance and interrupt response performance can be up to **40x** better than RT-Linux. When user-level library overheads are also included, the result is still **25x** better than RT-Linux.

### How is It Possible that the System is Lock-free?
&ensp;&ensp;This is made possible by extensively applying lock-free data structures and atomic operations. For more information, please refer to [this article](https://www.cs.tau.ac.il/~shanir/concurrent-data-structures.pdf).

### Why don't this Kernel Aim at Full POSIX/Linux Compilance?

### What are Microcontrollers and Why are They Important?

### List of System Calls

|System call            |Number|Description                                                       |
|:---------------------:|:----:|:----------------------------------------------------------------:|
|RME_SVC_INV_RET        |0     |Return from an invocation                                         |
|RME_SVC_INV_ACT        |1     |Activate an invocation                                            |
|RME_SVC_SIG_SND        |2     |Send to a signal endpoint                                         |
|RME_SVC_SIG_RCV        |3     |Receive from a signal endpoint                                    |
|RME_SVC_KFN            |4     |Call a kernel function                                            |
|RME_SVC_THD_SCHED_FREE |5     |Free a thread from its current processor                          |
|RME_SVC_THD_EXEC_SET   |6     |Set entry and stack of a thread                                   |
|RME_SVC_THD_SCHED_PRIO |7     |Changing thread priority                                          |
|RME_SVC_THD_TIME_XFER  |8     |Transfer time to a thread                                         |
|RME_SVC_THD_SWT        |9     |Switch to another thread                                          |
|RME_SVC_CPT_CRT        |10    |Create a capability table                                         |
|RME_SVC_CPT_DEL        |11    |Delete a capability table                                         |
|RME_SVC_CPT_FRZ        |12    |Freeze a capability                                               |
|RME_SVC_CPT_ADD        |13    |Delegate a capability                                             |
|RME_SVC_CPT_REM        |14    |Remove a capability                                               |
|RME_SVC_PGT_CRT        |15    |Create a page table                                               |
|RME_SVC_PGT_DEL        |16    |Delete a page table                                               |
|RME_SVC_PGT_ADD        |17    |Add a page to a page table                                        |
|RME_SVC_PGT_REM        |18    |Remove a page from a page table                                   |
|RME_SVC_PGT_CON        |19    |Construct a page table into another                               |
|RME_SVC_PGT_DES        |20    |Destruct a page table from another                                |
|RME_SVC_PRC_CRT        |21    |Create a process                                                  |
|RME_SVC_PRC_DEL        |22    |Delete a process                                                  |
|RME_SVC_PRC_CPT        |23    |Change a process's capability table                               |
|RME_SVC_PRC_PGT        |24    |Change a process's page table                                     |
|RME_SVC_THD_CRT        |25    |Create a thread                                                   |
|RME_SVC_THD_DEL        |26    |Delete a thread                                                   |
|RME_SVC_THD_SCHED_BIND |27    |Bind a thread to the current processor                            |
|RME_SVC_THD_SCHED_RCV  |28    |Try to receive scheduling notifications                           |
|RME_SVC_SIG_CRT        |29    |Create a signal endpoint                                          |
|RME_SVC_SIG_DEL        |30    |Delete a signal endpoint                                          |
|RME_SVC_INV_CRT        |31    |Create a synchronous invocation port                              |
|RME_SVC_INV_DEL        |32    |Delete a synchronous invocation port                              |
|RME_SVC_INV_SET        |33    |Set entry and stack of a synchronous invocation port              |
|...                    |34-63 |Reserved                                                          |
