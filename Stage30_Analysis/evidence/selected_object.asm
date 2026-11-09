
SHA256-pinned AALib.exe window:     file format binary


Disassembly of section .data:

000e6960 <.data>:
   e6960:	56                                              	push   esi
   e6961:	8b f1                                           	mov    esi,ecx
   e6963:	83 be 78 39 00 00 1b                            	cmp    DWORD PTR [esi+0x3978],0x1b
   e696a:	75 22                                           	jne    0xe698e
   e696c:	83 be 34 3f 00 00 05                            	cmp    DWORD PTR [esi+0x3f34],0x5
   e6973:	7d 19                                           	jge    0xe698e
   e6975:	8b 86 30 3f 00 00                               	mov    eax,DWORD PTR [esi+0x3f30]
   e697b:	6a ff                                           	push   0xffffffff
   e697d:	50                                              	push   eax
   e697e:	e8 1d 85 fb ff                                  	call   0x9eea0
   e6983:	83 c4 08                                        	add    esp,0x8
   e6986:	3b f0                                           	cmp    esi,eax
   e6988:	74 04                                           	je     0xe698e
   e698a:	8b f0                                           	mov    esi,eax
   e698c:	eb d5                                           	jmp    0xe6963
   e698e:	8d 86 78 39 00 00                               	lea    eax,[esi+0x3978]
   e6994:	5e                                              	pop    esi
   e6995:	c3                                              	ret
