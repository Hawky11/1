
SHA256-pinned AALib.exe window:     file format binary


Disassembly of section .data:

000ed188 <.data>:
   ed188:	6a 1e                                           	push   0x1e
   ed18a:	8d 86 2c 3a 00 00                               	lea    eax,[esi+0x3a2c]
   ed190:	6a 01                                           	push   0x1
   ed192:	50                                              	push   eax
   ed193:	e8 38 5d 00 00                                  	call   0xf2ed0
   ed198:	83 c4 0c                                        	add    esp,0xc
   ed19b:	84 c0                                           	test   al,al
   ed19d:	75 0a                                           	jne    0xed1a9
   ed19f:	81 a6 84 3b 00 00 ff ff f9 ff                   	and    DWORD PTR [esi+0x3b84],0xfff9ffff
