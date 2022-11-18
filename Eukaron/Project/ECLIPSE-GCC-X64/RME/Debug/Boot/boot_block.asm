
boot_block.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000007000 <Start_16>:
    7000:	fa                   	cli    
    7001:	31 c0                	xor    %eax,%eax
    7003:	8e d8                	mov    %eax,%ds
    7005:	8e c0                	mov    %eax,%es
    7007:	8e d0                	mov    %eax,%ss
    7009:	0f 01 16             	lgdt   (%rsi)
    700c:	5c                   	pop    %rsp
    700d:	70 0f                	jo     701e <Start_16+0x1e>
    700f:	20 c0                	and    %al,%al
    7011:	66 83 c8 01          	or     $0x1,%ax
    7015:	0f 22 c0             	mov    %rax,%cr0
    7018:	66 ea                	data16 (bad) 
    701a:	20 70 00             	and    %dh,0x0(%rax)
    701d:	00 08                	add    %cl,(%rax)
	...

0000000000007020 <Boot_32>:
    7020:	66 b8 10 00          	mov    $0x10,%ax
    7024:	8e d8                	mov    %eax,%ds
    7026:	8e c0                	mov    %eax,%es
    7028:	8e d0                	mov    %eax,%ss
    702a:	66 31 c0             	xor    %ax,%ax
    702d:	8e e0                	mov    %eax,%fs
    702f:	8e e8                	mov    %eax,%gs
    7031:	bb 01 00 00 00       	mov    $0x1,%ebx
    7036:	8b 25 fc 6f 00 00    	mov    0x6ffc(%rip),%esp        # e038 <_end+0x6fd0>
    703c:	ff 15 f8 6f 00 00    	callq  *0x6ff8(%rip)        # e03a <_end+0x6fd2>
    7042:	eb fe                	jmp    7042 <Boot_32+0x22>

0000000000007044 <Boot_GDT_16>:
	...
    704c:	ff                   	(bad)  
    704d:	ff 00                	incl   (%rax)
    704f:	00 00                	add    %al,(%rax)
    7051:	9a                   	(bad)  
    7052:	cf                   	iret   
    7053:	00 ff                	add    %bh,%bh
    7055:	ff 00                	incl   (%rax)
    7057:	00 00                	add    %al,(%rax)
    7059:	92                   	xchg   %eax,%edx
    705a:	cf                   	iret   
	...

000000000000705c <Boot_GDT_Desc_16>:
    705c:	17                   	(bad)  
    705d:	00 44 70 00          	add    %al,0x0(%rax,%rsi,2)
	...
