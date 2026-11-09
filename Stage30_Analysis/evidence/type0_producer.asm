
SHA256-pinned AALib.exe window:     file format binary


Disassembly of section .data:

00156da0 <.data>:
  156da0:	55                                              	push   ebp
  156da1:	8b ec                                           	mov    ebp,esp
  156da3:	8b 45 08                                        	mov    eax,DWORD PTR [ebp+0x8]
  156da6:	53                                              	push   ebx
  156da7:	56                                              	push   esi
  156da8:	8b 30                                           	mov    esi,DWORD PTR [eax]
  156daa:	8b 40 08                                        	mov    eax,DWORD PTR [eax+0x8]
  156dad:	8b 50 04                                        	mov    edx,DWORD PTR [eax+0x4]
  156db0:	8b 4e 04                                        	mov    ecx,DWORD PTR [esi+0x4]
  156db3:	8b 41 24                                        	mov    eax,DWORD PTR [ecx+0x24]
  156db6:	d9 41 7c                                        	fld    DWORD PTR [ecx+0x7c]
  156db9:	d9 42 7c                                        	fld    DWORD PTR [edx+0x7c]
  156dbc:	c1 e8 03                                        	shr    eax,0x3
  156dbf:	f6 d0                                           	not    al
  156dc1:	de d9                                           	fcompp
  156dc3:	24 01                                           	and    al,0x1
  156dc5:	88 45 0b                                        	mov    BYTE PTR [ebp+0xb],al
  156dc8:	32 db                                           	xor    bl,bl
  156dca:	df e0                                           	fnstsw ax
  156dcc:	57                                              	push   edi
  156dcd:	f6 c4 41                                        	test   ah,0x41
  156dd0:	75 08                                           	jne    0x156dda
  156dd2:	38 5d 0b                                        	cmp    BYTE PTR [ebp+0xb],bl
  156dd5:	0f 94 c3                                        	sete   bl
  156dd8:	eb 12                                           	jmp    0x156dec
  156dda:	d9 41 7c                                        	fld    DWORD PTR [ecx+0x7c]
  156ddd:	d9 42 7c                                        	fld    DWORD PTR [edx+0x7c]
  156de0:	de d9                                           	fcompp
  156de2:	df e0                                           	fnstsw ax
  156de4:	f6 c4 05                                        	test   ah,0x5
  156de7:	7a 03                                           	jp     0x156dec
  156de9:	8a 5d 0b                                        	mov    bl,BYTE PTR [ebp+0xb]
  156dec:	8b 8e 70 3b 00 00                               	mov    ecx,DWORD PTR [esi+0x3b70]
  156df2:	c1 e9 05                                        	shr    ecx,0x5
  156df5:	f6 c1 01                                        	test   cl,0x1
  156df8:	74 10                                           	je     0x156e0a
  156dfa:	33 d2                                           	xor    edx,edx
  156dfc:	84 db                                           	test   bl,bl
  156dfe:	0f 95 c2                                        	setne  dl
  156e01:	8d 3c 95 01 00 00 00                            	lea    edi,[edx*4+0x1]
  156e08:	eb 0a                                           	jmp    0x156e14
  156e0a:	0f b6 fb                                        	movzx  edi,bl
  156e0d:	f7 df                                           	neg    edi
  156e0f:	1b ff                                           	sbb    edi,edi
  156e11:	83 e7 04                                        	and    edi,0x4
  156e14:	8b 5e 04                                        	mov    ebx,DWORD PTR [esi+0x4]
  156e17:	8b cb                                           	mov    ecx,ebx
  156e19:	e8 92 d4 08 00                                  	call   0x1e42b0
  156e1e:	d9 ee                                           	fldz
  156e20:	d8 93 80 00 00 00                               	fcom   DWORD PTR [ebx+0x80]
  156e26:	df e0                                           	fnstsw ax
  156e28:	f6 c4 41                                        	test   ah,0x41
  156e2b:	75 0e                                           	jne    0x156e3b
  156e2d:	d9 93 80 00 00 00                               	fst    DWORD PTR [ebx+0x80]
  156e33:	d9 9b b0 00 00 00                               	fstp   DWORD PTR [ebx+0xb0]
  156e39:	eb 02                                           	jmp    0x156e3d
  156e3b:	dd d8                                           	fstp   st(0)
  156e3d:	57                                              	push   edi
  156e3e:	8d 86 2c 3a 00 00                               	lea    eax,[esi+0x3a2c]
  156e44:	6a 00                                           	push   0x0
  156e46:	50                                              	push   eax
  156e47:	e8 94 bf f9 ff                                  	call   0xf2de0
  156e4c:	83 c4 0c                                        	add    esp,0xc
  156e4f:	81 8e 70 3b 00 00 00 00 02 00                   	or     DWORD PTR [esi+0x3b70],0x20000
  156e59:	5f                                              	pop    edi
  156e5a:	5e                                              	pop    esi
  156e5b:	5b                                              	pop    ebx
  156e5c:	5d                                              	pop    ebp
  156e5d:	c3                                              	ret
