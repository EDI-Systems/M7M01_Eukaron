<h1 align="center">
	<img width="300" src="https://raw.githubusercontent.com/EDI-Systems/M7M1_MuEukaron/master/Documents/Demo/logo.png" alt="logo">
</h1>

# Unity OS (RME)

Click **[HERE](README.md)** for English version.

&emsp;&emsp;**RME** 是一个支持很多高级功能的系统内核. 这个系统支持许多其他系统如 _FreeRTOS_ 、 _RT-Thread_ 和 _VxWorks_ 等不支持的复杂高级功能，在多核方面则具备和 _Linux_ 一样极佳的可扩展性。在使用时，无需了解系统的方方面面，仅需要使用我们提供好的移植甚至二进制即可。本系统的一些高级功能包括：
- 基于[权能](https://en.wikipedia.org/wiki/Capability-based_security)（capability）的可变保护域（mutable protection domain）；
- 高度的[可扩展性](https://en.wikipedia.org/wiki/Scalability)（Scalability）和[并行性](https://en.wikipedia.org/wiki/Parallel_computing)（parallelism）；
- [容错性](https://en.wikipedia.org/wiki/Fault_tolerance)（fault-tolerance）和[抗攻击性](https://en.wikipedia.org/wiki/Resilience_(network))（attack resilience）；
- 用户态[层次化调度](https://en.wikipedia.org/wiki/Hierarchical_control_system)（hierachical scheduling）；
- [全虚拟化](https://en.wikipedia.org/wiki/Full_virtualization)技术和基于[容器](https://en.wikipedia.org/wiki/Operating-system-level_virtualization)（container）的[准虚拟化](https://en.wikipedia.org/wiki/Paravirtualization)（paravirtualization）技术；
- [非易失性内存](https://en.wikipedia.org/wiki/Non-volatile_memory)（Non-volatile memory，NVM）技术应用；
- 实时（real-time）多核[混合关键度](https://en.wikipedia.org/wiki/Mixed_criticality)（mixed-criticality）系统等等。

&emsp;&emsp;如果想要参与开发，请阅读 **[参与](CONTRIBUTING.md)** 和 **[规范](CODE_OF_CONDUCT.md)** 两个指导文档。如果要提交拉取请求，请使用 **[拉取请求模板](PULL_REQUEST_TEMPLATE.md)** 。
本软件采用 **三种不同的授权** ：你可以选择 **[LGPL v3](LICENSE.md)** ，也可以选择 **[经修改的MIT协议](MODMIT.md)** 。 如果有特殊需求， 也可以联系我们请求**商业授权**。

&emsp;&emsp;对于那些由微控制器厂商提供的硬件抽象层软件包，请到 **[M0P0_Library](https://github.com/EDI-Systems/M0P0_Library)** 软件仓库自行下载。

## 基于权能的多核系统简介

### 权能是什么？
&emsp;&emsp;权能是一种最早在多用户计算机系统中引入的用来控制访问权限的凭证。它是一种不可伪造的用来唯一标识某种资源以及允许对该资源所进行的操作的的凭证。比如，Unix的文件描述符就是一种权能；Windows的访问权限也是一种权能。从某种意义上讲，权能就是一个指向某种资源的胖指针。我们使用如下三条原则来保证系统的安全性：
- 权能是不可伪造和不可在用户态擅自修改的；
- 进程只能用良好定义的授权接口获取权能；
- 权能只会被给予那些系统设计时负责管理该资源的进程。

### 我们为什么需要基于权能的系统？
&emsp;&emsp;使用权能来进行权限控制是个很老的点子了。几千年以前，皇帝和国王们制作一种特别的符文，用来授予他们的将军调兵遣将的能力。通常而言，这些符文包含了不可复制的（或者极其难以复制的）文字，上面规定了哪一支或哪一种部队可以被调动。这样，皇帝就能把管理军队的任务安全地交给将军。同样地，基于权能的操作系统能够极为巧妙地提供非常细粒度的权限管理。系统中的所有权限都由权能管理，这样就可以由用户级程序来定义具体的系统策略，因此比传统的Unix系统的灵活性要好得多。其他的好处还包括强化的安全边界，彻底的错误隔离和容易进行形式化分析。

### 微内核设计不会拖累执行效率吗？
&emsp;&emsp;简而言之： **不会** 。  
&emsp;&emsp;详细解释：如果系统 **被设计的很好，并且使用方法也正确** 的话（尤其指通信机制），微内核设计实际上有助于在多个方面 **提高系统的效率** ，因为那些经常被访问的路径现在相当于被特别地大大优化了。实际上，在某些架构上，RME的线程切换效率和中断响应速度比RT-Linux能够快出整整40倍。当用户态库的时间消耗也被计算在内时，结果仍然比RT-Linux好整整25倍。

### 系统是如何做到无锁的？
&emsp;&emsp;这是通过大量使用无锁数据结构和原子操作做到的。有关无锁操作和原子操作的更多知识请查看 [这篇文章](https://www.cs.tau.ac.il/~shanir/concurrent-data-structures.pdf).

## 现有的系统组件
&emsp;&emsp;所有的现有系统组件列于下表。如果提供了github链接，那么该组件现在就是可用的。  
- [RVM](https://github.com/EDI-Systems/M7M2_MuAmmonite), which is a microcontroller-oriented virtual machine monitor capable of running multiple MCU applications or operating systems simutaneously. Scales up to 64 virtual machines on 1MB On-Chip SRAM.
    - [RVM/Lib](https://github.com/EDI-Systems/M7M2_MuAmmonite), the microcontroller-oriented user-level library for RME.
    - [RVM/RMP](https://github.com/EDI-Systems/M5P1_MuProkaron), a port of the simplistic RMP on RVM, with all functionalities retained.
    - [RVM/FreeRTOS](https://github.com/EDI-Systems/FreeRTOS), a port of the widely-used [FreeRTOS](https://www.freertos.org/) to RVM.
    - [RVM/RT-Thread](https://github.com/EDI-Systems/rt-thread), a port of the promising [RT-Thread](https://www.rt-thread.org/) to RVM, with all frameworks retained.
    - [RVM/uCOSIII](https://github.com/EDI-Systems/uCOSIII), a port of the famous [uC/OS III](https://www.micrium.com/) to RVM. You should have a commercial license to use this port.
    - [RVM/MicroPython](https://github.com/EDI-Systems/micropython), a port of the popular [MicroPython](https://micropython.org/) to RVM.
    - [RVM/Lua](https://github.com/EDI-Systems/lua), a port of the easy-to-use [Lua](https://www.lua.org/) language to RVM.
    - [RVM/Duktape](https://github.com/EDI-Systems/duktape), a port of the emerging [JavaScript](https://github.com/svaarala/duktape) language to RVM.
    - [RVM/Essentials](https://github.com/EDI-Systems/M5P1_MuProkaron), a port of [lwip](https://savannah.nongnu.org/projects/lwip/), [fatfs](http://elm-chan.org/fsw/ff/00index_e.html) and [emWin](https://www.segger.com/products/user-interface/emwin/) to RVM, all packed in one [RMP](https://github.com/EDI-Systems/M5P1_MuProkaron) virtual machine. Be sure to obtain license to use these softwares.

- UVM, which is a multi-core processor oriented virtual machine monitor capable of supporting full-virtualization and container-based virtualization with unprecedented performance.
    - UVM/Lib, the microprocessor-oriented user-level library for RME.
    - UVM/FV, the full virtualization platform constructed with UVM, which comes with similar functionalities as Virtual Box.

## List of system calls


### Typical performance figures for all supported architectures
**Single-core microcontrollers**

|Machine      |Toolchain     |Flash|SRAM|Yield|Asnd1|Asnd2|Sinv|Sret|Isnd|
|:-----------:|:------------:|:---:|:--:|:---:|:---:|:---:|:--:|:--:|:--:|
|Cortex-M4    |Keil uVision 5|     |    |     |     |     |    |    |    |
|Cortex-M7    |Keil uVision 5|     |    |     |     |     |    |    |    |
|Cortex-R4    |TI CCS7       |     |    |     |     |     |    |    |    |
|Cortex-R5    |TI CCS7       |     |    |     |     |     |    |    |    |
|MIPS M14k    |XC32-GCC      |     |    |     |     |     |    |    |    |

&ensp;&ensp;&ensp;&ensp;**Flash and SRAM consumption is calculated in kB, while the other figures are calculated in CPU clock cycles. All values listed here are typical (useful system) values, not minimum values, because minimum values on system size seldom make any real sense. HAL library are also included in the size numbers. The absolute minimum value for microcontroller-profile RME is about 32k ROM/16k RAM.**

<!-- |Cortex-M4    |GCC           |     |    |     |     |     |    |    |    | -->
<!-- |Cortex-M7    |GCC           |     |    |     |     |     |    |    |    | -->
<!-- |Cortex-R4    |GCC           |     |    |     |     |     |    |    |    | -->

- Cortex-M4 is evaluated with STM32F405RGT6.
- Cortex-M7 is evaluated with STM32F767IGT6.
- Cortex-R4 is evaluated with TMS570LS0432.
- Cortex-R5 is evaluated with TMS570LC4357.
- MIPS M14k is evaluated with PIC32MZEFM100.

**Multi-core MPU-based processors**

|Machine      |Toolchain     |Flash|SRAM|Yield|Asnd1|Asnd2|Sinv|Sret|Isnd|
|:-----------:|:------------:|:---:|:--:|:---:|:---:|:---:|:--:|:--:|:--:|
|Cortex-R7    |TBD           |     |    |     |     |     |    |    |    |
|Cortex-R8    |TBD           |     |    |     |     |     |    |    |    |
|TMS320C66X   |TI CCS7       |     |    |     |     |     |    |    |    |

&ensp;&ensp;&ensp;&ensp;**Flash and SRAM consumption is calculated in kB, while the other figures are calculated in CPU clock cycles. HAL library are also included in the size numbers. The absolute minimum value for MPU-based microprocessor-profile RME is about 64k ROM/32k RAM.**

- Cortex-R7 is evaluated with TBD.
- Cortex-R8 is evaluated with TBD.
- TMS320C66X is evaluated with TMS320C6678.

**Multi-core application processors (aka. Desktop/server processors)**

|Machine      |Toolchain     |.text|.data|Yield|Asnd1|Asnd2|Sinv|Sret|Isnd|
|:-----------:|:------------:|:---:|:---:|:---:|:---:|:---:|:--:|:--:|:--:|
|Cortex-A7  x4|GCC           |     |     |     |     |     |    |    |    |
|Cortex-A53 x4|GCC           |     |     |     |     |     |    |    |    |
|X86-64(I) x18|GCC           |     |     |     |     |     |    |    |    |
|X86-64(NI)x32|GCC           |     |     |     |     |     |    |    |    |
|X86-64(A) x16|GCC           |     |     |     |     |     |    |    |    |

&ensp;&ensp;&ensp;&ensp;**RAM consumption is calculated in MB, while the other figures are calculated in CPU clock cycles. Necessary software packages and drivers are also included in the size numbers. The absolute minimum value for application processor-profile RME is about 4MB RAM.**

- Cortex-A7 is evaluated with BCM2836, the exact chip used on Raspberry Pi 2.
- Cortex-A53 is evaluated with BCM2837, the exact chip used on Raspberry Pi 3.
- X86-64(I) is evaluated with a machine with 1x I9-7980XE processor and 128GB memory.
- X86-64(NI) is evaluated with a machine with 4x Xeon X7560 processor and 4x8GB memory.
- X86-64(A) is evaluated with a machine with Ryzen 1950X processor and 128GB memory. 

&ensp;&ensp;&ensp;&ensp;In the 3 tables above, all compiler options are the highest optimization (usually -O3) and optimized for time. 
- Yield: The time to yield between different threads.  
- Asnd1: Intra-process asynchronous send.
- Asnd2: Inter-process asynchronous send. 
- Sinv: Synchronous invocation entering time. 
- Sret: Synchronous invocation returning time. 
- Isnd: Intra-process interrupt sending time.

## Getting Started

&ensp;&ensp;&ensp;&ensp;These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

&ensp;&ensp;&ensp;&ensp;You need to choose a hardware platform listed above to run the tests. This general-purpose OS focuses on high-performance MCU and CPUs and do not concentrate on lower-end MCUs or legacy MPUs. Do not use QEMU simulator to test the projects because they do not behave correctly in many scenarios.  
Other platform supports should be simple to implement, however they are not scheduled yet. For Cortex-M or 16-bit microcontrollers, go [M5P1_MuProkaron](https://github.com/EDI-Systems/M5P1_MuProkaron) _Real-Time Kernel_ instead; M5P1 supports some Cortex-Ms and Cortex-Rs as well, though without protection support.

### Compilation
**For MCUs**  
&ensp;&ensp;&ensp;&ensp;The **Vendor Toolchain** or **GNU Makefile** projects for various microcontrollers are available in the **_Project_** folder. Refer to the readme files in each folder for specific instructions about how to run them. However, keep in mind that some examples may need vendor-specific libraries such as the STM HAL. Some additional drivers may be required too.

**For application processors**  
&ensp;&ensp;&ensp;&ensp;Only GNU makefile projects will be provided, and only GCC is supported at the moment. Other compilers may also be supported as long as it conforms to the GCC conventions.


### Running the tests
**For MCUs**  
&ensp;&ensp;&ensp;&ensp;To run the sample programs, simply download them into the development board and start step-by-step debugging. All hardware the example will use is the serial port, and it is configured for you in the example.

**For application processors**  
&ensp;&ensp;&ensp;&ensp;Boot the system with precompiled LiveCD.iso, just like how you would install any operating system (Ubuntu Linux or Windows). Follow the instructions and play with it!

### Deployment
**For MCUs**  
&ensp;&ensp;&ensp;&ensp;When deploying this into a production system, it is recommended that you read the manual in the **_Documents_** folder carefully to configure all options correctly. It is not recommended to configure the kernel yourself, anyway; it included too many details. Please use the default configuration file as much as possible. Also, read the user guide for the specific platform you are using.

**For application processors**  
&ensp;&ensp;&ensp;&ensp;Deploy it as if you are deploying any other operating system, or bare-metal hypervisor.

## Built With

- Keil uVision 5 (armcc)
- Code composer studio
- GCC/Clang-LLVM

&ensp;&ensp;&ensp;&ensp;Other toolchains are neither recommended nor supported at this point, though it might be possible to support them later on.

## Contributing

&ensp;&ensp;&ensp;&ensp;Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

## License

&ensp;&ensp;&ensp;&ensp;This project is licensed under the GPLv3 License - see the [LICENSE.md](LICENSE.md) file for details. However, commercial licenses are also available.

## EDI Project Information
&ensp;&ensp;&ensp;&ensp;Mutate - Mesazoa - Eukaron (M7M1 R3T1)
