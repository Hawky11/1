
SHA256-pinned AALib.exe window:     file format binary


Disassembly of section .data:

000991c0 <.data>:
   991c0:	55                                              	push   ebp
   991c1:	8b ec                                           	mov    ebp,esp
   991c3:	8b 45 10                                        	mov    eax,DWORD PTR [ebp+0x10]
   991c6:	8b 55 0c                                        	mov    edx,DWORD PTR [ebp+0xc]
   991c9:	8b c8                                           	mov    ecx,eax
   991cb:	c1 e1 04                                        	shl    ecx,0x4
   991ce:	2b c8                                           	sub    ecx,eax
   991d0:	8b 45 08                                        	mov    eax,DWORD PTR [ebp+0x8]
   991d3:	03 c9                                           	add    ecx,ecx
   991d5:	03 c9                                           	add    ecx,ecx
   991d7:	03 c9                                           	add    ecx,ecx
   991d9:	89 4c 90 04                                     	mov    DWORD PTR [eax+edx*4+0x4],ecx
   991dd:	5d                                              	pop    ebp
   991de:	c3                                              	ret
