
SHA256-pinned AALib.exe window:     file format binary


Disassembly of section .data:

000f5350 <.data>:
   f5350:	55                                              	push   ebp
   f5351:	8b ec                                           	mov    ebp,esp
   f5353:	8b 45 08                                        	mov    eax,DWORD PTR [ebp+0x8]
   f5356:	c7 40 20 10 27 00 00                            	mov    DWORD PTR [eax+0x20],0x2710
   f535d:	c7 40 24 64 00 00 00                            	mov    DWORD PTR [eax+0x24],0x64
   f5364:	66 c7 40 34 00 00                               	mov    WORD PTR [eax+0x34],0x0
   f536a:	5d                                              	pop    ebp
   f536b:	c3                                              	ret
   f536c:	cc                                              	int3
   f536d:	cc                                              	int3
   f536e:	cc                                              	int3
   f536f:	cc                                              	int3
   f5370:	55                                              	push   ebp
   f5371:	8b ec                                           	mov    ebp,esp
   f5373:	8b 45 08                                        	mov    eax,DWORD PTR [ebp+0x8]
   f5376:	33 c9                                           	xor    ecx,ecx
   f5378:	89 48 20                                        	mov    DWORD PTR [eax+0x20],ecx
   f537b:	c7 40 24 64 00 00 00                            	mov    DWORD PTR [eax+0x24],0x64
   f5382:	88 48 34                                        	mov    BYTE PTR [eax+0x34],cl
   f5385:	5d                                              	pop    ebp
   f5386:	c3                                              	ret
   f5387:	cc                                              	int3
   f5388:	cc                                              	int3
   f5389:	cc                                              	int3
   f538a:	cc                                              	int3
   f538b:	cc                                              	int3
   f538c:	cc                                              	int3
   f538d:	cc                                              	int3
   f538e:	cc                                              	int3
   f538f:	cc                                              	int3
   f5390:	55                                              	push   ebp
   f5391:	8b ec                                           	mov    ebp,esp
   f5393:	8b 45 0c                                        	mov    eax,DWORD PTR [ebp+0xc]
   f5396:	8b 4d 08                                        	mov    ecx,DWORD PTR [ebp+0x8]
   f5399:	89 41 24                                        	mov    DWORD PTR [ecx+0x24],eax
   f539c:	5d                                              	pop    ebp
   f539d:	c3                                              	ret
   f539e:	cc                                              	int3
   f539f:	cc                                              	int3
   f53a0:	55                                              	push   ebp
   f53a1:	8b ec                                           	mov    ebp,esp
   f53a3:	8a 45 0c                                        	mov    al,BYTE PTR [ebp+0xc]
   f53a6:	8b 4d 08                                        	mov    ecx,DWORD PTR [ebp+0x8]
   f53a9:	88 41 34                                        	mov    BYTE PTR [ecx+0x34],al
   f53ac:	5d                                              	pop    ebp
   f53ad:	c3                                              	ret
