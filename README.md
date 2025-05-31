<h1 align="center">
	<img width="300" src="https://raw.githubusercontent.com/EDI-Systems/M7M01_Eukaron/master/Document/Public/Demo/logo.png" alt="logo">
</h1>

# RME Concord Microkernel
<div align="center">

[![Github release](https://img.shields.io/github/release/EDI-Systems/M7M01_Eukaron.svg)](https://github.com/EDI-Systems/M7M01_Eukaron/releases/latest)
[![Github commits](https://img.shields.io/github/commits-since/EDI-Systems/M7M01_Eukaron/main@{30day}.svg)](https://github.com/EDI-Systems/M7M01_Eukaron/compare/main@{30day}...main)
[![Discord](https://img.shields.io/badge/chat-Discord-purple)](https://discord.gg/VxCFSFC6bW)
![QQ Group](https://img.shields.io/badge/QQ_Group-1038895132-red)

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

&ensp;&ensp;For vendor-supplied packages and hardware abstraction libraries, please refer to the **[M0A00_Library](https://github.com/EDI-Systems/M0A00_Library)** repo to download and use them properly.

## Why a New Microkernel?
&ensp;&ensp;Microkernels have been invented for at least 30 years, and numerous great designs that emphasis performance, parallelism, fault-tolerance, security and even formal correctness have appeared. However, **none of them were able to chart a concord abstraction over all computing devices**, which harms software portability and ecosystem coherence. More precisely, (1) few of them support cloud native environments with a high level of parallelism, (2) even fewer of them support microcontrollers with scarce resources, and (3) none of them support the two extremes and the continuum between them with **a single kernel design**. Moreover, the configurablity of existing systems were also restricted, as **very few of them allow you to pick the exact very lines of code that you need**. If the system was not designed with configurablity and reusability in mind, adding them as an afterthought probably won't end very well.

&ensp;&ensp;The utmost goal of RME is to enable **hyperadaptability**. It conjures an overarching abstraction over all types of hardware, regardless of whether they are cloud servers, edge routers, terminal nodes, or even battery-powered devices. When creating such an abstraction, we aim to **discover and express true common ground between different hardware models rather than coercing one into another**. To put differently, the same abstraction and the resulting implementation should be practically usable on both microprocessors and microcontollers: when deployed on the former, it can keep up with the state-of-the-art cloud-native operating systems while providing more **parallelism**; when deployed on the latter, it can keep up with the state-of-the-art RTOSes while providing more **security and fault-tolerance**. This unravels previously unaware research opportunities, as concerns from different scenarios are intertwined with each other. For any particular scenario, we ask what a system (including the kernel, drivers, middlewares and applications) **built entirely from scratch for that specific case** would look like, and evaluate **whether RME could be as good as that** (this process is remotely akin to proving the optimality of a greedy algorithm). If not, then opportunities for system design optimizations exist.

&ensp;&ensp;The second goal of RME is to enable **hyperreusability**. All system components including drivers and middlewares are fully decoupled, and written in a way that could be configured and deployed as a standalone software package. This allows them **to be used with other operating systems, or even in cases where no operating system is present**. The components are coded in a special fashion so that **function pointers can be totally eliminated** when only one instance of the library is present, minimizing redundancy and maximizing resource efficiency. Ideally, if the components were configured correctly, the resulting image will not contain a single line of garbage code. Stripping away unused code also helps with system security, as the vulnerabilities from ununsed garbage code would not be carried into the resulting image. 

&ensp;&ensp;The last goal of RME is to enable **hyperdeployability**. Many microkernels exist for a sole purpose - education or research, and their lack of software engineering concerns makes them impossible to use in commercial or even hobbyist projects. RME is different in that it **deliberately embraces the engineering difficulties**, and is willing to expend resources to **support as many production environments as possible**. This **particularly applies to the microcontrollers**, where the compilation toolchain is esoteric and the debugging facilities are restricted in nature. To this end, the **[RVM](https://github.com/EDI-Systems/M7M02_Ammonite)** microcontroller hypervisor takes care of every dirty work, and ready-to-use **Keil** or **Makefile** projects are just one click away. More toolchains such as **IAR** are planned as well. 

## Kernel Design

### Principles

&ensp;&ensp;We believe that the following principles are self-evident:
- Ideals are cheaper than ideas, ideas are cheaper than code.
- In theory, there is no difference between practice and theory; in practice there is.
- All performance issues are caused by an obscure division of rights and duties.
- Good overall system design are always preferred over peephole optimizations.
- True useful adaptability is better than false demonstrational portability.
- You have a wrong abstraction if you coerce some underlying hardware into it.

We ask what driving forces are behind these needs, and how to cater these driving forces.
We ask what fits the situation, rather than what is at hand.

### Permission

&ensp;&ensp;Modern microkernels employ a fine-grained resource control mechanism originating from EROS (the concept itself is even much earlier), that is, the **capabilities**. All resources and **mechanisms** are exposed to the user-level as unforgeable tokens that represent them, thus that the user-level can orchestrate **policies** in a fine-grained and flexible fashion using these mechanisms. Many systems, i.e. seL4, Fiasco.OC, NOVA, Barrelfish, Composite are designed this way and this has been proven successful, and we leverage a similar design as well. Capabilities can be delegated to or revoked from processes to granting them to or depriving them of certain priviledges to operate on certain resources. 

&ensp;&ensp;However, capability-based designs are not without its problems. In general, three types of issues are present in a naive design: (1) **capability look up latency**, (2) **capability memory organization**, and (3) **capability operation efficiency**. To deal with (1), we use a finite two-level capability table construction scheme heavily borrowed from the [Composite](https://github.com/gwsystems/composite), thus that the capability lookup worst-case execution time can be bounded without the use of preemption points. In [seL4](https://github.com/seL4/seL4) terms, our CSpace depth is limited to 2. To handle (2), our capabilities are uniform-sized and always aligned to a 8-word boundary to minimize management complexity, particularly on a racey multi-core environment. To optimize for (3), certain operations allow to operate on multiple capabilities at once in a transactional manner, minimizing the amortized overhead on capability operations.

&ensp;&ensp;Note that our protection domain design is significantly different from seL4: we don't attach capability spaces to threads, but processes, which already hold the address space (which could be seen as capabilities to physical memory). RME processes are entities consisting of a capability table and a page table, which are shared by all threads that run inside of it. It has nothing to do with traditional Unix processes beyond that; the name is there just to indicate some remote familiarity.

### Scheduler

&ensp;&ensp;**Preemption decisions** and **execution budget** are what scheduling is all about. To cater user-level scheduling policies, both facets must be **exported to the user-level**. This however creates efficiency issues, as all time-sensitive preemption decisions must incur two additional context switches - to the scheduler thread, and then to the thread selected by the scheduler. This does not suit time-sensitive systems, especially microcontrollers which does not feature fast processors. When implemented in this way, the preemption latency easily piles up to more than 1000 cycles, which is at least tens of microseconds. To cope with this issue, most microkernel designers left the entire scheduler in the kernel, with Composite as the only truly clean exception. In Composite, the **scheduling context** [(Temporal Capabilities, TCaps)](https://ieeexplore.ieee.org/document/8277280) are completely independent of the **execution context** (thread objects themselves). While preemption decisions are made in the kernel to minimize context switch latency, time budget transfers happen at the user-level. The in-kernel preemption decision logic compares the quality of the time consumed by the two threads, and the thread with higher quality wins. Quality is in turn determined by the priorities attached by different subsystems when the time is delegated down through them: only when all subsystems reach an agreement about the priority comparison result can we say that one quality is better than the other. This guarantees that the preemptions can only occur when all subsystem schedulers agree, thus bounding the interferences between them.

&ensp;&ensp;However, this design is not without pitfalls. Firstly, the TCaps have a mutable size which is decided by the number of subsystems, as each subsystems must correspond to a slot in the TCap kernel object to remember the priority. This creates problems when we don't know how many subsystems are there at compile time, and when we dynamically boot subsystems, things get worse as we may run out of the slots. Secondly, preemption decisions are made through an array comparison which is fast on superscalar processors (the compares do not have mutual dependency and thus can be issued at maximum pipeline width), but terribly slow on single-issue microcontrollers. This is ironic as the scheduler is slow where the problem it tries to solve truly matters. Lastly, TCaps must be specified in addition to the thread capabilities when scheduling threads, adding extra cost to capability validity checking. Again, this is okay on superscalar processors because the two checks does not have dependency between them, but creates problems on microcontollers. The result is an overall great kernel on x86 computers but awfully slow on microcontrollers. The complex handling logic also caused code size bloat, which is again unimportant on x86 but critical on microcontrollers. In addition, this design is not expandable when multiple accelerators are present; only when all subsystems' scheduler on all accelerators agree can we make the preemption decision, then we would have to do tensor comparisons in the kernel, and to do that ASAP we possibly need to leverage AVX extensions!

&ensp;&ensp;In these lights, we design a scheduling mechanism that (1) associates the budgets with the threads, (2) features a simple FPRR preemption logic in the kernel, but (3) keeps the budget management outside of the kernel. This mechanism looks like a weird FPRR scheduler that only expends the budgets but does not replenish or revoke them, and the user-level is responsible for the latter two. On microcontrollers where absolute latency is needed and time-sensitivity agrees with priority, the in-kernel FPRR scheduler can be leveraged directly (see [RVM](https://github.com/EDI-Systems/M7M02_Ammonite)) to provide minimum latency and basic budget control. On x86-based systems where complex MCS is constructed, one can arrange the priorities of the threads thus that the in-kernel FPRR mechanism is circumvented, and **arbitrarily complex user-level scheduling policies can be used**, including a mechanism similar to TCaps. In other words, we implement the entire MCS policies (i.e. bandwidth servers) at the user-level, tolerating the extra context switch to and from scheduler on x86-based systems. We argue that this only have a mild performance loss, as the superscalar pipeline of x86 will compensate all that overhead. Also, when we consider MCS, bounded interference is more important than absolute minimal latency.

### Memory

&ensp;&ensp;Memory consists of **user-level memory** and **kernel-level memory**, and all physical memory must be divided between the two. Managing user-level memory at user-level is nothing hard and were implemented in L4, etc. long ago, in flexpages. Flexpages expose too little of memory composition and still implies policy with respect to physical page usage, and we apply an [exokernel](https://pdos.csail.mit.edu/6.828/2008/readings/engler95exokernel.pdf) philosophy instead by exposing the actual construction of page tables. Managing kernel-level memory at the user-level is the real challenge: naive mechanism designs can easily lead to compromizes in confidentiality, integrity, availability, or accountability. To this end, seL4 and Composite makes use of memory retyping to avoid exposing in-use kernel memory as user-pages, and each page in use belongs either to kernel-level or the user-level. All memory start as untyped pages, and must be retyped into kernel type or user type before they could be mapped in. In addition, all kernel objects (Composite) or their pools (seL4) are a single page regardless of their real size, minimizing management complexity. seL4 does allow allocating different objects in the same pool; alignment restrictions apply however.

&ensp;&ensp;The above model is great if applied to normal x86-based systems but not that great on microcontrollers. It does not really fit some deeply embedded processors, either. Two contradicting issues are present on different types of systems: (1) allocation mechanism too flexible and (2) allocation mechanism too inflexible. For the "too flexible" part, most microcontrollers just need a fixed kernel- and user-memory dichotomy; all kernel objects are created at boot-time, and no further creation or deletions happen thereafter, and all that retyping logic is redundant. The "too inflexible" part comes in two cases: (1) on microcontrollers, forcing object alignment have no benefits, and only wastes memory; it's even worse when all objects must be a "page", which microcontrollers don't really have (what they do have are physical memory segments); (2) on microprocessors, manipulating pages one by one might be too slow as allocations may involve thousands of pages being mapped or unmapped together.

&ensp;&ensp;Considering all of the above, our design simply allows to **set a compile-time kernel address boundary to all kernel objects**, and all kernel objects shall be created below that boundary. The rest of the memory can be used as user memory. A kernel object registration table (bitmap) remembers which location has kernel objects with a very fine granularity, i.e. 32-byte. On microcontrollers, this boundary is a fixed value decided by the RVM project generator; on deeply embedded processors, this boundary is a fixed value determined by the system designer. In these two cases, reallocation between kernel and user memory is not needed, and a simple dichotomy is sufficient. Although this naive dichotomy alone seems very inflexible, **it could become very flexible with the addition of an user-level trusted page server**: on general-purpose processors, we set this boundary to the maximum address, which circumvents the in-kernel checking mechanisms, and all memory can be used as kernel memory as well as user memory. The trusted server is now responsible for making sure no kernel memory overlaps with the user memory: it could control what user pages and sub-kernel-memory capabilities are delegated to its subordinates so that no security hazards are present. The trusted server can be regarded as a mandatory kernel module that runs at the user-level; in other words, we **implement retyping at user-level**. This trusted server is at the user-level and cannot hurt the kernel WCET, so arbitrarily complex retyping policies can be used. This is akin to the scheduler design in the sense that we allow the user to circumvent the kernel and supply something instead. 

&ensp;&ensp;And it's not just that. The RME allows you to take this **"incomplete kernel + trusted server"** philosophy even further, and can be configured at compile-time to **go without in-kernel address space management mechanism altogether**. If this option is selected, the kernel accepts whatever physical address given at process creation time, and will just pass that into the HAL (i.e. CR3 of x86) without any checking at all! The user-level trusted process is now responsible for all that address space management stuff, including ASID, cache policies, etc. The benefits are very pronounced on the two extremes of the computing spectrum: (1) On microcontrollers that feature less than 64k RAM, this allows to load a fixed, in-flash Memory Protection Unit (MPU) table into the MPU directly, without all that page table logic and in-memory data structures. As long as you don't want to modify user-level memory mappings at run-time, this is a great choice and can save a lot of RAM. (2) On cloud-native servers that house terabytes of memory, many operations like demand paging involve thousands of pages at once, and allowing to manipulate the pages directly could boost performance to that of monolithic kernels. What's more, the trusted process can provide address space metaoperations in a way that matches the application nature (i.e. POSIX or Linux-like system calls), dramatically reducing amortized paging cost when compared to microkernel built-in operations. Note that this **goes even further than exokernels**: the kernel does not even protect itself and requires a carefully-designed trusted server to do so.

&ensp;&ensp;For kernel stacks, we keep one for each CPU, and nothing more. We don't need a preemptible kernel as we have no unbounded loops or recursions.

### Parallelism

&ensp;&ensp;Parallism is a hallmark of modern high-performance computing, and is increasingly prevalent in real-time systems as well. To support as much parallelism as well as determinism, we design the whole kernel with lockless data structures, and each system call is **a transaction with bounded execution time**. Races are resolved not by locks but atomic operations; we also adopt **quiescence-based Real-Time Scalable Memory Reclamation** ([RT-SMR](https://grace-liu.github.io/static/papers/18-RTAS-smr.pdf)) to reclaim abandoned kernel objects. This part is heavily borrowed from Composite, but with notable differences; RME allows to use a periodic timer as well as a timestamp counter, and requires to explicitly bind threads to cores before you can operate them on that core. Unfortunately, we don't have a runnable x86-64 port now.

&ensp;&ensp;Parallelism is out of the scope for microcontrollers (at least in the current RVM version) and might be added when microcontrollers featuring cache-coherent cores with atomic instructions are popular.

### Communication

&ensp;&ensp;Inter-Process Communication (IPC) design is always the key performance deciding factor in any operating system. For microkernels, even more importance is attached because such systems split functionality into multiple inter-communicating services. IPC can be further subdivided into two categories: **synchronous** and **asynchronous**. Synchronous IPC requires the client to block until the server responds like a procedure call, while asynchronous IPC does not have such a requirement and looks like a message queue send. In practise, synchronous IPC is often used in scenarios where the client and server are tightly coupled, and asynchronous IPC is often used in cases where the former two are loosely coupled.

&ensp;&ensp;For synchronous IPC, we implement a migrating thread model similar to that of Mach and Composite, and allow a thread to "enter" and "leave" a process just like calling a function in that process. Just like system calls which "move" threads into and back from the kernel space, when one client calls an invocation port, it will be temporarily "moved" to the server process and execute the server code; after successful execution, the thread returns to the original process. The server process could contain no threads when not being called. The highlight of this design is that **the client execute the server code with its own time budget**; temporal isolation is hereby enforced, as malicious threads only waste their own time when they request services excessively. It also enables more efficient context switching because it doesn't invoke the scheduler, as we are using the same thread after all. The key to understanding this model is to **think of threads as temporal protection domains holding budgets**; the same budget could of course be spent on different spatial protection domains (processes), and the code pieces sharing the same temporal protection domains are inherently closely coupled, just like code that share the same spatial protection domain. In system jargons, we allow one **scheduling context** to migrate between different **protection domains** with little hassle. Some may argue that the downside of this model is mandating a multi-threaded server implemented with lock-free (or at least, obstruction-free) data structures. However, we believe that **servers are inherently multi-threaded and lock-free** or we risk losing all of parallelism, determinism and temporal isolation. For those simple cases where a single-threaded servers are sufficient, we also provide asynchronous signals.

&ensp;&ensp;The invocation implementation however differs from Mach and Composite. Instead of using a single port for different threads, we use different ports for them so that the context before invocation can be stored in the invocation object rather than the thread object. This eliminates a compile-time configuration macro, and makes it easier to associate the invocation port with a stack that can be prepared beforehand.

&ensp;&ensp;For asynchronous IPC, we implement a model that behaves like a counting semaphore within one processor. When a thread requests a semaphore but none is present, it blocks until other thread on the same processor sends one to the endpoint. However, inter-processor sends only increase the semaphore count but don't unblock the waiting thread, and only one thread may block on a particular endpoint. In other words, we **implement inter-processor interrupts at the user-level**, and this makes our kernel look somewhat like separation kernels such as Barrelfish. This makes sense when you realize that inter-processor interrupts are hardware resources that needs to be governed by user-level policies, but is very different from seL4 and Composite which does not provide counting but provide inter-processor activation.

### Virtualization

&ensp;&ensp;Like many other microkernels, the ability to paravirtualize other operating systems without any hardware extensions is included at design time rather than an afterthought. Two major approaches exist: **thread-based** and **vCPU-based**. The former approach is taken by many research microkernels as it corresponds virtual machine threads to microkernel threads, thus that a combination of microkernel system calls can be used in place of guest HAL layers. The latter approach is more common in commercial products, and corresponds a single thread to all execution on a CPU core.

&ensp;&ensp;Both approaches have downsides. The thread-based approach used in L4Linux (Fiasco.OC) suffers notably from **operation amplification**: one guest operation is almost always converted to one or more microkernel operations, leading to long latencies. This is probably okay if you're virtualizing Linux, but causes problems when the system being virtualized is simple, i.e. Unikernels or RTOSes: these simple systems forgo permission checking altogether to meet stringent efficiency or latency requirements. Naively replacing their HAL with microkernel system calls will add (useless) permission checking back to these systems, bringing their performance down to an unacceptable level. This is particularly problematic if the original system's operation semantics are at odds with the microkernel natives (scheduler, etc); in this case, a long list of microkernel system calls must be supplied to become a drop-in replacement of the original, leading to performance degradations that are of orders of magnitude. Also, thread kernel objects have to be created for each guest thread, exhausting the microcontroller memory quickly.

&ensp;&ensp;The vCPU-based approach does not suffer from operation amplification because all guest operations are performed as-is without needing to translate them to microkernel system calls, but suffer from **vIRET atomicity** issues. To emulate the interrupt mechanism, the hypervisor forces the vCPU thread into a vIRQ sequence that mimicks the original hardware interrupt handler. The vIRQ then handles the interrupt then tries to return to normal execution by vIRET-ing. The problem here is with the vIRET; the hypervisor must know if the vCPU has quit the interrupt handler before it can force the vCPU into the sequence again. A simple "interrupt enable" flag in RAM won't work here, as the new vIRQ could be injected between setting the flag and returning to the original vCPU context. The naive solution is to provide a standalone vIRET hypercall and let the hypervisor handle the atomic interrupt enabling/returning, however this **bloats vIRQ latency by quite a bit** particularly on microcontrollers. The complex solution is used by Xen, where the hypervisor always forced vIRQ execution upon detecting the flag regardless of whether the vIRQ is performing the return; this does work, however the vIRQ entry must be able to fix the stack left by the incomplete return as if the entry happened after the return's full completion. This entry sequence is **difficult to write correctly** let alone it has to be written in architecture-specific assembly.

&ensp;&ensp;The final implementation could be seen as a modified vCPU-based approach that stays somewhere in the middle. It allows **very efficient type II paravirtualization**. Each vCPU corresponds to two threads, where one thread runs the normal code and the other thread runs the interrupt vectors. We call the former one user thread, and the latter one vector thread. The vector thread stays at a higher priority than the user thread, and blocks on an endpoint to receive interrupt activations. When an interrupt activates, the vector thread immediately preempts the user thread, imitating the interrupt mechanism on physical CPUs. Observing that many guest kernels modify interrupt context to implement system calls and context switches, we allow the user threads to be created as hypervisor dedicated, whose context will not be stored in kernel object but some user specified area. The area is always mapped to kernel space as well as the guest, so that both of them can access it without exceptions. Thus, the vector thread may modify user thread context without microkernel calls, dramatically reducing guest interrupt latency. When the vector thread finishes its processing, it blockes again on the signal endpoint waiting for further interrupts. The key here is that the RME will check the validity of the user thread register set befure using it, thus that illegal modifications do not defeat protection boundaries.

&ensp;&ensp;Some may argue that designing the entire microkernel as a type I hypervisor rather than using a standalone user-level type II hypervisor could be more efficient. However this (1) forces a virtualization mindset for all applications which bloats cases where only native applications are needed, (2) requires in-kernel hypervisor policies and hypercall implementations which violates the microkernel design principle, (3) bloats priviledged code which is the trusted computing base.

&ensp;&ensp;As for full virtualization, we leave that to hardware virtualization extensions. Though it is possible to fully virtualize any architecture by using obscure techniques such as shadow page tables and dynamic binary translation, we feel that current virtualization extensions have killed these feats. On these platforms however, the RME boots up as a type I hypervisor. In the future, full virtualization will only be brought to the x86-64 architecture where the hardware/software interfaces have a defacto standard, and we refrain from adding those to other architectures where a standard OS images are lackluster.

## User-level Design

### Microcontroller Components
&ensp;&ensp;All planned components are listed below. If the github link is valid, the component is currently available.  
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

&ensp;&ensp;The use of RVM as a user-level library is required on microcontrollers, which supports automatic generation of projects against multiple architectures, toolchains, and IDEs. It also enables virtualization on even microcontrollers, which allows seamless integration with existing bare-metal code or RTOSes. Only single-core microcontrollers are supported; multi-core support for microcontrollers is currently out of the scope.

|Chipname     |Platform    |Clock |Flash |RAM   |Build |Yield/S|Yield/2|Inv/2|Sig/1|Sig/2|Sig/S|Sig/I|
|:-----------:|:----------:|:----:|:----:|:----:|:----:|:-----:|:-----:|:---:|:---:|:---:|:---:|:---:|
|STM32L071CB  |Cortex-M0+  |32M   |128k  |20k   |Keil  |492    |763    |956  |718  |810  |749  |522  |
|...          |...         |...   |...   |...   |GCC   |513    |799    |939  |736  |830  |776  |534  |
|STM32F405RG  |Cortex-M4F  |168M  |1M    |192k  |Keil  |324    |524    |692  |576  |731  |568  |420  |
|...          |...         |...   |...   |...   |GCC   |332    |520    |684  |608  |735  |540  |416  |
|STM32F767IG  |Cortex-M7F  |216M  |1M    |512k  |Keil  |264    |400    |600  |438  |484  |477  |282  |
|...          |...         |...   |...   |...   |GCC   |294    |456    |644  |406  |460  |429  |321  |
|CH32V307VC   |RV32IMAFC   |144M  |128k  |192k  |GCC   |358    |669    |706  |703  |723  |624  |523  |

&ensp;&ensp;It can be observed that the protected mode kernel performance is generally on par with the baremetal RTOSes, with some ~3 microsecond interrupt latencies measured. Moreover, when the kernel is used to virtualize RTOSes, as low as 6% of virtualization overhead can be achieved; see the **[RVM](https://github.com/EDI-Systems/M7M02_Ammonite)** repo for details.

&ensp;&ensp;**FAQ:** Can XXX (some microcontroller) be supported? **Answer**: If your microcontroller has **at least 64k ROM and 20k RAM**, and features a memory protection unit, just like the majority of Cortex-M microcontrollers, then yes! However, it's best to have **128k ROM and 64k RAM** to start with, as such systems will be useful in real projects. Some **sub-$1 chips such as STM32G0B1CBT6** are more than sufficient for RME, and similar chips are being deployed in billions. 

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
