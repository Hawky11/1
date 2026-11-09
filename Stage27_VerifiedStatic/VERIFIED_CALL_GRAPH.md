# Stage27 static verification — exact AALib.exe (2026-11-09)

This follow-up supersedes **only the unverified CALL-target inference** from `Stage27_Analysis/STAGE27_LIFECYCLE_ANALYSIS.md`. It does not alter any historical source/evidence branch, the Stage26 runtime trace, or the 70 paired set/clear events.

## Provenance / integrity

- The user ran the original `Stage27_Analysis/tools/analyze_bit0_lifecycle.py --binary` locally against `G:\Arcana Heart 3 - LS XTEND\AALib.exe` and uploaded `Stage27_AALib_StaticReport.txt`.
- Its **exact original CRLF 2,389-byte uploaded report** is preserved in this branch (Git blob `7e3dc3a77f602fde1e82ca9fe70903d4d29dd9c7`; 14 lines).
- The reported original game executable SHA-256 matches the previously pinned fingerprint `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. The whole original binary is not uploaded.
- All four predicted writer signatures match: RVA `0x95F55`, `0x97031`, `0x975A6`, `0x97873`. The primary helper prologue at `0x97870` also matches.
- The report's `WARNING: direct call destination differs from primary helper entry` flags a **mistaken Stage27 interpretation**, *not* corrupted game code or failed fingerprint validation.

## Verified direct CALL graph

| Actual CALL RVA | Machine-code form | Verified destination | Return RVA |
| --- | --- | --- | --- |
| `0x979CC` | E8 rel32 | `0x97870` (primary clearer) | `0x979D1` |
| `0x97609` | E8 rel32 | **`0x97970`** (distinct function) | `0x9760E` |

The previous Stage27 report mistakenly proposed two *direct* CALLs to `0x97870`. Its **dynamic stack observations remain valid**:
- 34 Stage26 bit0 clears had `stack[1]=0x979D1`, and all 34 also held `0x9760E` deeper in the captured stack. This is consistent with `0x97609 → 0x97970`, then `0x979CC → 0x97870`.
- 20 Stage26 bit0 clears had `stack[1]=0x9760E` even though the static CALL at `0x97609` targets `0x97970`. An **unverified control transfer** (possible tail jump/trampoline or stack handling) links this caller-return context to `0x97870`; **the current report contains no bytes from the function `0x97970` itself**. Do not claim which transfer happens until the next static byte-window report is inspected.
- These same 54 events are the `0x97873` clear path. The other three clear sites are separate operations and cannot be folded into this graph without more evidence.

Working graph, with an explicitly unproven edge:

```text
AALib +0x97609  CALL +0x97970  (return +0x9760E)
                         |
                         +-- verified nested CALL +0x979CC -> +0x97870 (34 captures)
                         |                                      -> return +0x979D1
                         |
                         +-- ???? transfer to +0x97870 (20 captures)
                               stack[1] retained +0x9760E
```

## Stronger reconstruction from the byte windows already in the user's report

These are x86 machine-instruction decodings of the **provided hex windows**, without any claim to decode missing regions.

### Primary helper `RVA 0x97870`

```asm
97870  push edi
97871  mov edi, eax
97873  and dword ptr [edi+0xC8], 0xFFFFFFF2
9787A  lea eax, [edi+0x10]
9787D  push 0
9787F  push eax
97880  mov dword ptr [edi+0xD4], 2
9788A  call 0xF7EB0
9788F  push 1
97891  call 0x976D0
97896  push edi
97897  call 0x99150
9789C  or dword ptr [edi+0xC4], 0x400
978A6  add esp, 0x10
978A9  pop edi
978AA  ret
```

At the observed write, `EDI = player+0x30`: the helper clears selected bits at `player+0xF8`, writes `player+0x104 = 2`, then sets bit `0x400` at `player+0xF4`. It also calls helpers at `0xF7EB0`, `0x976D0`, and `0x99150`. This **does not** imply those values are the same as the controller's `[controller+8]` flag observed in Stage22, nor prove a specific gameplay meaning.

### Conditional bit0-only clearer `RVA 0x975A6`

```asm
97586  cmp word ptr [ebx+0x3A34], 1
9758E  jne 0x97649
97594  mov eax, [esi+0xC8]
9759A  test al, 1
9759C  je 0x97649
975A2  and eax, 0xFFFFFFFE
975A5  pop edi
975A6  mov [esi+0xC8], eax
975AC  pop esi
975AD  pop ebx
975AE  mov esp, ebp
975B0  pop ebp
975B1  ret
```

This observed path only executes its clear when the player-related 16-bit state at `[EBX+0x3A34]` is 1 **and** bit0 at `[ESI+0xC8]` was set. It preserves the other bits of the DWORD. The seven observed Stage26 hits were P1, but this is not evidence that P2 can never use it.

### Broader reset paths

- `RVA 0x95F4E: XOR EAX,EAX`; `0x95F55: MOV [EDI+0xC8],EAX`; `0x95F5B: MOV [EDI+0xD4],EAX`; calls `0xF7EB0` and `0x98E00`, and clears bits in `[EDI+0xCC]`. In the captured samples EDI pointed to player+0x30; this is a **multi-field reset**, not just bit0.
- `RVA 0x97020` function prologue; `0x97028: XOR EBX,EBX`; `0x9702B/0x97031/0x97037: MOV [ESI+0xC4/C8/CC],EBX`. This also resets several fields and calls additional helpers. Based on the bytes this is a **multi-field initializer/reset path**, exact gameplay trigger still unknown.

## Follow-up

The new read-only `Stage27_VerifiedStatic/tools/inspect_call_flow.py` verifies the *same SHA256*, both actual E8 CALL destinations and primary helper signature, then exports address-labeled code windows centered on `0x97970`, `0x97609` and related sites. It runs entirely offline without writing to game memory, injecting a DLL, or requiring Capstone. A short TXT report will resolve whether a branch, tail jump or another mechanism accounts for the 20 `0x9760E` stack returns.

Five synthetic regression tests of the inspector (PE section mapping, rel32 positive/negative checks, window coverage and reject x64 PE) **passed locally**. The exact user game executable was not available in the assistant environment, so the new script's real-game verification remains pending.
