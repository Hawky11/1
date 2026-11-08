# Stage19 static analysis — bit-113 state-code mapping

## Binary identity

The analysis uses the same PE32 `AALib.exe` examined for Stage18:

- Preferred ImageBase: `0x00400000`
- SizeOfImage: `0x0E0EB000`
- SHA-256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`

The file-backed `.text` section contains the converter and mapping helper. The primary and secondary state-table pointer arrays and their pointed-to table data are in the file-backed `.data` section.

## Corrected Stage18 table finding

The three Stage18 bytes are not evidence of three runtime writes. Their corresponding preferred-image addresses already contain `0xFA` in the on-disk PE:

| Selector | State index | Preferred VA | Runtime VA in Stage18 | PE byte |
|---:|---:|---:|---:|---:|
| `0x0A` | `0x0E` | `0x04C7DBDD` | `0x0559DBDD` | `0xFA` |
| `0x17` | `0x19` | `0x04C8073B` | `0x055A073B` | `0xFA` |
| `0x0A` | `0x18` | `0x04C7DCE1` | `0x0559DCE1` | `0xFA` |

The runtime module base inferred from Stage18 addresses is `0x00D20000`, a relocation delta of `+0x00920000` from the preferred base. The relocated addresses match the Stage18 log exactly.

Primary table pointer array: preferred VA `0x04C82518`. Its selector `0x0A` entry points to `0x04C7DA58`; selector `0x17` points to `0x04C80498`. The row stride is `0x1A`; key offset is `0x19`.

This establishes that the three observed `0xFA` values are present in the file's initialized `.data` image and are then observed at their relocated runtime addresses. It does **not** prove that the writable runtime tables can never be modified later. Stage19 therefore does not claim to have identified every possible writer. It changes the immediate question from "who wrote these three FA bytes?" to "which raw player states map to which table rows, and can a reachable row contain the `0xFF` sentinel?"

### Candidate AL=0 rows in the two observed selectors

A byte-by-byte read of the on-disk primary tables for key offset `0x19`, restricted to the state indices that can pass the caller's `<0x1C` check, gives:

| Selector | `0xFF` rows (valid indices `0x00..0x1B`) |
|---:|---|
| `0x0A` (observed P1) | `01, 03, 09, 0F, 14, 15, 16, 17` |
| `0x17` (observed P2) | `01, 03, 07, 09, 0D, 0F` |

All key-`0x19` cells at mapped rows `0x18..0x1B` are `0xFA` for these two primary-table selectors. Therefore, assuming the runtime table has not been changed, a genuine `AL=0` for the two observed selectors is most directly expected from a *direct* state index in the listed `0x00..0x17` rows, not from the mapped results `0x18..0x1B`.

The Stage19 hook at `0x4F6C08` only sees calls through the `0x4F02C0` conversion path. Direct raw-state returns (`<=0x17`) do not pass that hook. Stage18's gate observer at `0xC64A5` does observe the subsequent `0x4F2FD0` call for those direct states, so keep Stage18 enabled and use its actual `resultAL` as the authoritative AL=0 evidence.

## Caller and mapping helper

The state-index wrapper is `0x004F6BA0`, called through `0x004F6ED0`. It reads:

- `WORD [player+0x3A34]` — the first state-validity comparison expects `1`;
- signed `WORD [player+0x3A36]` — the raw state code.

For raw codes `<=0x17`, the value is returned directly. The intermediate raw range `0x18..0x1F` is rejected by this function's branch structure. For the mapped path, caller instructions at:

| RVA | Bytes | Instruction |
|---:|---|---|
| `0x000F6C01` | `56 57` | `push ESI; push EDI` |
| `0x000F6C03` | `E8 B8 96 FF FF` | `CALL 0x004F02C0` |
| `0x000F6C08` | `83 C4 08 83 F8 05` | `ADD ESP,8; CMP EAX,5` |
| `0x000F6C0E` | `75 0A` | `JNE 0x004F6C1A` |

The helper receives `[EBP+8]=player`, `[EBP+0xC]=rawCode`. It computes:

`rawIndex = rawCode - 0x20`

It checks `rawIndex < DWORD [player+0x1220]`, then reads a WORD at:

`player + 0x2B9E + rawIndex * 0x14`

The returned value is selected by the first matching flag test, in this order:

| WORD flag mask | Helper result |
|---:|---:|
| `0x0400` | 0 |
| `0x0800` | 1 |
| `0x1000` | 2 |
| `0x0200` | 5 |
| `0x2000` | 3 |
| `0x4000` | 7 |
| `0x8000` | 4 |
| `0x0008` | 6 |
| no listed flag / out of range | `0xFFFFFFFF` |

The caller converts helper result to its state index:

| Helper result | Returned state index |
|---:|---:|
| 0 | `0x18` |
| 1 | `0x19` |
| 2 | `0x1A` |
| 3 | `0x1B` |
| 4 | `0x1C` (subsequently rejected by caller) |
| 5 | `0x1A` |
| 6 | `0x1B` |
| 7 | `0x1C` (subsequently rejected by caller) |
| `0xFFFFFFFF` | `0xFFFFFFFF` (rejected) |

The caller then rejects values `>=0x1C` before calling the Stage18 table gate. Thus Stage18's mapped rows can only be `0x18..0x1B`.

Stage18's P1 raw code `0x20` / state index `0x18` implies helper result 0. Its P2 raw code `0x5C` / state index `0x19` implies helper result 1. Stage19 captures the raw code, the result from `0x4F02C0`, the WORD flags used for that conversion, and the state index derived by the caller.

## Stage19 hook contract

The passive observer is placed at RVA `0x000F6C08`, immediately after the call to `0x4F02C0`. It replaces the complete six-byte sequence:

- original: `ADD ESP,8; CMP EAX,5`;
- patch: `CALL ArcanaStage19StateMapHook; NOP`;
- hook cleanup: removes its synthetic CALL return address plus the original two cdecl arguments (`ADD ESP,0x0C`), recreates `CMP EAX,5`, then jumps to the untouched original `JNE` at `0x000F6C0E`.

The logger reads its arguments from the saved hook-entry context after accounting for the synthetic CALL return address. The helper's actual EAX is retained. The hook does not change the map result or branch flags.

It logs to `ArcanaAI_StateMapTrace.log`:

- raw player state code and derived raw index;
- original `0x4F02C0` return EAX;
- state index that the caller derives from that return;
- whether the derived index would pass the caller's `<0x1C` bound;
- player `+0x3A34` / `+0x3A36`, entry count, and the WORD flag field used by the helper;
- QPC, thread, P1/P2 side, EBP chain, registers, read mask and ring-drop warnings.

The logger's reread of the WORD flags occurs immediately after the helper returns, not inside its compare instruction. The helper's returned EAX remains authoritative if the game mutates the record concurrently. The log labels reread failures via `readMask`; do not interpret an unread `0xFFFFFFFF` sentinel as the helper's returned value.

No hardware debug registers are added or claimed by Stage19. Stage16 continues using DR0/DR1 for bit17, and the command trace retains DR2/DR3.

## Runtime questions

1. Do the raw codes and WORD flags explain the observed mappings `0x20 -> 0x18` and `0x5C -> 0x19`?
2. Do any reachable P1/P2 mapped rows lead to state index `0x1A` or `0x1B`?
3. Can a naturally reached, valid state index select a table cell containing `0xFF`, producing an actual Stage18 `AL=0`?
4. Are the table bytes still equal to their PE initial values at the time of Stage18's helper call?

The Stage19 hook covers the specific mapped path through `0x4F6C03). Direct raw-code returns (`<=0x17`), invalid-state exits and the special jump-table path are outside this hook. Keep Stage18 enabled to correlate only those map records that continue to its table gate.

## Verification status

This is a source-only diagnostic revision. It has not been built with Windows/MSVC or runtime-tested. The original Stage18 source and Stage18 evidence branches must remain unchanged.
