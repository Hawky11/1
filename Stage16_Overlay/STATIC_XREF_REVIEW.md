# Static xrefs to [base+0x3B84] in AALib.exe

Executable fingerprint:
- SHA-256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`
- PE32 / Intel 386
- ImageBase: `0x00400000`
- SizeOfImage: `0x0E0EB000`
- EntryPoint RVA: `0x0020EA6C`

Method: GNU objdump Intel-syntax disassembly of the executable `.text` section, collecting direct instructions whose memory operand contains displacement `0x3B84`. This yielded 107 direct references, 60 of which are write-like RMW operations. This scan cannot prove there are no writes through a copied pointer, computed address, helper, or whole-object copy; Stage16 hardware watchpoints target the actual P1/P2 addresses to catch such writes at runtime.

## Primary bit-17 candidate

VA `0x004ED19F` / RVA `0x000ED19F`:

```asm
004ED188  push 0x1E
004ED18A  lea eax,[esi+0x3A2C]
004ED190  push 1
004ED192  push eax
004ED193  call 0x004F2ED0
004ED198  add esp,0x0C
004ED19B  test al,al
004ED19D  jne 0x004ED1A9
004ED19F  and dword ptr [esi+0x3B84],0xFFF9FFFF
004ED1A9  mov eax,[esi+0x3978]
```

Mask `0xFFF9FFFF` clears bits 17 and 18 (mask bits `0x00060000`). It executes only when the preceding helper returns `AL=0`. In this path, ESI is used as the player object for nearby offsets `+0x3A2C`, `+0x3B84`, `+0x3978`, and `+0x3F30`. Stage16 must confirm whether the effective field address equals either watched P1/P2 address.

## Findings from direct write xrefs

- No `OR [reg+0x3B84],0x00020000` was found.
- `0x004F7BD4` is `or [edi+0x3B84],eax`, but the immediately preceding `cmp eax,1` plus the conditional branch establishes `EAX=1` at this site; it sets bit 0, not bit 17.
- `0x005BF48C` and `0x005BF495` OR EAX into the field after `mov eax,0x00800000`; these set bit 23.
- Other dynamic masks reviewed in the direct xrefs are concrete masks for other bits: `0xFFFFFFFB` (clear bit 2), `0xFFFFFFF7` (bit 3), `0xFFFFFFEF` (bit 4), `0xFFFFFFDF` (bit 5), `0xFDFFFFFF` (bit 25), `0xF7FFFFFF` (bit 27), and `0xFFFFBFFF` (bit 14).
- The remaining immediate RMW sites touch other bits. The complete write-like address list is stored in `WRITE_XREFS_3B84.txt`.

## Complete write-like xref list

The addresses below are VAs (subtract `0x00400000` for RVA). A mask is shown where the disassembly used an immediate operand; register operands are labelled for runtime/data-flow review.

```text
004100CD  OR  [EDI+3B84],04000000
004103A1  OR  [EBX+3B84],20000000
004103B5  OR  [EBX+3B84],01000000
004A70D9  OR  [ESI+3B84],00200000
004A70EF  AND [ESI+3B84],FFDFFFFF
004A72B1  OR  [ESI+3B84],00100000
004A72C7  AND [ESI+3B84],FFEFFFFF
004AA30B  OR  [ESI+3B84],00080000
004AA31E  AND [ESI+3B84],FFF7FFFF
004C8BD4  OR  [ESI+3B84],00000001
004E6C66  OR  [ESI+3B84],00010000
004E70CB  AND [ESI+3B84],FFFFFE7F
004E9E66  AND [ESI+3B84],FFFEFFFF
004EBD38  OR  [ESI+3B84],10000000
004EBD45  OR  [ESI+3B84],80000000
004EBDCA  OR  [ESI+3B84],80000000
004EC825  AND [ESI+3B84],EBX
004EC835  AND [ESI+3B84],EDX
004EC83B  AND [ESI+3B84],ECX
004EC846  AND [ESI+3B84],EAX
004EC880  AND [ESI+3B84],FEFFFFFF
004EC88A  AND [ESI+3B84],EDI
004ED0F6  AND [ESI+3B84],DFFFFFFF
004ED16B  AND [ESI+3B84],FFBFFFFF
004ED19F  AND [ESI+3B84],FFF9FFFF  ; clears bits 17 and 18
004ED3F2  AND [ESI+3B84],EFFFFFFF
004ED46C  AND [ESI+3B84],FBFFFFFF
004EE757  AND [ESI+3B84],FFFFFFFD
004EE778  AND [ESI+3B84],ECX
004EE793  AND [ESI+3B84],FFFFFFFE
004EED35  OR  [ESI+3B84],00000002
004EED3C  AND [ESI+3B84],FFFFFFFE
004EED67  AND [ESI+3B84],FFFFFFFD
004EF62F  OR  [ESI+3B84],00010000
004EF76F  OR  [ESI+3B84],00010000
004EF82A  OR  [ESI+3B84],80000000
004EF83A  OR  [ESI+3B84],40000000
004F0586  AND [EDI+3B84],FFFFFBFF
004F23DD  AND [EBX+3B84],FFFFEFFF
004F2403  OR  [EBX+3B84],00001000
004F247E  OR  [EBX+3B84],00001000
004F5AC4  AND [ESI+3B84],3FFFFFFF
004F5C53  AND [ESI+3B84],ECX
004F6477  AND [ESI+3B84],DFFFFFFF
004F788B  OR  [EDI+3B84],04000000
004F7BD4  OR  [EDI+3B84],EAX      ; EAX=1 at this site
0050E3A0  OR  [ESI+3B84],00008000
0050E3AC  AND [ESI+3B84],FFFF7FFF
00510629  OR  [EAX+3B84],01000000
005164D7  OR  [ESI+3B84],00004000
0052FF67  OR  [EDI+3B84],01000000
005314D4  OR  [ESI+3B84],40000000
00531594  OR  [EAX+3B84],01000000
0053423B  AND [EDI+3B84],FFFFF7FF
005BF48C  OR  [ESI+3B84],EAX      ; EAX=00800000
005BF495  OR  [EDI+3B84],EAX      ; EAX=00800000
005CAD58  OR  [ESI+3B84],01000000
005CAF10  OR  [ESI+3B84],20000000
005CAF24  OR  [ESI+3B84],01000000
005CC758  AND [ESI+3B84],3FFFFFFF
```

Static review is a lead for interpreting Stage16 events, not a substitute for runtime observation.
