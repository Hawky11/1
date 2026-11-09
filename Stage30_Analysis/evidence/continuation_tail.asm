
SHA256-pinned AALib.exe window:     file format binary


Disassembly of section .data:

0012fae9 <.data>:
  12fae9:	8b 1f                                           	mov    ebx,DWORD PTR [edi]
  12faeb:	8b 8b 7c 39 00 00                               	mov    ecx,DWORD PTR [ebx+0x397c]
  12faf1:	c1 e1 06                                        	shl    ecx,0x6
  12faf4:	8b 91 e4 1f 22 0a                               	mov    edx,DWORD PTR [ecx+0xa221fe4]
  12fafa:	52                                              	push   edx
  12fafb:	8d 83 3c 3a 00 00                               	lea    eax,[ebx+0x3a3c]
  12fb01:	6a 38                                           	push   0x38
  12fb03:	50                                              	push   eax
  12fb04:	e8 b7 96 f6 ff                                  	call   0x991c0
  12fb09:	83 c4 0c                                        	add    esp,0xc
  12fb0c:	e8 df a8 fc ff                                  	call   0xfa3f0
  12fb11:	8b cb                                           	mov    ecx,ebx
  12fb13:	e8 08 73 fb ff                                  	call   0xe6e20
  12fb18:	84 c0                                           	test   al,al
  12fb1a:	74 36                                           	je     0x12fb52
  12fb1c:	8b 8b 84 3b 00 00                               	mov    ecx,DWORD PTR [ebx+0x3b84]
  12fb22:	c1 e9 11                                        	shr    ecx,0x11
  12fb25:	f6 c1 01                                        	test   cl,0x1
  12fb28:	74 28                                           	je     0x12fb52
  12fb2a:	8b 17                                           	mov    edx,DWORD PTR [edi]
  12fb2c:	6a ff                                           	push   0xffffffff
  12fb2e:	81 c2 d0 3e 00 00                               	add    edx,0x3ed0
  12fb34:	68 20 6f 55 00                                  	push   0x556f20
  12fb39:	52                                              	push   edx
  12fb3a:	e8 31 83 fc ff                                  	call   0xf7e70
  12fb3f:	8b 07                                           	mov    eax,DWORD PTR [edi]
  12fb41:	83 c4 0c                                        	add    esp,0xc
  12fb44:	c7 80 dc 3e 00 00 00 00 00 00                   	mov    DWORD PTR [eax+0x3edc],0x0
  12fb4e:	6a 01                                           	push   0x1
  12fb50:	eb 20                                           	jmp    0x12fb72
  12fb52:	57                                              	push   edi
  12fb53:	e8 48 72 02 00                                  	call   0x156da0
  12fb58:	83 c4 04                                        	add    esp,0x4
  12fb5b:	8b ce                                           	mov    ecx,esi
  12fb5d:	e8 4e 47 0b 00                                  	call   0x1e42b0
  12fb62:	d9 ee                                           	fldz
  12fb64:	d9 96 50 01 00 00                               	fst    DWORD PTR [esi+0x150]
  12fb6a:	6a 00                                           	push   0x0
  12fb6c:	d9 9e 54 01 00 00                               	fstp   DWORD PTR [esi+0x154]
  12fb72:	8b cb                                           	mov    ecx,ebx
  12fb74:	e8 e7 6d fb ff                                  	call   0xe6960
  12fb79:	50                                              	push   eax
  12fb7a:	e8 21 58 fc ff                                  	call   0xf53a0
  12fb7f:	83 c4 08                                        	add    esp,0x8
  12fb82:	8b ce                                           	mov    ecx,esi
  12fb84:	e8 e7 4c 0b 00                                  	call   0x1e4870
  12fb89:	5f                                              	pop    edi
  12fb8a:	89 86 90 02 00 00                               	mov    DWORD PTR [esi+0x290],eax
  12fb90:	5e                                              	pop    esi
  12fb91:	5b                                              	pop    ebx
  12fb92:	8b e5                                           	mov    esp,ebp
  12fb94:	5d                                              	pop    ebp
  12fb95:	c3                                              	ret
