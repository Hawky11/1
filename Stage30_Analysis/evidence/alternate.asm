
SHA256-pinned AALib.exe window:     file format binary


Disassembly of section .data:

00130a10 <.data>:
  130a10:	55                                              	push   ebp
  130a11:	8b ec                                           	mov    ebp,esp
  130a13:	83 ec 14                                        	sub    esp,0x14
  130a16:	8b 45 08                                        	mov    eax,DWORD PTR [ebp+0x8]
  130a19:	53                                              	push   ebx
  130a1a:	56                                              	push   esi
  130a1b:	8b 30                                           	mov    esi,DWORD PTR [eax]
  130a1d:	8b 86 84 3b 00 00                               	mov    eax,DWORD PTR [esi+0x3b84]
  130a23:	8b 8e 78 39 00 00                               	mov    ecx,DWORD PTR [esi+0x3978]
  130a29:	8b d0                                           	mov    edx,eax
  130a2b:	c1 ea 11                                        	shr    edx,0x11
  130a2e:	33 db                                           	xor    ebx,ebx
  130a30:	57                                              	push   edi
  130a31:	8b 7e 04                                        	mov    edi,DWORD PTR [esi+0x4]
  130a34:	89 4d f0                                        	mov    DWORD PTR [ebp-0x10],ecx
  130a37:	f6 c2 01                                        	test   dl,0x1
  130a3a:	75 21                                           	jne    0x130a5d
  130a3c:	8b c8                                           	mov    ecx,eax
  130a3e:	c1 e9 13                                        	shr    ecx,0x13
  130a41:	f6 c1 01                                        	test   cl,0x1
  130a44:	75 17                                           	jne    0x130a5d
  130a46:	8b d0                                           	mov    edx,eax
  130a48:	c1 ea 14                                        	shr    edx,0x14
  130a4b:	f6 c2 01                                        	test   dl,0x1
  130a4e:	75 0d                                           	jne    0x130a5d
  130a50:	8b c8                                           	mov    ecx,eax
  130a52:	c1 e9 15                                        	shr    ecx,0x15
  130a55:	88 5d fe                                        	mov    BYTE PTR [ebp-0x2],bl
  130a58:	f6 c1 01                                        	test   cl,0x1
  130a5b:	74 04                                           	je     0x130a61
  130a5d:	c6 45 fe 01                                     	mov    BYTE PTR [ebp-0x2],0x1
  130a61:	8b d0                                           	mov    edx,eax
  130a63:	c1 ea 14                                        	shr    edx,0x14
  130a66:	f6 c2 01                                        	test   dl,0x1
  130a69:	75 0a                                           	jne    0x130a75
  130a6b:	c1 e8 15                                        	shr    eax,0x15
  130a6e:	88 5d ff                                        	mov    BYTE PTR [ebp-0x1],bl
  130a71:	a8 01                                           	test   al,0x1
  130a73:	74 04                                           	je     0x130a79
  130a75:	c6 45 ff 01                                     	mov    BYTE PTR [ebp-0x1],0x1
  130a79:	53                                              	push   ebx
  130a7a:	56                                              	push   esi
  130a7b:	89 9e 38 42 00 00                               	mov    DWORD PTR [esi+0x4238],ebx
  130a81:	89 9e 3c 42 00 00                               	mov    DWORD PTR [esi+0x423c],ebx
  130a87:	e8 84 50 ff ff                                  	call   0x125b10
  130a8c:	98                                              	cwde
  130a8d:	89 45 f4                                        	mov    DWORD PTR [ebp-0xc],eax
  130a90:	53                                              	push   ebx
  130a91:	56                                              	push   esi
  130a92:	db 45 f4                                        	fild   DWORD PTR [ebp-0xc]
  130a95:	d9 5d f4                                        	fstp   DWORD PTR [ebp-0xc]
  130a98:	e8 d3 50 ff ff                                  	call   0x125b70
  130a9d:	0f bf c8                                        	movsx  ecx,ax
  130aa0:	89 4d f8                                        	mov    DWORD PTR [ebp-0x8],ecx
  130aa3:	83 c4 10                                        	add    esp,0x10
  130aa6:	db 45 f8                                        	fild   DWORD PTR [ebp-0x8]
  130aa9:	d9 5d f8                                        	fstp   DWORD PTR [ebp-0x8]
  130aac:	38 5d ff                                        	cmp    BYTE PTR [ebp-0x1],bl
  130aaf:	75 66                                           	jne    0x130b17
  130ab1:	d9 45 f8                                        	fld    DWORD PTR [ebp-0x8]
  130ab4:	83 ec 08                                        	sub    esp,0x8
  130ab7:	d8 87 80 00 00 00                               	fadd   DWORD PTR [edi+0x80]
  130abd:	8d 8e 58 39 00 00                               	lea    ecx,[esi+0x3958]
  130ac3:	d9 5d ec                                        	fstp   DWORD PTR [ebp-0x14]
  130ac6:	d9 45 ec                                        	fld    DWORD PTR [ebp-0x14]
  130ac9:	d9 5c 24 04                                     	fstp   DWORD PTR [esp+0x4]
  130acd:	d9 47 7c                                        	fld    DWORD PTR [edi+0x7c]
  130ad0:	d8 45 f4                                        	fadd   DWORD PTR [ebp-0xc]
  130ad3:	d9 5d ec                                        	fstp   DWORD PTR [ebp-0x14]
  130ad6:	d9 45 ec                                        	fld    DWORD PTR [ebp-0x14]
  130ad9:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130adc:	57                                              	push   edi
  130add:	6a 49                                           	push   0x49
  130adf:	e8 bc de f8 ff                                  	call   0xbe9a0
  130ae4:	d9 45 f8                                        	fld    DWORD PTR [ebp-0x8]
  130ae7:	d8 87 80 00 00 00                               	fadd   DWORD PTR [edi+0x80]
  130aed:	83 ec 08                                        	sub    esp,0x8
  130af0:	8d 8e 58 39 00 00                               	lea    ecx,[esi+0x3958]
  130af6:	d9 5d ec                                        	fstp   DWORD PTR [ebp-0x14]
  130af9:	d9 45 ec                                        	fld    DWORD PTR [ebp-0x14]
  130afc:	d9 5c 24 04                                     	fstp   DWORD PTR [esp+0x4]
  130b00:	d9 47 7c                                        	fld    DWORD PTR [edi+0x7c]
  130b03:	d8 45 f4                                        	fadd   DWORD PTR [ebp-0xc]
  130b06:	d9 5d ec                                        	fstp   DWORD PTR [ebp-0x14]
  130b09:	d9 45 ec                                        	fld    DWORD PTR [ebp-0x14]
  130b0c:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130b0f:	57                                              	push   edi
  130b10:	6a 4a                                           	push   0x4a
  130b12:	e8 89 de f8 ff                                  	call   0xbe9a0
  130b17:	d9 45 f8                                        	fld    DWORD PTR [ebp-0x8]
  130b1a:	83 ec 08                                        	sub    esp,0x8
  130b1d:	d9 5c 24 04                                     	fstp   DWORD PTR [esp+0x4]
  130b21:	8d 8e 58 39 00 00                               	lea    ecx,[esi+0x3958]
  130b27:	d9 45 f4                                        	fld    DWORD PTR [ebp-0xc]
  130b2a:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130b2d:	57                                              	push   edi
  130b2e:	6a 4b                                           	push   0x4b
  130b30:	e8 6b de f8 ff                                  	call   0xbe9a0
  130b35:	d9 45 f8                                        	fld    DWORD PTR [ebp-0x8]
  130b38:	83 ec 08                                        	sub    esp,0x8
  130b3b:	d9 5c 24 04                                     	fstp   DWORD PTR [esp+0x4]
  130b3f:	8d 8e 58 39 00 00                               	lea    ecx,[esi+0x3958]
  130b45:	d9 45 f4                                        	fld    DWORD PTR [ebp-0xc]
  130b48:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130b4b:	57                                              	push   edi
  130b4c:	6a 4c                                           	push   0x4c
  130b4e:	e8 4d de f8 ff                                  	call   0xbe9a0
  130b53:	d9 45 f8                                        	fld    DWORD PTR [ebp-0x8]
  130b56:	83 ec 08                                        	sub    esp,0x8
  130b59:	d9 5c 24 04                                     	fstp   DWORD PTR [esp+0x4]
  130b5d:	8d 8e 58 39 00 00                               	lea    ecx,[esi+0x3958]
  130b63:	d9 45 f4                                        	fld    DWORD PTR [ebp-0xc]
  130b66:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130b69:	57                                              	push   edi
  130b6a:	6a 4d                                           	push   0x4d
  130b6c:	e8 2f de f8 ff                                  	call   0xbe9a0
  130b71:	d9 ee                                           	fldz
  130b73:	38 5d ff                                        	cmp    BYTE PTR [ebp-0x1],bl
  130b76:	75 5b                                           	jne    0x130bd3
  130b78:	d9 e8                                           	fld1
  130b7a:	8b 45 f0                                        	mov    eax,DWORD PTR [ebp-0x10]
  130b7d:	83 ec 08                                        	sub    esp,0x8
  130b80:	d9 54 24 04                                     	fst    DWORD PTR [esp+0x4]
  130b84:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130b87:	83 f8 10                                        	cmp    eax,0x10
  130b8a:	75 3d                                           	jne    0x130bc9
  130b8c:	50                                              	push   eax
  130b8d:	50                                              	push   eax
  130b8e:	50                                              	push   eax
  130b8f:	6a 21                                           	push   0x21
  130b91:	50                                              	push   eax
  130b92:	68 10 27 00 00                                  	push   0x2710
  130b97:	6a 64                                           	push   0x64
  130b99:	83 ec 14                                        	sub    esp,0x14
  130b9c:	d9 54 24 10                                     	fst    DWORD PTR [esp+0x10]
  130ba0:	d9 5c 24 0c                                     	fstp   DWORD PTR [esp+0xc]
  130ba4:	d9 05 a0 5f 6c 00                               	fld    DWORD PTR ds:0x6c5fa0
  130baa:	d9 5c 24 08                                     	fstp   DWORD PTR [esp+0x8]
  130bae:	d9 87 80 00 00 00                               	fld    DWORD PTR [edi+0x80]
  130bb4:	d9 5c 24 04                                     	fstp   DWORD PTR [esp+0x4]
  130bb8:	d9 47 7c                                        	fld    DWORD PTR [edi+0x7c]
  130bbb:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130bbe:	56                                              	push   esi
  130bbf:	e8 ec 86 fa ff                                  	call   0xd92b0
  130bc4:	83 c4 3c                                        	add    esp,0x3c
  130bc7:	eb 0c                                           	jmp    0x130bd5
  130bc9:	6a 21                                           	push   0x21
  130bcb:	6a 21                                           	push   0x21
  130bcd:	6a 21                                           	push   0x21
  130bcf:	50                                              	push   eax
  130bd0:	50                                              	push   eax
  130bd1:	eb bf                                           	jmp    0x130b92
  130bd3:	dd d8                                           	fstp   st(0)
  130bd5:	8b 5d 08                                        	mov    ebx,DWORD PTR [ebp+0x8]
  130bd8:	8b 0b                                           	mov    ecx,DWORD PTR [ebx]
  130bda:	6a 00                                           	push   0x0
  130bdc:	e8 7f 79 fb ff                                  	call   0xe8560
  130be1:	53                                              	push   ebx
  130be2:	e8 49 60 02 00                                  	call   0x156c30
  130be7:	83 67 1c e7                                     	and    DWORD PTR [edi+0x1c],0xffffffe7
  130beb:	83 c4 04                                        	add    esp,0x4
  130bee:	80 7d fe 00                                     	cmp    BYTE PTR [ebp-0x2],0x0
  130bf2:	75 1c                                           	jne    0x130c10
  130bf4:	8d 9f c8 01 00 00                               	lea    ebx,[edi+0x1c8]
  130bfa:	53                                              	push   ebx
  130bfb:	e8 90 ac 09 00                                  	call   0x1cb890
  130c00:	83 c4 04                                        	add    esp,0x4
  130c03:	84 c0                                           	test   al,al
  130c05:	74 06                                           	je     0x130c0d
  130c07:	8b 53 24                                        	mov    edx,DWORD PTR [ebx+0x24]
  130c0a:	01 53 10                                        	add    DWORD PTR [ebx+0x10],edx
  130c0d:	8b 5d 08                                        	mov    ebx,DWORD PTR [ebp+0x8]
  130c10:	8b 03                                           	mov    eax,DWORD PTR [ebx]
  130c12:	8b 48 04                                        	mov    ecx,DWORD PTR [eax+0x4]
  130c15:	c7 81 e4 01 00 00 00 00 00 00                   	mov    DWORD PTR [ecx+0x1e4],0x0
  130c1f:	c7 87 90 02 00 00 ff ff ff ff                   	mov    DWORD PTR [edi+0x290],0xffffffff
  130c29:	81 8e 80 3b 00 00 00 00 01 00                   	or     DWORD PTR [esi+0x3b80],0x10000
  130c33:	8b ce                                           	mov    ecx,esi
  130c35:	e8 26 5d fb ff                                  	call   0xe6960
  130c3a:	50                                              	push   eax
  130c3b:	e8 10 47 fc ff                                  	call   0xf5350
  130c40:	83 c4 04                                        	add    esp,0x4
  130c43:	6a 01                                           	push   0x1
  130c45:	8b ce                                           	mov    ecx,esi
  130c47:	e8 14 5d fb ff                                  	call   0xe6960
  130c4c:	50                                              	push   eax
  130c4d:	e8 4e 47 fc ff                                  	call   0xf53a0
  130c52:	8b 86 84 3b 00 00                               	mov    eax,DWORD PTR [esi+0x3b84]
  130c58:	8b d0                                           	mov    edx,eax
  130c5a:	c1 ea 14                                        	shr    edx,0x14
  130c5d:	83 c4 08                                        	add    esp,0x8
  130c60:	f6 c2 01                                        	test   dl,0x1
  130c63:	74 18                                           	je     0x130c7d
  130c65:	d9 05 e0 e2 7f 00                               	fld    DWORD PTR ds:0x7fe2e0
  130c6b:	51                                              	push   ecx
  130c6c:	8b ce                                           	mov    ecx,esi
  130c6e:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130c71:	e8 ea 5f fb ff                                  	call   0xe6c60
  130c76:	5f                                              	pop    edi
  130c77:	5e                                              	pop    esi
  130c78:	5b                                              	pop    ebx
  130c79:	8b e5                                           	mov    esp,ebp
  130c7b:	5d                                              	pop    ebp
  130c7c:	c3                                              	ret
  130c7d:	c1 e8 15                                        	shr    eax,0x15
  130c80:	a8 01                                           	test   al,0x1
  130c82:	74 18                                           	je     0x130c9c
  130c84:	d9 05 10 07 80 00                               	fld    DWORD PTR ds:0x800710
  130c8a:	51                                              	push   ecx
  130c8b:	8b ce                                           	mov    ecx,esi
  130c8d:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130c90:	e8 cb 5f fb ff                                  	call   0xe6c60
  130c95:	5f                                              	pop    edi
  130c96:	5e                                              	pop    esi
  130c97:	5b                                              	pop    ebx
  130c98:	8b e5                                           	mov    esp,ebp
  130c9a:	5d                                              	pop    ebp
  130c9b:	c3                                              	ret
  130c9c:	80 7d fe 00                                     	cmp    BYTE PTR [ebp-0x2],0x0
  130ca0:	0f 84 95 01 00 00                               	je     0x130e3b
  130ca6:	d9 05 60 47 7e 00                               	fld    DWORD PTR ds:0x7e4760
  130cac:	51                                              	push   ecx
  130cad:	8b ce                                           	mov    ecx,esi
  130caf:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130cb2:	e8 a9 5f fb ff                                  	call   0xe6c60
  130cb7:	8b 4b 08                                        	mov    ecx,DWORD PTR [ebx+0x8]
  130cba:	e8 d1 5d fb ff                                  	call   0xe6a90
  130cbf:	84 c0                                           	test   al,al
  130cc1:	74 13                                           	je     0x130cd6
  130cc3:	8b 43 08                                        	mov    eax,DWORD PTR [ebx+0x8]
  130cc6:	05 94 3b 00 00                                  	add    eax,0x3b94
  130ccb:	6a 5b                                           	push   0x5b
  130ccd:	50                                              	push   eax
  130cce:	e8 9d 3e fc ff                                  	call   0xf4b70
  130cd3:	83 c4 08                                        	add    esp,0x8
  130cd6:	8b ce                                           	mov    ecx,esi
  130cd8:	e8 43 61 fb ff                                  	call   0xe6e20
  130cdd:	84 c0                                           	test   al,al
  130cdf:	74 0b                                           	je     0x130cec
  130ce1:	6a 01                                           	push   0x1
  130ce3:	53                                              	push   ebx
  130ce4:	e8 47 f1 ed ff                                  	call   0xfe30
  130ce9:	83 c4 08                                        	add    esp,0x8
  130cec:	8b 8e 84 3b 00 00                               	mov    ecx,DWORD PTR [esi+0x3b84]
  130cf2:	c1 e9 13                                        	shr    ecx,0x13
  130cf5:	f6 c1 01                                        	test   cl,0x1
  130cf8:	75 09                                           	jne    0x130d03
  130cfa:	53                                              	push   ebx
  130cfb:	e8 b0 e6 ff ff                                  	call   0x12f3b0
  130d00:	83 c4 04                                        	add    esp,0x4
  130d03:	8b ce                                           	mov    ecx,esi
  130d05:	e8 26 61 fb ff                                  	call   0xe6e30
  130d0a:	84 c0                                           	test   al,al
  130d0c:	74 07                                           	je     0x130d15
  130d0e:	b9 16 00 00 00                                  	mov    ecx,0x16
  130d13:	eb 19                                           	jmp    0x130d2e
  130d15:	8b 8e 20 3f 00 00                               	mov    ecx,DWORD PTR [esi+0x3f20]
  130d1b:	83 f9 17                                        	cmp    ecx,0x17
  130d1e:	75 0e                                           	jne    0x130d2e
  130d20:	83 be 78 39 00 00 1a                            	cmp    DWORD PTR [esi+0x3978],0x1a
  130d27:	75 05                                           	jne    0x130d2e
  130d29:	b9 1a 00 00 00                                  	mov    ecx,0x1a
  130d2e:	83 c8 ff                                        	or     eax,0xffffffff
  130d31:	83 f9 1e                                        	cmp    ecx,0x1e
  130d34:	0f 87 ec 00 00 00                               	ja     0x130e26
  130d3a:	ff 24 8d a0 13 53 00                            	jmp    DWORD PTR [ecx*4+0x5313a0]
  130d41:	b8 63 00 00 00                                  	mov    eax,0x63
  130d46:	e9 db 00 00 00                                  	jmp    0x130e26
  130d4b:	b8 d3 00 00 00                                  	mov    eax,0xd3
  130d50:	e9 d1 00 00 00                                  	jmp    0x130e26
  130d55:	b8 4a 01 00 00                                  	mov    eax,0x14a
  130d5a:	e9 c7 00 00 00                                  	jmp    0x130e26
  130d5f:	b8 d3 01 00 00                                  	mov    eax,0x1d3
  130d64:	e9 bd 00 00 00                                  	jmp    0x130e26
  130d69:	b8 8e 02 00 00                                  	mov    eax,0x28e
  130d6e:	e9 b3 00 00 00                                  	jmp    0x130e26
  130d73:	b8 23 03 00 00                                  	mov    eax,0x323
  130d78:	e9 a9 00 00 00                                  	jmp    0x130e26
  130d7d:	b8 a8 03 00 00                                  	mov    eax,0x3a8
  130d82:	e9 9f 00 00 00                                  	jmp    0x130e26
  130d87:	b8 42 04 00 00                                  	mov    eax,0x442
  130d8c:	e9 95 00 00 00                                  	jmp    0x130e26
  130d91:	b8 ec 04 00 00                                  	mov    eax,0x4ec
  130d96:	e9 8b 00 00 00                                  	jmp    0x130e26
  130d9b:	b8 88 05 00 00                                  	mov    eax,0x588
  130da0:	e9 81 00 00 00                                  	jmp    0x130e26
  130da5:	b8 0e 06 00 00                                  	mov    eax,0x60e
  130daa:	eb 7a                                           	jmp    0x130e26
  130dac:	83 c8 ff                                        	or     eax,0xffffffff
  130daf:	eb 75                                           	jmp    0x130e26
  130db1:	b8 93 06 00 00                                  	mov    eax,0x693
  130db6:	eb 6e                                           	jmp    0x130e26
  130db8:	b8 12 07 00 00                                  	mov    eax,0x712
  130dbd:	eb 67                                           	jmp    0x130e26
  130dbf:	b8 a8 07 00 00                                  	mov    eax,0x7a8
  130dc4:	eb 60                                           	jmp    0x130e26
  130dc6:	b8 29 08 00 00                                  	mov    eax,0x829
  130dcb:	eb 59                                           	jmp    0x130e26
  130dcd:	b8 b1 08 00 00                                  	mov    eax,0x8b1
  130dd2:	eb 52                                           	jmp    0x130e26
  130dd4:	b8 5b 09 00 00                                  	mov    eax,0x95b
  130dd9:	eb 4b                                           	jmp    0x130e26
  130ddb:	b8 e9 09 00 00                                  	mov    eax,0x9e9
  130de0:	eb 44                                           	jmp    0x130e26
  130de2:	b8 86 0a 00 00                                  	mov    eax,0xa86
  130de7:	eb 3d                                           	jmp    0x130e26
  130de9:	b8 1f 0b 00 00                                  	mov    eax,0xb1f
  130dee:	eb 36                                           	jmp    0x130e26
  130df0:	b8 8d 0b 00 00                                  	mov    eax,0xb8d
  130df5:	eb 2f                                           	jmp    0x130e26
  130df7:	b8 f6 0b 00 00                                  	mov    eax,0xbf6
  130dfc:	eb 28                                           	jmp    0x130e26
  130dfe:	b8 ad 0c 00 00                                  	mov    eax,0xcad
  130e03:	eb 21                                           	jmp    0x130e26
  130e05:	b8 40 0d 00 00                                  	mov    eax,0xd40
  130e0a:	eb 1a                                           	jmp    0x130e26
  130e0c:	b8 f0 0b 00 00                                  	mov    eax,0xbf0
  130e11:	eb 13                                           	jmp    0x130e26
  130e13:	b8 d8 0d 00 00                                  	mov    eax,0xdd8
  130e18:	eb 0c                                           	jmp    0x130e26
  130e1a:	b8 bf 0e 00 00                                  	mov    eax,0xebf
  130e1f:	eb 05                                           	jmp    0x130e26
  130e21:	b8 87 0f 00 00                                  	mov    eax,0xf87
  130e26:	8b 96 30 3f 00 00                               	mov    edx,DWORD PTR [esi+0x3f30]
  130e2c:	52                                              	push   edx
  130e2d:	50                                              	push   eax
  130e2e:	e8 1d 27 0b 00                                  	call   0x1e3550
  130e33:	83 c4 08                                        	add    esp,0x8
  130e36:	e9 cc 01 00 00                                  	jmp    0x131007
  130e3b:	a1 68 57 82 00                                  	mov    eax,ds:0x825768
  130e40:	8b 0b                                           	mov    ecx,DWORD PTR [ebx]
  130e42:	6a 00                                           	push   0x0
  130e44:	50                                              	push   eax
  130e45:	e8 a6 4d fb ff                                  	call   0xe5bf0
  130e4a:	8b 8e 84 3b 00 00                               	mov    ecx,DWORD PTR [esi+0x3b84]
  130e50:	c1 e9 12                                        	shr    ecx,0x12
  130e53:	51                                              	push   ecx
  130e54:	f6 c1 01                                        	test   cl,0x1
  130e57:	8b ce                                           	mov    ecx,esi
  130e59:	74 2f                                           	je     0x130e8a
  130e5b:	d9 05 60 47 7e 00                               	fld    DWORD PTR ds:0x7e4760
  130e61:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130e64:	e8 f7 5d fb ff                                  	call   0xe6c60
  130e69:	68 81 00 00 00                                  	push   0x81
  130e6e:	8b ce                                           	mov    ecx,esi
  130e70:	e8 2b ec ed ff                                  	call   0xfaa0
  130e75:	8b 96 30 3f 00 00                               	mov    edx,DWORD PTR [esi+0x3f30]
  130e7b:	6a 01                                           	push   0x1
  130e7d:	52                                              	push   edx
  130e7e:	6a 1d                                           	push   0x1d
  130e80:	e8 0b 10 0c 00                                  	call   0x1f1e90
  130e85:	83 c4 0c                                        	add    esp,0xc
  130e88:	eb 0e                                           	jmp    0x130e98
  130e8a:	d9 05 88 63 6c 00                               	fld    DWORD PTR ds:0x6c6388
  130e90:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130e93:	e8 c8 5d fb ff                                  	call   0xe6c60
  130e98:	8b 03                                           	mov    eax,DWORD PTR [ebx]
  130e9a:	6a ff                                           	push   0xffffffff
  130e9c:	05 d0 3e 00 00                                  	add    eax,0x3ed0
  130ea1:	68 b0 f3 52 00                                  	push   0x52f3b0
  130ea6:	50                                              	push   eax
  130ea7:	e8 c4 6f fc ff                                  	call   0xf7e70
  130eac:	83 c4 0c                                        	add    esp,0xc
  130eaf:	8b ce                                           	mov    ecx,esi
  130eb1:	e8 6a 5f fb ff                                  	call   0xe6e20
  130eb6:	84 c0                                           	test   al,al
  130eb8:	74 0b                                           	je     0x130ec5
  130eba:	6a 00                                           	push   0x0
  130ebc:	53                                              	push   ebx
  130ebd:	e8 6e ef ed ff                                  	call   0xfe30
  130ec2:	83 c4 08                                        	add    esp,0x8
  130ec5:	81 4f 1c 00 10 00 00                            	or     DWORD PTR [edi+0x1c],0x1000
  130ecc:	80 7d ff 00                                     	cmp    BYTE PTR [ebp-0x1],0x0
  130ed0:	0f 85 31 01 00 00                               	jne    0x131007
  130ed6:	6a 02                                           	push   0x2
  130ed8:	8b ce                                           	mov    ecx,esi
  130eda:	e8 a1 8b fb ff                                  	call   0xe9a80
  130edf:	d9 05 20 a2 7f 00                               	fld    DWORD PTR ds:0x7fa220
  130ee5:	83 ec 08                                        	sub    esp,0x8
  130ee8:	d9 5c 24 04                                     	fstp   DWORD PTR [esp+0x4]
  130eec:	8b d8                                           	mov    ebx,eax
  130eee:	d9 ee                                           	fldz
  130ef0:	8b cb                                           	mov    ecx,ebx
  130ef2:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130ef5:	68 ff 01 01 00                                  	push   0x101ff
  130efa:	6a ff                                           	push   0xffffffff
  130efc:	e8 4f 40 0b 00                                  	call   0x1e4f50
  130f01:	c7 83 80 02 00 00 00 00 00 00                   	mov    DWORD PTR [ebx+0x280],0x0
  130f0b:	d9 87 80 00 00 00                               	fld    DWORD PTR [edi+0x80]
  130f11:	8b 4f 24                                        	mov    ecx,DWORD PTR [edi+0x24]
  130f14:	d9 5d f4                                        	fstp   DWORD PTR [ebp-0xc]
  130f17:	d9 47 7c                                        	fld    DWORD PTR [edi+0x7c]
  130f1a:	c1 e9 03                                        	shr    ecx,0x3
  130f1d:	d9 5d f8                                        	fstp   DWORD PTR [ebp-0x8]
  130f20:	80 e1 01                                        	and    cl,0x1
  130f23:	d9 ee                                           	fldz
  130f25:	88 4d ec                                        	mov    BYTE PTR [ebp-0x14],cl
  130f28:	8b 55 ec                                        	mov    edx,DWORD PTR [ebp-0x14]
  130f2b:	52                                              	push   edx
  130f2c:	83 ec 0c                                        	sub    esp,0xc
  130f2f:	d9 5c 24 08                                     	fstp   DWORD PTR [esp+0x8]
  130f33:	d9 45 f4                                        	fld    DWORD PTR [ebp-0xc]
  130f36:	dc 05 c8 86 7e 00                               	fadd   QWORD PTR ds:0x7e86c8
  130f3c:	d9 5d ec                                        	fstp   DWORD PTR [ebp-0x14]
  130f3f:	d9 45 ec                                        	fld    DWORD PTR [ebp-0x14]
  130f42:	d9 5c 24 04                                     	fstp   DWORD PTR [esp+0x4]
  130f46:	d9 45 f8                                        	fld    DWORD PTR [ebp-0x8]
  130f49:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130f4c:	6a 6e                                           	push   0x6e
  130f4e:	e8 ad 54 fa ff                                  	call   0xd6400
  130f53:	d9 ee                                           	fldz
  130f55:	83 c4 0c                                        	add    esp,0xc
  130f58:	d9 54 24 04                                     	fst    DWORD PTR [esp+0x4]
  130f5c:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130f5f:	8b d8                                           	mov    ebx,eax
  130f61:	e8 ca 35 f4 ff                                  	call   0x74530
  130f66:	25 ff 7f 00 00                                  	and    eax,0x7fff
  130f6b:	69 c0 68 01 00 00                               	imul   eax,eax,0x168
  130f71:	c1 f8 0f                                        	sar    eax,0xf
  130f74:	89 45 ec                                        	mov    DWORD PTR [ebp-0x14],eax
  130f77:	51                                              	push   ecx
  130f78:	8b cb                                           	mov    ecx,ebx
  130f7a:	db 45 ec                                        	fild   DWORD PTR [ebp-0x14]
  130f7d:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130f80:	e8 6b 41 0b 00                                  	call   0x1e50f0
  130f85:	d9 ee                                           	fldz
  130f87:	83 ec 0c                                        	sub    esp,0xc
  130f8a:	d9 5c 24 08                                     	fstp   DWORD PTR [esp+0x8]
  130f8e:	8b cb                                           	mov    ecx,ebx
  130f90:	d9 05 98 23 80 00                               	fld    DWORD PTR ds:0x802398
  130f96:	d9 5c 24 04                                     	fstp   DWORD PTR [esp+0x4]
  130f9a:	d9 05 94 23 80 00                               	fld    DWORD PTR ds:0x802394
  130fa0:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130fa3:	e8 e8 e9 ed ff                                  	call   0xf990
  130fa8:	d9 05 20 a2 7f 00                               	fld    DWORD PTR ds:0x7fa220
  130fae:	83 ec 08                                        	sub    esp,0x8
  130fb1:	d9 5c 24 04                                     	fstp   DWORD PTR [esp+0x4]
  130fb5:	d9 ee                                           	fldz
  130fb7:	8b cb                                           	mov    ecx,ebx
  130fb9:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130fbc:	68 ff ff ff 00                                  	push   0xffffff
  130fc1:	6a ff                                           	push   0xffffffff
  130fc3:	e8 88 3f 0b 00                                  	call   0x1e4f50
  130fc8:	d9 87 80 00 00 00                               	fld    DWORD PTR [edi+0x80]
  130fce:	8b 47 24                                        	mov    eax,DWORD PTR [edi+0x24]
  130fd1:	dc 05 c8 86 7e 00                               	fadd   QWORD PTR ds:0x7e86c8
  130fd7:	c1 e8 03                                        	shr    eax,0x3
  130fda:	6a 00                                           	push   0x0
  130fdc:	24 01                                           	and    al,0x1
  130fde:	0f b6 c8                                        	movzx  ecx,al
  130fe1:	51                                              	push   ecx
  130fe2:	6a 01                                           	push   0x1
  130fe4:	6a 00                                           	push   0x0
  130fe6:	6a 1d                                           	push   0x1d
  130fe8:	57                                              	push   edi
  130fe9:	d9 5d ec                                        	fstp   DWORD PTR [ebp-0x14]
  130fec:	83 ec 08                                        	sub    esp,0x8
  130fef:	d9 45 ec                                        	fld    DWORD PTR [ebp-0x14]
  130ff2:	d9 5c 24 04                                     	fstp   DWORD PTR [esp+0x4]
  130ff6:	d9 47 7c                                        	fld    DWORD PTR [edi+0x7c]
  130ff9:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  130ffc:	e8 df 57 fa ff                                  	call   0xd67e0
  131001:	8b 5d 08                                        	mov    ebx,DWORD PTR [ebp+0x8]
  131004:	83 c4 20                                        	add    esp,0x20
  131007:	8b 45 f0                                        	mov    eax,DWORD PTR [ebp-0x10]
  13100a:	83 c0 fe                                        	add    eax,0xfffffffe
  13100d:	83 f8 1b                                        	cmp    eax,0x1b
  131010:	0f 87 66 03 00 00                               	ja     0x13137c
  131016:	0f b6 90 5c 14 53 00                            	movzx  edx,BYTE PTR [eax+0x53145c]
  13101d:	ff 24 95 1c 14 53 00                            	jmp    DWORD PTR [edx*4+0x53141c]
  131024:	6a 01                                           	push   0x1
  131026:	8b ce                                           	mov    ecx,esi
  131028:	e8 53 8a fb ff                                  	call   0xe9a80
  13102d:	50                                              	push   eax
  13102e:	e8 fd 2c 0a 00                                  	call   0x1d3d30
  131033:	83 c4 04                                        	add    esp,0x4
  131036:	84 c0                                           	test   al,al
  131038:	0f 85 3e 03 00 00                               	jne    0x13137c
  13103e:	6a 01                                           	push   0x1
  131040:	8b ce                                           	mov    ecx,esi
  131042:	e8 39 8a fb ff                                  	call   0xe9a80
  131047:	50                                              	push   eax
  131048:	e8 93 2f 0a 00                                  	call   0x1d3fe0
  13104d:	8b 86 30 3f 00 00                               	mov    eax,DWORD PTR [esi+0x3f30]
  131053:	6a 26                                           	push   0x26
  131055:	50                                              	push   eax
  131056:	e8 f5 ea ed ff                                  	call   0xfb50
  13105b:	ff 86 68 43 00 00                               	inc    DWORD PTR [esi+0x4368]
  131061:	83 c4 0c                                        	add    esp,0xc
  131064:	83 be 68 43 00 00 0a                            	cmp    DWORD PTR [esi+0x4368],0xa
  13106b:	0f 8c 0b 03 00 00                               	jl     0x13137c
  131071:	8b 8e 30 3f 00 00                               	mov    ecx,DWORD PTR [esi+0x3f30]
  131077:	6a 01                                           	push   0x1
  131079:	51                                              	push   ecx
  13107a:	6a 31                                           	push   0x31
  13107c:	e9 f3 02 00 00                                  	jmp    0x131374
  131081:	8b be b4 3a 00 00                               	mov    edi,DWORD PTR [esi+0x3ab4]
  131087:	2b be b8 3a 00 00                               	sub    edi,DWORD PTR [esi+0x3ab8]
  13108d:	85 ff                                           	test   edi,edi
  13108f:	7f 04                                           	jg     0x131095
  131091:	33 c0                                           	xor    eax,eax
  131093:	eb 22                                           	jmp    0x1310b7
  131095:	6a 57                                           	push   0x57
  131097:	e8 f4 43 ff ff                                  	call   0x125490
  13109c:	83 c4 04                                        	add    esp,0x4
  13109f:	e8 dc 77 0d 00                                  	call   0x208880
  1310a4:	8b c8                                           	mov    ecx,eax
  1310a6:	c1 e1 04                                        	shl    ecx,0x4
  1310a9:	2b c8                                           	sub    ecx,eax
  1310ab:	03 c9                                           	add    ecx,ecx
  1310ad:	03 c9                                           	add    ecx,ecx
  1310af:	8b c7                                           	mov    eax,edi
  1310b1:	99                                              	cdq
  1310b2:	03 c9                                           	add    ecx,ecx
  1310b4:	f7 f9                                           	idiv   ecx
  1310b6:	40                                              	inc    eax
  1310b7:	8b 9e 3c 3f 00 00                               	mov    ebx,DWORD PTR [esi+0x3f3c]
  1310bd:	85 c0                                           	test   eax,eax
  1310bf:	7e 46                                           	jle    0x131107
  1310c1:	8b f8                                           	mov    edi,eax
  1310c3:	8b 8e 40 3f 00 00                               	mov    ecx,DWORD PTR [esi+0x3f40]
  1310c9:	81 e9 8e 01 00 00                               	sub    ecx,0x18e
  1310cf:	b8 11 29 40 a4                                  	mov    eax,0xa4402911
  1310d4:	f7 e9                                           	imul   ecx
  1310d6:	03 d1                                           	add    edx,ecx
  1310d8:	c1 fa 08                                        	sar    edx,0x8
  1310db:	8b c2                                           	mov    eax,edx
  1310dd:	c1 e8 1f                                        	shr    eax,0x1f
  1310e0:	03 c2                                           	add    eax,edx
  1310e2:	50                                              	push   eax
  1310e3:	8b ce                                           	mov    ecx,esi
  1310e5:	e8 b6 4a fb ff                                  	call   0xe5ba0
  1310ea:	8b 86 40 3f 00 00                               	mov    eax,DWORD PTR [esi+0x3f40]
  1310f0:	8b 8e 3c 3f 00 00                               	mov    ecx,DWORD PTR [esi+0x3f3c]
  1310f6:	3b c8                                           	cmp    ecx,eax
  1310f8:	7e 0a                                           	jle    0x131104
  1310fa:	2b c1                                           	sub    eax,ecx
  1310fc:	50                                              	push   eax
  1310fd:	8b ce                                           	mov    ecx,esi
  1310ff:	e8 9c 4a fb ff                                  	call   0xe5ba0
  131104:	4f                                              	dec    edi
  131105:	75 bc                                           	jne    0x1310c3
  131107:	8b 8e 3c 3f 00 00                               	mov    ecx,DWORD PTR [esi+0x3f3c]
  13110d:	2b cb                                           	sub    ecx,ebx
  13110f:	01 8e 68 43 00 00                               	add    DWORD PTR [esi+0x4368],ecx
  131115:	81 be 68 43 00 00 d0 07 00 00                   	cmp    DWORD PTR [esi+0x4368],0x7d0
  13111f:	7c 13                                           	jl     0x131134
  131121:	8b 96 30 3f 00 00                               	mov    edx,DWORD PTR [esi+0x3f30]
  131127:	6a 01                                           	push   0x1
  131129:	52                                              	push   edx
  13112a:	6a 2b                                           	push   0x2b
  13112c:	e8 5f 0d 0c 00                                  	call   0x1f1e90
  131131:	83 c4 0c                                        	add    esp,0xc
  131134:	8d 86 3c 3a 00 00                               	lea    eax,[esi+0x3a3c]
  13113a:	6a 1d                                           	push   0x1d
  13113c:	50                                              	push   eax
  13113d:	e8 6e 6d fc ff                                  	call   0xf7eb0
  131142:	8b 8e 30 3f 00 00                               	mov    ecx,DWORD PTR [esi+0x3f30]
  131148:	6a 1c                                           	push   0x1c
  13114a:	51                                              	push   ecx
  13114b:	e8 00 ea ed ff                                  	call   0xfb50
  131150:	83 c4 10                                        	add    esp,0x10
  131153:	e9 24 02 00 00                                  	jmp    0x13137c
  131158:	8b 3d c8 02 2e 0e                               	mov    edi,DWORD PTR ds:0xe2e02c8
  13115e:	85 ff                                           	test   edi,edi
  131160:	0f 84 16 02 00 00                               	je     0x13137c
  131166:	83 7f 38 ff                                     	cmp    DWORD PTR [edi+0x38],0xffffffff
  13116a:	0f 84 0c 02 00 00                               	je     0x13137c
  131170:	8b 86 30 3f 00 00                               	mov    eax,DWORD PTR [esi+0x3f30]
  131176:	83 c0 11                                        	add    eax,0x11
  131179:	50                                              	push   eax
  13117a:	8b cf                                           	mov    ecx,edi
  13117c:	e8 7f 30 0b 00                                  	call   0x1e4200
  131181:	84 c0                                           	test   al,al
  131183:	74 3c                                           	je     0x1311c1
  131185:	83 7f 3c 02                                     	cmp    DWORD PTR [edi+0x3c],0x2
  131189:	75 36                                           	jne    0x1311c1
  13118b:	8b 87 a4 01 00 00                               	mov    eax,DWORD PTR [edi+0x1a4]
  131191:	3d ca 00 00 00                                  	cmp    eax,0xca
  131196:	7c 07                                           	jl     0x13119f
  131198:	3d cc 00 00 00                                  	cmp    eax,0xcc
  13119d:	7e 07                                           	jle    0x1311a6
  13119f:	3d cf 00 00 00                                  	cmp    eax,0xcf
  1311a4:	75 1b                                           	jne    0x1311c1
  1311a6:	c7 87 8c 02 00 00 01 00 00 00                   	mov    DWORD PTR [edi+0x28c],0x1
  1311b0:	8b 96 30 3f 00 00                               	mov    edx,DWORD PTR [esi+0x3f30]
  1311b6:	6a 2e                                           	push   0x2e
  1311b8:	52                                              	push   edx
  1311b9:	e8 92 e9 ed ff                                  	call   0xfb50
  1311be:	83 c4 08                                        	add    esp,0x8
  1311c1:	8b 3f                                           	mov    edi,DWORD PTR [edi]
  1311c3:	85 ff                                           	test   edi,edi
  1311c5:	75 9f                                           	jne    0x131166
  1311c7:	e9 b0 01 00 00                                  	jmp    0x13137c
  1311cc:	8b 86 30 3f 00 00                               	mov    eax,DWORD PTR [esi+0x3f30]
  1311d2:	6a 27                                           	push   0x27
  1311d4:	50                                              	push   eax
  1311d5:	e8 76 e9 ed ff                                  	call   0xfb50
  1311da:	8b 8e 30 3f 00 00                               	mov    ecx,DWORD PTR [esi+0x3f30]
  1311e0:	6a 1e                                           	push   0x1e
  1311e2:	51                                              	push   ecx
  1311e3:	e8 68 e9 ed ff                                  	call   0xfb50
  1311e8:	83 c4 10                                        	add    esp,0x10
  1311eb:	e9 8c 01 00 00                                  	jmp    0x13137c
  1311f0:	8b ce                                           	mov    ecx,esi
  1311f2:	e8 39 5c fb ff                                  	call   0xe6e30
  1311f7:	84 c0                                           	test   al,al
  1311f9:	75 0d                                           	jne    0x131208
  1311fb:	83 be 20 3f 00 00 17                            	cmp    DWORD PTR [esi+0x3f20],0x17
  131202:	0f 84 74 01 00 00                               	je     0x13137c
  131208:	8b 96 30 3f 00 00                               	mov    edx,DWORD PTR [esi+0x3f30]
  13120e:	6a 1e                                           	push   0x1e
  131210:	52                                              	push   edx
  131211:	e8 3a e9 ed ff                                  	call   0xfb50
  131216:	83 c4 08                                        	add    esp,0x8
  131219:	e9 5e 01 00 00                                  	jmp    0x13137c
  13121e:	8b 86 30 3f 00 00                               	mov    eax,DWORD PTR [esi+0x3f30]
  131224:	6a 28                                           	push   0x28
  131226:	50                                              	push   eax
  131227:	e8 24 e9 ed ff                                  	call   0xfb50
  13122c:	83 c4 08                                        	add    esp,0x8
  13122f:	e9 48 01 00 00                                  	jmp    0x13137c
  131234:	8b 8e 30 3f 00 00                               	mov    ecx,DWORD PTR [esi+0x3f30]
  13123a:	6a 1c                                           	push   0x1c
  13123c:	51                                              	push   ecx
  13123d:	e8 0e e9 ed ff                                  	call   0xfb50
  131242:	83 c4 08                                        	add    esp,0x8
  131245:	e9 32 01 00 00                                  	jmp    0x13137c
  13124a:	8b 96 30 3f 00 00                               	mov    edx,DWORD PTR [esi+0x3f30]
  131250:	6a 22                                           	push   0x22
  131252:	52                                              	push   edx
  131253:	e8 f8 e8 ed ff                                  	call   0xfb50
  131258:	8b 86 30 3f 00 00                               	mov    eax,DWORD PTR [esi+0x3f30]
  13125e:	6a 23                                           	push   0x23
  131260:	50                                              	push   eax
  131261:	e8 ea e8 ed ff                                  	call   0xfb50
  131266:	83 c4 10                                        	add    esp,0x10
  131269:	c7 86 68 43 00 00 00 00 00 00                   	mov    DWORD PTR [esi+0x4368],0x0
  131273:	e9 04 01 00 00                                  	jmp    0x13137c
  131278:	6a 00                                           	push   0x0
  13127a:	6a ff                                           	push   0xffffffff
  13127c:	6a ff                                           	push   0xffffffff
  13127e:	53                                              	push   ebx
  13127f:	e8 dc 94 0a 00                                  	call   0x1da760
  131284:	8b 8e 30 3f 00 00                               	mov    ecx,DWORD PTR [esi+0x3f30]
  13128a:	6a 26                                           	push   0x26
  13128c:	51                                              	push   ecx
  13128d:	e8 be e8 ed ff                                  	call   0xfb50
  131292:	83 c4 18                                        	add    esp,0x18
  131295:	e9 e2 00 00 00                                  	jmp    0x13137c
  13129a:	8b 96 30 3f 00 00                               	mov    edx,DWORD PTR [esi+0x3f30]
  1312a0:	6a 40                                           	push   0x40
  1312a2:	52                                              	push   edx
  1312a3:	e8 a8 e8 ed ff                                  	call   0xfb50
  1312a8:	83 c4 08                                        	add    esp,0x8
  1312ab:	e9 cc 00 00 00                                  	jmp    0x13137c
  1312b0:	8b 86 30 3f 00 00                               	mov    eax,DWORD PTR [esi+0x3f30]
  1312b6:	6a 41                                           	push   0x41
  1312b8:	50                                              	push   eax
  1312b9:	e8 92 e8 ed ff                                  	call   0xfb50
  1312be:	83 c4 08                                        	add    esp,0x8
  1312c1:	e9 b6 00 00 00                                  	jmp    0x13137c
  1312c6:	8b 8e 30 3f 00 00                               	mov    ecx,DWORD PTR [esi+0x3f30]
  1312cc:	6a 33                                           	push   0x33
  1312ce:	51                                              	push   ecx
  1312cf:	e8 7c e8 ed ff                                  	call   0xfb50
  1312d4:	83 c4 08                                        	add    esp,0x8
  1312d7:	e9 a0 00 00 00                                  	jmp    0x13137c
  1312dc:	6a 1e                                           	push   0x1e
  1312de:	8b ce                                           	mov    ecx,esi
  1312e0:	e8 1b 48 f6 ff                                  	call   0x95b00
  1312e5:	84 c0                                           	test   al,al
  1312e7:	74 3a                                           	je     0x131323
  1312e9:	8b 96 30 3f 00 00                               	mov    edx,DWORD PTR [esi+0x3f30]
  1312ef:	6a 1e                                           	push   0x1e
  1312f1:	52                                              	push   edx
  1312f2:	e8 59 e8 ed ff                                  	call   0xfb50
  1312f7:	83 c4 08                                        	add    esp,0x8
  1312fa:	80 7d fe 00                                     	cmp    BYTE PTR [ebp-0x2],0x0
  1312fe:	51                                              	push   ecx
  1312ff:	8b ce                                           	mov    ecx,esi
  131301:	74 10                                           	je     0x131313
  131303:	d9 05 90 23 80 00                               	fld    DWORD PTR ds:0x802390
  131309:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  13130c:	e8 0f e8 ed ff                                  	call   0xfb20
  131311:	eb 21                                           	jmp    0x131334
  131313:	d9 05 40 ae 6f 00                               	fld    DWORD PTR ds:0x6fae40
  131319:	d9 1c 24                                        	fstp   DWORD PTR [esp]
  13131c:	e8 ff e7 ed ff                                  	call   0xfb20
  131321:	eb 11                                           	jmp    0x131334
  131323:	8b 86 30 3f 00 00                               	mov    eax,DWORD PTR [esi+0x3f30]
  131329:	6a 67                                           	push   0x67
  13132b:	50                                              	push   eax
  13132c:	e8 1f e8 ed ff                                  	call   0xfb50
  131331:	83 c4 08                                        	add    esp,0x8
  131334:	8b 8e 30 3f 00 00                               	mov    ecx,DWORD PTR [esi+0x3f30]
  13133a:	6a 08                                           	push   0x8
  13133c:	51                                              	push   ecx
  13133d:	e8 0e e8 ed ff                                  	call   0xfb50
  131342:	83 c4 08                                        	add    esp,0x8
  131345:	eb 35                                           	jmp    0x13137c
  131347:	8b 96 30 3f 00 00                               	mov    edx,DWORD PTR [esi+0x3f30]
  13134d:	6a 26                                           	push   0x26
  13134f:	52                                              	push   edx
  131350:	e8 fb e7 ed ff                                  	call   0xfb50
  131355:	83 c4 08                                        	add    esp,0x8
  131358:	eb 22                                           	jmp    0x13137c
  13135a:	ff 86 68 43 00 00                               	inc    DWORD PTR [esi+0x4368]
  131360:	83 be 68 43 00 00 0a                            	cmp    DWORD PTR [esi+0x4368],0xa
  131367:	7c 13                                           	jl     0x13137c
  131369:	8b 86 30 3f 00 00                               	mov    eax,DWORD PTR [esi+0x3f30]
  13136f:	6a 01                                           	push   0x1
  131371:	50                                              	push   eax
  131372:	6a 34                                           	push   0x34
  131374:	e8 17 0b 0c 00                                  	call   0x1f1e90
  131379:	83 c4 0c                                        	add    esp,0xc
  13137c:	8b 8e 30 3f 00 00                               	mov    ecx,DWORD PTR [esi+0x3f30]
  131382:	69 c9 98 00 00 00                               	imul   ecx,ecx,0x98
  131388:	81 c1 d0 17 0a 0e                               	add    ecx,0xe0a17d0
  13138e:	6a 44                                           	push   0x44
  131390:	51                                              	push   ecx
  131391:	e8 ca 1e f7 ff                                  	call   0xa3260
  131396:	83 c4 08                                        	add    esp,0x8
  131399:	5f                                              	pop    edi
  13139a:	5e                                              	pop    esi
  13139b:	5b                                              	pop    ebx
  13139c:	8b e5                                           	mov    esp,ebp
  13139e:	5d                                              	pop    ebp
  13139f:	c3                                              	ret
