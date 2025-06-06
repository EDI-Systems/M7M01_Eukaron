
./Object/UVM:     file format elf64-x86-64


Disassembly of section .text:

0000000020000000 <_UVM_Entry>:
              space very well.
Input       : None.
Output      : None.
*****************************************************************************/
_UVM_Entry:
    JMP                 main
    20000000:	e9 8b 05 00 00       	jmp    20000590 <main>

0000000020000005 <__UVM_X64_In>:
Output         : None.
Return         : ptr_t - The data received from that port.
Register Usage : None.
*****************************************************************************/
__UVM_X64_In:
    PUSHQ               %RDX
    20000005:	52                   	push   %rdx
    MOVQ                %RDI,%RDX
    20000006:	48 89 fa             	mov    %rdi,%rdx
    MOVQ                %RAX,%RAX
    20000009:	48 89 c0             	mov    %rax,%rax
    INB                 (%DX),%AL
    2000000c:	ec                   	in     (%dx),%al
    POPQ                %RDX
    2000000d:	5a                   	pop    %rdx
    RETQ
    2000000e:	c3                   	ret    

000000002000000f <__UVM_X64_Out>:
Output         : None.
Return         : None.
Register Usage : None.
*****************************************************************************/
__UVM_X64_Out:
    PUSHQ               %RDX
    2000000f:	52                   	push   %rdx
    PUSHQ               %RAX
    20000010:	50                   	push   %rax
    MOVQ                %RDI,%RDX
    20000011:	48 89 fa             	mov    %rdi,%rdx
    MOVQ                %RSI,%RAX
    20000014:	48 89 f0             	mov    %rsi,%rax
    OUTB                %AL,(%DX)
    20000017:	ee                   	out    %al,(%dx)
    POPQ                %RAX
    20000018:	58                   	pop    %rax
    POPQ                %RDX
    20000019:	5a                   	pop    %rdx
    RETQ
    2000001a:	c3                   	ret    

000000002000001b <__UVM_X64_Read_TSC>:
Output         : None.
Return         : ptr_t - The timestamp value returned.
Register Usage : None.
******************************************************************************/
__UVM_X64_Read_TSC:
    PUSHQ               %RDX
    2000001b:	52                   	push   %rdx
    RDTSC
    2000001c:	0f 31                	rdtsc  
    SHL                 $32,%RDX
    2000001e:	48 c1 e2 20          	shl    $0x20,%rdx
    ORQ                 %RDX,%RAX
    20000022:	48 09 d0             	or     %rdx,%rax
    POPQ                %RDX
    20000025:	5a                   	pop    %rdx
    RETQ
    20000026:	c3                   	ret    

0000000020000027 <_UVM_Set_TLS_Pos>:
Output      : None.
Return      : None.
*****************************************************************************/
_UVM_Set_TLS_Pos:
    /* The alignment mask is not used in x86-64 */
    WRFSBASE            %RSI
    20000027:	f3 48 0f ae d6       	wrfsbase %rsi
    RETQ
    2000002c:	c3                   	ret    

000000002000002d <_UVM_Get_TLS_Pos>:
Output      : None.
Return      : ptr_t* - The thread local storage position.
*****************************************************************************/
_UVM_Get_TLS_Pos:
    /* The alignment mask is not used in x86-64 */
    RDFSBASE            %RAX
    2000002d:	f3 48 0f ae c0       	rdfsbase %rax
    RETQ
    20000032:	c3                   	ret    

0000000020000033 <_UVM_Thd_Stub>:
Input       : RDI - The entry address.
              RSI - The stack address that we are using now.
Output      : None.
*****************************************************************************/
_UVM_Thd_Stub:
    MOVQ                (%RSP),%RDI
    20000033:	48 8b 3c 24          	mov    (%rsp),%rdi
    MOVQ                8(%RSP),%RSI
    20000037:	48 8b 74 24 08       	mov    0x8(%rsp),%rsi
    MOVQ                16(%RSP),%RDX
    2000003c:	48 8b 54 24 10       	mov    0x10(%rsp),%rdx
    MOVQ                24(%RSP),%RCX
    20000041:	48 8b 4c 24 18       	mov    0x18(%rsp),%rcx
    JMP                 *32(%RSP)           /* Jump to the actual entry address */
    20000046:	ff 64 24 20          	jmp    *0x20(%rsp)

000000002000004a <_UVM_Inv_Stub>:
Input       : R4 - The entry address.
              R5 - The stack address that we are using now.
Output      : None.
*****************************************************************************/
_UVM_Inv_Stub:
    MOVQ                %RSI,%RDI           /* Pass the parameter */
    2000004a:	48 89 f7             	mov    %rsi,%rdi
    CALLQ               *32(%RSP)           /* Branch to the actual entry address */
    2000004d:	ff 54 24 20          	call   *0x20(%rsp)

    XORQ                %RDI,%RDI           /* UVM_SVC_INV_RET */
    20000051:	48 31 ff             	xor    %rdi,%rdi
    MOVQ                %RAX,%RSI           /* return value in RSI */
    20000054:	48 89 c6             	mov    %rax,%rsi
    SYSCALL                                 /* System call */
    20000057:	0f 05                	syscall 

0000000020000059 <UVM_Inv_Act>:
              RSI - ptr_t Param - The parameter for the call.
Output      : RDX - ptr_t* Retval - The return value from the call.
Return      : RAX - ptr_t - The return value of the system call itself.
*****************************************************************************/
UVM_Inv_Act:
    PUSHQ               %RBX                /* The user-level should push all context */
    20000059:	53                   	push   %rbx
    PUSHQ               %RCX
    2000005a:	51                   	push   %rcx
    PUSHQ               %RDX
    2000005b:	52                   	push   %rdx
    PUSHQ               %RBP
    2000005c:	55                   	push   %rbp
    PUSHQ               %R8
    2000005d:	41 50                	push   %r8
    PUSHQ               %R9
    2000005f:	41 51                	push   %r9
    PUSHQ               %R10
    20000061:	41 52                	push   %r10
    PUSHQ               %R11
    20000063:	41 53                	push   %r11
    PUSHQ               %R12
    20000065:	41 54                	push   %r12
    PUSHQ               %R13
    20000067:	41 55                	push   %r13
    PUSHQ               %R14
    20000069:	41 56                	push   %r14
    PUSHQ               %R15
    2000006b:	41 57                	push   %r15
    PUSHFQ
    2000006d:	9c                   	pushf  

    MOVQ                %RSI,%RDX           /* Param */
    2000006e:	48 89 f2             	mov    %rsi,%rdx
    MOVQ                %RDI,%RSI           /* Cap_Inv */
    20000071:	48 89 fe             	mov    %rdi,%rsi
    MOVQ                $0x100000000,%RDI   /* UVM_SVC_INV_ACT */
    20000074:	48 bf 00 00 00 00 01 	movabs $0x100000000,%rdi
    2000007b:	00 00 00 
    SYSCALL
    2000007e:	0f 05                	syscall 

    POPFQ
    20000080:	9d                   	popf   
    POPQ                %R15
    20000081:	41 5f                	pop    %r15
    POPQ                %R14
    20000083:	41 5e                	pop    %r14
    POPQ                %R13
    20000085:	41 5d                	pop    %r13
    POPQ                %R12
    20000087:	41 5c                	pop    %r12
    POPQ                %R11
    20000089:	41 5b                	pop    %r11
    POPQ                %R10
    2000008b:	41 5a                	pop    %r10
    POPQ                %R9
    2000008d:	41 59                	pop    %r9
    POPQ                %R8
    2000008f:	41 58                	pop    %r8
    POPQ                %RBP
    20000091:	5d                   	pop    %rbp
    POPQ                %RDX
    20000092:	5a                   	pop    %rdx
    POPQ                %RCX
    20000093:	59                   	pop    %rcx
    POPQ                %RBX                /* POP all saved registers now */
    20000094:	5b                   	pop    %rbx

    CMPQ                $0,%RDX             /* See if this return value is desired */
    20000095:	48 83 fa 00          	cmp    $0x0,%rdx
    JZ                  No_Retval
    20000099:	74 03                	je     2000009e <No_Retval>
    MOVQ                %RSI,(%RDX)
    2000009b:	48 89 32             	mov    %rsi,(%rdx)

000000002000009e <No_Retval>:
No_Retval:
    RETQ
    2000009e:	c3                   	ret    

000000002000009f <UVM_Inv_Act_Dummy>:

UVM_Inv_Act_Dummy:
    .global UVM_Inv_Act_Dummy
    PUSHQ               %RBX                /* The user-level should push all context */
    2000009f:	53                   	push   %rbx
    PUSHQ               %RCX
    200000a0:	51                   	push   %rcx
    PUSHQ               %RDX
    200000a1:	52                   	push   %rdx
    PUSHQ               %RBP
    200000a2:	55                   	push   %rbp
    PUSHQ               %R8
    200000a3:	41 50                	push   %r8
    PUSHQ               %R9
    200000a5:	41 51                	push   %r9
    PUSHQ               %R10
    200000a7:	41 52                	push   %r10
    PUSHQ               %R11
    200000a9:	41 53                	push   %r11
    PUSHQ               %R12
    200000ab:	41 54                	push   %r12
    PUSHQ               %R13
    200000ad:	41 55                	push   %r13
    PUSHQ               %R14
    200000af:	41 56                	push   %r14
    PUSHQ               %R15
    200000b1:	41 57                	push   %r15
    PUSHFQ
    200000b3:	9c                   	pushf  

    MOVQ                %RSI,%RDX           /* Param */
    200000b4:	48 89 f2             	mov    %rsi,%rdx
    MOVQ                %RDI,%RSI           /* Cap_Inv */
    200000b7:	48 89 fe             	mov    %rdi,%rsi
    MOVQ                $0x100000000,%RDI   /* UVM_SVC_INV_ACT */
    200000ba:	48 bf 00 00 00 00 01 	movabs $0x100000000,%rdi
    200000c1:	00 00 00 

    POPFQ
    200000c4:	9d                   	popf   
    POPQ                %R15
    200000c5:	41 5f                	pop    %r15
    POPQ                %R14
    200000c7:	41 5e                	pop    %r14
    POPQ                %R13
    200000c9:	41 5d                	pop    %r13
    POPQ                %R12
    200000cb:	41 5c                	pop    %r12
    POPQ                %R11
    200000cd:	41 5b                	pop    %r11
    POPQ                %R10
    200000cf:	41 5a                	pop    %r10
    POPQ                %R9
    200000d1:	41 59                	pop    %r9
    POPQ                %R8
    200000d3:	41 58                	pop    %r8
    POPQ                %RBP
    200000d5:	5d                   	pop    %rbp
    POPQ                %RDX
    200000d6:	5a                   	pop    %rdx
    POPQ                %RCX
    200000d7:	59                   	pop    %rcx
    POPQ                %RBX                /* POP all saved registers now */
    200000d8:	5b                   	pop    %rbx

    CMPQ                $0,%RDX             /* See if this return value is desired */
    200000d9:	48 83 fa 00          	cmp    $0x0,%rdx
    JZ                  No_Retval_Dummy
    200000dd:	74 03                	je     200000e2 <No_Retval_Dummy>
    MOVQ                %RSI,(%RDX)
    200000df:	48 89 32             	mov    %rsi,(%rdx)

00000000200000e2 <No_Retval_Dummy>:
No_Retval_Dummy:
    RETQ
    200000e2:	c3                   	ret    

00000000200000e3 <UVM_cret>:

UVM_cret:
    .global     UVM_cret
    RETQ
    200000e3:	c3                   	ret    

00000000200000e4 <UVM_Inv_Ret>:
Input       : RDI - The returning result from the invocation.
Output      : None.
Return      : None.
*****************************************************************************/
UVM_Inv_Ret:
    MOVQ                %RDI,%RSI           /* Set return value to the register */
    200000e4:	48 89 fe             	mov    %rdi,%rsi
    XORQ                %RDI,%RDI           /* UVM_SVC_INV_RET */
    200000e7:	48 31 ff             	xor    %rdi,%rdi
    SYSCALL                                 /* System call */
    200000ea:	0f 05                	syscall 
    RETQ
    200000ec:	c3                   	ret    

00000000200000ed <UVM_Svc>:
              RCX - Argument 3. We need to move this to R8 because SYSCALL will use RCX.
Output      : None.
Retun       : RAX - The return value.
*****************************************************************************/
UVM_Svc:
    MOV                 %RCX,%R8
    200000ed:	49 89 c8             	mov    %rcx,%r8
    PUSH                %R11
    200000f0:	41 53                	push   %r11
    SYSCALL                                 /* Do the system call directly */
    200000f2:	0f 05                	syscall 
    POP                 %R11
    200000f4:	41 5b                	pop    %r11
    RETQ
    200000f6:	c3                   	ret    

00000000200000f7 <_UVM_MSB_Get>:
;Output         : None.
;Return         : ptr_t - The MSB position.   
;Register Usage : None. 
;*****************************************************************************/
_UVM_MSB_Get:
    LZCNTQ              %RDI,%RDI
    200000f7:	f3 48 0f bd ff       	lzcnt  %rdi,%rdi
    MOVQ                $63,%RAX
    200000fc:	48 c7 c0 3f 00 00 00 	mov    $0x3f,%rax
    SUBQ                %RDI,%RAX
    20000103:	48 29 f8             	sub    %rdi,%rax
    RETQ
    20000106:	c3                   	ret    
    20000107:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    2000010e:	00 00 

0000000020000110 <TEST_THD_FUNC2>:
    UVM_Thd_Swt(UVM_CAPID(UVM_BOOT_TBL_THD,0),0);
}

/*This function is for tread switching test*/
void TEST_THD_FUNC2(void)
{
    20000110:	f3 0f 1e fa          	endbr64 
    20000114:	50                   	push   %rax
    20000115:	58                   	pop    %rax
    20000116:	48 83 ec 08          	sub    $0x8,%rsp
    2000011a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    while(1)
    {
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD1),0);
    20000120:	31 f6                	xor    %esi,%esi
    20000122:	bf 00 80 09 00       	mov    $0x98000,%edi
    20000127:	e8 d4 21 00 00       	call   20002300 <UVM_Thd_Swt>
    while(1)
    2000012c:	eb f2                	jmp    20000120 <TEST_THD_FUNC2+0x10>
    2000012e:	66 90                	xchg   %ax,%ax

0000000020000130 <TEST_THD_FUNC4>:
    UVM_Thd_Swt(UVM_CAPID(UVM_BOOT_TBL_THD,0),0);
}

/*This function is for cross-process tread switching test*/
void TEST_THD_FUNC4(void)
{
    20000130:	f3 0f 1e fa          	endbr64 
    20000134:	50                   	push   %rax
    20000135:	58                   	pop    %rax
    20000136:	48 83 ec 08          	sub    $0x8,%rsp
    2000013a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    while(1)
    {
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD3),0);
    20000140:	31 f6                	xor    %esi,%esi
    20000142:	bf 02 80 09 00       	mov    $0x98002,%edi
    20000147:	e8 b4 21 00 00       	call   20002300 <UVM_Thd_Swt>
    while(1)
    2000014c:	eb f2                	jmp    20000140 <TEST_THD_FUNC4+0x10>
    2000014e:	66 90                	xchg   %ax,%ax

0000000020000150 <TEST_THD_FUNC1>:
{
    20000150:	f3 0f 1e fa          	endbr64 
    sum=0;
    20000154:	48 c7 05 b1 2e 00 00 	movq   $0x0,0x2eb1(%rip)        # 20003010 <sum>
    2000015b:	00 00 00 00 
{
    2000015f:	53                   	push   %rbx
    sum=0;
    20000160:	bb 10 27 00 00       	mov    $0x2710,%ebx
    20000165:	0f 1f 00             	nopl   (%rax)
        start=__UVM_X64_Read_TSC();
    20000168:	e8 ae fe ff ff       	call   2000001b <__UVM_X64_Read_TSC>
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD2),0);
    2000016d:	31 f6                	xor    %esi,%esi
    2000016f:	bf 01 80 09 00       	mov    $0x98001,%edi
        start=__UVM_X64_Read_TSC();
    20000174:	48 89 05 ad 2e 00 00 	mov    %rax,0x2ead(%rip)        # 20003028 <start>
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD2),0);
    2000017b:	e8 80 21 00 00       	call   20002300 <UVM_Thd_Swt>
        end=__UVM_X64_Read_TSC();
    20000180:	e8 96 fe ff ff       	call   2000001b <__UVM_X64_Read_TSC>
    20000185:	48 89 05 8c 2e 00 00 	mov    %rax,0x2e8c(%rip)        # 20003018 <end>
        sum+=end-start;
    2000018c:	48 8b 05 85 2e 00 00 	mov    0x2e85(%rip),%rax        # 20003018 <end>
    20000193:	48 8b 15 8e 2e 00 00 	mov    0x2e8e(%rip),%rdx        # 20003028 <start>
    2000019a:	48 29 d0             	sub    %rdx,%rax
    2000019d:	48 01 05 6c 2e 00 00 	add    %rax,0x2e6c(%rip)        # 20003010 <sum>
    for(Cnt=0;Cnt<10000;Cnt++)
    200001a4:	48 83 eb 01          	sub    $0x1,%rbx
    200001a8:	75 be                	jne    20000168 <TEST_THD_FUNC1+0x18>
    UVM_LOG_S("\r\nThread Switching takes clock cycles:");
    200001aa:	bf b8 24 00 20       	mov    $0x200024b8,%edi
    200001af:	e8 4c 1c 00 00       	call   20001e00 <UVM_Print_String>
    UVM_LOG_I(sum/10000);
    200001b4:	48 ba 4b 59 86 38 d6 	movabs $0x346dc5d63886594b,%rdx
    200001bb:	c5 6d 34 
    200001be:	48 89 d0             	mov    %rdx,%rax
    200001c1:	48 f7 25 48 2e 00 00 	mulq   0x2e48(%rip)        # 20003010 <sum>
    200001c8:	48 89 d7             	mov    %rdx,%rdi
    200001cb:	48 c1 ef 0b          	shr    $0xb,%rdi
    200001cf:	e8 fc 19 00 00       	call   20001bd0 <UVM_Print_Int>
    UVM_Thd_Swt(UVM_CAPID(UVM_BOOT_TBL_THD,0),0);
    200001d4:	31 f6                	xor    %esi,%esi
    200001d6:	bf 00 80 03 00       	mov    $0x38000,%edi
}
    200001db:	5b                   	pop    %rbx
    UVM_Thd_Swt(UVM_CAPID(UVM_BOOT_TBL_THD,0),0);
    200001dc:	e9 1f 21 00 00       	jmp    20002300 <UVM_Thd_Swt>
    200001e1:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    200001e8:	00 00 00 00 
    200001ec:	0f 1f 40 00          	nopl   0x0(%rax)

00000000200001f0 <TEST_THD_FUNC3>:
{
    200001f0:	f3 0f 1e fa          	endbr64 
    sum=0;
    200001f4:	48 c7 05 11 2e 00 00 	movq   $0x0,0x2e11(%rip)        # 20003010 <sum>
    200001fb:	00 00 00 00 
{
    200001ff:	53                   	push   %rbx
    sum=0;
    20000200:	bb 10 27 00 00       	mov    $0x2710,%ebx
    20000205:	0f 1f 00             	nopl   (%rax)
        start=__UVM_X64_Read_TSC();
    20000208:	e8 0e fe ff ff       	call   2000001b <__UVM_X64_Read_TSC>
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD4),0);
    2000020d:	31 f6                	xor    %esi,%esi
    2000020f:	bf 03 80 09 00       	mov    $0x98003,%edi
        start=__UVM_X64_Read_TSC();
    20000214:	48 89 05 0d 2e 00 00 	mov    %rax,0x2e0d(%rip)        # 20003028 <start>
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD4),0);
    2000021b:	e8 e0 20 00 00       	call   20002300 <UVM_Thd_Swt>
        end=__UVM_X64_Read_TSC();
    20000220:	e8 f6 fd ff ff       	call   2000001b <__UVM_X64_Read_TSC>
    20000225:	48 89 05 ec 2d 00 00 	mov    %rax,0x2dec(%rip)        # 20003018 <end>
        sum+=end-start;
    2000022c:	48 8b 05 e5 2d 00 00 	mov    0x2de5(%rip),%rax        # 20003018 <end>
    20000233:	48 8b 15 ee 2d 00 00 	mov    0x2dee(%rip),%rdx        # 20003028 <start>
    2000023a:	48 29 d0             	sub    %rdx,%rax
    2000023d:	48 01 05 cc 2d 00 00 	add    %rax,0x2dcc(%rip)        # 20003010 <sum>
    for(Cnt=0;Cnt<10000;Cnt++)
    20000244:	48 83 eb 01          	sub    $0x1,%rbx
    20000248:	75 be                	jne    20000208 <TEST_THD_FUNC3+0x18>
    UVM_LOG_S("\r\nCross-process thread Switching takes clock cycles:");
    2000024a:	bf e0 24 00 20       	mov    $0x200024e0,%edi
    2000024f:	e8 ac 1b 00 00       	call   20001e00 <UVM_Print_String>
    UVM_LOG_I(sum/10000);
    20000254:	48 ba 4b 59 86 38 d6 	movabs $0x346dc5d63886594b,%rdx
    2000025b:	c5 6d 34 
    2000025e:	48 89 d0             	mov    %rdx,%rax
    20000261:	48 f7 25 a8 2d 00 00 	mulq   0x2da8(%rip)        # 20003010 <sum>
    20000268:	48 89 d7             	mov    %rdx,%rdi
    2000026b:	48 c1 ef 0b          	shr    $0xb,%rdi
    2000026f:	e8 5c 19 00 00       	call   20001bd0 <UVM_Print_Int>
    UVM_Thd_Swt(UVM_CAPID(UVM_BOOT_TBL_THD,0),0);
    20000274:	31 f6                	xor    %esi,%esi
    20000276:	bf 00 80 03 00       	mov    $0x38000,%edi
}
    2000027b:	5b                   	pop    %rbx
    UVM_Thd_Swt(UVM_CAPID(UVM_BOOT_TBL_THD,0),0);
    2000027c:	e9 7f 20 00 00       	jmp    20002300 <UVM_Thd_Swt>
    20000281:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    20000288:	00 00 00 00 
    2000028c:	0f 1f 40 00          	nopl   0x0(%rax)

0000000020000290 <TEST_SIG_FUNC1>:
    }
}

