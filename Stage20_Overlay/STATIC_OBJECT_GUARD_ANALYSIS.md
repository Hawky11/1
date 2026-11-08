# Stage20 static analysis — downstream object bit-2 guard

## Binary identity

The source of truth is the user's `AALib.exe` from `AALib.zip`:

- PE32 / Intel 386
- Preferred ImageBase: `0x00400000`
- SHA-256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`

## Caller path after the Stage18 table gate

At caller RVA `0x000C64ED`, the original instructions are:

| RVA | Bytes | Instruction |
|---:|---|---|
| `0x000C64ED` | `8B 0B` | `MOV ECX,[EBX]` |
| `0x000C64EF` | `6A 02` | `PUSH 2` |
| `0x000C64F1` | `E8 0A F6 FC FF` | `CALL 0x00495B00` |
| `0x000C64F6` | `84 C0` | `TEST AL,AL` |
| `0x000C64F8` | `0F 85 0D FF FF FF` | `JNE 0x004C640B` |
| `0x000C64FE` | `6A 71` | `PUSH 0x71` |
| `0x000C6500` | `8B CE` | `MOV ECX,ESI` |
| `0x000C6502` | `E8 99 95 F4 FF` | `CALL 0x0040FAA0` |

The Stage18 table helper is earlier in this function. When it allows the path, execution reaches this second guard. The original JNE skips the setter when the helper returns nonzero; the bit-113 setter is reached only when this helper returns AL=0.

## Helper 0x00495B00

Static disassembly:

```asm
00495B00  push ebp
00495B01  mov ebp,esp
00495B03  mov eax,[ebp+8]             ; argument = 2
00495B06  push ebx
00495B07  push esi
00495B08  mov esi,ecx                ; object from [EBX]
...
00495B19  cdq
00495B1A  and edx,1Fh
00495B1D  mov ebx,1
00495B22  shl ebx,cl                  ; mask for argument 2 = 0x4
00495B24  add eax,edx
00495B26  sar eax,5
00495B29  and ebx,[esi+eax*4+3B70h]   ; bit test in object field +0x3B70
...
00495B37  mov al,bl
00495B39  pop ebx
00495B3A  pop ebp
00495B3B  ret 4
```

With the fixed caller argument `2`:

- mask = `1 << 2 = 0x00000004`;
- array DWORD index = 0;
- tested DWORD = `[object+0x3B70]`;
- AL = 1 when bit `0x4` is set; AL = 0 when it is clear.

The exact AL from the helper is the authoritative branch input. The post-return read of the object DWORD is corroboration; a concurrent mutation could make a reread differ.

## Why Stage20 wraps the CALL rather than patching TEST/JNE

Stage20 changes only the five-byte CALL at RVA `0x000C64F1`. The wrapper:

1. Receives the original callsite return address and original argument `2`.
2. Duplicates argument `2` and calls the original helper at `0x00495B00) with the original ECX object pointer.
3. Lets the original helper execute normally, including its own `RET 4` of the duplicated argument.
4. Saves all returned registers and EFLAGS, records AL/EAX, the original ECX object pointer saved by the wrapper, EBX slot pointer, `[EBX]`, `[object+0x3B70]`, bit-2 state, player fields, QPC and stack context.
5. Restores the exact helper return registers/flags and executes `RET 4` to consume the original caller argument and return to `0x000C64F6).

The original `TEST AL,AL` at `0xC64F6` and JNE at `0xC64F8` remain byte-for-byte untouched. Stage20 adds no DR0–DR3 watchpoints and does not modify the helper's result or the caller branch.

New runtime output: `ArcanaAI_Bit113ObjectGuardTrace.log`.

## Stage18/19 correlation target

Stage19's 2026-11-08 run produced:

- 31 P2 gate calls with `stateIndex=0x19`, `AL=1`, with no subsequent Stage17 setter call.
- Four setter episodes (P1/P1/P2/P1). The corresponding Stage17 records show `OtherObject3B70` values `0x440`, `0x640`, `0x460`, `0x640`, all with bit `0x4` clear.

A set bit-2 in the object checked by helper `0x495B00` is a plausible explanation for why the 31 P2 table-accepted calls do not reach the setter, but Stage19 does not record the actual object guard return for those calls. Stage20 must test this directly:

| Expected group | Object guard AL | `[object+0x3B70] & 4` | Setter |
|---|---:|---:|---|
| Four known setter episodes | Expected 0 | Expected 0 | Observed |
| P2 index-`0x19` gate-only calls | Hypothesis: 1 | Hypothesis: nonzero | Not observed |

These expected values are hypotheses to test, not prior runtime facts.

## Startup validation and upload

The log must contain:

- `STAGE20_BIT113_OBJECT_BIT2_GUARD_BUILD=2026-11-08-r1`;
- `STAGE20_CONFIG: EnableStage20Bit113ObjectGuardTrace=1 ...`;
- successful wrapper installation at RVA `0x000C64F1`, original helper RVA `0x00095B00`;
- `Stage20 trace active`.

Upload Build.log plus:

- `ArcanaAI_Bit113ObjectGuardTrace.log`;
- `ArcanaAI_Bit113GateTrace.log`;
- `ArcanaAI_Bit113SetterTrace.log`;
- `ArcanaAI_StateMapTrace.log`;
- `ArcanaAI_PlayerBit17WriteTrace.log`;
- `ArcanaAI_ProducerCallTrace.log`;
- `ArcanaAI_QueueTrace.log`;
- `ArcanaAI_EventDispatchTrace.log`;
- `ArcanaAI_FinalCommandTrace.log` and `ArcanaAI_AlternateCallTrace.log` when available.

Do not force-edit object fields or player state to manufacture a branch outcome. Keep the test natural. If the wrapper fails validation or installation, stop before gameplay and diagnose the Stage20-specific log.

## Coverage limitation

This wrapper records calls that reach `0xC64F1). A state-table AL=0 path returns before this guard; the Stage18 helper trace remains required to observe table AL=0. Preserve Stage16–Stage19 and all earlier evidence logs for QPC correlation.
