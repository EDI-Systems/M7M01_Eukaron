                 .code16
                 .global        Start_16
Start_16:        JMP            .
                 CLI
                 XORW           %AX,%AX
                 MOVW           %AX,%DS
                 MOVW           %AX,%ES
                 MOVW           %AX,%SS
                 LGDT           Boot_GDT_Desc_16
                 MOV            %CR0,%EAX
                 ORL            $0x01,%EAX
                 MOVL           %EAX,%CR0
                 LJMPL          $8,$(Boot_32)
                 .code32
Boot_32:
                 MOVW           $16,%AX
                 MOVW           %AX,%DS
                 MOVW           %AX,%ES
                 MOVW           %AX,%SS
                 XORW           %AX,%AX
                 MOVW           %AX,%FS
                 MOVW           %AX,%GS
                 MOV            $1,%EBX
                 MOVL           (Start_16-4),%ESP
                 CALL           *(Start_16-8)
                 JMP            .

                 .p2align 2
Boot_GDT_16:
                 .word          0x0000,0x0000,0x0000,0x0000;
                 .word          0xFFFF,0x0000
                 .byte          0x00,0x9A,0xCF,0x00
                 .word          0xFFFF,0x0000
                 .byte          0x00,0x92,0xCF,0x00
Boot_GDT_Desc_16:
                 .word          (Boot_GDT_Desc_16-Boot_GDT_16-1)
                 .long          Boot_GDT_16