/*This function is for signal sending-receiving test*/
void TEST_SIG_FUNC1(void)
{
    20000290:	f3 0f 1e fa          	endbr64 
    20000294:	53                   	push   %rbx
    cnt_t Cnt;
    sum=0;
    20000295:	bb 10 27 00 00       	mov    $0x2710,%ebx
    2000029a:	48 c7 05 6b 2d 00 00 	movq   $0x0,0x2d6b(%rip)        # 20003010 <sum>
    200002a1:	00 00 00 00 
    for(Cnt=0;Cnt<10000;Cnt++)
    200002a5:	eb 33                	jmp    200002da <TEST_SIG_FUNC1+0x4a>
    200002a7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    200002ae:	00 00 
    {
        start=__UVM_X64_Read_TSC();
        UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG1,RME_RCV_BS));
        end=__UVM_X64_Read_TSC();
    200002b0:	e8 66 fd ff ff       	call   2000001b <__UVM_X64_Read_TSC>
    200002b5:	48 89 05 5c 2d 00 00 	mov    %rax,0x2d5c(%rip)        # 20003018 <end>
        sum+=end-start;
    200002bc:	48 8b 05 55 2d 00 00 	mov    0x2d55(%rip),%rax        # 20003018 <end>
    200002c3:	48 8b 15 5e 2d 00 00 	mov    0x2d5e(%rip),%rdx        # 20003028 <start>
    200002ca:	48 29 d0             	sub    %rdx,%rax
    200002cd:	48 01 05 3c 2d 00 00 	add    %rax,0x2d3c(%rip)        # 20003010 <sum>
    for(Cnt=0;Cnt<10000;Cnt++)
    200002d4:	48 83 eb 01          	sub    $0x1,%rbx
    200002d8:	74 7e                	je     20000358 <TEST_SIG_FUNC1+0xc8>
        start=__UVM_X64_Read_TSC();
    200002da:	e8 3c fd ff ff       	call   2000001b <__UVM_X64_Read_TSC>
        UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG1,RME_RCV_BS));
    200002df:	31 f6                	xor    %esi,%esi
    200002e1:	bf 0b 00 00 00       	mov    $0xb,%edi
        start=__UVM_X64_Read_TSC();
    200002e6:	48 89 05 3b 2d 00 00 	mov    %rax,0x2d3b(%rip)        # 20003028 <start>
        UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG1,RME_RCV_BS));
    200002ed:	e8 8e 20 00 00       	call   20002380 <UVM_Sig_Rcv>
    200002f2:	48 85 c0             	test   %rax,%rax
    200002f5:	75 b9                	jne    200002b0 <TEST_SIG_FUNC1+0x20>
    200002f7:	bf 18 25 00 20       	mov    $0x20002518,%edi
    200002fc:	e8 ff 1a 00 00       	call   20001e00 <UVM_Print_String>
    20000301:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000306:	e8 f5 1a 00 00       	call   20001e00 <UVM_Print_String>
    2000030b:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000310:	e8 eb 1a 00 00       	call   20001e00 <UVM_Print_String>
    20000315:	bf 9b 00 00 00       	mov    $0x9b,%edi
    2000031a:	e8 b1 18 00 00       	call   20001bd0 <UVM_Print_Int>
    2000031f:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000324:	e8 d7 1a 00 00       	call   20001e00 <UVM_Print_String>
    20000329:	bf 57 27 00 20       	mov    $0x20002757,%edi
    2000032e:	e8 cd 1a 00 00       	call   20001e00 <UVM_Print_String>
    20000333:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000338:	e8 c3 1a 00 00       	call   20001e00 <UVM_Print_String>
    2000033d:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000342:	e8 b9 1a 00 00       	call   20001e00 <UVM_Print_String>
    20000347:	bf 54 27 00 20       	mov    $0x20002754,%edi
    2000034c:	e8 af 1a 00 00       	call   20001e00 <UVM_Print_String>
    20000351:	eb fe                	jmp    20000351 <TEST_SIG_FUNC1+0xc1>
    20000353:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    }
    UVM_LOG_S("\r\nSignal sending-receiving takes clock cycles:");
    20000358:	bf 50 25 00 20       	mov    $0x20002550,%edi
    2000035d:	e8 9e 1a 00 00       	call   20001e00 <UVM_Print_String>
    UVM_LOG_I(sum/10000);
    20000362:	48 ba 4b 59 86 38 d6 	movabs $0x346dc5d63886594b,%rdx
    20000369:	c5 6d 34 
    2000036c:	48 89 d0             	mov    %rdx,%rax
    2000036f:	48 f7 25 9a 2c 00 00 	mulq   0x2c9a(%rip)        # 20003010 <sum>
    20000376:	48 89 d7             	mov    %rdx,%rdi
    20000379:	48 c1 ef 0b          	shr    $0xb,%rdi
    2000037d:	e8 4e 18 00 00       	call   20001bd0 <UVM_Print_Int>
    UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG1,RME_RCV_BS));
    20000382:	31 f6                	xor    %esi,%esi
    20000384:	bf 0b 00 00 00       	mov    $0xb,%edi
    20000389:	e8 f2 1f 00 00       	call   20002380 <UVM_Sig_Rcv>
    2000038e:	48 85 c0             	test   %rax,%rax
    20000391:	74 02                	je     20000395 <TEST_SIG_FUNC1+0x105>
}
    20000393:	5b                   	pop    %rbx
    20000394:	c3                   	ret    
    UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG1,RME_RCV_BS));
    20000395:	bf 18 25 00 20       	mov    $0x20002518,%edi
    2000039a:	e8 61 1a 00 00       	call   20001e00 <UVM_Print_String>
    2000039f:	bf 40 27 00 20       	mov    $0x20002740,%edi
    200003a4:	e8 57 1a 00 00       	call   20001e00 <UVM_Print_String>
    200003a9:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    200003ae:	e8 4d 1a 00 00       	call   20001e00 <UVM_Print_String>
    200003b3:	bf a1 00 00 00       	mov    $0xa1,%edi
    200003b8:	e8 13 18 00 00       	call   20001bd0 <UVM_Print_Int>
    200003bd:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200003c2:	e8 39 1a 00 00       	call   20001e00 <UVM_Print_String>
    200003c7:	bf 57 27 00 20       	mov    $0x20002757,%edi
    200003cc:	e8 2f 1a 00 00       	call   20001e00 <UVM_Print_String>
    200003d1:	bf 63 27 00 20       	mov    $0x20002763,%edi
    200003d6:	e8 25 1a 00 00       	call   20001e00 <UVM_Print_String>
    200003db:	bf 66 27 00 20       	mov    $0x20002766,%edi
    200003e0:	e8 1b 1a 00 00       	call   20001e00 <UVM_Print_String>
    200003e5:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200003ea:	e8 11 1a 00 00       	call   20001e00 <UVM_Print_String>
    200003ef:	eb fe                	jmp    200003ef <TEST_SIG_FUNC1+0x15f>
    200003f1:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    200003f8:	00 00 00 00 
    200003fc:	0f 1f 40 00          	nopl   0x0(%rax)

0000000020000400 <TEST_SIG_FUNC2>:

/*This function is for Cross-process signal sending-receiving test*/
void TEST_SIG_FUNC2(void)
{
    20000400:	f3 0f 1e fa          	endbr64 
    20000404:	53                   	push   %rbx
    cnt_t Cnt;
    sum=0;
    20000405:	bb 10 27 00 00       	mov    $0x2710,%ebx
    2000040a:	48 c7 05 fb 2b 00 00 	movq   $0x0,0x2bfb(%rip)        # 20003010 <sum>
    20000411:	00 00 00 00 
    for(Cnt=0;Cnt<10000;Cnt++)
    20000415:	eb 33                	jmp    2000044a <TEST_SIG_FUNC2+0x4a>
    20000417:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    2000041e:	00 00 
    {
        start=__UVM_X64_Read_TSC();
        UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG2,RME_RCV_BS));
        end=__UVM_X64_Read_TSC();
    20000420:	e8 f6 fb ff ff       	call   2000001b <__UVM_X64_Read_TSC>
    20000425:	48 89 05 ec 2b 00 00 	mov    %rax,0x2bec(%rip)        # 20003018 <end>
        sum+=end-start;
    2000042c:	48 8b 05 e5 2b 00 00 	mov    0x2be5(%rip),%rax        # 20003018 <end>
    20000433:	48 8b 15 ee 2b 00 00 	mov    0x2bee(%rip),%rdx        # 20003028 <start>
    2000043a:	48 29 d0             	sub    %rdx,%rax
    2000043d:	48 01 05 cc 2b 00 00 	add    %rax,0x2bcc(%rip)        # 20003010 <sum>
    for(Cnt=0;Cnt<10000;Cnt++)
    20000444:	48 83 eb 01          	sub    $0x1,%rbx
    20000448:	74 7e                	je     200004c8 <TEST_SIG_FUNC2+0xc8>
        start=__UVM_X64_Read_TSC();
    2000044a:	e8 cc fb ff ff       	call   2000001b <__UVM_X64_Read_TSC>
        UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG2,RME_RCV_BS));
    2000044f:	31 f6                	xor    %esi,%esi
    20000451:	bf 0c 00 00 00       	mov    $0xc,%edi
        start=__UVM_X64_Read_TSC();
    20000456:	48 89 05 cb 2b 00 00 	mov    %rax,0x2bcb(%rip)        # 20003028 <start>
        UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG2,RME_RCV_BS));
    2000045d:	e8 1e 1f 00 00       	call   20002380 <UVM_Sig_Rcv>
    20000462:	48 85 c0             	test   %rax,%rax
    20000465:	75 b9                	jne    20000420 <TEST_SIG_FUNC2+0x20>
    20000467:	bf 18 25 00 20       	mov    $0x20002518,%edi
    2000046c:	e8 8f 19 00 00       	call   20001e00 <UVM_Print_String>
    20000471:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000476:	e8 85 19 00 00       	call   20001e00 <UVM_Print_String>
    2000047b:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000480:	e8 7b 19 00 00       	call   20001e00 <UVM_Print_String>
    20000485:	bf ac 00 00 00       	mov    $0xac,%edi
    2000048a:	e8 41 17 00 00       	call   20001bd0 <UVM_Print_Int>
    2000048f:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000494:	e8 67 19 00 00       	call   20001e00 <UVM_Print_String>
    20000499:	bf 57 27 00 20       	mov    $0x20002757,%edi
    2000049e:	e8 5d 19 00 00       	call   20001e00 <UVM_Print_String>
    200004a3:	bf 63 27 00 20       	mov    $0x20002763,%edi
    200004a8:	e8 53 19 00 00       	call   20001e00 <UVM_Print_String>
    200004ad:	bf 66 27 00 20       	mov    $0x20002766,%edi
    200004b2:	e8 49 19 00 00       	call   20001e00 <UVM_Print_String>
    200004b7:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200004bc:	e8 3f 19 00 00       	call   20001e00 <UVM_Print_String>
    200004c1:	eb fe                	jmp    200004c1 <TEST_SIG_FUNC2+0xc1>
    200004c3:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    }
    UVM_LOG_S("\r\nCross-process signal sending-receiving takes clock cycles:");
    200004c8:	bf 80 25 00 20       	mov    $0x20002580,%edi
    200004cd:	e8 2e 19 00 00       	call   20001e00 <UVM_Print_String>
    UVM_LOG_I(sum/10000);
    200004d2:	48 ba 4b 59 86 38 d6 	movabs $0x346dc5d63886594b,%rdx
    200004d9:	c5 6d 34 
    200004dc:	48 89 d0             	mov    %rdx,%rax
    200004df:	48 f7 25 2a 2b 00 00 	mulq   0x2b2a(%rip)        # 20003010 <sum>
    200004e6:	48 89 d7             	mov    %rdx,%rdi
    200004e9:	48 c1 ef 0b          	shr    $0xb,%rdi
    200004ed:	e8 de 16 00 00       	call   20001bd0 <UVM_Print_Int>
    UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG2,RME_RCV_BS));
    200004f2:	31 f6                	xor    %esi,%esi
    200004f4:	bf 0c 00 00 00       	mov    $0xc,%edi
    200004f9:	e8 82 1e 00 00       	call   20002380 <UVM_Sig_Rcv>
    200004fe:	48 85 c0             	test   %rax,%rax
    20000501:	74 02                	je     20000505 <TEST_SIG_FUNC2+0x105>
}
    20000503:	5b                   	pop    %rbx
    20000504:	c3                   	ret    
    UVM_ASSERT(UVM_Sig_Rcv(TEST_SIG2,RME_RCV_BS));
    20000505:	bf 18 25 00 20       	mov    $0x20002518,%edi
    2000050a:	e8 f1 18 00 00       	call   20001e00 <UVM_Print_String>
    2000050f:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000514:	e8 e7 18 00 00       	call   20001e00 <UVM_Print_String>
    20000519:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    2000051e:	e8 dd 18 00 00       	call   20001e00 <UVM_Print_String>
    20000523:	bf b2 00 00 00       	mov    $0xb2,%edi
    20000528:	e8 a3 16 00 00       	call   20001bd0 <UVM_Print_Int>
    2000052d:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000532:	e8 c9 18 00 00       	call   20001e00 <UVM_Print_String>
    20000537:	bf 57 27 00 20       	mov    $0x20002757,%edi
    2000053c:	e8 bf 18 00 00       	call   20001e00 <UVM_Print_String>
    20000541:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000546:	e8 b5 18 00 00       	call   20001e00 <UVM_Print_String>
    2000054b:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000550:	e8 ab 18 00 00       	call   20001e00 <UVM_Print_String>
    20000555:	bf 54 27 00 20       	mov    $0x20002754,%edi
    2000055a:	e8 a1 18 00 00       	call   20001e00 <UVM_Print_String>
    2000055f:	eb fe                	jmp    2000055f <TEST_SIG_FUNC2+0x15f>
    20000561:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    20000568:	00 00 00 00 
    2000056c:	0f 1f 40 00          	nopl   0x0(%rax)

0000000020000570 <TEST_INV1_FUNC>:

void TEST_INV1_FUNC(ptr_t param)
{
    20000570:	f3 0f 1e fa          	endbr64 
    20000574:	55                   	push   %rbp
    20000575:	48 89 fd             	mov    %rdi,%rbp
    middle=__UVM_X64_Read_TSC();
    20000578:	e8 9e fa ff ff       	call   2000001b <__UVM_X64_Read_TSC>
    UVM_Inv_Ret(param);
    2000057d:	48 89 ef             	mov    %rbp,%rdi
}
    20000580:	5d                   	pop    %rbp
    middle=__UVM_X64_Read_TSC();
    20000581:	48 89 05 98 2a 00 00 	mov    %rax,0x2a98(%rip)        # 20003020 <middle>
    UVM_Inv_Ret(param);
    20000588:	e9 57 fb ff ff       	jmp    200000e4 <UVM_Inv_Ret>
    2000058d:	0f 1f 00             	nopl   (%rax)

