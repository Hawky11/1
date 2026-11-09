
SHA256-pinned AALib.exe window:     file format binary


Disassembly of section .data:

00156c00 <.data>:
  156c00:	55                                              	push   ebp
  156c01:	8b ec                                           	mov    ebp,esp
  156c03:	8b 45 0c                                        	mov    eax,DWORD PTR [ebp+0xc]
  156c06:	8b 4d 08                                        	mov    ecx,DWORD PTR [ebp+0x8]
  156c09:	89 01                                           	mov    DWORD PTR [ecx],eax
  156c0b:	8b 90 d4 3e 00 00                               	mov    edx,DWORD PTR [eax+0x3ed4]
  156c11:	89 51 0c                                        	mov    DWORD PTR [ecx+0xc],edx
  156c14:	8b 10                                           	mov    edx,DWORD PTR [eax]
  156c16:	89 51 08                                        	mov    DWORD PTR [ecx+0x8],edx
  156c19:	8b 40 04                                        	mov    eax,DWORD PTR [eax+0x4]
  156c1c:	89 41 04                                        	mov    DWORD PTR [ecx+0x4],eax
  156c1f:	5d                                              	pop    ebp
  156c20:	c3                                              	ret
