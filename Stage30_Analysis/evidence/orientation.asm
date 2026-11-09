
SHA256-pinned AALib.exe window:     file format binary


Disassembly of section .data:

000e8560 <.data>:
   e8560:	55                                              	push   ebp
   e8561:	8b ec                                           	mov    ebp,esp
   e8563:	51                                              	push   ecx
   e8564:	53                                              	push   ebx
   e8565:	56                                              	push   esi
   e8566:	8b f1                                           	mov    esi,ecx
   e8568:	8b 46 04                                        	mov    eax,DWORD PTR [esi+0x4]
   e856b:	8b 48 24                                        	mov    ecx,DWORD PTR [eax+0x24]
   e856e:	8b 16                                           	mov    edx,DWORD PTR [esi]
   e8570:	8b 5a 04                                        	mov    ebx,DWORD PTR [edx+0x4]
   e8573:	c1 e9 03                                        	shr    ecx,0x3
   e8576:	f6 d1                                           	not    cl
   e8578:	80 e1 01                                        	and    cl,0x1
   e857b:	80 be 7c 3f 00 00 00                            	cmp    BYTE PTR [esi+0x3f7c],0x0
   e8582:	88 4d ff                                        	mov    BYTE PTR [ebp-0x1],cl
   e8585:	74 0a                                           	je     0xe8591
   e8587:	5e                                              	pop    esi
   e8588:	32 c0                                           	xor    al,al
   e858a:	5b                                              	pop    ebx
   e858b:	8b e5                                           	mov    esp,ebp
   e858d:	5d                                              	pop    ebp
   e858e:	c2 04 00                                        	ret    0x4
   e8591:	57                                              	push   edi
   e8592:	8b be 78 39 00 00                               	mov    edi,DWORD PTR [esi+0x3978]
   e8598:	83 ff 1b                                        	cmp    edi,0x1b
   e859b:	75 0d                                           	jne    0xe85aa
   e859d:	83 be 34 3f 00 00 05                            	cmp    DWORD PTR [esi+0x3f34],0x5
   e85a4:	0f 8c 84 00 00 00                               	jl     0xe862e
   e85aa:	80 7d 08 00                                     	cmp    BYTE PTR [ebp+0x8],0x0
   e85ae:	75 0e                                           	jne    0xe85be
   e85b0:	8b 8e 70 3b 00 00                               	mov    ecx,DWORD PTR [esi+0x3b70]
   e85b6:	c1 e9 02                                        	shr    ecx,0x2
   e85b9:	f6 c1 01                                        	test   cl,0x1
   e85bc:	75 70                                           	jne    0xe862e
   e85be:	66 83 be 34 3a 00 00 01                         	cmp    WORD PTR [esi+0x3a34],0x1
   e85c6:	75 22                                           	jne    0xe85ea
   e85c8:	8b 50 10                                        	mov    edx,DWORD PTR [eax+0x10]
   e85cb:	c1 ea 12                                        	shr    edx,0x12
   e85ce:	f6 c2 01                                        	test   dl,0x1
   e85d1:	75 17                                           	jne    0xe85ea
   e85d3:	66 83 be 36 3a 00 00 20                         	cmp    WORD PTR [esi+0x3a36],0x20
   e85db:	7c 0d                                           	jl     0xe85ea
   e85dd:	6a 33                                           	push   0x33
   e85df:	8b ce                                           	mov    ecx,esi
   e85e1:	e8 4a 99 f1 ff                                  	call   0x1f30
   e85e6:	84 c0                                           	test   al,al
   e85e8:	75 44                                           	jne    0xe862e
   e85ea:	83 ff 1d                                        	cmp    edi,0x1d
   e85ed:	75 16                                           	jne    0xe8605
   e85ef:	f6 86 86 3b 00 00 01                            	test   BYTE PTR [esi+0x3b86],0x1
   e85f6:	74 0d                                           	je     0xe8605
   e85f8:	8b 86 88 3b 00 00                               	mov    eax,DWORD PTR [esi+0x3b88]
   e85fe:	c1 e8 1c                                        	shr    eax,0x1c
   e8601:	a8 01                                           	test   al,0x1
   e8603:	74 34                                           	je     0xe8639
   e8605:	83 be 20 3f 00 00 10                            	cmp    DWORD PTR [esi+0x3f20],0x10
   e860c:	75 2b                                           	jne    0xe8639
   e860e:	6a 22                                           	push   0x22
   e8610:	8d 8e 2c 3a 00 00                               	lea    ecx,[esi+0x3a2c]
   e8616:	6a 00                                           	push   0x0
   e8618:	51                                              	push   ecx
   e8619:	e8 b2 a8 00 00                                  	call   0xf2ed0
   e861e:	83 c4 0c                                        	add    esp,0xc
   e8621:	84 c0                                           	test   al,al
   e8623:	74 14                                           	je     0xe8639
   e8625:	83 be 54 3f 00 00 03                            	cmp    DWORD PTR [esi+0x3f54],0x3
   e862c:	75 0b                                           	jne    0xe8639
   e862e:	5f                                              	pop    edi
   e862f:	5e                                              	pop    esi
   e8630:	32 c0                                           	xor    al,al
   e8632:	5b                                              	pop    ebx
   e8633:	8b e5                                           	mov    esp,ebp
   e8635:	5d                                              	pop    ebp
   e8636:	c2 04 00                                        	ret    0x4
   e8639:	6a 00                                           	push   0x0
   e863b:	56                                              	push   esi
   e863c:	e8 9f 52 0f 00                                  	call   0x1dd8e0
   e8641:	83 c4 08                                        	add    esp,0x8
   e8644:	84 c0                                           	test   al,al
   e8646:	75 34                                           	jne    0xe867c
   e8648:	f6 86 90 3b 00 00 04                            	test   BYTE PTR [esi+0x3b90],0x4
   e864f:	74 2b                                           	je     0xe867c
   e8651:	8b 4e 04                                        	mov    ecx,DWORD PTR [esi+0x4]
   e8654:	d9 41 7c                                        	fld    DWORD PTR [ecx+0x7c]
   e8657:	d9 43 7c                                        	fld    DWORD PTR [ebx+0x7c]
   e865a:	de d9                                           	fcompp
   e865c:	df e0                                           	fnstsw ax
   e865e:	f6 c4 41                                        	test   ah,0x41
   e8661:	75 06                                           	jne    0xe8669
   e8663:	83 61 24 f7                                     	and    DWORD PTR [ecx+0x24],0xfffffff7
   e8667:	eb 13                                           	jmp    0xe867c
   e8669:	d9 41 7c                                        	fld    DWORD PTR [ecx+0x7c]
   e866c:	d9 43 7c                                        	fld    DWORD PTR [ebx+0x7c]
   e866f:	de d9                                           	fcompp
   e8671:	df e0                                           	fnstsw ax
   e8673:	f6 c4 05                                        	test   ah,0x5
   e8676:	7a 04                                           	jp     0xe867c
   e8678:	83 49 24 08                                     	or     DWORD PTR [ecx+0x24],0x8
   e867c:	8b 56 04                                        	mov    edx,DWORD PTR [esi+0x4]
   e867f:	8b 42 24                                        	mov    eax,DWORD PTR [edx+0x24]
   e8682:	c1 e8 03                                        	shr    eax,0x3
   e8685:	f6 d0                                           	not    al
   e8687:	24 01                                           	and    al,0x1
   e8689:	38 45 ff                                        	cmp    BYTE PTR [ebp-0x1],al
   e868c:	0f 95 c3                                        	setne  bl
   e868f:	84 db                                           	test   bl,bl
   e8691:	74 2f                                           	je     0xe86c2
   e8693:	6a 02                                           	push   0x2
   e8695:	8b ce                                           	mov    ecx,esi
   e8697:	e8 64 d4 fa ff                                  	call   0x95b00
   e869c:	84 c0                                           	test   al,al
   e869e:	74 22                                           	je     0xe86c2
   e86a0:	8b 46 04                                        	mov    eax,DWORD PTR [esi+0x4]
   e86a3:	d9 80 88 00 00 00                               	fld    DWORD PTR [eax+0x88]
   e86a9:	d9 e0                                           	fchs
   e86ab:	d9 98 88 00 00 00                               	fstp   DWORD PTR [eax+0x88]
   e86b1:	8b 76 04                                        	mov    esi,DWORD PTR [esi+0x4]
   e86b4:	d9 86 90 00 00 00                               	fld    DWORD PTR [esi+0x90]
   e86ba:	d9 e0                                           	fchs
   e86bc:	d9 9e 90 00 00 00                               	fstp   DWORD PTR [esi+0x90]
   e86c2:	5f                                              	pop    edi
   e86c3:	5e                                              	pop    esi
   e86c4:	8a c3                                           	mov    al,bl
   e86c6:	5b                                              	pop    ebx
   e86c7:	8b e5                                           	mov    esp,ebp
   e86c9:	5d                                              	pop    ebp
   e86ca:	c2 04 00                                        	ret    0x4
