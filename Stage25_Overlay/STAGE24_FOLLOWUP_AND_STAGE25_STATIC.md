# Stage24 completed follow-up / Stage25 static basis

2026-11-09. Prior evidence commit: 983a7f4f205c99cd1c8f44814c58cc09ed76f0c8. Published Stage24 source: 0bca8a6d8e851990563b281c517f01bd1a0c81a2. Historical evidence remains unchanged.

## Large logs now fully read

Direct raw download succeeded. Exact Git blob SHA checks:
- PlayerBit17WriteTrace: 11721014 bytes, c378308424964cb9ef63ee64437543977841a7a3.
- ProducerCallTrace: 8120267 bytes, 674fefc9b1d0ac2f81529cbd5929fff36d0b00f4.

Write trace: 21886 records, seq1..21886, no missing/duplicate sequence IDs. There are 15 adjacent sequence discontinuities due to blocks appearing out of sequence in the file; do not treat file order as execution order. All IDs are present. P1=10083 records, P2=11803. Producer trace: 10463 records, seq1..10463 in order, no gaps/duplicates. Both end with newline and contain no explicit ERROR/WARNING/DROPPED/OVERFLOW markers. This does not prove no writes occurred on uninstrumented threads; the logger's shadow-state limitation still applies.

Exactly one bit17 set and one clear, both P2 tid7572 tick28309390:
- Stage17 first setter QPC 0x41E9BA1075.
- Write seq15206 QPC 0x41E9BA11CE: old0 -> new0x20000, post-EIP RVA FAD2. The writing instruction is OR [EAX],EDX at FAD0. Delta345 ticks.
- Second setter QPC 0x41E9BA1362 sees bit17 already set.
- Write seq15215 QPC 0x41E9BA204B: old0x30000 -> new0x10000. Post-EIP ED1A9; instruction ED19F is `AND [ESI+3B84],0xFFF9FFFF`, clearing bits17 and18. Bit18 was already zero in this sample. Earlier descriptions of this instruction as mask0xFFFDFFFF are incorrect for the fingerprinted binary.

Stage14 and15 both install5/5 probes. Both P1 payload1E episodes show bit17=0, F6A68 branchTaken1, F6A9D branchTaken1 and then alternate queue call F6AD7; F6AAC is bypassed in these two episodes. All eight branch samples have fieldsMask0x0FFF. This completes the previously unread producer side of the Stage24 correlation.

## Why arg3/arg4 are zero — corrected dataflow

Fresh disassembly of archived AALib.exe, SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba, confirms:

```
EE285 movzx ecx, byte ptr [eax+4]    ; resolver byte18
EE289 movzx edx, byte ptr [eax+2]    ; resolver byte3
EE28D movzx edi, byte ptr [eax+3]    ; selector31
EE2BE push edx
EE2BF push esi
EE2C0 mov [esi+3F64],ecx
EE2C6 call 158EC0
...
EE2F0 mov ecx,[esi+3F34]
EE2F6 mov edx,[esi+3F30]
EE2FC push ecx                     ; arg4
EE2FD push edx                     ; arg3
EE2FE push eax                     ; mapped selector
EE2FF lea eax,[esi+3A08]
EE305 push eax                     ; output
EE306 call F57D0
```

Thus arg3 is player+3F30; arg4 is player+3F34. The earlier prediction3/18 mistakenly skipped the intervening reloads. Zero/zero in Stage24 is consistent with the actual code, not evidence of argument corruption. Resolver byte4 is stored at player+3F64; byte2 is passed to another helper. Do not label these bytes as the final F57D0 arguments.

## Secondary path, with corrected argument identity

F57D0 first calls 9EEA0(arg3,arg4). It then maps the three-byte table, clears +4/+8 when flags bit0 is set, writes the selector and sets mapped bits. For03,00,10 this yields mask0x10001.

When flags bit1 is set:
```
F58BB mov eax,[ebp+10]   ; primary arg3, NOT arg4
F58BE push -1
F58C0 push eax
F58C1 call 9EF00
F58C6 add esp,8
F58C9 or DWORD PTR [eax+F8],1
F58D0 pop ebp
F58D1 ret
```
9EF00 with second argument -1 maps first argument0->slot1 and1->slot0, loading the active-object pointer from preferred VA0x0E0A1B54 + slot*4. Stage24's returned EAX=0x0E27CCA0 matches P2 for the observed P1/arg3=0 calls. This supports the opposite-side target interpretation; direct before/after F8 observation is the Stage25 task. Higher-level gameplay semantics remain unproven.

The Stage25 observer uses the primary helper EBP to recover output/selector/args and the saved dispatcher EBP to filter type1/payload1E. Stage24's wrapper leaves EBP unchanged, so this chain is valid even though the helper return address points into the Stage24 DLL wrapper. The observer return is independently checked against F58CE.