0000000020000590 <main>:
Input       : None.
Output      : None.
Return      : int - This function shall never return.
******************************************************************************/
int main(ptr_t CPUID)
{
    20000590:	f3 0f 1e fa          	endbr64 
    20000594:	41 57                	push   %r15
    20000596:	41 56                	push   %r14
    20000598:	41 55                	push   %r13
    2000059a:	41 54                	push   %r12
    2000059c:	55                   	push   %rbp
    2000059d:	53                   	push   %rbx
    2000059e:	48 89 fb             	mov    %rdi,%rbx
    ptr_t Cur_Addr;
    cnt_t Count;
    cnt_t Count1;
    cnt_t Count2;
    UVM_LOG_S("........Booting RME system........");
    200005a1:	bf c0 25 00 20       	mov    $0x200025c0,%edi
{
    200005a6:	48 83 ec 08          	sub    $0x8,%rsp
    UVM_LOG_S("........Booting RME system........");
    200005aa:	e8 51 18 00 00       	call   20001e00 <UVM_Print_String>
    UVM_LOG_S("\r\nEnter user mode success!Welcome to RME system!");
    200005af:	bf e8 25 00 20       	mov    $0x200025e8,%edi
    200005b4:	e8 47 18 00 00       	call   20001e00 <UVM_Print_String>
    UVM_LOG_S("\r\nNow we are running init thread on cpu:");
    200005b9:	bf 20 26 00 20       	mov    $0x20002620,%edi
    200005be:	e8 3d 18 00 00       	call   20001e00 <UVM_Print_String>
    UVM_LOG_I(CPUID);
    200005c3:	48 89 df             	mov    %rbx,%rdi
    200005c6:	e8 05 16 00 00       	call   20001bd0 <UVM_Print_Int>
    if(CPUID==0) 
    200005cb:	48 85 db             	test   %rbx,%rbx
    200005ce:	0f 85 10 05 00 00    	jne    20000ae4 <main+0x554>
    {
        /*Empty test begins here*/
        sum=0;
    200005d4:	48 c7 05 31 2a 00 00 	movq   $0x0,0x2a31(%rip)        # 20003010 <sum>
    200005db:	00 00 00 00 
    200005df:	bb 10 27 00 00       	mov    $0x2710,%ebx
    200005e4:	0f 1f 40 00          	nopl   0x0(%rax)
        for(Count=0;Count<10000;Count++)
        {
            start=__UVM_X64_Read_TSC();
    200005e8:	e8 2e fa ff ff       	call   2000001b <__UVM_X64_Read_TSC>
    200005ed:	48 89 05 34 2a 00 00 	mov    %rax,0x2a34(%rip)        # 20003028 <start>
            end=__UVM_X64_Read_TSC();
    200005f4:	e8 22 fa ff ff       	call   2000001b <__UVM_X64_Read_TSC>
    200005f9:	48 89 05 18 2a 00 00 	mov    %rax,0x2a18(%rip)        # 20003018 <end>
            sum+=end-start;
    20000600:	48 8b 05 11 2a 00 00 	mov    0x2a11(%rip),%rax        # 20003018 <end>
    20000607:	48 8b 15 1a 2a 00 00 	mov    0x2a1a(%rip),%rdx        # 20003028 <start>
    2000060e:	48 29 d0             	sub    %rdx,%rax
    20000611:	48 01 05 f8 29 00 00 	add    %rax,0x29f8(%rip)        # 20003010 <sum>
        for(Count=0;Count<10000;Count++)
    20000618:	48 83 eb 01          	sub    $0x1,%rbx
    2000061c:	75 ca                	jne    200005e8 <main+0x58>
        }
        UVM_LOG_S("\r\nEmpty test takes clock cycles:");
    2000061e:	bf 50 26 00 20       	mov    $0x20002650,%edi
        UVM_LOG_I(sum/10000);
        /*Empty test ends here*/

        /*Empty system call test begins here*/
        sum=0;
    20000623:	bb 10 27 00 00       	mov    $0x2710,%ebx
        UVM_LOG_S("\r\nEmpty test takes clock cycles:");
    20000628:	e8 d3 17 00 00       	call   20001e00 <UVM_Print_String>
        UVM_LOG_I(sum/10000);
    2000062d:	48 8b 05 dc 29 00 00 	mov    0x29dc(%rip),%rax        # 20003010 <sum>
    20000634:	b9 10 27 00 00       	mov    $0x2710,%ecx
    20000639:	31 d2                	xor    %edx,%edx
    2000063b:	48 f7 f1             	div    %rcx
    2000063e:	48 89 c7             	mov    %rax,%rdi
    20000641:	e8 8a 15 00 00       	call   20001bd0 <UVM_Print_Int>
        sum=0;
    20000646:	48 c7 05 bf 29 00 00 	movq   $0x0,0x29bf(%rip)        # 20003010 <sum>
    2000064d:	00 00 00 00 
        for(Count=0;Count<10000;Count++)
    20000651:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
        {
            start=__UVM_X64_Read_TSC();
    20000658:	e8 be f9 ff ff       	call   2000001b <__UVM_X64_Read_TSC>
            UVM_Svc(-1,-1,-1,-1);
    2000065d:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
    20000664:	48 c7 c1 ff ff ff ff 	mov    $0xffffffffffffffff,%rcx
    2000066b:	48 c7 c6 ff ff ff ff 	mov    $0xffffffffffffffff,%rsi
    20000672:	48 c7 c7 ff ff ff ff 	mov    $0xffffffffffffffff,%rdi
            start=__UVM_X64_Read_TSC();
    20000679:	48 89 05 a8 29 00 00 	mov    %rax,0x29a8(%rip)        # 20003028 <start>
            UVM_Svc(-1,-1,-1,-1);
    20000680:	e8 68 fa ff ff       	call   200000ed <UVM_Svc>
            end=__UVM_X64_Read_TSC();
    20000685:	e8 91 f9 ff ff       	call   2000001b <__UVM_X64_Read_TSC>
    2000068a:	48 89 05 87 29 00 00 	mov    %rax,0x2987(%rip)        # 20003018 <end>
            sum+=end-start;
    20000691:	48 8b 05 80 29 00 00 	mov    0x2980(%rip),%rax        # 20003018 <end>
    20000698:	48 8b 15 89 29 00 00 	mov    0x2989(%rip),%rdx        # 20003028 <start>
    2000069f:	48 29 d0             	sub    %rdx,%rax
    200006a2:	48 01 05 67 29 00 00 	add    %rax,0x2967(%rip)        # 20003010 <sum>
        for(Count=0;Count<10000;Count++)
    200006a9:	48 83 eb 01          	sub    $0x1,%rbx
    200006ad:	75 a9                	jne    20000658 <main+0xc8>
        }
        UVM_LOG_S("\r\nEmpty system call takes clock cycles:");
    200006af:	bf 78 26 00 20       	mov    $0x20002678,%edi
    200006b4:	e8 47 17 00 00       	call   20001e00 <UVM_Print_String>
        UVM_LOG_I(sum/10000);
    200006b9:	48 8b 05 50 29 00 00 	mov    0x2950(%rip),%rax        # 20003010 <sum>
    200006c0:	b9 10 27 00 00       	mov    $0x2710,%ecx
    200006c5:	31 d2                	xor    %edx,%edx
    200006c7:	48 f7 f1             	div    %rcx
    200006ca:	48 89 c7             	mov    %rax,%rdi
    200006cd:	e8 fe 14 00 00       	call   20001bd0 <UVM_Print_Int>
        /*Now we begin to create UVM kernel objects*/
        Cur_Addr=UVM_OBJ_BASE;
        
        /*Thread switching test begins here, We place the thread stack at 12MB */
        /*Create test thread capability table*/
        UVM_ASSERT(UVM_Captbl_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_THD_TBL,Cur_Addr,16)>=0);
    200006d2:	31 ff                	xor    %edi,%edi
    200006d4:	41 b8 10 00 00 00    	mov    $0x10,%r8d
    200006da:	b9 00 00 a0 0e       	mov    $0xea00000,%ecx
    200006df:	ba 09 00 00 00       	mov    $0x9,%edx
    200006e4:	be 00 80 05 00       	mov    $0x58000,%esi
    200006e9:	e8 52 17 00 00       	call   20001e40 <UVM_Captbl_Crt>
    200006ee:	48 85 c0             	test   %rax,%rax
    200006f1:	0f 88 fb 03 00 00    	js     20000af2 <main+0x562>
        Cur_Addr+=UVM_CAPTBL_SIZE(16);
        UVM_ASSERT(UVM_Thd_Crt(TEST_THD_TBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_THD1, UVM_BOOT_INIT_PROC, 10, Cur_Addr)>=0);
    200006f7:	31 d2                	xor    %edx,%edx
    200006f9:	41 b9 00 04 a0 0e    	mov    $0xea00400,%r9d
    200006ff:	41 b8 0a 00 00 00    	mov    $0xa,%r8d
    20000705:	b9 02 00 00 00       	mov    $0x2,%ecx
    2000070a:	be 00 80 05 00       	mov    $0x58000,%esi
    2000070f:	bf 09 00 00 00       	mov    $0x9,%edi
    20000714:	e8 97 1a 00 00       	call   200021b0 <UVM_Thd_Crt>
    20000719:	48 85 c0             	test   %rax,%rax
    2000071c:	0f 88 2c 04 00 00    	js     20000b4e <main+0x5be>
        UVM_ASSERT(UVM_Thd_Sched_Bind(UVM_CAPID(TEST_THD_TBL,TEST_THD1),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_CAPID_NULL,0,0)>=0);
    20000722:	45 31 c0             	xor    %r8d,%r8d
    20000725:	31 c9                	xor    %ecx,%ecx
    20000727:	ba 00 00 00 80       	mov    $0x80000000,%edx
    2000072c:	be 00 80 03 00       	mov    $0x38000,%esi
    20000731:	bf 00 80 09 00       	mov    $0x98000,%edi
    20000736:	e8 15 1b 00 00       	call   20002250 <UVM_Thd_Sched_Bind>
    2000073b:	48 85 c0             	test   %rax,%rax
    2000073e:	0f 88 66 04 00 00    	js     20000baa <main+0x61a>
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD1),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
    20000744:	be 00 80 03 00       	mov    $0x38000,%esi
    20000749:	bf 00 80 09 00       	mov    $0x98000,%edi
    Word_Inc=(ptr_t*)Addr;
    2000074e:	bb 00 00 c0 00       	mov    $0xc00000,%ebx
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD1),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
    20000753:	48 ba fe ff ff ff ff 	movabs $0x7ffffffffffffffe,%rdx
    2000075a:	ff ff 7f 
    2000075d:	e8 7e 1b 00 00       	call   200022e0 <UVM_Thd_Time_Xfer>
    20000762:	48 85 c0             	test   %rax,%rax
    20000765:	0f 88 f7 04 00 00    	js     20000c62 <main+0x6d2>
        *Word_Inc=0;
    2000076b:	48 c7 03 00 00 00 00 	movq   $0x0,(%rbx)
        Word_Inc++;
    20000772:	48 83 c3 08          	add    $0x8,%rbx
    for(Words=Size/sizeof(ptr_t);Words>0;Words--)
    20000776:	48 81 fb 00 00 d0 00 	cmp    $0xd00000,%rbx
    2000077d:	75 ec                	jne    2000076b <main+0x1db>
        UVM_Clear((void*)(12*UVM_POW2(RME_PGT_SIZE_1M)), UVM_POW2(RME_PGT_SIZE_1M));
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD1),(ptr_t)TEST_THD_FUNC1,12*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
    2000077f:	31 c9                	xor    %ecx,%ecx
    20000781:	ba 00 00 c0 20       	mov    $0x20c00000,%edx
    20000786:	be 50 01 00 20       	mov    $0x20000150,%esi
    2000078b:	bf 00 80 09 00       	mov    $0x98000,%edi
    20000790:	e8 7b 1a 00 00       	call   20002210 <UVM_Thd_Exec_Set>
    20000795:	48 85 c0             	test   %rax,%rax
    20000798:	0f 88 68 04 00 00    	js     20000c06 <main+0x676>
        Cur_Addr+=UVM_THD_SIZE;
        UVM_ASSERT(UVM_Thd_Crt(TEST_THD_TBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0), TEST_THD2, UVM_BOOT_INIT_PROC, 10, Cur_Addr)>=0);
    2000079e:	41 b9 d0 0d a0 0e    	mov    $0xea00dd0,%r9d
    200007a4:	41 b8 0a 00 00 00    	mov    $0xa,%r8d
    200007aa:	b9 02 00 00 00       	mov    $0x2,%ecx
    200007af:	ba 01 00 00 00       	mov    $0x1,%edx
    200007b4:	be 00 80 05 00       	mov    $0x58000,%esi
    200007b9:	bf 09 00 00 00       	mov    $0x9,%edi
    200007be:	e8 ed 19 00 00       	call   200021b0 <UVM_Thd_Crt>
    200007c3:	48 85 c0             	test   %rax,%rax
    200007c6:	0f 88 f2 04 00 00    	js     20000cbe <main+0x72e>
        UVM_ASSERT(UVM_Thd_Sched_Bind(UVM_CAPID(TEST_THD_TBL,TEST_THD2),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_CAPID_NULL,0,0)>=0);
    200007cc:	45 31 c0             	xor    %r8d,%r8d
    200007cf:	31 c9                	xor    %ecx,%ecx
    200007d1:	ba 00 00 00 80       	mov    $0x80000000,%edx
    200007d6:	be 00 80 03 00       	mov    $0x38000,%esi
    200007db:	bf 01 80 09 00       	mov    $0x98001,%edi
    200007e0:	e8 6b 1a 00 00       	call   20002250 <UVM_Thd_Sched_Bind>
    200007e5:	48 85 c0             	test   %rax,%rax
    200007e8:	0f 88 2c 05 00 00    	js     20000d1a <main+0x78a>
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD2),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
    200007ee:	48 ba fe ff ff ff ff 	movabs $0x7ffffffffffffffe,%rdx
    200007f5:	ff ff 7f 
    200007f8:	be 00 80 03 00       	mov    $0x38000,%esi
    200007fd:	bf 01 80 09 00       	mov    $0x98001,%edi
    20000802:	e8 d9 1a 00 00       	call   200022e0 <UVM_Thd_Time_Xfer>
    20000807:	48 85 c0             	test   %rax,%rax
    2000080a:	0f 88 c2 05 00 00    	js     20000dd2 <main+0x842>
        *Word_Inc=0;
    20000810:	48 c7 03 00 00 00 00 	movq   $0x0,(%rbx)
        Word_Inc++;
    20000817:	48 83 c3 08          	add    $0x8,%rbx
    for(Words=Size/sizeof(ptr_t);Words>0;Words--)
    2000081b:	48 81 fb 00 00 e0 00 	cmp    $0xe00000,%rbx
    20000822:	75 ec                	jne    20000810 <main+0x280>
        UVM_Clear((void*)(13*UVM_POW2(RME_PGT_SIZE_1M)), UVM_POW2(RME_PGT_SIZE_1M));
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD2),(ptr_t)TEST_THD_FUNC2,13*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,1)>=0);
    20000824:	b9 01 00 00 00       	mov    $0x1,%ecx
    20000829:	ba 00 00 d0 20       	mov    $0x20d00000,%edx
    2000082e:	be 10 01 00 20       	mov    $0x20000110,%esi
    20000833:	bf 01 80 09 00       	mov    $0x98001,%edi
    20000838:	e8 d3 19 00 00       	call   20002210 <UVM_Thd_Exec_Set>
    2000083d:	48 85 c0             	test   %rax,%rax
    20000840:	0f 88 30 05 00 00    	js     20000d76 <main+0x7e6>
        Cur_Addr+=UVM_THD_SIZE;
        UVM_LOG_S("\r\nSwtching thread...");
    20000846:	bf 6f 27 00 20       	mov    $0x2000276f,%edi
    2000084b:	e8 b0 15 00 00       	call   20001e00 <UVM_Print_String>
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD1),0);
    20000850:	31 f6                	xor    %esi,%esi
    20000852:	bf 00 80 09 00       	mov    $0x98000,%edi
    20000857:	e8 a4 1a 00 00       	call   20002300 <UVM_Thd_Swt>
        UVM_LOG_S("\r\nBack to main thread!");
    2000085c:	bf 84 27 00 20       	mov    $0x20002784,%edi
    20000861:	e8 9a 15 00 00       	call   20001e00 <UVM_Print_String>
        /*Thread switching test ends here*/
        
        /*Signal Sending-receiving test begins here*/
        /*create endpoint first*/
        UVM_ASSERT(UVM_Sig_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_SIG1, Cur_Addr)>=0);
    20000866:	b9 a0 17 a0 0e       	mov    $0xea017a0,%ecx
    2000086b:	ba 0b 00 00 00       	mov    $0xb,%edx
    20000870:	31 ff                	xor    %edi,%edi
    20000872:	be 00 80 05 00       	mov    $0x58000,%esi
    20000877:	e8 a4 1a 00 00       	call   20002320 <UVM_Sig_Crt>
    2000087c:	49 89 c0             	mov    %rax,%r8
    Word_Inc=(ptr_t*)Addr;
    2000087f:	b8 00 00 c0 00       	mov    $0xc00000,%eax
        UVM_ASSERT(UVM_Sig_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_SIG1, Cur_Addr)>=0);
    20000884:	4d 85 c0             	test   %r8,%r8
    20000887:	0f 88 a1 05 00 00    	js     20000e2e <main+0x89e>
        *Word_Inc=0;
    2000088d:	48 c7 00 00 00 00 00 	movq   $0x0,(%rax)
        Word_Inc++;
    20000894:	48 83 c0 08          	add    $0x8,%rax
    for(Words=Size/sizeof(ptr_t);Words>0;Words--)
    20000898:	48 3d 00 00 d0 00    	cmp    $0xd00000,%rax
    2000089e:	75 ed                	jne    2000088d <main+0x2fd>
        Cur_Addr+=UVM_SIG_SIZE;
        /*reset two threads*/
        UVM_Clear((void*)(12*UVM_POW2(RME_PGT_SIZE_1M)), UVM_POW2(RME_PGT_SIZE_1M));
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD1),(ptr_t)TEST_SIG_FUNC1,12*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
    200008a0:	31 c9                	xor    %ecx,%ecx
    200008a2:	ba 00 00 c0 20       	mov    $0x20c00000,%edx
    200008a7:	be 90 02 00 20       	mov    $0x20000290,%esi
    200008ac:	bf 00 80 09 00       	mov    $0x98000,%edi
    200008b1:	e8 5a 19 00 00       	call   20002210 <UVM_Thd_Exec_Set>
    200008b6:	48 85 c0             	test   %rax,%rax
    200008b9:	0f 88 27 06 00 00    	js     20000ee6 <main+0x956>
        /*throw away thread1 infinity time slices,and add finity time slices to it*/
        //UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(UVM_BOOT_TBL_THD,0),TEST_THD1,UVM_THD_INIT_TIME)>=0);
        //UVM_ASSERT(UVM_Thd_Time_Xfer(TEST_THD1,UVM_CAPID(UVM_BOOT_TBL_THD,0),100)>=0);
        UVM_ASSERT(UVM_Thd_Sched_Prio(UVM_CAPID(TEST_THD_TBL,TEST_THD1),1)>=0);
    200008bf:	be 01 00 00 00       	mov    $0x1,%esi
    200008c4:	bf 00 80 09 00       	mov    $0x98000,%edi
    200008c9:	bd 10 27 00 00       	mov    $0x2710,%ebp
    200008ce:	e8 cd 19 00 00       	call   200022a0 <UVM_Thd_Sched_Prio>
    200008d3:	48 85 c0             	test   %rax,%rax
    200008d6:	0f 88 ae 05 00 00    	js     20000e8a <main+0x8fa>
        for(Count=0;Count<10000;Count++)
        {
            UVM_Sig_Snd(TEST_SIG1,1);
    200008dc:	be 01 00 00 00       	mov    $0x1,%esi
    200008e1:	bf 0b 00 00 00       	mov    $0xb,%edi
    200008e6:	e8 75 1a 00 00       	call   20002360 <UVM_Sig_Snd>
        for(Count=0;Count<10000;Count++)
    200008eb:	48 83 ed 01          	sub    $0x1,%rbp
    200008ef:	75 eb                	jne    200008dc <main+0x34c>
        }
        UVM_ASSERT(UVM_Thd_Sched_Prio(UVM_CAPID(TEST_THD_TBL,TEST_THD1),0)>=0);
    200008f1:	31 f6                	xor    %esi,%esi
    200008f3:	bf 00 80 09 00       	mov    $0x98000,%edi
    200008f8:	e8 a3 19 00 00       	call   200022a0 <UVM_Thd_Sched_Prio>
    200008fd:	48 85 c0             	test   %rax,%rax
    20000900:	0f 88 24 12 00 00    	js     20001b2a <main+0x159a>
        /*Signal Sending-receiving test ends here*/

        /*Cross-process thread switching test begins here*/

        /*Create test process capability table*/
        UVM_ASSERT(UVM_Captbl_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_PROC_CAPTBL,Cur_Addr,16)>=0);
    20000906:	31 ff                	xor    %edi,%edi
    20000908:	41 b8 10 00 00 00    	mov    $0x10,%r8d
    2000090e:	b9 c0 17 a0 0e       	mov    $0xea017c0,%ecx
    20000913:	ba 0d 00 00 00       	mov    $0xd,%edx
    20000918:	be 00 80 05 00       	mov    $0x58000,%esi
    2000091d:	e8 1e 15 00 00       	call   20001e40 <UVM_Captbl_Crt>
    20000922:	48 85 c0             	test   %rax,%rax
    20000925:	0f 88 a3 11 00 00    	js     20001ace <main+0x153e>
        Cur_Addr+=UVM_CAPTBL_SIZE(16);
        /*Create test process page table*/
        UVM_ASSERT(UVM_Captbl_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_PROCESS_PGT,Cur_Addr,1+16+8192)>=0);
    2000092b:	31 ff                	xor    %edi,%edi
    2000092d:	41 b8 11 20 00 00    	mov    $0x2011,%r8d
    20000933:	b9 c0 1b a0 0e       	mov    $0xea01bc0,%ecx
    20000938:	ba 0e 00 00 00       	mov    $0xe,%edx
    2000093d:	be 00 80 05 00       	mov    $0x58000,%esi
    20000942:	e8 f9 14 00 00       	call   20001e40 <UVM_Captbl_Crt>
    20000947:	48 85 c0             	test   %rax,%rax
    2000094a:	0f 88 22 11 00 00    	js     20001a72 <main+0x14e2>
        Cur_Addr+=UVM_CAPTBL_SIZE(1+16+8192);
        /*Create test process PML4*/
        Cur_Addr=UVM_ROUND_UP(Cur_Addr,12);
        UVM_ASSERT(UVM_Pgtbl_Crt(TEST_PROCESS_PGT,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_PROCESS_PML4,Cur_Addr,0,1U,RME_PGT_SIZE_512G,RME_PGT_NUM_512)>=0);
    20000950:	6a 09                	push   $0x9
    20000952:	41 b9 01 00 00 00    	mov    $0x1,%r9d
    20000958:	45 31 c0             	xor    %r8d,%r8d
    2000095b:	b9 00 20 a8 0e       	mov    $0xea82000,%ecx
    20000960:	6a 27                	push   $0x27
    20000962:	31 d2                	xor    %edx,%edx
    20000964:	be 00 80 05 00       	mov    $0x58000,%esi
    20000969:	bf 0e 00 00 00       	mov    $0xe,%edi
    2000096e:	e8 9d 16 00 00       	call   20002010 <UVM_Pgtbl_Crt>
    20000973:	41 59                	pop    %r9
    20000975:	41 5a                	pop    %r10
    20000977:	48 85 c0             	test   %rax,%rax
    2000097a:	0f 88 96 10 00 00    	js     20001a16 <main+0x1486>
        Cur_Addr+=UVM_PGTBL_SIZE_NOM(RME_PGT_NUM_512);
        /* Create 16 PDPs*/
        for(Count=0;Count<16;Count++)
    20000980:	45 31 e4             	xor    %r12d,%r12d
        Cur_Addr+=UVM_PGTBL_SIZE_NOM(RME_PGT_NUM_512);
    20000983:	bd 00 a0 a8 0e       	mov    $0xea8a000,%ebp
        {
            UVM_ASSERT(UVM_Pgtbl_Crt(UVM_CAPID(UVM_BOOT_CAPTBL,TEST_PROCESS_PGT), UVM_CAPID(UVM_BOOT_TBL_KMEM,0), RME_TEST_PDP(Count),
    20000988:	6a 09                	push   $0x9
    2000098a:	4d 89 e5             	mov    %r12,%r13
    2000098d:	49 83 c4 01          	add    $0x1,%r12
    20000991:	bf 0e 80 00 00       	mov    $0x800e,%edi
    20000996:	6a 1e                	push   $0x1e
    20000998:	4d 89 e8             	mov    %r13,%r8
    2000099b:	45 31 c9             	xor    %r9d,%r9d
    2000099e:	48 89 e9             	mov    %rbp,%rcx
    200009a1:	49 c1 e0 27          	shl    $0x27,%r8
    200009a5:	4c 89 e2             	mov    %r12,%rdx
    200009a8:	be 00 80 05 00       	mov    $0x58000,%esi
    200009ad:	e8 5e 16 00 00       	call   20002010 <UVM_Pgtbl_Crt>
    200009b2:	5f                   	pop    %rdi
    200009b3:	41 58                	pop    %r8
    200009b5:	48 85 c0             	test   %rax,%rax
    200009b8:	0f 88 e0 05 00 00    	js     20000f9e <main+0xa0e>
                                           Cur_Addr, (ptr_t)UVM_POW2(RME_PGT_SIZE_512G)*Count, 0U, RME_PGT_SIZE_1G, RME_PGT_NUM_512)>=0);
            Cur_Addr+=UVM_PGTBL_SIZE_NOM(RME_PGT_NUM_512);
            UVM_ASSERT(UVM_Pgtbl_Con(UVM_CAPID(TEST_PROCESS_PGT,TEST_PROCESS_PML4),Count,
    200009be:	4c 89 e2             	mov    %r12,%rdx
    200009c1:	b9 3f 00 00 00       	mov    $0x3f,%ecx
    200009c6:	4c 89 ee             	mov    %r13,%rsi
    200009c9:	bf 00 80 0e 00       	mov    $0xe8000,%edi
    200009ce:	48 81 ca 00 80 0e 00 	or     $0xe8000,%rdx
            Cur_Addr+=UVM_PGTBL_SIZE_NOM(RME_PGT_NUM_512);
    200009d5:	48 81 c5 00 80 00 00 	add    $0x8000,%rbp
            UVM_ASSERT(UVM_Pgtbl_Con(UVM_CAPID(TEST_PROCESS_PGT,TEST_PROCESS_PML4),Count,
    200009dc:	e8 ff 16 00 00       	call   200020e0 <UVM_Pgtbl_Con>
    200009e1:	48 85 c0             	test   %rax,%rax
    200009e4:	0f 88 58 05 00 00    	js     20000f42 <main+0x9b2>
        for(Count=0;Count<16;Count++)
    200009ea:	48 81 fd 00 a0 b0 0e 	cmp    $0xeb0a000,%rbp
    200009f1:	75 95                	jne    20000988 <main+0x3f8>
                                                UVM_CAPID(TEST_PROCESS_PGT,RME_TEST_PDP(Count)),RME_PGT_ALL_PERM)>=0);
        }
        UVM_LOG_S("\r\nCreate PDPs success!");
    200009f3:	bf 9b 27 00 20       	mov    $0x2000279b,%edi
        /* Create 8192 PDEs*/
        for(Count=0;Count<8192;Count++)
    200009f8:	45 31 e4             	xor    %r12d,%r12d
        UVM_LOG_S("\r\nCreate PDPs success!");
    200009fb:	e8 00 14 00 00       	call   20001e00 <UVM_Print_String>
        for(Count=0;Count<8192;Count++)
    20000a00:	eb 53                	jmp    20000a55 <main+0x4c5>
        {
            UVM_ASSERT(UVM_Pgtbl_Crt(UVM_CAPID(UVM_BOOT_CAPTBL,TEST_PROCESS_PGT), UVM_CAPID(UVM_BOOT_TBL_KMEM,0), RME_TEST_PDE(Count),
                                           Cur_Addr, (ptr_t)UVM_POW2(RME_PGT_SIZE_1G)*Count, 0U,  RME_PGT_SIZE_2M, RME_PGT_NUM_512)>=0);
            Cur_Addr+=UVM_PGTBL_SIZE_NOM(RME_PGT_NUM_512);
            UVM_ASSERT(UVM_Pgtbl_Con(UVM_CAPID(TEST_PROCESS_PGT,RME_TEST_PDP(Count>>9)),Count&0x1FF,
    20000a02:	4c 89 e7             	mov    %r12,%rdi
    20000a05:	4c 89 f2             	mov    %r14,%rdx
    20000a08:	4c 89 e6             	mov    %r12,%rsi
    20000a0b:	b9 3f 00 00 00       	mov    $0x3f,%ecx
    20000a10:	48 c1 ff 09          	sar    $0x9,%rdi
    20000a14:	48 81 ca 00 80 0e 00 	or     $0xe8000,%rdx
    20000a1b:	81 e6 ff 01 00 00    	and    $0x1ff,%esi
    20000a21:	48 83 c7 01          	add    $0x1,%rdi
            Cur_Addr+=UVM_PGTBL_SIZE_NOM(RME_PGT_NUM_512);
    20000a25:	4c 8d ad 00 80 00 00 	lea    0x8000(%rbp),%r13
            UVM_ASSERT(UVM_Pgtbl_Con(UVM_CAPID(TEST_PROCESS_PGT,RME_TEST_PDP(Count>>9)),Count&0x1FF,
    20000a2c:	48 81 cf 00 80 0e 00 	or     $0xe8000,%rdi
    20000a33:	e8 a8 16 00 00       	call   200020e0 <UVM_Pgtbl_Con>
    20000a38:	48 85 c0             	test   %rax,%rax
    20000a3b:	0f 88 6a 06 00 00    	js     200010ab <main+0xb1b>
        for(Count=0;Count<8192;Count++)
    20000a41:	49 83 c4 01          	add    $0x1,%r12
    20000a45:	49 81 fc 00 20 00 00 	cmp    $0x2000,%r12
    20000a4c:	0f 84 a8 05 00 00    	je     20000ffa <main+0xa6a>
            Cur_Addr+=UVM_PGTBL_SIZE_NOM(RME_PGT_NUM_512);
    20000a52:	4c 89 ed             	mov    %r13,%rbp
            UVM_ASSERT(UVM_Pgtbl_Crt(UVM_CAPID(UVM_BOOT_CAPTBL,TEST_PROCESS_PGT), UVM_CAPID(UVM_BOOT_TBL_KMEM,0), RME_TEST_PDE(Count),
    20000a55:	6a 09                	push   $0x9
    20000a57:	4d 8d 74 24 11       	lea    0x11(%r12),%r14
    20000a5c:	4d 89 e0             	mov    %r12,%r8
    20000a5f:	48 89 e9             	mov    %rbp,%rcx
    20000a62:	6a 15                	push   $0x15
    20000a64:	be 00 80 05 00       	mov    $0x58000,%esi
    20000a69:	45 31 c9             	xor    %r9d,%r9d
    20000a6c:	49 c1 e0 1e          	shl    $0x1e,%r8
    20000a70:	4c 89 f2             	mov    %r14,%rdx
    20000a73:	bf 0e 80 00 00       	mov    $0x800e,%edi
    20000a78:	e8 93 15 00 00       	call   20002010 <UVM_Pgtbl_Crt>
    20000a7d:	59                   	pop    %rcx
    20000a7e:	5e                   	pop    %rsi
    20000a7f:	48 85 c0             	test   %rax,%rax
    20000a82:	0f 89 7a ff ff ff    	jns    20000a02 <main+0x472>
    20000a88:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000a8d:	e8 6e 13 00 00       	call   20001e00 <UVM_Print_String>
    20000a92:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000a97:	e8 64 13 00 00       	call   20001e00 <UVM_Print_String>
    20000a9c:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000aa1:	e8 5a 13 00 00       	call   20001e00 <UVM_Print_String>
    20000aa6:	bf 29 01 00 00       	mov    $0x129,%edi
    20000aab:	e8 20 11 00 00       	call   20001bd0 <UVM_Print_Int>
    20000ab0:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000ab5:	e8 46 13 00 00       	call   20001e00 <UVM_Print_String>
    20000aba:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000abf:	e8 3c 13 00 00       	call   20001e00 <UVM_Print_String>
    20000ac4:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000ac9:	e8 32 13 00 00       	call   20001e00 <UVM_Print_String>
    20000ace:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000ad3:	e8 28 13 00 00       	call   20001e00 <UVM_Print_String>
    20000ad8:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000add:	e8 1e 13 00 00       	call   20001e00 <UVM_Print_String>
    20000ae2:	eb fe                	jmp    20000ae2 <main+0x552>
        /*Idle*/
        UVM_LOG_S("\r\nIdle......");
        while (1);
    }
    return 0;
}
    20000ae4:	5a                   	pop    %rdx
    20000ae5:	31 c0                	xor    %eax,%eax
    20000ae7:	5b                   	pop    %rbx
    20000ae8:	5d                   	pop    %rbp
    20000ae9:	41 5c                	pop    %r12
    20000aeb:	41 5d                	pop    %r13
    20000aed:	41 5e                	pop    %r14
    20000aef:	41 5f                	pop    %r15
    20000af1:	c3                   	ret    
        UVM_ASSERT(UVM_Captbl_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_THD_TBL,Cur_Addr,16)>=0);
    20000af2:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000af7:	e8 04 13 00 00       	call   20001e00 <UVM_Print_String>
    20000afc:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000b01:	e8 fa 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b06:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000b0b:	e8 f0 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b10:	bf eb 00 00 00       	mov    $0xeb,%edi
    20000b15:	e8 b6 10 00 00       	call   20001bd0 <UVM_Print_Int>
    20000b1a:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000b1f:	e8 dc 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b24:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000b29:	e8 d2 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b2e:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000b33:	e8 c8 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b38:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000b3d:	e8 be 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b42:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000b47:	e8 b4 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b4c:	eb fe                	jmp    20000b4c <main+0x5bc>
        UVM_ASSERT(UVM_Thd_Crt(TEST_THD_TBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_THD1, UVM_BOOT_INIT_PROC, 10, Cur_Addr)>=0);
    20000b4e:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000b53:	e8 a8 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b58:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000b5d:	e8 9e 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b62:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000b67:	e8 94 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b6c:	bf ed 00 00 00       	mov    $0xed,%edi
    20000b71:	e8 5a 10 00 00       	call   20001bd0 <UVM_Print_Int>
    20000b76:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000b7b:	e8 80 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b80:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000b85:	e8 76 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b8a:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000b8f:	e8 6c 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b94:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000b99:	e8 62 12 00 00       	call   20001e00 <UVM_Print_String>
    20000b9e:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000ba3:	e8 58 12 00 00       	call   20001e00 <UVM_Print_String>
    20000ba8:	eb fe                	jmp    20000ba8 <main+0x618>
        UVM_ASSERT(UVM_Thd_Sched_Bind(UVM_CAPID(TEST_THD_TBL,TEST_THD1),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_CAPID_NULL,0,0)>=0);
    20000baa:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000baf:	e8 4c 12 00 00       	call   20001e00 <UVM_Print_String>
    20000bb4:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000bb9:	e8 42 12 00 00       	call   20001e00 <UVM_Print_String>
    20000bbe:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000bc3:	e8 38 12 00 00       	call   20001e00 <UVM_Print_String>
    20000bc8:	bf ee 00 00 00       	mov    $0xee,%edi
    20000bcd:	e8 fe 0f 00 00       	call   20001bd0 <UVM_Print_Int>
    20000bd2:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000bd7:	e8 24 12 00 00       	call   20001e00 <UVM_Print_String>
    20000bdc:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000be1:	e8 1a 12 00 00       	call   20001e00 <UVM_Print_String>
    20000be6:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000beb:	e8 10 12 00 00       	call   20001e00 <UVM_Print_String>
    20000bf0:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000bf5:	e8 06 12 00 00       	call   20001e00 <UVM_Print_String>
    20000bfa:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000bff:	e8 fc 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c04:	eb fe                	jmp    20000c04 <main+0x674>
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD1),(ptr_t)TEST_THD_FUNC1,12*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
    20000c06:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000c0b:	e8 f0 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c10:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000c15:	e8 e6 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c1a:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000c1f:	e8 dc 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c24:	bf f1 00 00 00       	mov    $0xf1,%edi
    20000c29:	e8 a2 0f 00 00       	call   20001bd0 <UVM_Print_Int>
    20000c2e:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000c33:	e8 c8 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c38:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000c3d:	e8 be 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c42:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000c47:	e8 b4 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c4c:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000c51:	e8 aa 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c56:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000c5b:	e8 a0 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c60:	eb fe                	jmp    20000c60 <main+0x6d0>
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD1),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
    20000c62:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000c67:	e8 94 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c6c:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000c71:	e8 8a 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c76:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000c7b:	e8 80 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c80:	bf ef 00 00 00       	mov    $0xef,%edi
    20000c85:	e8 46 0f 00 00       	call   20001bd0 <UVM_Print_Int>
    20000c8a:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000c8f:	e8 6c 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c94:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000c99:	e8 62 11 00 00       	call   20001e00 <UVM_Print_String>
    20000c9e:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000ca3:	e8 58 11 00 00       	call   20001e00 <UVM_Print_String>
    20000ca8:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000cad:	e8 4e 11 00 00       	call   20001e00 <UVM_Print_String>
    20000cb2:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000cb7:	e8 44 11 00 00       	call   20001e00 <UVM_Print_String>
    20000cbc:	eb fe                	jmp    20000cbc <main+0x72c>
        UVM_ASSERT(UVM_Thd_Crt(TEST_THD_TBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0), TEST_THD2, UVM_BOOT_INIT_PROC, 10, Cur_Addr)>=0);
    20000cbe:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000cc3:	e8 38 11 00 00       	call   20001e00 <UVM_Print_String>
    20000cc8:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000ccd:	e8 2e 11 00 00       	call   20001e00 <UVM_Print_String>
    20000cd2:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000cd7:	e8 24 11 00 00       	call   20001e00 <UVM_Print_String>
    20000cdc:	bf f3 00 00 00       	mov    $0xf3,%edi
    20000ce1:	e8 ea 0e 00 00       	call   20001bd0 <UVM_Print_Int>
    20000ce6:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000ceb:	e8 10 11 00 00       	call   20001e00 <UVM_Print_String>
    20000cf0:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000cf5:	e8 06 11 00 00       	call   20001e00 <UVM_Print_String>
    20000cfa:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000cff:	e8 fc 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d04:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000d09:	e8 f2 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d0e:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000d13:	e8 e8 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d18:	eb fe                	jmp    20000d18 <main+0x788>
        UVM_ASSERT(UVM_Thd_Sched_Bind(UVM_CAPID(TEST_THD_TBL,TEST_THD2),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_CAPID_NULL,0,0)>=0);
    20000d1a:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000d1f:	e8 dc 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d24:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000d29:	e8 d2 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d2e:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000d33:	e8 c8 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d38:	bf f4 00 00 00       	mov    $0xf4,%edi
    20000d3d:	e8 8e 0e 00 00       	call   20001bd0 <UVM_Print_Int>
    20000d42:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000d47:	e8 b4 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d4c:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000d51:	e8 aa 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d56:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000d5b:	e8 a0 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d60:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000d65:	e8 96 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d6a:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000d6f:	e8 8c 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d74:	eb fe                	jmp    20000d74 <main+0x7e4>
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD2),(ptr_t)TEST_THD_FUNC2,13*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,1)>=0);
    20000d76:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000d7b:	e8 80 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d80:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000d85:	e8 76 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d8a:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000d8f:	e8 6c 10 00 00       	call   20001e00 <UVM_Print_String>
    20000d94:	bf f7 00 00 00       	mov    $0xf7,%edi
    20000d99:	e8 32 0e 00 00       	call   20001bd0 <UVM_Print_Int>
    20000d9e:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000da3:	e8 58 10 00 00       	call   20001e00 <UVM_Print_String>
    20000da8:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000dad:	e8 4e 10 00 00       	call   20001e00 <UVM_Print_String>
    20000db2:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000db7:	e8 44 10 00 00       	call   20001e00 <UVM_Print_String>
    20000dbc:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000dc1:	e8 3a 10 00 00       	call   20001e00 <UVM_Print_String>
    20000dc6:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000dcb:	e8 30 10 00 00       	call   20001e00 <UVM_Print_String>
    20000dd0:	eb fe                	jmp    20000dd0 <main+0x840>
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD2),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
    20000dd2:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000dd7:	e8 24 10 00 00       	call   20001e00 <UVM_Print_String>
    20000ddc:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000de1:	e8 1a 10 00 00       	call   20001e00 <UVM_Print_String>
    20000de6:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000deb:	e8 10 10 00 00       	call   20001e00 <UVM_Print_String>
    20000df0:	bf f5 00 00 00       	mov    $0xf5,%edi
    20000df5:	e8 d6 0d 00 00       	call   20001bd0 <UVM_Print_Int>
    20000dfa:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000dff:	e8 fc 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e04:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000e09:	e8 f2 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e0e:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000e13:	e8 e8 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e18:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000e1d:	e8 de 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e22:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000e27:	e8 d4 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e2c:	eb fe                	jmp    20000e2c <main+0x89c>
        UVM_ASSERT(UVM_Sig_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_SIG1, Cur_Addr)>=0);
    20000e2e:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000e33:	e8 c8 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e38:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000e3d:	e8 be 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e42:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000e47:	e8 b4 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e4c:	bf 00 01 00 00       	mov    $0x100,%edi
    20000e51:	e8 7a 0d 00 00       	call   20001bd0 <UVM_Print_Int>
    20000e56:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000e5b:	e8 a0 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e60:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000e65:	e8 96 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e6a:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000e6f:	e8 8c 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e74:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000e79:	e8 82 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e7e:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000e83:	e8 78 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e88:	eb fe                	jmp    20000e88 <main+0x8f8>
        UVM_ASSERT(UVM_Thd_Sched_Prio(UVM_CAPID(TEST_THD_TBL,TEST_THD1),1)>=0);
    20000e8a:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000e8f:	e8 6c 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e94:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000e99:	e8 62 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000e9e:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000ea3:	e8 58 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000ea8:	bf 08 01 00 00       	mov    $0x108,%edi
    20000ead:	e8 1e 0d 00 00       	call   20001bd0 <UVM_Print_Int>
    20000eb2:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000eb7:	e8 44 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000ebc:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000ec1:	e8 3a 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000ec6:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000ecb:	e8 30 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000ed0:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000ed5:	e8 26 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000eda:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000edf:	e8 1c 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000ee4:	eb fe                	jmp    20000ee4 <main+0x954>
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD1),(ptr_t)TEST_SIG_FUNC1,12*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
    20000ee6:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000eeb:	e8 10 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000ef0:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000ef5:	e8 06 0f 00 00       	call   20001e00 <UVM_Print_String>
    20000efa:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000eff:	e8 fc 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f04:	bf 04 01 00 00       	mov    $0x104,%edi
    20000f09:	e8 c2 0c 00 00       	call   20001bd0 <UVM_Print_Int>
    20000f0e:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000f13:	e8 e8 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f18:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000f1d:	e8 de 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f22:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000f27:	e8 d4 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f2c:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000f31:	e8 ca 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f36:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000f3b:	e8 c0 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f40:	eb fe                	jmp    20000f40 <main+0x9b0>
            UVM_ASSERT(UVM_Pgtbl_Con(UVM_CAPID(TEST_PROCESS_PGT,TEST_PROCESS_PML4),Count,
    20000f42:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000f47:	e8 b4 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f4c:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000f51:	e8 aa 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f56:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000f5b:	e8 a0 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f60:	bf 22 01 00 00       	mov    $0x122,%edi
    20000f65:	e8 66 0c 00 00       	call   20001bd0 <UVM_Print_Int>
    20000f6a:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000f6f:	e8 8c 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f74:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000f79:	e8 82 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f7e:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000f83:	e8 78 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f88:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000f8d:	e8 6e 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f92:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000f97:	e8 64 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000f9c:	eb fe                	jmp    20000f9c <main+0xa0c>
            UVM_ASSERT(UVM_Pgtbl_Crt(UVM_CAPID(UVM_BOOT_CAPTBL,TEST_PROCESS_PGT), UVM_CAPID(UVM_BOOT_TBL_KMEM,0), RME_TEST_PDP(Count),
    20000f9e:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20000fa3:	e8 58 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000fa8:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20000fad:	e8 4e 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000fb2:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20000fb7:	e8 44 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000fbc:	bf 1f 01 00 00       	mov    $0x11f,%edi
    20000fc1:	e8 0a 0c 00 00       	call   20001bd0 <UVM_Print_Int>
    20000fc6:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000fcb:	e8 30 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000fd0:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20000fd5:	e8 26 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000fda:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20000fdf:	e8 1c 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000fe4:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20000fe9:	e8 12 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000fee:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20000ff3:	e8 08 0e 00 00       	call   20001e00 <UVM_Print_String>
    20000ff8:	eb fe                	jmp    20000ff8 <main+0xa68>
        UVM_LOG_S("\r\nCreate PDEs success!");
    20000ffa:	bf b2 27 00 20       	mov    $0x200027b2,%edi
    20000fff:	41 bc 11 00 00 00    	mov    $0x11,%r12d
    20001005:	e8 f6 0d 00 00       	call   20001e00 <UVM_Print_String>
                UVM_ASSERT(UVM_Pgtbl_Add(UVM_CAPID(TEST_PROCESS_PGT,RME_TEST_PDE(Count1)),Count2,(RME_PGT_READ|RME_PGT_WRITE|RME_PGT_EXECUTE|RME_PGT_CACHE|RME_PGT_BUFFER),
    2000100a:	4d 89 e6             	mov    %r12,%r14
            for (Count2=0;Count2<512;Count2++)
    2000100d:	45 31 ff             	xor    %r15d,%r15d
                UVM_ASSERT(UVM_Pgtbl_Add(UVM_CAPID(TEST_PROCESS_PGT,RME_TEST_PDE(Count1)),Count2,(RME_PGT_READ|RME_PGT_WRITE|RME_PGT_EXECUTE|RME_PGT_CACHE|RME_PGT_BUFFER),
    20001010:	49 81 ce 00 80 01 00 	or     $0x18000,%r14
    20001017:	eb 11                	jmp    2000102a <main+0xa9a>
            for (Count2=0;Count2<512;Count2++)
    20001019:	49 83 c7 01          	add    $0x1,%r15
    2000101d:	49 81 ff 00 02 00 00 	cmp    $0x200,%r15
    20001024:	0f 84 dd 00 00 00    	je     20001107 <main+0xb77>
                UVM_ASSERT(UVM_Pgtbl_Add(UVM_CAPID(TEST_PROCESS_PGT,RME_TEST_PDE(Count1)),Count2,(RME_PGT_READ|RME_PGT_WRITE|RME_PGT_EXECUTE|RME_PGT_CACHE|RME_PGT_BUFFER),
    2000102a:	4c 89 e7             	mov    %r12,%rdi
    2000102d:	45 31 c9             	xor    %r9d,%r9d
    20001030:	4d 89 f8             	mov    %r15,%r8
    20001033:	4c 89 f1             	mov    %r14,%rcx
    20001036:	48 81 cf 00 80 0e 00 	or     $0xe8000,%rdi
    2000103d:	ba 1f 00 00 00       	mov    $0x1f,%edx
    20001042:	4c 89 fe             	mov    %r15,%rsi
    20001045:	e8 36 10 00 00       	call   20002080 <UVM_Pgtbl_Add>
    2000104a:	48 85 c0             	test   %rax,%rax
    2000104d:	79 ca                	jns    20001019 <main+0xa89>
    2000104f:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20001054:	e8 a7 0d 00 00       	call   20001e00 <UVM_Print_String>
    20001059:	bf 40 27 00 20       	mov    $0x20002740,%edi
    2000105e:	e8 9d 0d 00 00       	call   20001e00 <UVM_Print_String>
    20001063:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20001068:	e8 93 0d 00 00       	call   20001e00 <UVM_Print_String>
    2000106d:	bf 35 01 00 00       	mov    $0x135,%edi
    20001072:	e8 59 0b 00 00       	call   20001bd0 <UVM_Print_Int>
    20001077:	bf 54 27 00 20       	mov    $0x20002754,%edi
    2000107c:	e8 7f 0d 00 00       	call   20001e00 <UVM_Print_String>
    20001081:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20001086:	e8 75 0d 00 00       	call   20001e00 <UVM_Print_String>
    2000108b:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20001090:	e8 6b 0d 00 00       	call   20001e00 <UVM_Print_String>
    20001095:	bf 66 27 00 20       	mov    $0x20002766,%edi
    2000109a:	e8 61 0d 00 00       	call   20001e00 <UVM_Print_String>
    2000109f:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200010a4:	e8 57 0d 00 00       	call   20001e00 <UVM_Print_String>
    200010a9:	eb fe                	jmp    200010a9 <main+0xb19>
            UVM_ASSERT(UVM_Pgtbl_Con(UVM_CAPID(TEST_PROCESS_PGT,RME_TEST_PDP(Count>>9)),Count&0x1FF,
    200010ab:	bf 18 25 00 20       	mov    $0x20002518,%edi
    200010b0:	e8 4b 0d 00 00       	call   20001e00 <UVM_Print_String>
    200010b5:	bf 40 27 00 20       	mov    $0x20002740,%edi
    200010ba:	e8 41 0d 00 00       	call   20001e00 <UVM_Print_String>
    200010bf:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    200010c4:	e8 37 0d 00 00       	call   20001e00 <UVM_Print_String>
    200010c9:	bf 2c 01 00 00       	mov    $0x12c,%edi
    200010ce:	e8 fd 0a 00 00       	call   20001bd0 <UVM_Print_Int>
    200010d3:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200010d8:	e8 23 0d 00 00       	call   20001e00 <UVM_Print_String>
    200010dd:	bf 57 27 00 20       	mov    $0x20002757,%edi
    200010e2:	e8 19 0d 00 00       	call   20001e00 <UVM_Print_String>
    200010e7:	bf 63 27 00 20       	mov    $0x20002763,%edi
    200010ec:	e8 0f 0d 00 00       	call   20001e00 <UVM_Print_String>
    200010f1:	bf 66 27 00 20       	mov    $0x20002766,%edi
    200010f6:	e8 05 0d 00 00       	call   20001e00 <UVM_Print_String>
    200010fb:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001100:	e8 fb 0c 00 00       	call   20001e00 <UVM_Print_String>
    20001105:	eb fe                	jmp    20001105 <main+0xb75>
        for (Count1=0;Count1<4;Count1++)
    20001107:	49 83 c4 01          	add    $0x1,%r12
    2000110b:	49 83 fc 15          	cmp    $0x15,%r12
    2000110f:	0f 85 f5 fe ff ff    	jne    2000100a <main+0xa7a>
        UVM_LOG_S("\r\nAdd pages success!");
    20001115:	bf c9 27 00 20       	mov    $0x200027c9,%edi
    2000111a:	e8 e1 0c 00 00       	call   20001e00 <UVM_Print_String>
        UVM_ASSERT(UVM_Proc_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_PROCESS,UVM_BOOT_CAPTBL,
    2000111f:	31 c9                	xor    %ecx,%ecx
    20001121:	31 ff                	xor    %edi,%edi
    20001123:	4d 89 e9             	mov    %r13,%r9
    20001126:	41 b8 00 80 0e 00    	mov    $0xe8000,%r8d
    2000112c:	ba 0f 00 00 00       	mov    $0xf,%edx
    20001131:	be 00 80 05 00       	mov    $0x58000,%esi
    20001136:	e8 f5 0f 00 00       	call   20002130 <UVM_Proc_Crt>
    2000113b:	48 85 c0             	test   %rax,%rax
    2000113e:	0f 88 76 08 00 00    	js     200019ba <main+0x142a>
        UVM_ASSERT(UVM_Thd_Crt(TEST_THD_TBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_THD3, TEST_PROCESS, 10, Cur_Addr)>=0);
    20001144:	41 b8 0a 00 00 00    	mov    $0xa,%r8d
    2000114a:	b9 0f 00 00 00       	mov    $0xf,%ecx
    2000114f:	ba 02 00 00 00       	mov    $0x2,%edx
    20001154:	be 00 80 05 00       	mov    $0x58000,%esi
    20001159:	4c 8d 8d 20 80 00 00 	lea    0x8020(%rbp),%r9
    20001160:	bf 09 00 00 00       	mov    $0x9,%edi
    20001165:	e8 46 10 00 00       	call   200021b0 <UVM_Thd_Crt>
    2000116a:	48 85 c0             	test   %rax,%rax
    2000116d:	0f 88 eb 07 00 00    	js     2000195e <main+0x13ce>
        UVM_ASSERT(UVM_Thd_Sched_Bind(UVM_CAPID(TEST_THD_TBL,TEST_THD3),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_CAPID_NULL,0,0)>=0);
    20001173:	45 31 c0             	xor    %r8d,%r8d
    20001176:	31 c9                	xor    %ecx,%ecx
    20001178:	ba 00 00 00 80       	mov    $0x80000000,%edx
    2000117d:	be 00 80 03 00       	mov    $0x38000,%esi
    20001182:	bf 02 80 09 00       	mov    $0x98002,%edi
    20001187:	e8 c4 10 00 00       	call   20002250 <UVM_Thd_Sched_Bind>
    2000118c:	48 85 c0             	test   %rax,%rax
    2000118f:	0f 88 6d 07 00 00    	js     20001902 <main+0x1372>
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD3),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
    20001195:	be 00 80 03 00       	mov    $0x38000,%esi
    2000119a:	bf 02 80 09 00       	mov    $0x98002,%edi
    2000119f:	48 ba fe ff ff ff ff 	movabs $0x7ffffffffffffffe,%rdx
    200011a6:	ff ff 7f 
    Word_Inc=(ptr_t*)Addr;
    200011a9:	41 bc 00 00 e0 00    	mov    $0xe00000,%r12d
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD3),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
    200011af:	e8 2c 11 00 00       	call   200022e0 <UVM_Thd_Time_Xfer>
    200011b4:	48 85 c0             	test   %rax,%rax
    200011b7:	0f 88 e9 06 00 00    	js     200018a6 <main+0x1316>
        *Word_Inc=0;
    200011bd:	49 c7 04 24 00 00 00 	movq   $0x0,(%r12)
    200011c4:	00 
        Word_Inc++;
    200011c5:	49 83 c4 08          	add    $0x8,%r12
    for(Words=Size/sizeof(ptr_t);Words>0;Words--)
    200011c9:	49 81 fc 00 00 f0 00 	cmp    $0xf00000,%r12
    200011d0:	75 eb                	jne    200011bd <main+0xc2d>
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD3),(ptr_t)TEST_THD_FUNC3,14*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
    200011d2:	31 c9                	xor    %ecx,%ecx
    200011d4:	ba 00 00 e0 20       	mov    $0x20e00000,%edx
    200011d9:	be f0 01 00 20       	mov    $0x200001f0,%esi
    200011de:	bf 02 80 09 00       	mov    $0x98002,%edi
    200011e3:	e8 28 10 00 00       	call   20002210 <UVM_Thd_Exec_Set>
    200011e8:	48 85 c0             	test   %rax,%rax
    200011eb:	0f 88 79 03 00 00    	js     2000156a <main+0xfda>
        UVM_ASSERT(UVM_Thd_Crt(TEST_THD_TBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_THD4, UVM_BOOT_INIT_PROC, 10, Cur_Addr)>=0);
    200011f1:	41 b8 0a 00 00 00    	mov    $0xa,%r8d
    200011f7:	b9 02 00 00 00       	mov    $0x2,%ecx
    200011fc:	ba 03 00 00 00       	mov    $0x3,%edx
    20001201:	be 00 80 05 00       	mov    $0x58000,%esi
    20001206:	4c 8d 8d f0 89 00 00 	lea    0x89f0(%rbp),%r9
    2000120d:	bf 09 00 00 00       	mov    $0x9,%edi
    20001212:	e8 99 0f 00 00       	call   200021b0 <UVM_Thd_Crt>
    20001217:	48 85 c0             	test   %rax,%rax
    2000121a:	0f 88 ee 02 00 00    	js     2000150e <main+0xf7e>
        UVM_ASSERT(UVM_Thd_Sched_Bind(UVM_CAPID(TEST_THD_TBL,TEST_THD4),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_CAPID_NULL,0,0)>=0);
    20001220:	45 31 c0             	xor    %r8d,%r8d
    20001223:	31 c9                	xor    %ecx,%ecx
    20001225:	ba 00 00 00 80       	mov    $0x80000000,%edx
    2000122a:	be 00 80 03 00       	mov    $0x38000,%esi
    2000122f:	bf 03 80 09 00       	mov    $0x98003,%edi
    20001234:	e8 17 10 00 00       	call   20002250 <UVM_Thd_Sched_Bind>
    20001239:	48 85 c0             	test   %rax,%rax
    2000123c:	0f 88 84 03 00 00    	js     200015c6 <main+0x1036>
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD4),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
    20001242:	48 ba fe ff ff ff ff 	movabs $0x7ffffffffffffffe,%rdx
    20001249:	ff ff 7f 
    2000124c:	be 00 80 03 00       	mov    $0x38000,%esi
    20001251:	bf 03 80 09 00       	mov    $0x98003,%edi
    20001256:	e8 85 10 00 00       	call   200022e0 <UVM_Thd_Time_Xfer>
    2000125b:	48 85 c0             	test   %rax,%rax
    2000125e:	0f 88 4e 02 00 00    	js     200014b2 <main+0xf22>
        *Word_Inc=0;
    20001264:	49 c7 04 24 00 00 00 	movq   $0x0,(%r12)
    2000126b:	00 
        Word_Inc++;
    2000126c:	49 83 c4 08          	add    $0x8,%r12
    for(Words=Size/sizeof(ptr_t);Words>0;Words--)
    20001270:	49 81 fc 00 00 00 01 	cmp    $0x1000000,%r12
    20001277:	75 eb                	jne    20001264 <main+0xcd4>
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD4),(ptr_t)TEST_THD_FUNC4,15*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
    20001279:	31 c9                	xor    %ecx,%ecx
    2000127b:	ba 00 00 f0 20       	mov    $0x20f00000,%edx
    20001280:	be 30 01 00 20       	mov    $0x20000130,%esi
    20001285:	bf 03 80 09 00       	mov    $0x98003,%edi
    2000128a:	e8 81 0f 00 00       	call   20002210 <UVM_Thd_Exec_Set>
    2000128f:	48 85 c0             	test   %rax,%rax
    20001292:	0f 88 42 04 00 00    	js     200016da <main+0x114a>
        UVM_LOG_S("\r\nCross-process swtching thread...");
    20001298:	bf a0 26 00 20       	mov    $0x200026a0,%edi
    2000129d:	e8 5e 0b 00 00       	call   20001e00 <UVM_Print_String>
        UVM_Thd_Swt(UVM_CAPID(TEST_THD_TBL,TEST_THD3),0);
    200012a2:	31 f6                	xor    %esi,%esi
    200012a4:	bf 02 80 09 00       	mov    $0x98002,%edi
    200012a9:	e8 52 10 00 00       	call   20002300 <UVM_Thd_Swt>
        UVM_LOG_S("\r\nBack to main thread!");
    200012ae:	bf 84 27 00 20       	mov    $0x20002784,%edi
    200012b3:	e8 48 0b 00 00       	call   20001e00 <UVM_Print_String>
        UVM_ASSERT(UVM_Sig_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_SIG2, Cur_Addr)>=0);
    200012b8:	31 ff                	xor    %edi,%edi
    200012ba:	ba 0c 00 00 00       	mov    $0xc,%edx
        Cur_Addr+=UVM_THD_SIZE;
    200012bf:	48 8d 8d c0 93 00 00 	lea    0x93c0(%rbp),%rcx
        UVM_ASSERT(UVM_Sig_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_SIG2, Cur_Addr)>=0);
    200012c6:	be 00 80 05 00       	mov    $0x58000,%esi
    200012cb:	e8 50 10 00 00       	call   20002320 <UVM_Sig_Crt>
    200012d0:	48 85 c0             	test   %rax,%rax
    200012d3:	0f 88 a5 03 00 00    	js     2000167e <main+0x10ee>
        *Word_Inc=0;
    200012d9:	48 c7 03 00 00 00 00 	movq   $0x0,(%rbx)
        Word_Inc++;
    200012e0:	48 83 c3 08          	add    $0x8,%rbx
    for(Words=Size/sizeof(ptr_t);Words>0;Words--)
    200012e4:	48 81 fb 00 00 f0 00 	cmp    $0xf00000,%rbx
    200012eb:	75 ec                	jne    200012d9 <main+0xd49>
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD3),(ptr_t)TEST_SIG_FUNC2,14*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
    200012ed:	31 c9                	xor    %ecx,%ecx
    200012ef:	ba 00 00 e0 20       	mov    $0x20e00000,%edx
    200012f4:	be 00 04 00 20       	mov    $0x20000400,%esi
    200012f9:	bf 02 80 09 00       	mov    $0x98002,%edi
    200012fe:	e8 0d 0f 00 00       	call   20002210 <UVM_Thd_Exec_Set>
    20001303:	48 85 c0             	test   %rax,%rax
    20001306:	0f 88 2a 04 00 00    	js     20001736 <main+0x11a6>
        UVM_ASSERT(UVM_Thd_Sched_Prio(UVM_CAPID(TEST_THD_TBL,TEST_THD3),1)>=0);
    2000130c:	be 01 00 00 00       	mov    $0x1,%esi
    20001311:	bf 02 80 09 00       	mov    $0x98002,%edi
    20001316:	41 bc 10 27 00 00    	mov    $0x2710,%r12d
    2000131c:	e8 7f 0f 00 00       	call   200022a0 <UVM_Thd_Sched_Prio>
    20001321:	48 85 c0             	test   %rax,%rax
    20001324:	0f 88 f8 02 00 00    	js     20001622 <main+0x1092>
            UVM_Sig_Snd(TEST_SIG2,1);
    2000132a:	be 01 00 00 00       	mov    $0x1,%esi
    2000132f:	bf 0c 00 00 00       	mov    $0xc,%edi
    20001334:	e8 27 10 00 00       	call   20002360 <UVM_Sig_Snd>
        for(Count=0;Count<10000;Count++)
    20001339:	49 83 ec 01          	sub    $0x1,%r12
    2000133d:	75 eb                	jne    2000132a <main+0xd9a>
        UVM_ASSERT(UVM_Thd_Sched_Prio(UVM_CAPID(TEST_THD_TBL,TEST_THD3),0)>=0);
    2000133f:	31 f6                	xor    %esi,%esi
    20001341:	bf 02 80 09 00       	mov    $0x98002,%edi
    20001346:	e8 55 0f 00 00       	call   200022a0 <UVM_Thd_Sched_Prio>
    2000134b:	48 85 c0             	test   %rax,%rax
    2000134e:	0f 88 f6 04 00 00    	js     2000184a <main+0x12ba>
        UVM_ASSERT(UVM_Inv_Crt(UVM_BOOT_CAPTBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0), TEST_INV1, TEST_PROCESS, Cur_Addr)>=0);
    20001354:	31 ff                	xor    %edi,%edi
    20001356:	b9 0f 00 00 00       	mov    $0xf,%ecx
    2000135b:	ba 0a 00 00 00       	mov    $0xa,%edx
    20001360:	be 00 80 05 00       	mov    $0x58000,%esi
    20001365:	4c 8d 85 e0 93 00 00 	lea    0x93e0(%rbp),%r8
    2000136c:	e8 2f 10 00 00       	call   200023a0 <UVM_Inv_Crt>
    20001371:	48 85 c0             	test   %rax,%rax
    20001374:	0f 88 74 04 00 00    	js     200017ee <main+0x125e>
        *Word_Inc=0;
    2000137a:	48 c7 03 00 00 00 00 	movq   $0x0,(%rbx)
        Word_Inc++;
    20001381:	48 83 c3 08          	add    $0x8,%rbx
    for(Words=Size/sizeof(ptr_t);Words>0;Words--)
    20001385:	48 81 fb 00 00 00 01 	cmp    $0x1000000,%rbx
    2000138c:	75 ec                	jne    2000137a <main+0xdea>
        UVM_ASSERT(UVM_Inv_Set(TEST_INV1,(ptr_t)TEST_INV1_FUNC,15*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
    2000138e:	31 c9                	xor    %ecx,%ecx
    20001390:	ba 00 00 f0 20       	mov    $0x20f00000,%edx
    20001395:	be 70 05 00 20       	mov    $0x20000570,%esi
    2000139a:	bf 0a 00 00 00       	mov    $0xa,%edi
    2000139f:	e8 4c 10 00 00       	call   200023f0 <UVM_Inv_Set>
    200013a4:	48 85 c0             	test   %rax,%rax
    200013a7:	0f 88 e5 03 00 00    	js     20001792 <main+0x1202>
        sum=0;
    200013ad:	48 c7 05 58 1c 00 00 	movq   $0x0,0x1c58(%rip)        # 20003010 <sum>
    200013b4:	00 00 00 00 
        sumout=0;
    200013b8:	bb 10 27 00 00       	mov    $0x2710,%ebx
        sumin=0;
    200013bd:	48 c7 05 40 1c 00 00 	movq   $0x0,0x1c40(%rip)        # 20003008 <sumin>
    200013c4:	00 00 00 00 
        sumout=0;
    200013c8:	48 c7 05 2d 1c 00 00 	movq   $0x0,0x1c2d(%rip)        # 20003000 <sumout>
    200013cf:	00 00 00 00 
            start=__UVM_X64_Read_TSC();
    200013d3:	e8 43 ec ff ff       	call   2000001b <__UVM_X64_Read_TSC>
            UVM_Inv_Act(TEST_INV1,0,0);
    200013d8:	31 d2                	xor    %edx,%edx
    200013da:	31 f6                	xor    %esi,%esi
    200013dc:	bf 0a 00 00 00       	mov    $0xa,%edi
            start=__UVM_X64_Read_TSC();
    200013e1:	48 89 05 40 1c 00 00 	mov    %rax,0x1c40(%rip)        # 20003028 <start>
            UVM_Inv_Act(TEST_INV1,0,0);
    200013e8:	e8 6c ec ff ff       	call   20000059 <UVM_Inv_Act>
            end=__UVM_X64_Read_TSC();
    200013ed:	e8 29 ec ff ff       	call   2000001b <__UVM_X64_Read_TSC>
    200013f2:	48 89 05 1f 1c 00 00 	mov    %rax,0x1c1f(%rip)        # 20003018 <end>
            sum+=end-start;
    200013f9:	48 8b 05 18 1c 00 00 	mov    0x1c18(%rip),%rax        # 20003018 <end>
    20001400:	48 8b 15 21 1c 00 00 	mov    0x1c21(%rip),%rdx        # 20003028 <start>
    20001407:	48 29 d0             	sub    %rdx,%rax
    2000140a:	48 01 05 ff 1b 00 00 	add    %rax,0x1bff(%rip)        # 20003010 <sum>
            sumin+=middle-start;
    20001411:	48 8b 05 08 1c 00 00 	mov    0x1c08(%rip),%rax        # 20003020 <middle>
    20001418:	48 8b 15 09 1c 00 00 	mov    0x1c09(%rip),%rdx        # 20003028 <start>
    2000141f:	48 29 d0             	sub    %rdx,%rax
    20001422:	48 01 05 df 1b 00 00 	add    %rax,0x1bdf(%rip)        # 20003008 <sumin>
            sumout+=end-middle;
    20001429:	48 8b 05 e8 1b 00 00 	mov    0x1be8(%rip),%rax        # 20003018 <end>
    20001430:	48 8b 15 e9 1b 00 00 	mov    0x1be9(%rip),%rdx        # 20003020 <middle>
    20001437:	48 29 d0             	sub    %rdx,%rax
    2000143a:	48 01 05 bf 1b 00 00 	add    %rax,0x1bbf(%rip)        # 20003000 <sumout>
        for(Count=0;Count<10000;Count++)
    20001441:	48 83 eb 01          	sub    $0x1,%rbx
    20001445:	75 8c                	jne    200013d3 <main+0xe43>
        UVM_LOG_S("\r\nInvocation total takes clock cycles:");
    20001447:	bf c8 26 00 20       	mov    $0x200026c8,%edi
        UVM_LOG_I(sum/10000);
    2000144c:	bb 10 27 00 00       	mov    $0x2710,%ebx
        UVM_LOG_S("\r\nInvocation total takes clock cycles:");
    20001451:	e8 aa 09 00 00       	call   20001e00 <UVM_Print_String>
        UVM_LOG_I(sum/10000);
    20001456:	48 8b 05 b3 1b 00 00 	mov    0x1bb3(%rip),%rax        # 20003010 <sum>
    2000145d:	31 d2                	xor    %edx,%edx
    2000145f:	48 f7 f3             	div    %rbx
    20001462:	48 89 c7             	mov    %rax,%rdi
    20001465:	e8 66 07 00 00       	call   20001bd0 <UVM_Print_Int>
        UVM_LOG_S("\r\nInvocation entry takes clock cycles:");
    2000146a:	bf f0 26 00 20       	mov    $0x200026f0,%edi
    2000146f:	e8 8c 09 00 00       	call   20001e00 <UVM_Print_String>
        UVM_LOG_I(sumin/10000);
    20001474:	48 8b 05 8d 1b 00 00 	mov    0x1b8d(%rip),%rax        # 20003008 <sumin>
    2000147b:	31 d2                	xor    %edx,%edx
    2000147d:	48 f7 f3             	div    %rbx
    20001480:	48 89 c7             	mov    %rax,%rdi
    20001483:	e8 48 07 00 00       	call   20001bd0 <UVM_Print_Int>
        UVM_LOG_S("\r\nInvocation return takes clock cycles:");
    20001488:	bf 18 27 00 20       	mov    $0x20002718,%edi
    2000148d:	e8 6e 09 00 00       	call   20001e00 <UVM_Print_String>
        UVM_LOG_I(sumout/10000);
    20001492:	48 8b 05 67 1b 00 00 	mov    0x1b67(%rip),%rax        # 20003000 <sumout>
    20001499:	31 d2                	xor    %edx,%edx
    2000149b:	48 f7 f3             	div    %rbx
    2000149e:	48 89 c7             	mov    %rax,%rdi
    200014a1:	e8 2a 07 00 00       	call   20001bd0 <UVM_Print_Int>
        UVM_LOG_S("\r\nIdle......");
    200014a6:	bf de 27 00 20       	mov    $0x200027de,%edi
    200014ab:	e8 50 09 00 00       	call   20001e00 <UVM_Print_String>
        while (1);
    200014b0:	eb fe                	jmp    200014b0 <main+0xf20>
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD4),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
    200014b2:	bf 18 25 00 20       	mov    $0x20002518,%edi
    200014b7:	e8 44 09 00 00       	call   20001e00 <UVM_Print_String>
    200014bc:	bf 40 27 00 20       	mov    $0x20002740,%edi
    200014c1:	e8 3a 09 00 00       	call   20001e00 <UVM_Print_String>
    200014c6:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    200014cb:	e8 30 09 00 00       	call   20001e00 <UVM_Print_String>
    200014d0:	bf 48 01 00 00       	mov    $0x148,%edi
    200014d5:	e8 f6 06 00 00       	call   20001bd0 <UVM_Print_Int>
    200014da:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200014df:	e8 1c 09 00 00       	call   20001e00 <UVM_Print_String>
    200014e4:	bf 57 27 00 20       	mov    $0x20002757,%edi
    200014e9:	e8 12 09 00 00       	call   20001e00 <UVM_Print_String>
    200014ee:	bf 63 27 00 20       	mov    $0x20002763,%edi
    200014f3:	e8 08 09 00 00       	call   20001e00 <UVM_Print_String>
    200014f8:	bf 66 27 00 20       	mov    $0x20002766,%edi
    200014fd:	e8 fe 08 00 00       	call   20001e00 <UVM_Print_String>
    20001502:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001507:	e8 f4 08 00 00       	call   20001e00 <UVM_Print_String>
    2000150c:	eb fe                	jmp    2000150c <main+0xf7c>
        UVM_ASSERT(UVM_Thd_Crt(TEST_THD_TBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_THD4, UVM_BOOT_INIT_PROC, 10, Cur_Addr)>=0);
    2000150e:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20001513:	e8 e8 08 00 00       	call   20001e00 <UVM_Print_String>
    20001518:	bf 40 27 00 20       	mov    $0x20002740,%edi
    2000151d:	e8 de 08 00 00       	call   20001e00 <UVM_Print_String>
    20001522:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20001527:	e8 d4 08 00 00       	call   20001e00 <UVM_Print_String>
    2000152c:	bf 46 01 00 00       	mov    $0x146,%edi
    20001531:	e8 9a 06 00 00       	call   20001bd0 <UVM_Print_Int>
    20001536:	bf 54 27 00 20       	mov    $0x20002754,%edi
    2000153b:	e8 c0 08 00 00       	call   20001e00 <UVM_Print_String>
    20001540:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20001545:	e8 b6 08 00 00       	call   20001e00 <UVM_Print_String>
    2000154a:	bf 63 27 00 20       	mov    $0x20002763,%edi
    2000154f:	e8 ac 08 00 00       	call   20001e00 <UVM_Print_String>
    20001554:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20001559:	e8 a2 08 00 00       	call   20001e00 <UVM_Print_String>
    2000155e:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001563:	e8 98 08 00 00       	call   20001e00 <UVM_Print_String>
    20001568:	eb fe                	jmp    20001568 <main+0xfd8>
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD3),(ptr_t)TEST_THD_FUNC3,14*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
    2000156a:	bf 18 25 00 20       	mov    $0x20002518,%edi
    2000156f:	e8 8c 08 00 00       	call   20001e00 <UVM_Print_String>
    20001574:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20001579:	e8 82 08 00 00       	call   20001e00 <UVM_Print_String>
    2000157e:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20001583:	e8 78 08 00 00       	call   20001e00 <UVM_Print_String>
    20001588:	bf 43 01 00 00       	mov    $0x143,%edi
    2000158d:	e8 3e 06 00 00       	call   20001bd0 <UVM_Print_Int>
    20001592:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001597:	e8 64 08 00 00       	call   20001e00 <UVM_Print_String>
    2000159c:	bf 57 27 00 20       	mov    $0x20002757,%edi
    200015a1:	e8 5a 08 00 00       	call   20001e00 <UVM_Print_String>
    200015a6:	bf 63 27 00 20       	mov    $0x20002763,%edi
    200015ab:	e8 50 08 00 00       	call   20001e00 <UVM_Print_String>
    200015b0:	bf 66 27 00 20       	mov    $0x20002766,%edi
    200015b5:	e8 46 08 00 00       	call   20001e00 <UVM_Print_String>
    200015ba:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200015bf:	e8 3c 08 00 00       	call   20001e00 <UVM_Print_String>
    200015c4:	eb fe                	jmp    200015c4 <main+0x1034>
        UVM_ASSERT(UVM_Thd_Sched_Bind(UVM_CAPID(TEST_THD_TBL,TEST_THD4),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_CAPID_NULL,0,0)>=0);
    200015c6:	bf 18 25 00 20       	mov    $0x20002518,%edi
    200015cb:	e8 30 08 00 00       	call   20001e00 <UVM_Print_String>
    200015d0:	bf 40 27 00 20       	mov    $0x20002740,%edi
    200015d5:	e8 26 08 00 00       	call   20001e00 <UVM_Print_String>
    200015da:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    200015df:	e8 1c 08 00 00       	call   20001e00 <UVM_Print_String>
    200015e4:	bf 47 01 00 00       	mov    $0x147,%edi
    200015e9:	e8 e2 05 00 00       	call   20001bd0 <UVM_Print_Int>
    200015ee:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200015f3:	e8 08 08 00 00       	call   20001e00 <UVM_Print_String>
    200015f8:	bf 57 27 00 20       	mov    $0x20002757,%edi
    200015fd:	e8 fe 07 00 00       	call   20001e00 <UVM_Print_String>
    20001602:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20001607:	e8 f4 07 00 00       	call   20001e00 <UVM_Print_String>
    2000160c:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20001611:	e8 ea 07 00 00       	call   20001e00 <UVM_Print_String>
    20001616:	bf 54 27 00 20       	mov    $0x20002754,%edi
    2000161b:	e8 e0 07 00 00       	call   20001e00 <UVM_Print_String>
    20001620:	eb fe                	jmp    20001620 <main+0x1090>
        UVM_ASSERT(UVM_Thd_Sched_Prio(UVM_CAPID(TEST_THD_TBL,TEST_THD3),1)>=0);
    20001622:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20001627:	e8 d4 07 00 00       	call   20001e00 <UVM_Print_String>
    2000162c:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20001631:	e8 ca 07 00 00       	call   20001e00 <UVM_Print_String>
    20001636:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    2000163b:	e8 c0 07 00 00       	call   20001e00 <UVM_Print_String>
    20001640:	bf 5a 01 00 00       	mov    $0x15a,%edi
    20001645:	e8 86 05 00 00       	call   20001bd0 <UVM_Print_Int>
    2000164a:	bf 54 27 00 20       	mov    $0x20002754,%edi
    2000164f:	e8 ac 07 00 00       	call   20001e00 <UVM_Print_String>
    20001654:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20001659:	e8 a2 07 00 00       	call   20001e00 <UVM_Print_String>
    2000165e:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20001663:	e8 98 07 00 00       	call   20001e00 <UVM_Print_String>
    20001668:	bf 66 27 00 20       	mov    $0x20002766,%edi
    2000166d:	e8 8e 07 00 00       	call   20001e00 <UVM_Print_String>
    20001672:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001677:	e8 84 07 00 00       	call   20001e00 <UVM_Print_String>
    2000167c:	eb fe                	jmp    2000167c <main+0x10ec>
        UVM_ASSERT(UVM_Sig_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_SIG2, Cur_Addr)>=0);
    2000167e:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20001683:	e8 78 07 00 00       	call   20001e00 <UVM_Print_String>
    20001688:	bf 40 27 00 20       	mov    $0x20002740,%edi
    2000168d:	e8 6e 07 00 00       	call   20001e00 <UVM_Print_String>
    20001692:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20001697:	e8 64 07 00 00       	call   20001e00 <UVM_Print_String>
    2000169c:	bf 55 01 00 00       	mov    $0x155,%edi
    200016a1:	e8 2a 05 00 00       	call   20001bd0 <UVM_Print_Int>
    200016a6:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200016ab:	e8 50 07 00 00       	call   20001e00 <UVM_Print_String>
    200016b0:	bf 57 27 00 20       	mov    $0x20002757,%edi
    200016b5:	e8 46 07 00 00       	call   20001e00 <UVM_Print_String>
    200016ba:	bf 63 27 00 20       	mov    $0x20002763,%edi
    200016bf:	e8 3c 07 00 00       	call   20001e00 <UVM_Print_String>
    200016c4:	bf 66 27 00 20       	mov    $0x20002766,%edi
    200016c9:	e8 32 07 00 00       	call   20001e00 <UVM_Print_String>
    200016ce:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200016d3:	e8 28 07 00 00       	call   20001e00 <UVM_Print_String>
    200016d8:	eb fe                	jmp    200016d8 <main+0x1148>
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD4),(ptr_t)TEST_THD_FUNC4,15*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
    200016da:	bf 18 25 00 20       	mov    $0x20002518,%edi
    200016df:	e8 1c 07 00 00       	call   20001e00 <UVM_Print_String>
    200016e4:	bf 40 27 00 20       	mov    $0x20002740,%edi
    200016e9:	e8 12 07 00 00       	call   20001e00 <UVM_Print_String>
    200016ee:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    200016f3:	e8 08 07 00 00       	call   20001e00 <UVM_Print_String>
    200016f8:	bf 4a 01 00 00       	mov    $0x14a,%edi
    200016fd:	e8 ce 04 00 00       	call   20001bd0 <UVM_Print_Int>
    20001702:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001707:	e8 f4 06 00 00       	call   20001e00 <UVM_Print_String>
    2000170c:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20001711:	e8 ea 06 00 00       	call   20001e00 <UVM_Print_String>
    20001716:	bf 63 27 00 20       	mov    $0x20002763,%edi
    2000171b:	e8 e0 06 00 00       	call   20001e00 <UVM_Print_String>
    20001720:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20001725:	e8 d6 06 00 00       	call   20001e00 <UVM_Print_String>
    2000172a:	bf 54 27 00 20       	mov    $0x20002754,%edi
    2000172f:	e8 cc 06 00 00       	call   20001e00 <UVM_Print_String>
    20001734:	eb fe                	jmp    20001734 <main+0x11a4>
        UVM_ASSERT(UVM_Thd_Exec_Set(UVM_CAPID(TEST_THD_TBL,TEST_THD3),(ptr_t)TEST_SIG_FUNC2,14*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
    20001736:	bf 18 25 00 20       	mov    $0x20002518,%edi
    2000173b:	e8 c0 06 00 00       	call   20001e00 <UVM_Print_String>
    20001740:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20001745:	e8 b6 06 00 00       	call   20001e00 <UVM_Print_String>
    2000174a:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    2000174f:	e8 ac 06 00 00       	call   20001e00 <UVM_Print_String>
    20001754:	bf 58 01 00 00       	mov    $0x158,%edi
    20001759:	e8 72 04 00 00       	call   20001bd0 <UVM_Print_Int>
    2000175e:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001763:	e8 98 06 00 00       	call   20001e00 <UVM_Print_String>
    20001768:	bf 57 27 00 20       	mov    $0x20002757,%edi
    2000176d:	e8 8e 06 00 00       	call   20001e00 <UVM_Print_String>
    20001772:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20001777:	e8 84 06 00 00       	call   20001e00 <UVM_Print_String>
    2000177c:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20001781:	e8 7a 06 00 00       	call   20001e00 <UVM_Print_String>
    20001786:	bf 54 27 00 20       	mov    $0x20002754,%edi
    2000178b:	e8 70 06 00 00       	call   20001e00 <UVM_Print_String>
    20001790:	eb fe                	jmp    20001790 <main+0x1200>
        UVM_ASSERT(UVM_Inv_Set(TEST_INV1,(ptr_t)TEST_INV1_FUNC,15*UVM_POW2(RME_PGT_SIZE_1M)+0x20000000ULL,0)>=0);
    20001792:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20001797:	e8 64 06 00 00       	call   20001e00 <UVM_Print_String>
    2000179c:	bf 40 27 00 20       	mov    $0x20002740,%edi
    200017a1:	e8 5a 06 00 00       	call   20001e00 <UVM_Print_String>
    200017a6:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    200017ab:	e8 50 06 00 00       	call   20001e00 <UVM_Print_String>
    200017b0:	bf 68 01 00 00       	mov    $0x168,%edi
    200017b5:	e8 16 04 00 00       	call   20001bd0 <UVM_Print_Int>
    200017ba:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200017bf:	e8 3c 06 00 00       	call   20001e00 <UVM_Print_String>
    200017c4:	bf 57 27 00 20       	mov    $0x20002757,%edi
    200017c9:	e8 32 06 00 00       	call   20001e00 <UVM_Print_String>
    200017ce:	bf 63 27 00 20       	mov    $0x20002763,%edi
    200017d3:	e8 28 06 00 00       	call   20001e00 <UVM_Print_String>
    200017d8:	bf 66 27 00 20       	mov    $0x20002766,%edi
    200017dd:	e8 1e 06 00 00       	call   20001e00 <UVM_Print_String>
    200017e2:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200017e7:	e8 14 06 00 00       	call   20001e00 <UVM_Print_String>
    200017ec:	eb fe                	jmp    200017ec <main+0x125c>
        UVM_ASSERT(UVM_Inv_Crt(UVM_BOOT_CAPTBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0), TEST_INV1, TEST_PROCESS, Cur_Addr)>=0);
    200017ee:	bf 18 25 00 20       	mov    $0x20002518,%edi
    200017f3:	e8 08 06 00 00       	call   20001e00 <UVM_Print_String>
    200017f8:	bf 40 27 00 20       	mov    $0x20002740,%edi
    200017fd:	e8 fe 05 00 00       	call   20001e00 <UVM_Print_String>
    20001802:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20001807:	e8 f4 05 00 00       	call   20001e00 <UVM_Print_String>
    2000180c:	bf 65 01 00 00       	mov    $0x165,%edi
    20001811:	e8 ba 03 00 00       	call   20001bd0 <UVM_Print_Int>
    20001816:	bf 54 27 00 20       	mov    $0x20002754,%edi
    2000181b:	e8 e0 05 00 00       	call   20001e00 <UVM_Print_String>
    20001820:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20001825:	e8 d6 05 00 00       	call   20001e00 <UVM_Print_String>
    2000182a:	bf 63 27 00 20       	mov    $0x20002763,%edi
    2000182f:	e8 cc 05 00 00       	call   20001e00 <UVM_Print_String>
    20001834:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20001839:	e8 c2 05 00 00       	call   20001e00 <UVM_Print_String>
    2000183e:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001843:	e8 b8 05 00 00       	call   20001e00 <UVM_Print_String>
    20001848:	eb fe                	jmp    20001848 <main+0x12b8>
        UVM_ASSERT(UVM_Thd_Sched_Prio(UVM_CAPID(TEST_THD_TBL,TEST_THD3),0)>=0);
    2000184a:	bf 18 25 00 20       	mov    $0x20002518,%edi
    2000184f:	e8 ac 05 00 00       	call   20001e00 <UVM_Print_String>
    20001854:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20001859:	e8 a2 05 00 00       	call   20001e00 <UVM_Print_String>
    2000185e:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20001863:	e8 98 05 00 00       	call   20001e00 <UVM_Print_String>
    20001868:	bf 5f 01 00 00       	mov    $0x15f,%edi
    2000186d:	e8 5e 03 00 00       	call   20001bd0 <UVM_Print_Int>
    20001872:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001877:	e8 84 05 00 00       	call   20001e00 <UVM_Print_String>
    2000187c:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20001881:	e8 7a 05 00 00       	call   20001e00 <UVM_Print_String>
    20001886:	bf 63 27 00 20       	mov    $0x20002763,%edi
    2000188b:	e8 70 05 00 00       	call   20001e00 <UVM_Print_String>
    20001890:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20001895:	e8 66 05 00 00       	call   20001e00 <UVM_Print_String>
    2000189a:	bf 54 27 00 20       	mov    $0x20002754,%edi
    2000189f:	e8 5c 05 00 00       	call   20001e00 <UVM_Print_String>
    200018a4:	eb fe                	jmp    200018a4 <main+0x1314>
        UVM_ASSERT(UVM_Thd_Time_Xfer(UVM_CAPID(TEST_THD_TBL,TEST_THD3),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_THD_INF_TIME)>=0);
    200018a6:	bf 18 25 00 20       	mov    $0x20002518,%edi
    200018ab:	e8 50 05 00 00       	call   20001e00 <UVM_Print_String>
    200018b0:	bf 40 27 00 20       	mov    $0x20002740,%edi
    200018b5:	e8 46 05 00 00       	call   20001e00 <UVM_Print_String>
    200018ba:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    200018bf:	e8 3c 05 00 00       	call   20001e00 <UVM_Print_String>
    200018c4:	bf 41 01 00 00       	mov    $0x141,%edi
    200018c9:	e8 02 03 00 00       	call   20001bd0 <UVM_Print_Int>
    200018ce:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200018d3:	e8 28 05 00 00       	call   20001e00 <UVM_Print_String>
    200018d8:	bf 57 27 00 20       	mov    $0x20002757,%edi
    200018dd:	e8 1e 05 00 00       	call   20001e00 <UVM_Print_String>
    200018e2:	bf 63 27 00 20       	mov    $0x20002763,%edi
    200018e7:	e8 14 05 00 00       	call   20001e00 <UVM_Print_String>
    200018ec:	bf 66 27 00 20       	mov    $0x20002766,%edi
    200018f1:	e8 0a 05 00 00       	call   20001e00 <UVM_Print_String>
    200018f6:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200018fb:	e8 00 05 00 00       	call   20001e00 <UVM_Print_String>
    20001900:	eb fe                	jmp    20001900 <main+0x1370>
        UVM_ASSERT(UVM_Thd_Sched_Bind(UVM_CAPID(TEST_THD_TBL,TEST_THD3),UVM_CAPID(UVM_BOOT_TBL_THD,0),UVM_CAPID_NULL,0,0)>=0);
    20001902:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20001907:	e8 f4 04 00 00       	call   20001e00 <UVM_Print_String>
    2000190c:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20001911:	e8 ea 04 00 00       	call   20001e00 <UVM_Print_String>
    20001916:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    2000191b:	e8 e0 04 00 00       	call   20001e00 <UVM_Print_String>
    20001920:	bf 40 01 00 00       	mov    $0x140,%edi
    20001925:	e8 a6 02 00 00       	call   20001bd0 <UVM_Print_Int>
    2000192a:	bf 54 27 00 20       	mov    $0x20002754,%edi
    2000192f:	e8 cc 04 00 00       	call   20001e00 <UVM_Print_String>
    20001934:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20001939:	e8 c2 04 00 00       	call   20001e00 <UVM_Print_String>
    2000193e:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20001943:	e8 b8 04 00 00       	call   20001e00 <UVM_Print_String>
    20001948:	bf 66 27 00 20       	mov    $0x20002766,%edi
    2000194d:	e8 ae 04 00 00       	call   20001e00 <UVM_Print_String>
    20001952:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001957:	e8 a4 04 00 00       	call   20001e00 <UVM_Print_String>
    2000195c:	eb fe                	jmp    2000195c <main+0x13cc>
        UVM_ASSERT(UVM_Thd_Crt(TEST_THD_TBL, UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_THD3, TEST_PROCESS, 10, Cur_Addr)>=0);
    2000195e:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20001963:	e8 98 04 00 00       	call   20001e00 <UVM_Print_String>
    20001968:	bf 40 27 00 20       	mov    $0x20002740,%edi
    2000196d:	e8 8e 04 00 00       	call   20001e00 <UVM_Print_String>
    20001972:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20001977:	e8 84 04 00 00       	call   20001e00 <UVM_Print_String>
    2000197c:	bf 3f 01 00 00       	mov    $0x13f,%edi
    20001981:	e8 4a 02 00 00       	call   20001bd0 <UVM_Print_Int>
    20001986:	bf 54 27 00 20       	mov    $0x20002754,%edi
    2000198b:	e8 70 04 00 00       	call   20001e00 <UVM_Print_String>
    20001990:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20001995:	e8 66 04 00 00       	call   20001e00 <UVM_Print_String>
    2000199a:	bf 63 27 00 20       	mov    $0x20002763,%edi
    2000199f:	e8 5c 04 00 00       	call   20001e00 <UVM_Print_String>
    200019a4:	bf 66 27 00 20       	mov    $0x20002766,%edi
    200019a9:	e8 52 04 00 00       	call   20001e00 <UVM_Print_String>
    200019ae:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200019b3:	e8 48 04 00 00       	call   20001e00 <UVM_Print_String>
    200019b8:	eb fe                	jmp    200019b8 <main+0x1428>
        UVM_ASSERT(UVM_Proc_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_PROCESS,UVM_BOOT_CAPTBL,
    200019ba:	bf 18 25 00 20       	mov    $0x20002518,%edi
    200019bf:	e8 3c 04 00 00       	call   20001e00 <UVM_Print_String>
    200019c4:	bf 40 27 00 20       	mov    $0x20002740,%edi
    200019c9:	e8 32 04 00 00       	call   20001e00 <UVM_Print_String>
    200019ce:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    200019d3:	e8 28 04 00 00       	call   20001e00 <UVM_Print_String>
    200019d8:	bf 3c 01 00 00       	mov    $0x13c,%edi
    200019dd:	e8 ee 01 00 00       	call   20001bd0 <UVM_Print_Int>
    200019e2:	bf 54 27 00 20       	mov    $0x20002754,%edi
    200019e7:	e8 14 04 00 00       	call   20001e00 <UVM_Print_String>
    200019ec:	bf 57 27 00 20       	mov    $0x20002757,%edi
    200019f1:	e8 0a 04 00 00       	call   20001e00 <UVM_Print_String>
    200019f6:	bf 63 27 00 20       	mov    $0x20002763,%edi
    200019fb:	e8 00 04 00 00       	call   20001e00 <UVM_Print_String>
    20001a00:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20001a05:	e8 f6 03 00 00       	call   20001e00 <UVM_Print_String>
    20001a0a:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001a0f:	e8 ec 03 00 00       	call   20001e00 <UVM_Print_String>
    20001a14:	eb fe                	jmp    20001a14 <main+0x1484>
        UVM_ASSERT(UVM_Pgtbl_Crt(TEST_PROCESS_PGT,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_PROCESS_PML4,Cur_Addr,0,1U,RME_PGT_SIZE_512G,RME_PGT_NUM_512)>=0);
    20001a16:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20001a1b:	e8 e0 03 00 00       	call   20001e00 <UVM_Print_String>
    20001a20:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20001a25:	e8 d6 03 00 00       	call   20001e00 <UVM_Print_String>
    20001a2a:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20001a2f:	e8 cc 03 00 00       	call   20001e00 <UVM_Print_String>
    20001a34:	bf 1a 01 00 00       	mov    $0x11a,%edi
    20001a39:	e8 92 01 00 00       	call   20001bd0 <UVM_Print_Int>
    20001a3e:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001a43:	e8 b8 03 00 00       	call   20001e00 <UVM_Print_String>
    20001a48:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20001a4d:	e8 ae 03 00 00       	call   20001e00 <UVM_Print_String>
    20001a52:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20001a57:	e8 a4 03 00 00       	call   20001e00 <UVM_Print_String>
    20001a5c:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20001a61:	e8 9a 03 00 00       	call   20001e00 <UVM_Print_String>
    20001a66:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001a6b:	e8 90 03 00 00       	call   20001e00 <UVM_Print_String>
    20001a70:	eb fe                	jmp    20001a70 <main+0x14e0>
        UVM_ASSERT(UVM_Captbl_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_PROCESS_PGT,Cur_Addr,1+16+8192)>=0);
    20001a72:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20001a77:	e8 84 03 00 00       	call   20001e00 <UVM_Print_String>
    20001a7c:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20001a81:	e8 7a 03 00 00       	call   20001e00 <UVM_Print_String>
    20001a86:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20001a8b:	e8 70 03 00 00       	call   20001e00 <UVM_Print_String>
    20001a90:	bf 16 01 00 00       	mov    $0x116,%edi
    20001a95:	e8 36 01 00 00       	call   20001bd0 <UVM_Print_Int>
    20001a9a:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001a9f:	e8 5c 03 00 00       	call   20001e00 <UVM_Print_String>
    20001aa4:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20001aa9:	e8 52 03 00 00       	call   20001e00 <UVM_Print_String>
    20001aae:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20001ab3:	e8 48 03 00 00       	call   20001e00 <UVM_Print_String>
    20001ab8:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20001abd:	e8 3e 03 00 00       	call   20001e00 <UVM_Print_String>
    20001ac2:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001ac7:	e8 34 03 00 00       	call   20001e00 <UVM_Print_String>
    20001acc:	eb fe                	jmp    20001acc <main+0x153c>
        UVM_ASSERT(UVM_Captbl_Crt(UVM_BOOT_CAPTBL,UVM_CAPID(UVM_BOOT_TBL_KMEM,0),TEST_PROC_CAPTBL,Cur_Addr,16)>=0);
    20001ace:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20001ad3:	e8 28 03 00 00       	call   20001e00 <UVM_Print_String>
    20001ad8:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20001add:	e8 1e 03 00 00       	call   20001e00 <UVM_Print_String>
    20001ae2:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20001ae7:	e8 14 03 00 00       	call   20001e00 <UVM_Print_String>
    20001aec:	bf 13 01 00 00       	mov    $0x113,%edi
    20001af1:	e8 da 00 00 00       	call   20001bd0 <UVM_Print_Int>
    20001af6:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001afb:	e8 00 03 00 00       	call   20001e00 <UVM_Print_String>
    20001b00:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20001b05:	e8 f6 02 00 00       	call   20001e00 <UVM_Print_String>
    20001b0a:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20001b0f:	e8 ec 02 00 00       	call   20001e00 <UVM_Print_String>
    20001b14:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20001b19:	e8 e2 02 00 00       	call   20001e00 <UVM_Print_String>
    20001b1e:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001b23:	e8 d8 02 00 00       	call   20001e00 <UVM_Print_String>
    20001b28:	eb fe                	jmp    20001b28 <main+0x1598>
        UVM_ASSERT(UVM_Thd_Sched_Prio(UVM_CAPID(TEST_THD_TBL,TEST_THD1),0)>=0);
    20001b2a:	bf 18 25 00 20       	mov    $0x20002518,%edi
    20001b2f:	e8 cc 02 00 00       	call   20001e00 <UVM_Print_String>
    20001b34:	bf 40 27 00 20       	mov    $0x20002740,%edi
    20001b39:	e8 c2 02 00 00       	call   20001e00 <UVM_Print_String>
    20001b3e:	bf 4c 27 00 20       	mov    $0x2000274c,%edi
    20001b43:	e8 b8 02 00 00       	call   20001e00 <UVM_Print_String>
    20001b48:	bf 0d 01 00 00       	mov    $0x10d,%edi
    20001b4d:	e8 7e 00 00 00       	call   20001bd0 <UVM_Print_Int>
    20001b52:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001b57:	e8 a4 02 00 00       	call   20001e00 <UVM_Print_String>
    20001b5c:	bf 57 27 00 20       	mov    $0x20002757,%edi
    20001b61:	e8 9a 02 00 00       	call   20001e00 <UVM_Print_String>
    20001b66:	bf 63 27 00 20       	mov    $0x20002763,%edi
    20001b6b:	e8 90 02 00 00       	call   20001e00 <UVM_Print_String>
    20001b70:	bf 66 27 00 20       	mov    $0x20002766,%edi
    20001b75:	e8 86 02 00 00       	call   20001e00 <UVM_Print_String>
    20001b7a:	bf 54 27 00 20       	mov    $0x20002754,%edi
    20001b7f:	e8 7c 02 00 00       	call   20001e00 <UVM_Print_String>
    20001b84:	eb fe                	jmp    20001b84 <main+0x15f4>
    20001b86:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    20001b8d:	00 00 00 

0000000020001b90 <UVM_List_Crt>:
Input       : volatile struct UVM_List* Head - The pointer to the list head.
Output      : None.
Return      : None.
******************************************************************************/
void UVM_List_Crt(volatile struct UVM_List* Head)
{
    20001b90:	f3 0f 1e fa          	endbr64 
    Head->Prev=(struct UVM_List*)Head;
    20001b94:	48 89 3f             	mov    %rdi,(%rdi)
    Head->Next=(struct UVM_List*)Head;
    20001b97:	48 89 7f 08          	mov    %rdi,0x8(%rdi)
}
    20001b9b:	c3                   	ret    
    20001b9c:	0f 1f 40 00          	nopl   0x0(%rax)

0000000020001ba0 <UVM_List_Del>:
              volatile struct UVM_List* Next - The next node of the target node.
Output      : None.
Return      : None.
******************************************************************************/
void UVM_List_Del(volatile struct UVM_List* Prev,volatile struct UVM_List* Next)
{
    20001ba0:	f3 0f 1e fa          	endbr64 
    Next->Prev=(struct UVM_List*)Prev;
    20001ba4:	48 89 3e             	mov    %rdi,(%rsi)
    Prev->Next=(struct UVM_List*)Next;
    20001ba7:	48 89 77 08          	mov    %rsi,0x8(%rdi)
}
    20001bab:	c3                   	ret    
    20001bac:	0f 1f 40 00          	nopl   0x0(%rax)

0000000020001bb0 <UVM_List_Ins>:
Return      : None.
******************************************************************************/
void UVM_List_Ins(volatile struct UVM_List* New,
                  volatile struct UVM_List* Prev,
                  volatile struct UVM_List* Next)
{
    20001bb0:	f3 0f 1e fa          	endbr64 
    Next->Prev=(struct UVM_List*)New;
    20001bb4:	48 89 3a             	mov    %rdi,(%rdx)
    New->Next=(struct UVM_List*)Next;
    20001bb7:	48 89 57 08          	mov    %rdx,0x8(%rdi)
    New->Prev=(struct UVM_List*)Prev;
    20001bbb:	48 89 37             	mov    %rsi,(%rdi)
    Prev->Next=(struct UVM_List*)New;
    20001bbe:	48 89 7e 08          	mov    %rdi,0x8(%rsi)
}
    20001bc2:	c3                   	ret    
    20001bc3:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    20001bca:	00 00 00 00 
    20001bce:	66 90                	xchg   %ax,%ax

0000000020001bd0 <UVM_Print_Int>:
Input       : cnt_t Int - The integer to print.
Output      : None.
Return      : cnt_t - The length of the string printed.
******************************************************************************/
cnt_t UVM_Print_Int(cnt_t Int)
{
    20001bd0:	f3 0f 1e fa          	endbr64 
    20001bd4:	41 57                	push   %r15
    20001bd6:	41 56                	push   %r14
    20001bd8:	41 55                	push   %r13
    20001bda:	41 54                	push   %r12
    20001bdc:	55                   	push   %rbp
    20001bdd:	53                   	push   %rbx
    20001bde:	48 83 ec 08          	sub    $0x8,%rsp
    cnt_t Count;
    cnt_t Num;
    ptr_t Div;

    /* how many digits are there? */
    if(Int==0)
    20001be2:	48 85 ff             	test   %rdi,%rdi
    20001be5:	0f 84 4d 01 00 00    	je     20001d38 <UVM_Print_Int+0x168>
    {
        UVM_Putchar('0');
        return 1;
    }
    else if(Int<0)
    20001beb:	0f 88 a7 00 00 00    	js     20001c98 <UVM_Print_Int+0xc8>
    else
    {
        /* How many digits are there? */
        Count=0;
        Div=1;
        Iter=Int;
    20001bf1:	48 89 fa             	mov    %rdi,%rdx
        Div=1;
    20001bf4:	bb 01 00 00 00       	mov    $0x1,%ebx
        Count=0;
    20001bf9:	45 31 e4             	xor    %r12d,%r12d
        while(Iter!=0)
        {
            Iter/=10;
    20001bfc:	48 be cd cc cc cc cc 	movabs $0xcccccccccccccccd,%rsi
    20001c03:	cc cc cc 
    20001c06:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    20001c0d:	00 00 00 
    20001c10:	48 89 d0             	mov    %rdx,%rax
    20001c13:	48 89 d1             	mov    %rdx,%rcx
            Count++;
            Div*=10;
    20001c16:	48 8d 1c 9b          	lea    (%rbx,%rbx,4),%rbx
            Count++;
    20001c1a:	49 83 c4 01          	add    $0x1,%r12
            Iter/=10;
    20001c1e:	48 f7 e6             	mul    %rsi
            Div*=10;
    20001c21:	48 01 db             	add    %rbx,%rbx
            Iter/=10;
    20001c24:	48 c1 ea 03          	shr    $0x3,%rdx
        while(Iter!=0)
    20001c28:	48 83 f9 09          	cmp    $0x9,%rcx
    20001c2c:	77 e2                	ja     20001c10 <UVM_Print_Int+0x40>
        while(Count>0)
        {
            Count--;
            UVM_Putchar(Iter/Div+'0');
            Iter=Iter%Div;
            Div/=10;
    20001c2e:	48 bd cd cc cc cc cc 	movabs $0xcccccccccccccccd,%rbp
    20001c35:	cc cc cc 
        Div/=10;
    20001c38:	48 89 d8             	mov    %rbx,%rax
            Count++;
    20001c3b:	4d 89 e6             	mov    %r12,%r14
        Div/=10;
    20001c3e:	48 f7 e6             	mul    %rsi
    20001c41:	48 89 d3             	mov    %rdx,%rbx
    20001c44:	48 c1 eb 03          	shr    $0x3,%rbx
        while(Count>0)
    20001c48:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    20001c4f:	00 
            Count--;
    20001c50:	48 89 f8             	mov    %rdi,%rax
    20001c53:	31 d2                	xor    %edx,%edx
    20001c55:	49 83 ee 01          	sub    $0x1,%r14
            UVM_Putchar(Iter/Div+'0');
    20001c59:	48 f7 f3             	div    %rbx
    20001c5c:	48 89 c7             	mov    %rax,%rdi
    20001c5f:	49 89 d5             	mov    %rdx,%r13
    20001c62:	83 c7 30             	add    $0x30,%edi
    20001c65:	40 0f be ff          	movsbl %dil,%edi
    20001c69:	e8 b2 07 00 00       	call   20002420 <UVM_Putchar>
            Div/=10;
    20001c6e:	48 89 d8             	mov    %rbx,%rax
            Iter=Iter%Div;
    20001c71:	4c 89 ef             	mov    %r13,%rdi
            Div/=10;
    20001c74:	48 f7 e5             	mul    %rbp
    20001c77:	48 89 d3             	mov    %rdx,%rbx
    20001c7a:	48 c1 eb 03          	shr    $0x3,%rbx
        while(Count>0)
    20001c7e:	4d 85 f6             	test   %r14,%r14
    20001c81:	75 cd                	jne    20001c50 <UVM_Print_Int+0x80>
        }
    }
    
    return Num;
}
    20001c83:	48 83 c4 08          	add    $0x8,%rsp
    20001c87:	4c 89 e0             	mov    %r12,%rax
    20001c8a:	5b                   	pop    %rbx
    20001c8b:	5d                   	pop    %rbp
    20001c8c:	41 5c                	pop    %r12
    20001c8e:	41 5d                	pop    %r13
    20001c90:	41 5e                	pop    %r14
    20001c92:	41 5f                	pop    %r15
    20001c94:	c3                   	ret    
    20001c95:	0f 1f 00             	nopl   (%rax)
        Iter=-Int;
    20001c98:	48 f7 df             	neg    %rdi
        Div=1;
    20001c9b:	bb 01 00 00 00       	mov    $0x1,%ebx
        Count=0;
    20001ca0:	45 31 ff             	xor    %r15d,%r15d
            Iter/=10;
    20001ca3:	48 be cd cc cc cc cc 	movabs $0xcccccccccccccccd,%rsi
    20001caa:	cc cc cc 
        Iter=-Int;
    20001cad:	49 89 fe             	mov    %rdi,%r14
    20001cb0:	48 89 fa             	mov    %rdi,%rdx
    20001cb3:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
            Iter/=10;
    20001cb8:	48 89 d0             	mov    %rdx,%rax
    20001cbb:	48 89 d1             	mov    %rdx,%rcx
            Div*=10;
    20001cbe:	48 8d 1c 9b          	lea    (%rbx,%rbx,4),%rbx
    20001cc2:	4d 89 fc             	mov    %r15,%r12
            Iter/=10;
    20001cc5:	48 f7 e6             	mul    %rsi
            Count++;
    20001cc8:	49 83 c7 01          	add    $0x1,%r15
            Div*=10;
    20001ccc:	48 01 db             	add    %rbx,%rbx
            Iter/=10;
    20001ccf:	48 c1 ea 03          	shr    $0x3,%rdx
        while(Iter!=0)
    20001cd3:	48 83 f9 09          	cmp    $0x9,%rcx
    20001cd7:	77 df                	ja     20001cb8 <UVM_Print_Int+0xe8>
        Div/=10;
    20001cd9:	48 89 d8             	mov    %rbx,%rax
        UVM_Putchar('-');
    20001cdc:	bf 2d 00 00 00       	mov    $0x2d,%edi
        Num=Count+1;
    20001ce1:	49 83 c4 02          	add    $0x2,%r12
            Div/=10;
    20001ce5:	49 bd cd cc cc cc cc 	movabs $0xcccccccccccccccd,%r13
    20001cec:	cc cc cc 
        Div/=10;
    20001cef:	48 f7 e6             	mul    %rsi
    20001cf2:	48 89 d3             	mov    %rdx,%rbx
        UVM_Putchar('-');
    20001cf5:	e8 26 07 00 00       	call   20002420 <UVM_Putchar>
        Div/=10;
    20001cfa:	48 c1 eb 03          	shr    $0x3,%rbx
        while(Count>0)
    20001cfe:	66 90                	xchg   %ax,%ax
            Count--;
    20001d00:	4c 89 f0             	mov    %r14,%rax
    20001d03:	31 d2                	xor    %edx,%edx
    20001d05:	49 83 ef 01          	sub    $0x1,%r15
            UVM_Putchar(Iter/Div+'0');
    20001d09:	48 f7 f3             	div    %rbx
    20001d0c:	83 c0 30             	add    $0x30,%eax
    20001d0f:	49 89 d6             	mov    %rdx,%r14
    20001d12:	0f be f8             	movsbl %al,%edi
    20001d15:	e8 06 07 00 00       	call   20002420 <UVM_Putchar>
            Div/=10;
    20001d1a:	48 89 d8             	mov    %rbx,%rax
    20001d1d:	49 f7 e5             	mul    %r13
    20001d20:	48 89 d3             	mov    %rdx,%rbx
    20001d23:	48 c1 eb 03          	shr    $0x3,%rbx
        while(Count>0)
    20001d27:	4d 85 ff             	test   %r15,%r15
    20001d2a:	75 d4                	jne    20001d00 <UVM_Print_Int+0x130>
    20001d2c:	e9 52 ff ff ff       	jmp    20001c83 <UVM_Print_Int+0xb3>
    20001d31:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
        UVM_Putchar('0');
    20001d38:	bf 30 00 00 00       	mov    $0x30,%edi
        return 1;
    20001d3d:	41 bc 01 00 00 00    	mov    $0x1,%r12d
        UVM_Putchar('0');
    20001d43:	e8 d8 06 00 00       	call   20002420 <UVM_Putchar>
        return 1;
    20001d48:	e9 36 ff ff ff       	jmp    20001c83 <UVM_Print_Int+0xb3>
    20001d4d:	0f 1f 00             	nopl   (%rax)

0000000020001d50 <UVM_Print_Uint>:
Input       : ptr_t Uint - The unsigned integer to print.
Output      : None.
Return      : cnt_t - The length of the string printed.
******************************************************************************/
cnt_t UVM_Print_Uint(ptr_t Uint)
{
    20001d50:	f3 0f 1e fa          	endbr64 
    20001d54:	41 54                	push   %r12
    20001d56:	55                   	push   %rbp
    20001d57:	53                   	push   %rbx
    ptr_t Iter;
    cnt_t Count;
    cnt_t Num;
    
    /* how many digits are there? */
    if(Uint==0)
    20001d58:	48 85 ff             	test   %rdi,%rdi
    20001d5b:	0f 84 87 00 00 00    	je     20001de8 <UVM_Print_Uint+0x98>
    else
    {
        /* Filter out all the zeroes */
        Count=0;
        Iter=Uint;
        while((Iter>>((sizeof(ptr_t)*8)-4))==0)
    20001d61:	48 89 f9             	mov    %rdi,%rcx
        Count=0;
    20001d64:	31 d2                	xor    %edx,%edx
    20001d66:	48 89 fd             	mov    %rdi,%rbp
        while((Iter>>((sizeof(ptr_t)*8)-4))==0)
    20001d69:	48 89 f8             	mov    %rdi,%rax
    20001d6c:	48 c1 e9 3c          	shr    $0x3c,%rcx
        {
            Iter<<=4;
            Count++;
        }
        /* Count is the number of pts to print */
        Count=sizeof(ptr_t)*2-Count;
    20001d70:	41 bc 10 00 00 00    	mov    $0x10,%r12d
        while((Iter>>((sizeof(ptr_t)*8)-4))==0)
    20001d76:	75 27                	jne    20001d9f <UVM_Print_Uint+0x4f>
    20001d78:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    20001d7f:	00 
            Iter<<=4;
    20001d80:	48 c1 e0 04          	shl    $0x4,%rax
            Count++;
    20001d84:	48 83 c2 01          	add    $0x1,%rdx
        while((Iter>>((sizeof(ptr_t)*8)-4))==0)
    20001d88:	48 89 c6             	mov    %rax,%rsi
    20001d8b:	48 c1 ee 3c          	shr    $0x3c,%rsi
    20001d8f:	74 ef                	je     20001d80 <UVM_Print_Uint+0x30>
        Count=sizeof(ptr_t)*2-Count;
    20001d91:	41 bc 10 00 00 00    	mov    $0x10,%r12d
    20001d97:	49 29 d4             	sub    %rdx,%r12
        Num=Count;
        while(Count>0)
    20001d9a:	4d 85 e4             	test   %r12,%r12
    20001d9d:	7e 3d                	jle    20001ddc <UVM_Print_Uint+0x8c>
        Count=sizeof(ptr_t)*2-Count;
    20001d9f:	4c 89 e3             	mov    %r12,%rbx
    20001da2:	eb 11                	jmp    20001db5 <UVM_Print_Uint+0x65>
    20001da4:	0f 1f 40 00          	nopl   0x0(%rax)
        {
            Count--;
            Iter=(Uint>>(Count*4))&0x0F;
            if(Iter<10)
                UVM_Putchar('0'+Iter);
    20001da8:	8d 78 30             	lea    0x30(%rax),%edi
    20001dab:	e8 70 06 00 00       	call   20002420 <UVM_Putchar>
        while(Count>0)
    20001db0:	48 85 db             	test   %rbx,%rbx
    20001db3:	74 27                	je     20001ddc <UVM_Print_Uint+0x8c>
            Count--;
    20001db5:	48 83 eb 01          	sub    $0x1,%rbx
            Iter=(Uint>>(Count*4))&0x0F;
    20001db9:	48 89 e8             	mov    %rbp,%rax
    20001dbc:	8d 0c 9d 00 00 00 00 	lea    0x0(,%rbx,4),%ecx
    20001dc3:	48 d3 e8             	shr    %cl,%rax
    20001dc6:	83 e0 0f             	and    $0xf,%eax
            if(Iter<10)
    20001dc9:	48 83 f8 09          	cmp    $0x9,%rax
    20001dcd:	76 d9                	jbe    20001da8 <UVM_Print_Uint+0x58>
            else
                UVM_Putchar('A'+Iter-10);
    20001dcf:	8d 78 37             	lea    0x37(%rax),%edi
    20001dd2:	e8 49 06 00 00       	call   20002420 <UVM_Putchar>
        while(Count>0)
    20001dd7:	48 85 db             	test   %rbx,%rbx
    20001dda:	75 d9                	jne    20001db5 <UVM_Print_Uint+0x65>
        }
    }
    
    return Num;
}
    20001ddc:	4c 89 e0             	mov    %r12,%rax
    20001ddf:	5b                   	pop    %rbx
    20001de0:	5d                   	pop    %rbp
    20001de1:	41 5c                	pop    %r12
    20001de3:	c3                   	ret    
    20001de4:	0f 1f 40 00          	nopl   0x0(%rax)
        return 1;
    20001de8:	41 bc 01 00 00 00    	mov    $0x1,%r12d
        UVM_Putchar('0');
    20001dee:	bf 30 00 00 00       	mov    $0x30,%edi
    20001df3:	e8 28 06 00 00       	call   20002420 <UVM_Putchar>
}
    20001df8:	4c 89 e0             	mov    %r12,%rax
    20001dfb:	5b                   	pop    %rbx
    20001dfc:	5d                   	pop    %rbp
    20001dfd:	41 5c                	pop    %r12
    20001dff:	c3                   	ret    

0000000020001e00 <UVM_Print_String>:
Input       : s8* String - The string to print.
Output      : None.
Return      : cnt_t - The length of the string printed, the '\0' is not included.
******************************************************************************/
cnt_t UVM_Print_String(s8* String)
{
    20001e00:	f3 0f 1e fa          	endbr64 
    20001e04:	41 54                	push   %r12
    cnt_t Count;
    
    Count=0;
    20001e06:	45 31 e4             	xor    %r12d,%r12d
{
    20001e09:	53                   	push   %rbx
    20001e0a:	48 89 fb             	mov    %rdi,%rbx
    20001e0d:	48 83 ec 08          	sub    $0x8,%rsp
    20001e11:	eb 17                	jmp    20001e2a <UVM_Print_String+0x2a>
    20001e13:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    while(Count<UVM_USER_DEBUG_MAX_STR)
    {
        if(String[Count]=='\0')
            break;
        
        UVM_Putchar(String[Count++]);
    20001e18:	49 83 c4 01          	add    $0x1,%r12
    20001e1c:	e8 ff 05 00 00       	call   20002420 <UVM_Putchar>
    while(Count<UVM_USER_DEBUG_MAX_STR)
    20001e21:	49 81 fc 80 00 00 00 	cmp    $0x80,%r12
    20001e28:	74 0a                	je     20001e34 <UVM_Print_String+0x34>
        if(String[Count]=='\0')
    20001e2a:	42 0f be 3c 23       	movsbl (%rbx,%r12,1),%edi
    20001e2f:	40 84 ff             	test   %dil,%dil
    20001e32:	75 e4                	jne    20001e18 <UVM_Print_String+0x18>
    }

    return Count;
}
    20001e34:	48 83 c4 08          	add    $0x8,%rsp
    20001e38:	4c 89 e0             	mov    %r12,%rax
    20001e3b:	5b                   	pop    %rbx
    20001e3c:	41 5c                	pop    %r12
    20001e3e:	c3                   	ret    
    20001e3f:	90                   	nop

0000000020001e40 <UVM_Captbl_Crt>:
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Captbl_Crt(cid_t Cap_Captbl_Crt, cid_t Cap_Kmem, 
                     cid_t Cap_Captbl, ptr_t Vaddr, ptr_t Entry_Num)
{
    20001e40:	f3 0f 1e fa          	endbr64 
    20001e44:	49 89 d1             	mov    %rdx,%r9
    return UVM_CAP_OP(RME_SVC_CPT_CRT, Cap_Captbl_Crt,
    20001e47:	48 c1 e6 20          	shl    $0x20,%rsi
{
    20001e4b:	48 89 ca             	mov    %rcx,%rdx
    return UVM_CAP_OP(RME_SVC_CPT_CRT, Cap_Captbl_Crt,
    20001e4e:	48 b8 00 00 00 00 0a 	movabs $0xa00000000,%rax
    20001e55:	00 00 00 
    20001e58:	45 89 c9             	mov    %r9d,%r9d
{
    20001e5b:	4c 89 c1             	mov    %r8,%rcx
    return UVM_CAP_OP(RME_SVC_CPT_CRT, Cap_Captbl_Crt,
    20001e5e:	48 09 c7             	or     %rax,%rdi
    20001e61:	4c 09 ce             	or     %r9,%rsi
    20001e64:	e9 84 e2 ff ff       	jmp    200000ed <UVM_Svc>
    20001e69:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000020001e70 <UVM_Captbl_Del>:
              cid_t Cap_Del - The capability to the capability table to be deleted. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Captbl_Del(cid_t Cap_Captbl_Del, cid_t Cap_Del)
{
    20001e70:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_CPT_DEL, Cap_Captbl_Del,
    20001e74:	48 b8 00 00 00 00 0b 	movabs $0xb00000000,%rax
    20001e7b:	00 00 00 
    20001e7e:	31 c9                	xor    %ecx,%ecx
    20001e80:	31 d2                	xor    %edx,%edx
    20001e82:	48 09 c7             	or     %rax,%rdi
    20001e85:	e9 63 e2 ff ff       	jmp    200000ed <UVM_Svc>
    20001e8a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

0000000020001e90 <UVM_Captbl_Frz>:
              cid_t Cap_Frz - The cap to the kernel object being freezed. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Captbl_Frz(cid_t Cap_Captbl_Frz, cid_t Cap_Frz)
{
    20001e90:	f3 0f 1e fa          	endbr64 
        return UVM_CAP_OP(RME_SVC_CPT_FRZ, Cap_Captbl_Frz,
    20001e94:	48 b8 00 00 00 00 0c 	movabs $0xc00000000,%rax
    20001e9b:	00 00 00 
    20001e9e:	31 c9                	xor    %ecx,%ecx
    20001ea0:	31 d2                	xor    %edx,%edx
    20001ea2:	48 09 c7             	or     %rax,%rdi
    20001ea5:	e9 43 e2 ff ff       	jmp    200000ed <UVM_Svc>
    20001eaa:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

0000000020001eb0 <UVM_Captbl_Add>:
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Captbl_Add(cid_t Cap_Captbl_Dst, cid_t Cap_Dst, 
                     cid_t Cap_Captbl_Src, cid_t Cap_Src, ptr_t Flags)
{
    20001eb0:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_CPT_ADD, 0,
    20001eb4:	48 c1 e7 20          	shl    $0x20,%rdi
{
    20001eb8:	49 89 c9             	mov    %rcx,%r9
    return UVM_CAP_OP(RME_SVC_CPT_ADD, 0,
    20001ebb:	48 c1 e2 20          	shl    $0x20,%rdx
    20001ebf:	89 f6                	mov    %esi,%esi
    20001ec1:	45 89 c9             	mov    %r9d,%r9d
    20001ec4:	48 09 fe             	or     %rdi,%rsi
{
    20001ec7:	4c 89 c1             	mov    %r8,%rcx
    return UVM_CAP_OP(RME_SVC_CPT_ADD, 0,
    20001eca:	48 bf 00 00 00 00 0d 	movabs $0xd00000000,%rdi
    20001ed1:	00 00 00 
    20001ed4:	4c 09 ca             	or     %r9,%rdx
    20001ed7:	e9 11 e2 ff ff       	jmp    200000ed <UVM_Svc>
    20001edc:	0f 1f 40 00          	nopl   0x0(%rax)

0000000020001ee0 <UVM_Captbl_Pgtbl>:
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Captbl_Pgtbl(cid_t Cap_Captbl_Dst, cid_t Cap_Dst, 
                       cid_t Cap_Captbl_Src, cid_t Cap_Src,
                       ptr_t Start, ptr_t End, ptr_t Flags)
{
    20001ee0:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_CPT_ADD, 0,
    20001ee4:	49 c1 e1 24          	shl    $0x24,%r9
{
    20001ee8:	49 89 f2             	mov    %rsi,%r10
    20001eeb:	49 89 cb             	mov    %rcx,%r11
    return UVM_CAP_OP(RME_SVC_CPT_ADD, 0,
    20001eee:	48 89 fe             	mov    %rdi,%rsi
    20001ef1:	4c 89 c1             	mov    %r8,%rcx
    20001ef4:	48 c1 e2 20          	shl    $0x20,%rdx
    20001ef8:	45 89 db             	mov    %r11d,%r11d
    20001efb:	45 89 d2             	mov    %r10d,%r10d
    20001efe:	48 c1 e1 08          	shl    $0x8,%rcx
    20001f02:	48 c1 e6 20          	shl    $0x20,%rsi
    20001f06:	4c 09 da             	or     %r11,%rdx
    20001f09:	48 0b 4c 24 08       	or     0x8(%rsp),%rcx
    20001f0e:	48 bf 00 00 00 00 0d 	movabs $0xd00000000,%rdi
    20001f15:	00 00 00 
    20001f18:	4c 09 c9             	or     %r9,%rcx
    20001f1b:	4c 09 d6             	or     %r10,%rsi
    20001f1e:	e9 ca e1 ff ff       	jmp    200000ed <UVM_Svc>
    20001f23:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    20001f2a:	00 00 00 00 
    20001f2e:	66 90                	xchg   %ax,%ax

0000000020001f30 <UVM_Captbl_Kern>:
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Captbl_Kern(cid_t Cap_Captbl_Dst, cid_t Cap_Dst, 
                      cid_t Cap_Captbl_Src, cid_t Cap_Src,
                      ptr_t Start, ptr_t End)
{
    20001f30:	f3 0f 1e fa          	endbr64 
    20001f34:	49 89 f2             	mov    %rsi,%r10
    20001f37:	49 89 cb             	mov    %rcx,%r11
    return UVM_CAP_OP(RME_SVC_CPT_ADD, 0,
    20001f3a:	48 89 fe             	mov    %rdi,%rsi
    20001f3d:	4c 89 c9             	mov    %r9,%rcx
    20001f40:	48 c1 e1 20          	shl    $0x20,%rcx
    20001f44:	48 c1 e2 20          	shl    $0x20,%rdx
    20001f48:	45 89 db             	mov    %r11d,%r11d
    20001f4b:	45 89 d2             	mov    %r10d,%r10d
    20001f4e:	48 c1 e6 20          	shl    $0x20,%rsi
    20001f52:	4c 09 c1             	or     %r8,%rcx
    20001f55:	4c 09 da             	or     %r11,%rdx
    20001f58:	48 bf 00 00 00 00 0d 	movabs $0xd00000000,%rdi
    20001f5f:	00 00 00 
    20001f62:	4c 09 d6             	or     %r10,%rsi
    20001f65:	e9 83 e1 ff ff       	jmp    200000ed <UVM_Svc>
    20001f6a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

0000000020001f70 <UVM_Captbl_Kmem>:
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Captbl_Kmem(cid_t Cap_Captbl_Dst, cid_t Cap_Dst, 
                      cid_t Cap_Captbl_Src, cid_t Cap_Src,
                      ptr_t Start, ptr_t End, ptr_t Flags)
{
    20001f70:	f3 0f 1e fa          	endbr64 
    20001f74:	49 89 cb             	mov    %rcx,%r11
    20001f77:	49 89 f2             	mov    %rsi,%r10
    return UVM_CAP_OP(UVM_KMEM_SVC(End,RME_SVC_CPT_ADD), UVM_KMEM_CAPID(Start,Flags),
    20001f7a:	4c 89 c0             	mov    %r8,%rax
    20001f7d:	48 89 fe             	mov    %rdi,%rsi
    20001f80:	4c 89 c7             	mov    %r8,%rdi
    20001f83:	48 c1 e8 20          	shr    $0x20,%rax
    20001f87:	45 89 db             	mov    %r11d,%r11d
    20001f8a:	45 89 d2             	mov    %r10d,%r10d
    20001f8d:	48 c1 e2 20          	shl    $0x20,%rdx
    20001f91:	83 e7 c0             	and    $0xffffffc0,%edi
    20001f94:	48 c1 e6 20          	shl    $0x20,%rsi
    20001f98:	48 b9 00 00 00 00 ff 	movabs $0xffffffff00000000,%rcx
    20001f9f:	ff ff ff 
    20001fa2:	4c 21 c9             	and    %r9,%rcx
    20001fa5:	49 c1 e9 06          	shr    $0x6,%r9
    20001fa9:	4c 09 da             	or     %r11,%rdx
    20001fac:	4c 09 d6             	or     %r10,%rsi
    20001faf:	49 c1 e1 26          	shl    $0x26,%r9
    20001fb3:	48 09 c1             	or     %rax,%rcx
    20001fb6:	48 b8 00 00 00 00 0d 	movabs $0xd00000000,%rax
    20001fbd:	00 00 00 
    20001fc0:	4c 09 cf             	or     %r9,%rdi
    20001fc3:	48 0b 7c 24 08       	or     0x8(%rsp),%rdi
    20001fc8:	48 09 c7             	or     %rax,%rdi
    20001fcb:	e9 1d e1 ff ff       	jmp    200000ed <UVM_Svc>

0000000020001fd0 <UVM_Captbl_Rem>:
              cid_t Cap_Rem - The capability slot you want to remove. 1-Level.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Captbl_Rem(cid_t Cap_Captbl_Rem, cid_t Cap_Rem)
{
    20001fd0:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_CPT_REM, Cap_Captbl_Rem,
    20001fd4:	48 b8 00 00 00 00 0e 	movabs $0xe00000000,%rax
    20001fdb:	00 00 00 
    20001fde:	31 c9                	xor    %ecx,%ecx
    20001fe0:	31 d2                	xor    %edx,%edx
    20001fe2:	48 09 c7             	or     %rax,%rdi
    20001fe5:	e9 03 e1 ff ff       	jmp    200000ed <UVM_Svc>
    20001fea:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

0000000020001ff0 <UVM_Kern_Act>:
                      kernel function ever causes a context switch, it is responsible
                      for setting the return value. On failure, a context switch 
                      in the kernel fucntion is banned.
******************************************************************************/
ret_t UVM_Kern_Act(cid_t Cap_Kern, ptr_t Func_ID, ptr_t Sub_ID, ptr_t Param1, ptr_t Param2)
{
    20001ff0:	f3 0f 1e fa          	endbr64 
    20001ff4:	49 89 c9             	mov    %rcx,%r9
    return UVM_CAP_OP(RME_SVC_KFN, Cap_Kern,
    20001ff7:	48 c1 e2 20          	shl    $0x20,%rdx
    20001ffb:	89 f6                	mov    %esi,%esi
{
    20001ffd:	4c 89 c1             	mov    %r8,%rcx
    return UVM_CAP_OP(RME_SVC_KFN, Cap_Kern,
    20002000:	48 09 d6             	or     %rdx,%rsi
    20002003:	48 0f ba ef 22       	bts    $0x22,%rdi
    20002008:	4c 89 ca             	mov    %r9,%rdx
    2000200b:	e9 dd e0 ff ff       	jmp    200000ed <UVM_Svc>

0000000020002010 <UVM_Pgtbl_Crt>:
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Pgtbl_Crt(cid_t Cap_Captbl, cid_t Cap_Kmem, cid_t Cap_Pgtbl, 
                    ptr_t Vaddr, ptr_t Start_Addr, ptr_t Top_Flag,
                    ptr_t Size_Order, ptr_t Num_Order)
{
    20002010:	f3 0f 1e fa          	endbr64 
    20002014:	49 89 fb             	mov    %rdi,%r11
    
    return UVM_CAP_OP(UVM_PGTBL_SVC(Num_Order,RME_SVC_PGT_CRT), Cap_Captbl,
    20002017:	48 c1 e2 10          	shl    $0x10,%rdx
    2000201b:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
{
    20002020:	49 89 ca             	mov    %rcx,%r10
    return UVM_CAP_OP(UVM_PGTBL_SVC(Num_Order,RME_SVC_PGT_CRT), Cap_Captbl,
    20002023:	0f b7 44 24 08       	movzwl 0x8(%rsp),%eax
    20002028:	89 d2                	mov    %edx,%edx
    2000202a:	48 c1 e6 20          	shl    $0x20,%rsi
    2000202e:	4c 89 c1             	mov    %r8,%rcx
    20002031:	48 09 d6             	or     %rdx,%rsi
    20002034:	48 c1 e7 30          	shl    $0x30,%rdi
    20002038:	4c 09 c9             	or     %r9,%rcx
    2000203b:	4c 89 d2             	mov    %r10,%rdx
    2000203e:	48 09 c6             	or     %rax,%rsi
    20002041:	4c 09 df             	or     %r11,%rdi
    20002044:	48 b8 00 00 00 00 0f 	movabs $0xf00000000,%rax
    2000204b:	00 00 00 
    2000204e:	48 09 c7             	or     %rax,%rdi
    20002051:	e9 97 e0 ff ff       	jmp    200000ed <UVM_Svc>
    20002056:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    2000205d:	00 00 00 

0000000020002060 <UVM_Pgtbl_Del>:
                                page table capability to be in. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Pgtbl_Del(cid_t Cap_Captbl, cid_t Cap_Pgtbl)
{
    20002060:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_PGT_DEL, Cap_Captbl,
    20002064:	48 0f ba ef 24       	bts    $0x24,%rdi
    20002069:	31 c9                	xor    %ecx,%ecx
    2000206b:	31 d2                	xor    %edx,%edx
    2000206d:	e9 7b e0 ff ff       	jmp    200000ed <UVM_Svc>
    20002072:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    20002079:	00 00 00 00 
    2000207d:	0f 1f 00             	nopl   (%rax)

0000000020002080 <UVM_Pgtbl_Add>:
Output      : None.
Return      : ret_t - If the unmapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Pgtbl_Add(cid_t Cap_Pgtbl_Dst, ptr_t Pos_Dst, ptr_t Flags_Dst,
                    cid_t Cap_Pgtbl_Src, ptr_t Pos_Src, ptr_t Index)
{
    20002080:	f3 0f 1e fa          	endbr64 
    20002084:	49 89 d2             	mov    %rdx,%r10
    return UVM_CAP_OP(RME_SVC_PGT_ADD, Flags_Dst,
    20002087:	48 c1 e1 20          	shl    $0x20,%rcx
    2000208b:	89 f6                	mov    %esi,%esi
    2000208d:	45 89 c0             	mov    %r8d,%r8d
    20002090:	48 c1 e7 20          	shl    $0x20,%rdi
    20002094:	48 89 ca             	mov    %rcx,%rdx
    20002097:	4c 89 c9             	mov    %r9,%rcx
    2000209a:	48 b8 00 00 00 00 11 	movabs $0x1100000000,%rax
    200020a1:	00 00 00 
    200020a4:	48 09 fe             	or     %rdi,%rsi
    200020a7:	4c 89 d7             	mov    %r10,%rdi
    200020aa:	4c 09 c2             	or     %r8,%rdx
    200020ad:	48 09 c7             	or     %rax,%rdi
    200020b0:	e9 38 e0 ff ff       	jmp    200000ed <UVM_Svc>
    200020b5:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    200020bc:	00 00 00 00 

00000000200020c0 <UVM_Pgtbl_Rem>:
              ptr_t Pos - The virtual address position to unmap from.
Output      : None.
Return      : ret_t - If the unmapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Pgtbl_Rem(cid_t Cap_Pgtbl, ptr_t Pos)
{
    200020c0:	f3 0f 1e fa          	endbr64 
    200020c4:	48 89 f2             	mov    %rsi,%rdx
    return UVM_CAP_OP(RME_SVC_PGT_REM, 0,
    200020c7:	31 c9                	xor    %ecx,%ecx
    200020c9:	48 89 fe             	mov    %rdi,%rsi
    200020cc:	48 bf 00 00 00 00 12 	movabs $0x1200000000,%rdi
    200020d3:	00 00 00 
    200020d6:	e9 12 e0 ff ff       	jmp    200000ed <UVM_Svc>
    200020db:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

00000000200020e0 <UVM_Pgtbl_Con>:
              ptr_t Flags_Child - This have no effect on MPU-based architectures.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Pgtbl_Con(cid_t Cap_Pgtbl_Parent, ptr_t Pos, cid_t Cap_Pgtbl_Child, ptr_t Flags_Child)
{
    200020e0:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_PGT_CON, 0,
    200020e4:	48 c1 e7 20          	shl    $0x20,%rdi
{
    200020e8:	49 89 f0             	mov    %rsi,%r8
    return UVM_CAP_OP(RME_SVC_PGT_CON, 0,
    200020eb:	89 d2                	mov    %edx,%edx
    200020ed:	48 89 fe             	mov    %rdi,%rsi
    200020f0:	48 bf 00 00 00 00 13 	movabs $0x1300000000,%rdi
    200020f7:	00 00 00 
    200020fa:	48 09 d6             	or     %rdx,%rsi
    200020fd:	4c 89 c2             	mov    %r8,%rdx
    20002100:	e9 e8 df ff ff       	jmp    200000ed <UVM_Svc>
    20002105:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    2000210c:	00 00 00 00 

0000000020002110 <UVM_Pgtbl_Des>:
                          table from.
Output      : None.
Return      : ret_t - If the mapping is successful, it will return 0; else error code.
******************************************************************************/
ret_t UVM_Pgtbl_Des(cid_t Cap_Pgtbl, ptr_t Pos)
{
    20002110:	f3 0f 1e fa          	endbr64 
    20002114:	48 89 f2             	mov    %rsi,%rdx
    return UVM_CAP_OP(RME_SVC_PGT_DES, 0,
    20002117:	31 c9                	xor    %ecx,%ecx
    20002119:	48 89 fe             	mov    %rdi,%rsi
    2000211c:	48 bf 00 00 00 00 14 	movabs $0x1400000000,%rdi
    20002123:	00 00 00 
    20002126:	e9 c2 df ff ff       	jmp    200000ed <UVM_Svc>
    2000212b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000020002130 <UVM_Proc_Crt>:
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Proc_Crt(cid_t Cap_Captbl_Crt, cid_t Cap_Kmem, cid_t Cap_Proc,
                   cid_t Cap_Captbl, cid_t Cap_Pgtbl, ptr_t Vaddr)
{
    20002130:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_PRC_CRT, Cap_Captbl_Crt,
    20002134:	48 b8 00 00 00 00 15 	movabs $0x1500000000,%rax
    2000213b:	00 00 00 
{
    2000213e:	48 89 d6             	mov    %rdx,%rsi
    20002141:	48 89 ca             	mov    %rcx,%rdx
    20002144:	4c 89 c1             	mov    %r8,%rcx
    return UVM_CAP_OP(RME_SVC_PRC_CRT, Cap_Captbl_Crt,
    20002147:	48 09 c7             	or     %rax,%rdi
    2000214a:	e9 9e df ff ff       	jmp    200000ed <UVM_Svc>
    2000214f:	90                   	nop

0000000020002150 <UVM_Proc_Del>:
              cid_t Cap_Proc - The capability to the process. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Proc_Del(cid_t Cap_Captbl, cid_t Cap_Proc)
{
    20002150:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_PRC_DEL, Cap_Captbl,
    20002154:	48 b8 00 00 00 00 16 	movabs $0x1600000000,%rax
    2000215b:	00 00 00 
    2000215e:	31 c9                	xor    %ecx,%ecx
    20002160:	31 d2                	xor    %edx,%edx
    20002162:	48 09 c7             	or     %rax,%rdi
    20002165:	e9 83 df ff ff       	jmp    200000ed <UVM_Svc>
    2000216a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

0000000020002170 <UVM_Proc_Cpt>:
                                 this process. 2-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Proc_Cpt(cid_t Cap_Proc, cid_t Cap_Captbl)
{
    20002170:	f3 0f 1e fa          	endbr64 
    20002174:	48 89 f2             	mov    %rsi,%rdx
    return UVM_CAP_OP(RME_SVC_PRC_CPT, 0,
    20002177:	31 c9                	xor    %ecx,%ecx
    20002179:	48 89 fe             	mov    %rdi,%rsi
    2000217c:	48 bf 00 00 00 00 17 	movabs $0x1700000000,%rdi
    20002183:	00 00 00 
    20002186:	e9 62 df ff ff       	jmp    200000ed <UVM_Svc>
    2000218b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000020002190 <UVM_Proc_Pgt>:
                                process. 2-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Proc_Pgt(cid_t Cap_Proc, cid_t Cap_Pgtbl)
{
    20002190:	f3 0f 1e fa          	endbr64 
    20002194:	48 89 f2             	mov    %rsi,%rdx
    return UVM_CAP_OP(RME_SVC_PRC_PGT, 0,
    20002197:	31 c9                	xor    %ecx,%ecx
    20002199:	48 89 fe             	mov    %rdi,%rsi
    2000219c:	48 bf 00 00 00 00 18 	movabs $0x1800000000,%rdi
    200021a3:	00 00 00 
    200021a6:	e9 42 df ff ff       	jmp    200000ed <UVM_Svc>
    200021ab:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

00000000200021b0 <UVM_Thd_Crt>:
Output      : None.
Return      : ret_t - If successful, the Thread ID; or an error code.
******************************************************************************/
ret_t UVM_Thd_Crt(cid_t Cap_Captbl, cid_t Cap_Kmem, cid_t Cap_Thd,
                  cid_t Cap_Proc, ptr_t Max_Prio, ptr_t Vaddr)
{
    200021b0:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_THD_CRT, Cap_Captbl, 
    200021b4:	48 c1 e1 20          	shl    $0x20,%rcx
{
    200021b8:	49 89 d2             	mov    %rdx,%r10
    return UVM_CAP_OP(RME_SVC_THD_CRT, Cap_Captbl, 
    200021bb:	45 89 c0             	mov    %r8d,%r8d
    200021be:	48 c1 e6 20          	shl    $0x20,%rsi
    200021c2:	48 b8 00 00 00 00 19 	movabs $0x1900000000,%rax
    200021c9:	00 00 00 
    200021cc:	48 89 ca             	mov    %rcx,%rdx
    200021cf:	45 89 d2             	mov    %r10d,%r10d
    200021d2:	4c 89 c9             	mov    %r9,%rcx
    200021d5:	4c 09 c2             	or     %r8,%rdx
    200021d8:	4c 09 d6             	or     %r10,%rsi
    200021db:	48 09 c7             	or     %rax,%rdi
    200021de:	e9 0a df ff ff       	jmp    200000ed <UVM_Svc>
    200021e3:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    200021ea:	00 00 00 00 
    200021ee:	66 90                	xchg   %ax,%ax

00000000200021f0 <UVM_Thd_Del>:
              cid_t Cap_Thd - The capability to the thread in the captbl. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Del(cid_t Cap_Captbl, cid_t Cap_Thd)
{
    200021f0:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_THD_DEL, Cap_Captbl,
    200021f4:	48 b8 00 00 00 00 1a 	movabs $0x1a00000000,%rax
    200021fb:	00 00 00 
    200021fe:	31 c9                	xor    %ecx,%ecx
    20002200:	31 d2                	xor    %edx,%edx
    20002202:	48 09 c7             	or     %rax,%rdi
    20002205:	e9 e3 de ff ff       	jmp    200000ed <UVM_Svc>
    2000220a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

0000000020002210 <UVM_Thd_Exec_Set>:
              ptr_t Param - The parameter of the thread.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Exec_Set(cid_t Cap_Thd, ptr_t Entry, ptr_t Stack, ptr_t Param)
{
    20002210:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_THD_EXEC_SET, Cap_Thd,
    20002214:	48 b8 00 00 00 00 06 	movabs $0x600000000,%rax
    2000221b:	00 00 00 
    2000221e:	48 09 c7             	or     %rax,%rdi
    20002221:	e9 c7 de ff ff       	jmp    200000ed <UVM_Svc>
    20002226:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    2000222d:	00 00 00 

0000000020002230 <UVM_Thd_Hyp_Set>:
                            thread's register sets.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Hyp_Set(cid_t Cap_Thd, ptr_t Kaddr)
{
    20002230:	f3 0f 1e fa          	endbr64 
    20002234:	48 89 f2             	mov    %rsi,%rdx
    return UVM_CAP_OP(RME_SVC_THD_EXEC_SET, 0,
    20002237:	31 c9                	xor    %ecx,%ecx
    20002239:	48 89 fe             	mov    %rdi,%rsi
    2000223c:	48 bf 00 00 00 00 06 	movabs $0x600000000,%rdi
    20002243:	00 00 00 
    20002246:	e9 a2 de ff ff       	jmp    200000ed <UVM_Svc>
    2000224b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000020002250 <UVM_Thd_Sched_Bind>:
              ptr_t Prio - The priority level, higher is more critical.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Sched_Bind(cid_t Cap_Thd, cid_t Cap_Thd_Sched, cid_t Cap_Sig, tid_t TID, ptr_t Prio)
{
    20002250:	f3 0f 1e fa          	endbr64 
    20002254:	49 89 d1             	mov    %rdx,%r9
    return UVM_CAP_OP(RME_SVC_THD_SCHED_BIND, Cap_Thd,
    20002257:	48 c1 e6 20          	shl    $0x20,%rsi
{
    2000225b:	48 89 ca             	mov    %rcx,%rdx
    return UVM_CAP_OP(RME_SVC_THD_SCHED_BIND, Cap_Thd,
    2000225e:	48 b8 00 00 00 00 1b 	movabs $0x1b00000000,%rax
    20002265:	00 00 00 
    20002268:	45 89 c9             	mov    %r9d,%r9d
{
    2000226b:	4c 89 c1             	mov    %r8,%rcx
    return UVM_CAP_OP(RME_SVC_THD_SCHED_BIND, Cap_Thd,
    2000226e:	48 09 c7             	or     %rax,%rdi
    20002271:	4c 09 ce             	or     %r9,%rsi
    20002274:	e9 74 de ff ff       	jmp    200000ed <UVM_Svc>
    20002279:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000020002280 <UVM_Thd_Sched_Rcv>:
                              is simply not allowed. 2-Level.
Output      : None.
Return      : ret_t - If successful, the thread ID; or an error code.
******************************************************************************/
ret_t UVM_Thd_Sched_Rcv(cid_t Cap_Thd)
{
    20002280:	f3 0f 1e fa          	endbr64 
    20002284:	48 89 fe             	mov    %rdi,%rsi
    return UVM_CAP_OP(RME_SVC_THD_SCHED_RCV, 0,
    20002287:	31 c9                	xor    %ecx,%ecx
    20002289:	31 d2                	xor    %edx,%edx
    2000228b:	48 bf 00 00 00 00 1c 	movabs $0x1c00000000,%rdi
    20002292:	00 00 00 
    20002295:	e9 53 de ff ff       	jmp    200000ed <UVM_Svc>
    2000229a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

00000000200022a0 <UVM_Thd_Sched_Prio>:
              ptr_t Prio - The priority level, higher is more critical.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Sched_Prio(cid_t Cap_Thd, ptr_t Prio)
{
    200022a0:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_THD_SCHED_PRIO, 1,
    200022a4:	0f b7 ce             	movzwl %si,%ecx
    200022a7:	31 d2                	xor    %edx,%edx
    200022a9:	89 fe                	mov    %edi,%esi
    200022ab:	48 bf 01 00 00 00 07 	movabs $0x700000001,%rdi
    200022b2:	00 00 00 
    200022b5:	e9 33 de ff ff       	jmp    200000ed <UVM_Svc>
    200022ba:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

00000000200022c0 <UVM_Thd_Sched_Free>:
Input       : cid_t Cap_Thd - The capability to the thread. 2-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Sched_Free(cid_t Cap_Thd)
{
    200022c0:	f3 0f 1e fa          	endbr64 
    200022c4:	48 89 fe             	mov    %rdi,%rsi
    return UVM_CAP_OP(RME_SVC_THD_SCHED_FREE, 0,
    200022c7:	31 c9                	xor    %ecx,%ecx
    200022c9:	31 d2                	xor    %edx,%edx
    200022cb:	48 bf 00 00 00 00 05 	movabs $0x500000000,%rdi
    200022d2:	00 00 00 
    200022d5:	e9 13 de ff ff       	jmp    200000ed <UVM_Svc>
    200022da:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

00000000200022e0 <UVM_Thd_Time_Xfer>:
                           order of 100us or 1ms.
Output      : None.
Return      : ret_t - If successful, the destination time amount; or an error code.
******************************************************************************/
ret_t UVM_Thd_Time_Xfer(cid_t Cap_Thd_Dst, cid_t Cap_Thd_Src, ptr_t Time)
{
    200022e0:	f3 0f 1e fa          	endbr64 
    200022e4:	48 89 d1             	mov    %rdx,%rcx
    return UVM_CAP_OP(RME_SVC_THD_TIME_XFER, 0,
    200022e7:	48 89 f2             	mov    %rsi,%rdx
    200022ea:	48 89 fe             	mov    %rdi,%rsi
    200022ed:	48 bf 00 00 00 00 08 	movabs $0x800000000,%rdi
    200022f4:	00 00 00 
    200022f7:	e9 f1 dd ff ff       	jmp    200000ed <UVM_Svc>
    200022fc:	0f 1f 40 00          	nopl   0x0(%rax)

0000000020002300 <UVM_Thd_Swt>:
                                 this thread.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Thd_Swt(cid_t Cap_Thd, ptr_t Full_Yield)
{
    20002300:	f3 0f 1e fa          	endbr64 
    20002304:	48 89 f2             	mov    %rsi,%rdx
    return UVM_CAP_OP(RME_SVC_THD_SWT, 0,
    20002307:	31 c9                	xor    %ecx,%ecx
    20002309:	48 89 fe             	mov    %rdi,%rsi
    2000230c:	48 bf 00 00 00 00 09 	movabs $0x900000000,%rdi
    20002313:	00 00 00 
    20002316:	e9 d2 dd ff ff       	jmp    200000ed <UVM_Svc>
    2000231b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000020002320 <UVM_Sig_Crt>:
                            within the kernel virtual address.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Sig_Crt(cid_t Cap_Captbl, cid_t Cap_Kmem, cid_t Cap_Sig, ptr_t Vaddr)
{
    20002320:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_SIG_CRT, Cap_Captbl,
    20002324:	48 b8 00 00 00 00 1d 	movabs $0x1d00000000,%rax
    2000232b:	00 00 00 
{
    2000232e:	49 89 f0             	mov    %rsi,%r8
    20002331:	48 89 d6             	mov    %rdx,%rsi
    return UVM_CAP_OP(RME_SVC_SIG_CRT, Cap_Captbl,
    20002334:	48 09 c7             	or     %rax,%rdi
    20002337:	4c 89 c2             	mov    %r8,%rdx
    2000233a:	e9 ae dd ff ff       	jmp    200000ed <UVM_Svc>
    2000233f:	90                   	nop

0000000020002340 <UVM_Sig_Del>:
              cid_t Cap_Sig - The capability to the signal. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Sig_Del(cid_t Cap_Captbl, cid_t Cap_Sig)
{
    20002340:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_SIG_DEL, Cap_Captbl,
    20002344:	48 b8 00 00 00 00 1e 	movabs $0x1e00000000,%rax
    2000234b:	00 00 00 
    2000234e:	31 c9                	xor    %ecx,%ecx
    20002350:	31 d2                	xor    %edx,%edx
    20002352:	48 09 c7             	or     %rax,%rdi
    20002355:	e9 93 dd ff ff       	jmp    200000ed <UVM_Svc>
    2000235a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

0000000020002360 <UVM_Sig_Snd>:
              ptr_t Number- The Number of signals to send.
Output      : None.
Return      : ret_t - If successful, 0, or an error code.
******************************************************************************/
ret_t UVM_Sig_Snd(cid_t Cap_Sig,ptr_t Number)
{
    20002360:	f3 0f 1e fa          	endbr64 
    20002364:	48 89 f2             	mov    %rsi,%rdx
    return UVM_CAP_OP(RME_SVC_SIG_SND, 0,
    20002367:	31 c9                	xor    %ecx,%ecx
    20002369:	48 89 fe             	mov    %rdi,%rsi
    2000236c:	48 bf 00 00 00 00 02 	movabs $0x200000000,%rdi
    20002373:	00 00 00 
    20002376:	e9 72 dd ff ff       	jmp    200000ed <UVM_Svc>
    2000237b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000020002380 <UVM_Sig_Rcv>:
Output      : None.
Return      : ret_t - If successful, a non-negative number containing the number of signals
                      received will be returned; else an error code.
******************************************************************************/
ret_t UVM_Sig_Rcv(cid_t Cap_Sig, ptr_t Option)
{
    20002380:	f3 0f 1e fa          	endbr64 
    20002384:	48 89 f2             	mov    %rsi,%rdx
    return UVM_CAP_OP(RME_SVC_SIG_RCV, 0,
    20002387:	31 c9                	xor    %ecx,%ecx
    20002389:	48 89 fe             	mov    %rdi,%rsi
    2000238c:	48 bf 00 00 00 00 03 	movabs $0x300000000,%rdi
    20002393:	00 00 00 
    20002396:	e9 52 dd ff ff       	jmp    200000ed <UVM_Svc>
    2000239b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

00000000200023a0 <UVM_Inv_Crt>:
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Inv_Crt(cid_t Cap_Captbl, cid_t Cap_Kmem, 
                  cid_t Cap_Inv, cid_t Cap_Proc, ptr_t Vaddr)
{
    200023a0:	f3 0f 1e fa          	endbr64 
    200023a4:	49 89 d1             	mov    %rdx,%r9
    return UVM_CAP_OP(RME_SVC_INV_CRT, Cap_Captbl,
    200023a7:	48 c1 e6 20          	shl    $0x20,%rsi
{
    200023ab:	48 89 ca             	mov    %rcx,%rdx
    return UVM_CAP_OP(RME_SVC_INV_CRT, Cap_Captbl,
    200023ae:	48 b8 00 00 00 00 1f 	movabs $0x1f00000000,%rax
    200023b5:	00 00 00 
    200023b8:	45 89 c9             	mov    %r9d,%r9d
{
    200023bb:	4c 89 c1             	mov    %r8,%rcx
    return UVM_CAP_OP(RME_SVC_INV_CRT, Cap_Captbl,
    200023be:	48 09 c7             	or     %rax,%rdi
    200023c1:	4c 09 ce             	or     %r9,%rsi
    200023c4:	e9 24 dd ff ff       	jmp    200000ed <UVM_Svc>
    200023c9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

00000000200023d0 <UVM_Inv_Del>:
              cid_t Cap_Inv - The capability to the invocation stub. 1-Level.
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Inv_Del(cid_t Cap_Captbl, cid_t Cap_Inv)
{
    200023d0:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_INV_DEL, Cap_Captbl,
    200023d4:	48 0f ba ef 25       	bts    $0x25,%rdi
    200023d9:	31 c9                	xor    %ecx,%ecx
    200023db:	31 d2                	xor    %edx,%edx
    200023dd:	e9 0b dd ff ff       	jmp    200000ed <UVM_Svc>
    200023e2:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    200023e9:	00 00 00 00 
    200023ed:	0f 1f 00             	nopl   (%rax)

00000000200023f0 <UVM_Inv_Set>:
                                     immediately, or we wait for fault handling?
Output      : None.
Return      : ret_t - If successful, 0; or an error code.
******************************************************************************/
ret_t UVM_Inv_Set(cid_t Cap_Inv, ptr_t Entry, ptr_t Stack, ptr_t Fault_Ret_Flag)
{
    200023f0:	f3 0f 1e fa          	endbr64 
    return UVM_CAP_OP(RME_SVC_INV_SET, 0,
    200023f4:	48 c1 e1 20          	shl    $0x20,%rcx
{
    200023f8:	49 89 f0             	mov    %rsi,%r8
    return UVM_CAP_OP(RME_SVC_INV_SET, 0,
    200023fb:	89 ff                	mov    %edi,%edi
    200023fd:	48 89 ce             	mov    %rcx,%rsi
    20002400:	48 89 d1             	mov    %rdx,%rcx
    20002403:	4c 89 c2             	mov    %r8,%rdx
    20002406:	48 09 fe             	or     %rdi,%rsi
    20002409:	48 bf 00 00 00 00 21 	movabs $0x2100000000,%rdi
    20002410:	00 00 00 
    20002413:	e9 d5 dc ff ff       	jmp    200000ed <UVM_Svc>
    20002418:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    2000241f:	00 

0000000020002420 <UVM_Putchar>:
Input       : char Char - The character to print.
Output      : None.
Return      : ptr_t - Always 0.
******************************************************************************/
ptr_t UVM_Putchar(char Char)
{
    20002420:	f3 0f 1e fa          	endbr64 
    20002424:	41 54                	push   %r12
    20002426:	41 89 fc             	mov    %edi,%r12d
    /* Wait until we have transmitted */
    while((__UVM_X64_In(UVM_X64_COM1+5)&0x20)==0);
    20002429:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    20002430:	bf fd 03 00 00       	mov    $0x3fd,%edi
    20002435:	e8 cb db ff ff       	call   20000005 <__UVM_X64_In>
    2000243a:	a8 20                	test   $0x20,%al
    2000243c:	74 f2                	je     20002430 <UVM_Putchar+0x10>
    __UVM_X64_Out(UVM_X64_COM1, Char);
    2000243e:	4d 0f be e4          	movsbq %r12b,%r12
    20002442:	bf f8 03 00 00       	mov    $0x3f8,%edi
    20002447:	4c 89 e6             	mov    %r12,%rsi
    2000244a:	e8 c0 db ff ff       	call   2000000f <__UVM_X64_Out>
    UVM_Kern_Act(UVM_BOOT_INIT_KERN,0,0,(ptr_t)Char,0);
    2000244f:	4c 89 e1             	mov    %r12,%rcx
    20002452:	45 31 c0             	xor    %r8d,%r8d
    20002455:	31 d2                	xor    %edx,%edx
    20002457:	31 f6                	xor    %esi,%esi
    20002459:	bf 04 00 00 00       	mov    $0x4,%edi
    2000245e:	e8 8d fb ff ff       	call   20001ff0 <UVM_Kern_Act>
    return 0;
}
    20002463:	31 c0                	xor    %eax,%eax
    20002465:	41 5c                	pop    %r12
    20002467:	c3                   	ret    
    20002468:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    2000246f:	00 

0000000020002470 <_UVM_Stack_Init>:
Output      : None.
Return      : ptr_t - The actual stack address to use for system call.
******************************************************************************/
ptr_t _UVM_Stack_Init(ptr_t Stack, ptr_t Size, ptr_t Stub, ptr_t Entry,
                      ptr_t Param1, ptr_t Param2, ptr_t Param3, ptr_t Param4)
{
    20002470:	f3 0f 1e fa          	endbr64 
	ptr_t* Stack_Ptr;

    Stack_Ptr=(ptr_t*)(Stack+Size-UVM_STACK_SAFE_SIZE*sizeof(ptr_t));
    Stack_Ptr[0]=Param1;
    20002474:	66 49 0f 6e c0       	movq   %r8,%xmm0
    20002479:	66 49 0f 6e c9       	movq   %r9,%xmm1
    Stack_Ptr=(ptr_t*)(Stack+Size-UVM_STACK_SAFE_SIZE*sizeof(ptr_t));
    2000247e:	48 8d 84 37 00 ff ff 	lea    -0x100(%rdi,%rsi,1),%rax
    20002485:	ff 
    Stack_Ptr[0]=Param1;
    20002486:	66 0f 6c c1          	punpcklqdq %xmm1,%xmm0
    Stack_Ptr[1]=Param2;
    Stack_Ptr[2]=Param3;
    Stack_Ptr[3]=Param4;
    Stack_Ptr[4]=Entry;
    2000248a:	48 89 48 20          	mov    %rcx,0x20(%rax)
    Stack_Ptr[0]=Param1;
    2000248e:	0f 11 00             	movups %xmm0,(%rax)
    20002491:	f3 0f 7e 44 24 08    	movq   0x8(%rsp),%xmm0
    20002497:	0f 16 44 24 10       	movhps 0x10(%rsp),%xmm0
    2000249c:	0f 11 40 10          	movups %xmm0,0x10(%rax)

    return (ptr_t)Stack_Ptr;
}
    200024a0:	c3                   	ret    
    200024a1:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    200024a8:	00 00 00 00 
    200024ac:	0f 1f 40 00          	nopl   0x0(%rax)

00000000200024b0 <_UVM_Idle>:
Input       : None.
Output      : None.
Return      : None.
******************************************************************************/
void _UVM_Idle(void)
{
    200024b0:	f3 0f 1e fa          	endbr64 
    /* Do nothing. In the future we may call a kernel function to put us to sleep */
}
    200024b4:	c3                   	ret    
