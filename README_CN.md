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

&emsp;&emsp;本系统的手册可以在 **[这里](https://github.com/EDI-Systems/M5P1_MuProkaron/blob/master/Documents/M5P1_%E8%BD%BB%E9%87%8F%E7%BA%A7%E5%AE%9E%E6%97%B6%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E%E4%B9%A6.pdf)** 找到。

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
- **[RVM](https://github.com/EDI-Systems/M7M2_MuAmmonite)**，一个面向微控制器的虚拟机监视器，可以在一个MCU上运行多个RTOS。它最多可以在1MB片上内存中运行多达64个虚拟机。
    - **[RVM/Lib](https://github.com/EDI-Systems/M7M2_MuAmmonite)**，一份RME的微控制器用户态库。
    - **[RVM/RMP](https://github.com/EDI-Systems/M5P1_MuProkaron)**，RMP在RME上的一个全功能移植。
    - **[RVM/FreeRTOS](https://github.com/EDI-Systems/FreeRTOS)**，广为应用的[FreeRTOS](https://www.freertos.org/)在RVM上的一个全功能移植。
    - **[RVM/RT-Thread](https://github.com/EDI-Systems/rt-thread)**，颇有前景的[RT-Thread](https://www.rt-thread.org/)在RVM上的一个全功能移植，包括其所有框架。
    - **[RVM/uCOSIII](https://github.com/EDI-Systems/uCOSIII)**，著名的[uC/OS III](https://www.micrium.com/)在RVM上的一个移植。如果要在产品中使用该系统，你应当持有该系统的商业许可。
    - **[RVM/MicroPython](https://github.com/EDI-Systems/micropython)**，广为应用的[MicroPython](https://micropython.org/)在RVM上的一个移植。
    - **[RVM/Lua](https://github.com/EDI-Systems/lua)**，广为应用的[Lua](https://www.lua.org/)在RVM上的一个移植。
    - **[RVM/Duktape](https://github.com/EDI-Systems/duktape)**，新出现的[JavaScript](https://github.com/svaarala/duktape)在RVM上的一个移植。
    - **[RVM/Essentials](https://github.com/EDI-Systems/M5P1_MuProkaron)**，一个包含了[lwip](https://savannah.nongnu.org/projects/lwip/)、[fatfs](http://elm-chan.org/fsw/ff/00index_e.html)和[emWin](https://www.segger.com/products/user-interface/emwin/)的[RMP](https://github.com/EDI-Systems/M5P1_MuProkaron)的RVM移植。要在产品中使用该系统，你应当持有相应的商业许可。

- UVM，一个面向应用处理器和服务器的虚拟机监视器，能够以前所未有的性能实现全虚拟化和基于容器的半虚拟化。
    - UVM/Lib，一份RME的微处理器用户态库。
    - UVM/FV，一个基于UVM的全虚拟化平台，可实现类似Virtual Box的功能。

## 系统调用列表


### 所有受支持架构上的典型性能数据
**单核微控制器**

|架构          |工具链        |Flash|SRAM|Yield|Asnd1|Asnd2|Sinv|Sret|Isnd|
|:-----------:|:------------:|:---:|:--:|:---:|:---:|:---:|:--:|:--:|:--:|
|Cortex-M4    |Keil uVision 5|     |    |     |     |     |    |    |    |
|Cortex-M7    |Keil uVision 5|     |    |     |     |     |    |    |    |
|Cortex-R4    |TI CCS7       |     |    |     |     |     |    |    |    |
|Cortex-R5    |TI CCS7       |     |    |     |     |     |    |    |    |
|MIPS M14k    |XC32-GCC      |     |    |     |     |     |    |    |    |

*Cortex-R4和Cortex-R5在这里被列为单核架构，因为它们的主要卖点是CPU冗余。因此从开发者的视角看来，它们的行为和单核系统一致。Cortex-R4和Cortex-R5上的双核模式不被RME支持。

&emsp;&emsp;**Flash和SRAM消耗以kB计，其他数据以CPU指令周期计。这里列出的所有值都是典型（有意义的系统配置）值而非绝对意义上的最小值，因为纯技术层面的最小配置在实际工程中很少是真正有用的。HAL库所造成的额外存储器消耗也被计算在内。在单核微控制器模型下，本系统的绝对最小值在32k ROM/16k RAM左右。**

- Cortex-M4平台使用STM32F405RGT6进行评估。
- Cortex-M7平台使用STM32F767IGT6进行评估。
- Cortex-R4平台使用TMS570LS0432进行评估。
- Cortex-R5平台使用TMS570LC4357进行评估。
- MIPS M14k平台使用PIC32MZ2048EFM100进行评估。

**多核微控制器**

|架构          |工具链        |Flash|SRAM|Yield|Asnd1|Asnd2|Sinv|Sret|Isnd|
|:-----------:|:------------:|:---:|:--:|:---:|:---:|:---:|:--:|:--:|:--:|
|Cortex-R7    |TBD           |     |    |     |     |     |    |    |    |
|Cortex-R8    |TBD           |     |    |     |     |     |    |    |    |
|TMS320C66X   |TI CCS7       |     |    |     |     |     |    |    |    |

&emsp;&emsp;**Flash和SRAM消耗以kB计，其他数据以CPU指令周期计。HAL库所造成的额外存储器消耗也被计算在内。在多核微控制器模型下，本系统的绝对最小值在64k ROM/32k RAM左右。**

- Cortex-R7平台使用（尚未决定）进行评估。
- Cortex-R8平台使用（尚未决定）进行评估。
- TMS320C66X平台使用TMS320C6678进行评估。

**多核应用微处理器（桌面和服务器处理器）**

|架构          |工具链        |.text|.data|Yield|Asnd1|Asnd2|Sinv|Sret|Isnd|
|:-----------:|:------------:|:---:|:---:|:---:|:---:|:---:|:--:|:--:|:--:|
|Cortex-A7  x4|GCC           |     |     |     |     |     |    |    |    |
|Cortex-A53 x4|GCC           |     |     |     |     |     |    |    |    |
|X86-64(I) x18|GCC           |     |     |     |     |     |    |    |    |
|X86-64(NI)x32|GCC           |     |     |     |     |     |    |    |    |
|X86-64(A) x16|GCC           |     |     |     |     |     |    |    |    |

&emsp;&emsp;**RAM消耗以MB计，其他数据以CPU时钟周期计。必要的软件包和驱动造成的额外消耗也被包括在内。在多核应用微处理器模型下，本系统的绝对最小值在4MB RAM左右。**

- Cortex-A7平台使用BCM2836进行评估，它也是Raspberri Pi 2的主芯片。
- Cortex-A53平台使用BCM2837进行评估，它也是Raspberri Pi 3的主芯片。
- X86-64(I)平台使用一台具备单颗I9-7980XE处理器和128GB内存的PC进行评估。
- X86-64(NI)平台使用一台具备四颗Xeon X7560处理器和4条8GB内存条（分别插于四个处理器上）的服务器进行评估。
- X86-64(A)平台使用一台具备单颗Ryzen 1950X处理器和128GB内存的PC进行评估。 

&emsp;&emsp;在上面所列的三个表格中，所有的编译器选项都被设置为最高优化（通常是-O3），而且针对运行时间进行了优化。
- Yield   ：同一进程内部两线程间进行切换所用的时间。  
- Asnd1   ：进程内两线程异步通信时间。  
- Asnd2   ：进程间两线程异步通信时间。  
- Sinv    ：线程迁移调用的进入耗时。  
- Sret    ：线程迁移调用的退出耗时。 
- Isnd    ：从中断发送异步信号的耗时。

## 新手上路

&emsp;&emsp;下面的说明会帮助你在本地快速建立一个可用来评估测试本系统的工程。请参看系统的中文文档以获取更多信息。

### 准备工作

&emsp;&emsp;要运行测试，你需要一个上面列出的硬件平台。本通用操作系统主要面向高性能MCU和CPU，而并不支持低端MCU或者MPU。不要使用QEMU模拟器来测试本系统，因为QEMU有很多不完善之处，与真正的硬件行为并不一致。

&emsp;&emsp;如果你没有单独的硬件平台，那么也可以使用VMware、Virtual Box等虚拟机软件运行本系统的x86-64 ISO镜像。

&emsp;&emsp;对于其他平台的支持应该也是容易实现的，但是当前并没有支持计划。对于低端Cortex-M和大多数16位微控制器，，可以使用[M5P1_MuProkaron](https://github.com/EDI-Systems/M5P1_MuProkaron) _轻量级实时操作系统_；M5P1支持全部的Cortex-M和部分的Cortex-R，但是不提供内存保护。

### 编译指南
**微控制器**  
&emsp;&emsp;在 **_Project_** 文件夹下能够找到多种微控制器的移植好的 **厂商集成开发环境** 或 **Eclipse** 的工程样板。参看各个工程文件夹下的自述文件以获取更多关于如何编译和运行该工程的信息。某些工程需要额外的厂商硬件抽象层库的支持，它们可以在 **[M0P0_Library](https://github.com/EDI-Systems/M0P0_Library)** 软件仓库被找到。

**微处理器**  
&emsp;&emsp;仅有做好的GNU Makefile工程提供，而且只有GCC和Clang-LLVM工具链受支持。如果你有GCC兼容的编译器，那么也是可以支持的。

### 运行测试
**微控制器**  
&emsp;&emsp;要运行测试，只要将测试下载到对应的开发板并开始单步调试即可。某些例子会采用两个LED来指示系统当前的状态，此时要填充LED的点亮和熄灭函数来运行该示例。

**微处理器**   
&emsp;&emsp;对于每种微处理器，启动流程都不同。对于x86-64架构，本系统使用GRUB作为其启动器，并且你可以像安装其他任何操作系统（如Ubuntu或Windows）一样安装它的Live CD；对于其他嵌入式架构则采用U-Boot作为其启动器。

### 生产部署
**微控制器**  
&emsp;&emsp;当部署本系统到生产环境时，请仔细阅读本系统自带的手册，以确保各项配置正确。本系统的手册可以在 **_Documents_** 文件夹下找到。不推荐由用户自己配置内核；它包含太多的细节。请尽量使用提供好的默认配置文件。此外，一定要阅读对应架构的用户手册。

**微处理器**  
&emsp;&emsp;使用部署其他操作系统或者虚拟机监视器的方法直接部署这个系统。

## 支持的工具链

- Keil uVision 5 (armcc)
- Code composer studio
- MPLAB X XC32
- GCC/Clang-LLVM

&emsp;&emsp;其他的工具链现在不推荐或者当前不受支持，虽然要增加新的支持应该也很简单。

## 参与项目

&emsp;&emsp;请阅读[CONTRIBUTING.md](CONTRIBUTING.md)文档来获得关于行为规范和提交代码的相关信息。

## EDI 工程信息
&emsp;&emsp;演进 - 中生 - 真核 (M7M1 R3T1)
