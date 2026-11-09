
SHA256-pinned AALib.exe window:     file format binary


Disassembly of section .data:

000e6c60 <.data>:
   e6c60:	55                                              	push   ebp
   e6c61:	8b ec                                           	mov    ebp,esp
   e6c63:	56                                              	push   esi
   e6c64:	8b f1                                           	mov    esi,ecx
   e6c66:	81 8e 84 3b 00 00 00 00 01 00                   	or     DWORD PTR [esi+0x3b84],0x10000
   e6c70:	83 be 78 39 00 00 19                            	cmp    DWORD PTR [esi+0x3978],0x19
   e6c77:	57                                              	push   edi
   e6c78:	0f 85 a9 00 00 00                               	jne    0xe6d27
   e6c7e:	d9 05 88 63 6c 00                               	fld    DWORD PTR ds:0x6c6388
   e6c84:	53                                              	push   ebx
   e6c85:	d9 45 08                                        	fld    DWORD PTR [ebp+0x8]
   e6c88:	bb 02 00 00 00                                  	mov    ebx,0x2
   e6c8d:	d8 d1                                           	fcom   st(1)
   e6c8f:	df e0                                           	fnstsw ax
   e6c91:	dd d9                                           	fstp   st(1)
   e6c93:	f6 c4 01                                        	test   ah,0x1
   e6c96:	74 05                                           	je     0xe6c9d
   e6c98:	bb 01 00 00 00                                  	mov    ebx,0x1
   e6c9d:	db 86 6c 3f 00 00                               	fild   DWORD PTR [esi+0x3f6c]
   e6ca3:	dc 0d 38 ae 6f 00                               	fmul   QWORD PTR ds:0x6fae38
   e6ca9:	de c1                                           	faddp  st(1),st
   e6cab:	d9 5d 08                                        	fstp   DWORD PTR [ebp+0x8]
   e6cae:	d9 05 e0 e2 7f 00                               	fld    DWORD PTR ds:0x7fe2e0
   e6cb4:	d8 55 08                                        	fcom   DWORD PTR [ebp+0x8]
   e6cb7:	df e0                                           	fnstsw ax
   e6cb9:	f6 c4 41                                        	test   ah,0x41
   e6cbc:	74 05                                           	je     0xe6cc3
   e6cbe:	d9 5d 08                                        	fstp   DWORD PTR [ebp+0x8]
   e6cc1:	eb 02                                           	jmp    0xe6cc5
   e6cc3:	dd d8                                           	fstp   st(0)
   e6cc5:	33 ff                                           	xor    edi,edi
   e6cc7:	85 db                                           	test   ebx,ebx
   e6cc9:	7e 5b                                           	jle    0xe6d26
   e6ccb:	eb 03                                           	jmp    0xe6cd0
   e6ccd:	8d 49 00                                        	lea    ecx,[ecx+0x0]
   e6cd0:	8b 86 6c 3f 00 00                               	mov    eax,DWORD PTR [esi+0x3f6c]
   e6cd6:	83 f8 10                                        	cmp    eax,0x10
   e6cd9:	7d 46                                           	jge    0xe6d21
   e6cdb:	40                                              	inc    eax
   e6cdc:	89 86 6c 3f 00 00                               	mov    DWORD PTR [esi+0x3f6c],eax
   e6ce2:	85 ff                                           	test   edi,edi
   e6ce4:	75 1e                                           	jne    0xe6d04
   e6ce6:	8b 86 30 3f 00 00                               	mov    eax,DWORD PTR [esi+0x3f30]
   e6cec:	69 c0 98 00 00 00                               	imul   eax,eax,0x98
   e6cf2:	05 d0 17 0a 0e                                  	add    eax,0xe0a17d0
   e6cf7:	6a 36                                           	push   0x36
   e6cf9:	50                                              	push   eax
   e6cfa:	e8 61 c5 fb ff                                  	call   0xa3260
   e6cff:	83 c4 08                                        	add    esp,0x8
   e6d02:	eb 1d                                           	jmp    0xe6d21
   e6d04:	6a 05                                           	push   0x5
   e6d06:	8d 8e 3c 3a 00 00                               	lea    ecx,[esi+0x3a3c]
   e6d0c:	6a 40                                           	push   0x40
   e6d0e:	51                                              	push   ecx
   e6d0f:	c7 86 4c 43 00 00 36 00 00 00                   	mov    DWORD PTR [esi+0x434c],0x36
   e6d19:	e8 a2 24 fb ff                                  	call   0x991c0
   e6d1e:	83 c4 0c                                        	add    esp,0xc
   e6d21:	47                                              	inc    edi
   e6d22:	3b fb                                           	cmp    edi,ebx
   e6d24:	7c aa                                           	jl     0xe6cd0
   e6d26:	5b                                              	pop    ebx
   e6d27:	83 be 78 39 00 00 1b                            	cmp    DWORD PTR [esi+0x3978],0x1b
   e6d2e:	75 29                                           	jne    0xe6d59
   e6d30:	83 be 34 3f 00 00 05                            	cmp    DWORD PTR [esi+0x3f34],0x5
   e6d37:	7d 20                                           	jge    0xe6d59
   e6d39:	8b 96 30 3f 00 00                               	mov    edx,DWORD PTR [esi+0x3f30]
   e6d3f:	6a ff                                           	push   0xffffffff
   e6d41:	52                                              	push   edx
   e6d42:	e8 59 81 fb ff                                  	call   0x9eea0
   e6d47:	83 c4 08                                        	add    esp,0x8
   e6d4a:	3b f0                                           	cmp    esi,eax
   e6d4c:	74 0b                                           	je     0xe6d59
   e6d4e:	8b c8                                           	mov    ecx,eax
   e6d50:	e8 0b fc ff ff                                  	call   0xe6960
   e6d55:	8b f0                                           	mov    esi,eax
   e6d57:	eb 06                                           	jmp    0xe6d5f
   e6d59:	81 c6 78 39 00 00                               	add    esi,0x3978
   e6d5f:	d9 45 08                                        	fld    DWORD PTR [ebp+0x8]
   e6d62:	dc 0d c8 6a 6c 00                               	fmul   QWORD PTR ds:0x6c6ac8
   e6d68:	e8 13 1b 12 00                                  	call   0x208880
   e6d6d:	50                                              	push   eax
   e6d6e:	56                                              	push   esi
   e6d6f:	e8 1c e6 00 00                                  	call   0xf5390
   e6d74:	83 c4 08                                        	add    esp,0x8
   e6d77:	5f                                              	pop    edi
   e6d78:	5e                                              	pop    esi
   e6d79:	5d                                              	pop    ebp
   e6d7a:	c2 04 00                                        	ret    0x4
