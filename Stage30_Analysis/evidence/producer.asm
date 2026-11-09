
SHA256-pinned AALib.exe window:     file format binary


Disassembly of section .data:

000f6a63 <.data>:
   f6a63:	83 ff 1e                                        	cmp    edi,0x1e
   f6a66:	75 60                                           	jne    0xf6ac8
   f6a68:	8b 86 84 3b 00 00                               	mov    eax,DWORD PTR [esi+0x3b84]
   f6a6e:	c1 e8 11                                        	shr    eax,0x11
   f6a71:	a8 01                                           	test   al,0x1
   f6a73:	74 28                                           	je     0xf6a9d
   f6a75:	6a 00                                           	push   0x0
   f6a77:	8b ce                                           	mov    ecx,esi
   f6a79:	e8 e2 1a ff ff                                  	call   0xe8560
   f6a7e:	8d 4d e8                                        	lea    ecx,[ebp-0x18]
   f6a81:	56                                              	push   esi
   f6a82:	51                                              	push   ecx
   f6a83:	e8 78 01 06 00                                  	call   0x156c00
   f6a88:	8d 55 e8                                        	lea    edx,[ebp-0x18]
   f6a8b:	52                                              	push   edx
   f6a8c:	e8 7f 9f 03 00                                  	call   0x130a10
   f6a91:	83 c4 0c                                        	add    esp,0xc
   f6a94:	5f                                              	pop    edi
   f6a95:	5e                                              	pop    esi
   f6a96:	b0 01                                           	mov    al,0x1
   f6a98:	5b                                              	pop    ebx
   f6a99:	8b e5                                           	mov    esp,ebp
   f6a9b:	5d                                              	pop    ebp
   f6a9c:	c3                                              	ret
   f6a9d:	83 7d fc 02                                     	cmp    DWORD PTR [ebp-0x4],0x2
   f6aa1:	75 25                                           	jne    0xf6ac8
   f6aa3:	8b ce                                           	mov    ecx,esi
   f6aa5:	e8 e6 ff fe ff                                  	call   0xe6a90
   f6aaa:	84 c0                                           	test   al,al
   f6aac:	75 1a                                           	jne    0xf6ac8
   f6aae:	8b 86 30 3f 00 00                               	mov    eax,DWORD PTR [esi+0x3f30]
   f6ab4:	6a 03                                           	push   0x3
   f6ab6:	50                                              	push   eax
   f6ab7:	e8 94 90 f1 ff                                  	call   0xfb50
   f6abc:	83 c4 08                                        	add    esp,0x8
   f6abf:	6a 72                                           	push   0x72
   f6ac1:	8b ce                                           	mov    ecx,esi
   f6ac3:	e8 d8 8f f1 ff                                  	call   0xfaa0
   f6ac8:	6a 00                                           	push   0x0
   f6aca:	8b ce                                           	mov    ecx,esi
   f6acc:	e8 8f 1a ff ff                                  	call   0xe8560
   f6ad1:	8b 4d f8                                        	mov    ecx,DWORD PTR [ebp-0x8]
   f6ad4:	57                                              	push   edi
   f6ad5:	53                                              	push   ebx
   f6ad6:	51                                              	push   ecx
   f6ad7:	e8 04 c3 ff ff                                  	call   0xf2de0
   f6adc:	83 c4 0c                                        	add    esp,0xc
   f6adf:	5f                                              	pop    edi
   f6ae0:	5e                                              	pop    esi
   f6ae1:	b0 01                                           	mov    al,0x1
   f6ae3:	5b                                              	pop    ebx
   f6ae4:	8b e5                                           	mov    esp,ebp
   f6ae6:	5d                                              	pop    ebp
   f6ae7:	c3                                              	ret
