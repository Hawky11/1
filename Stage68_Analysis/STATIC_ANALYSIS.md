# Stage68 — deferred-action origin: static-first boundary audit

Date: 2026-11-11. **STATIC / EXISTING-RUNTIME EVIDENCE ONLY.** Based on `stage67-runtime-native-paths-evidence` commit `a327fe2fa57ce315a75396f6daee19e1184b61d5`. All user uploads and the Stage67 working source/DLL remain unchanged. No Stage68 hooks, DLL, game run, or behavior changes were made.

## Scope and provenance

Original `AALib.exe`: SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, PE32/i386, image base `00400000`. Binary obtained from the existing Library `AALib.zip`; neither is committed. Static statements refer to exact original executable bytes, *not* injected DLL instructions. The QPC rows below all belong to the **2026-11-11 01:30:28** Stage67 session; Stage67 was installed **2/2**, and existing Stage67/65/59 HEALTH showed no recorded drops or read failures.

All addresses below are `AALib.exe` **RVAs** (subtract image base `00400000` from original VA). P1 object: `2A393020`; P2 object: `2A393C50`; runtime player-thread TID `10788`.

## 1. Initial write of deferred ID25: narrowed, but native writer not yet identified

The **same P1 object** has the following observed states:

| Evidence | QPC | O+30 | O+60 active | O+64 pending | O+68 deferred |
|---|---:|---:|---:|---:|---:|
| Stage67 kind=0 seq4412 POST | 1467958766094 | 0 | 22 | 22 | FFFFFFFF |
| Stage41 seq278 PRE (Stage40 request 2F) | 1467958862984 | not recorded | 22 | 22 | 00000025 |
| Stage41 seq278 POST | 1467958862991 | not recorded | 22 | 2F | 00000025 |
| Stage67 kind=0 seq4414 PRE | 1467958863044 | 0 | 22 | 2F | 00000025 |
| Stage67 seq4414 POST / nested Stage42 seq217 | 1467958863108 | 0 | 2F | 2F | 00000025 |

**Observed writer window:** after QPC `1467958766094` and no later than Stage41 seq278 PRE snapshot near `1467958862984`. This is a bracket, **not** a captured store instruction or an exact instant. Stage65 seq276 (`1467958862442–2446`) supplied request `2F` at P+`3F4C`; because Stage41 seq278 already sees deferred `25` at PRE and does not change it, request `2F` is **not proven** to have produced the deferred `25`. A separate origin remains unknown. No PRE/POST snapshot in either Stage41 or Stage67 directly catches `FFFFFFFF→00000025`.

Original `1E9F40` only **reads/consumes** O+68 at `1E9F46`, copies it into pending O+64 at `1E9F4E`, and clears deferred to `FFFFFFFF` at `1E9F51`. The examined `1E8DC0` arming function does not directly write O+68. Several original paths **clear** O+68 (e.g. `1E9F11`, `0EDC41`, `1904EE`) or initialize it (`1E55F9`); these are not positive-ID origins. A search for an immediate-store `MOV [reg+68], 0x25` in decoded text showed no such simple instruction; dynamic register, alias, indexed, and indirect writes are **not excluded**. Numerous `+68` stores belong to unrelated object types: the offset alone is not a trustworthy provenance filter.

## 2. Separate subsequent O+30 arming: native static writer found

P1 deferred `25` remains intact while O+30 stays zero for consecutive Stage67 windows `4414, 4416, …, 4434`. Then:

| Evidence | QPC | O+30 | O+68 |
|---|---:|---:|---:|
| Stage67 seq4434 POST | 1467959867548 | 0 | 25 |
| Stage67 seq4436 PRE | 1467959966659 | 1 | 25 |
| Stage67 seq4436 POST | 1467959966687 | 0 | FFFFFFFF |

**Observed arming window:** `(1467959867548, 1467959966659]`. This is separated from the initial deferred-ID window. Stage67 seq4436 encloses Stage42 seq218, which activates `25` from pending `2F`. One current-run promotion is established; the prior-run Stage42 seq39/155 entry states were never observed and cannot be reconstructed.

A **concrete native candidate** for O+30 arming is `AALib+1E8FEB`: bytes `C6 46 30 01`, **`MOV BYTE PTR [ESI+30],1`** inside function `AALib+1E8DC0`, whose prologue `1E8DC8: MOV ESI,ECX` binds the object. The arming instruction is reached through specific code gates:

```asm
1E8FDF  lea edx,[eax+1]        ; eax loaded from O+6C on this branch
1E8FE2  cmp edx,[ebp-8]        ; branch-local threshold
1E8FE5  jl  1E8FF1
1E8FE7  mov byte [ebp-2],1
1E8FEB  mov byte [esi+30],1    ; O+30 := 1
```

The object-update loop at `AE59A` sets `ECX=ESI` before `AE59C: CALL 1E8DC0`; the activation loop at `AD75A` sets `ECX=ESI` before `AD75C: CALL 1E9F40`. Both loops traverse objects from global `0E2E02C8` and screen `[ESI+38]` against `FFFFFFFF`. This provides a *plausible same-object execution route*. It does **not** prove that `1E8FEB` executed during the P1 arming interval: no instruction-level write to O+30 was recorded. There may be additional native writers or aliases. Avoid describing the threshold's gameplay meaning without evidence.

## 3. Independent P2 instance supports generic deferred-slot behavior

Stage67 P2 seq3463 POST `1467911310004` has deferred `FFFFFFFF`; seq3465 PRE `1467911409222` has deferred `00000026` and O+30=0. During seq3465, pending/active action `33` activates while deferred `26` stays unchanged. By seq3467 PRE `1467911508930`, deferred is again `FFFFFFFF`. The write and clearing instruction addresses are **not** observed here. Thus `O+68` can hold a queued ID even when a different requested action activates; this is not exclusive to `25` or P1.

## Reproducibility

`Stage68_Analysis/tools/verify_stage68.py` takes the original EXE and (optionally) the root of the evidence checkout containing Stage67/Stage41 logs. It verifies the original bytes and event ordering; no external Python packages, Windows APIs, game installation, or compilation are needed:

```powershell
py -3 .\Stage68_Analysis\tools\verify_stage68.py "D:\PATH\TO\AALib.exe" --logs-root .
```

The script deliberately reports the writer **unknown**, never converts a bracket or static candidate into observed instruction-level provenance, and rejects log sessions or state mismatches.

## Decision / next stage

**Stage68 finding:** original pending/deferred promotion remains explained, but a concrete positive-ID writer to O+68 is **NOT FOUND**. O+30 has one exact native static writer candidate at `1E8FEB`; capture does not verify that IP was hit. Next work should first statically trace callsites that can write the same O+68 through aliases/indirect calls, determine whether a passive scoped debugger/watch can observe `O+68` on both P1/P2, and only then design a narrowly gated Stage69 instrument. No new DLL/build/test/log upload is needed for this Stage68 report. Preserve Stage67 DLL **1,887,232 bytes** and prior Stage65 **1,879,552**, Stage59 r2 **1,153,024** rollback builds.

**Integrity:** do not conflate `0x25` deferred value with `0x2F` request value; do not conflate current seq4436 with older seq39/155; and do not infer a missing log event from the one out-of-file-order Stage59 record (its full sequence set is present).
