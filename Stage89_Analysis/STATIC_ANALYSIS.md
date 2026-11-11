# Stage89 — static localization of the first native reader for Stage88 controller bits

**Status: STATIC ANALYSIS ONLY — no Stage89 hook, build, runtime log, or AI behavior change.** Original user Library `/AALib.zip` → `AALib.exe` SHA-256 verified: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Used RVA-correct `.text` disassembly (RVA 0x1000, raw 0x400). Do **not** use EXE file offsets as RVA.

[Stage88 runtime predecessor](../Stage88_Runtime/RUNTIME_ANALYSIS.md) · [Stage88 same-session upload](https://github.com/Hawky11/1/commit/ccbbf6a41cd7ed5ba0fdf194f557a4bbf112b3ca).

## Native helper F58E0

Function RVA `0xF58E0` takes cdecl `(bitmapPtr, bitIndex)`. At `F58EA CMP EAX,0x26`, the special-case `bitIndex=0x26` reads `[bitmapPtr+4]` at **F58EF**. If `bitmapPtr=P+0x3A08`, this is `P+0x3A0C` — the second DWORD Stage88 observes.

- `F58F2 TEST ECX,0x200`
- `F58FF TEST ECX,0x400`
- `F5907 TEST ECX,0x800`
- If **any** is nonzero, `F58FA MOV AL,1`, then return (same for all three, no individual selector outcome).
- Otherwise execution reaches the ordinary index-check path `F590F..F593E` for bit index `0x26`. Consequently `AL=1` **may still occur** with all three special bits zero if the generic bit is set (bit38 → `[bitmapPtr+8]` bit 0x40 = `P+3A10 & 0x40`). A future logger must include all three output DWORDs and actual return AL, not assume `AL=0` whenever `P+3A0C & 0xE00==0`.

This is a genuine **read** of the output Stage88 proved F57D0 can write. It is **not** yet shown to execute in temporal proximity to any Stage88 event.

## Direct native CALL sites with literal 0x26

Disassembly of the 2.86 MB original `.text` finds 48 direct `CALL 0xF58E0` instructions; **four verified sites** pass immediate `0x26` and a pointer to a player's `+3A08` bitmap:

| CALL RVA | Native CALL bytes | Read argument construction | Result branch |
|---|---|---|---|
| `0x9A870` | `E8 6B B0 05 00` | `9A868 ADD EAX,3A08; 9A86D PUSH 26; 9A86F PUSH EAX` | `9A878 TEST AL,AL; 9A87A JE 9A8CA`; nonzero enters state-write path |
| `0x9AA20` | `E8 BB AE 05 00` | `9AA18 ADD EAX,3A08; 9AA1D PUSH 26; 9AA1F PUSH EAX` | `9AA28 TEST AL,AL; 9AA2A JE 9AA62`; nonzero enters state-write path |
| `0x9AB00` | `E8 DB AD 05 00` | `9AAF8 ADD EAX,3A08; 9AAFD PUSH 26; 9AAFF PUSH EAX` | `9AB09 TEST AL,AL; 9AB0B JE 9AB4C`; nonzero enters state-write path |
| `0xAAB55` | `E8 86 AD 04 00` | `AAB4C LEA EAX,[ESI+3A08]; AAB52 PUSH 26; AAB54 PUSH EAX` | `AAB5D TEST AL,AL; AAB5F JNE AAB72`; zero can continue to `AAB65 CALL E8560` |

The first three callsites are reached only after checking `[P+3B70]` bit2 (at 9A863/9AA13/9AAF3), a **conditional guard**, not proof of execution. The AAB55 route has additional earlier guards at AAB17..AAB4A. Do **not** call any of these an attack/hit/animation executor merely from these instructions.

## Recommended Stage89 instrumentation, not implemented

1. Choose a specific one (or carefully verified set) of these four **CALL sites**, rather than hooking function `F58E0` globally, avoiding unrelated paths and existing Stage36 conflict sites.
2. Before patch: exact PE fingerprint and original bytes/signatures, prevent overlap, suspend peers with EIP audit, rollback on failed patch, mutex. Preserve legacy hooks. A CALL wrapper should invoke original `F58E0` **once** with original arguments and preserve AL/EFLAGS/stack/XMM/x87. Record only to bounded memory, flush off the game thread.
3. Capture callsite RVA, QPC entry/exit, side/P pointer, three bitmap DWORDs (read masks), input bitIndex `0x26`, returned AL, native test outcome, and branch condition. Correlate only with same-session Stage88 by P/TID/QPC and explicitly distinguish **immediate/nested causal** evidence from temporal proximity.
4. Retain `EnableStage36ControlToCommandTrace=0` unless its prior Stage82 collision is independently resolved. **No synthetic inputs, no changed controller bitmap, no second EE306 hook.**
5. Seek naturally observed selector2/bit0x800 too; Stage88 this run saw selectors 0 and 1 only.

**Conclusion:** a specific downstream reader of the Stage88 output is statically identified, with four concrete native callsites. Stage89 runtime, and any causal link to actual COM behavior, remain **unverified**.
