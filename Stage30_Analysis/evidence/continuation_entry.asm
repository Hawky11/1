
SHA256-pinned AALib.exe window:     file format binary


Disassembly of section .data:

0012f3b0 <.data>:
  12f3b0:	55                                              	push   ebp
  12f3b1:	8b ec                                           	mov    ebp,esp
  12f3b3:	83 ec 14                                        	sub    esp,0x14
  12f3b6:	53                                              	push   ebx
  12f3b7:	56                                              	push   esi
  12f3b8:	57                                              	push   edi
  12f3b9:	8b 7d 08                                        	mov    edi,DWORD PTR [ebp+0x8]
  12f3bc:	8b 1f                                           	mov    ebx,DWORD PTR [edi]
  12f3be:	8b 73 04                                        	mov    esi,DWORD PTR [ebx+0x4]
  12f3c1:	8b 4e 20                                        	mov    ecx,DWORD PTR [esi+0x20]
  12f3c4:	d1 e9                                           	shr    ecx,1
  12f3c6:	f6 c1 01                                        	test   cl,0x1
  12f3c9:	74 36                                           	je     0x12f401
  12f3cb:	6a ff                                           	push   0xffffffff
  12f3cd:	68 20 6f 55 00                                  	push   0x556f20
  12f3d2:	81 c3 d0 3e 00 00                               	add    ebx,0x3ed0
  12f3d8:	53                                              	push   ebx
  12f3d9:	e8 92 8a fc ff                                  	call   0xf7e70
  12f3de:	8b 17                                           	mov    edx,DWORD PTR [edi]
  12f3e0:	83 c4 0c                                        	add    esp,0xc
  12f3e3:	8b ce                                           	mov    ecx,esi
  12f3e5:	c7 82 dc 3e 00 00 00 00 00 00                   	mov    DWORD PTR [edx+0x3edc],0x0
  12f3ef:	e8 7c 54 0b 00                                  	call   0x1e4870
  12f3f4:	5f                                              	pop    edi
  12f3f5:	89 86 90 02 00 00                               	mov    DWORD PTR [esi+0x290],eax
  12f3fb:	5e                                              	pop    esi
  12f3fc:	5b                                              	pop    ebx
  12f3fd:	8b e5                                           	mov    esp,ebp
  12f3ff:	5d                                              	pop    ebp
  12f400:	c3                                              	ret
  12f401:	8b 83 84 3b 00 00                               	mov    eax,DWORD PTR [ebx+0x3b84]
  12f407:	c1 e8 11                                        	shr    eax,0x11
  12f40a:	a8 01                                           	test   al,0x1
  12f40c:	0f 85 d7 06 00 00                               	jne    0x12fae9
