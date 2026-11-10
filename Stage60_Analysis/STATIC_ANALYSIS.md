# Stage60 static analysis — original X writers belong to object-motion and pair-adjustment chains

**Date:** 2026-11-10. **Status:** static/original-PE provenance VERIFIED; read-only offline verifier prepared. **No new DLL/hook, Windows build or Stage60 gameplay runtime.** No evidence of smarter AI or modified moves.

## Inputs / provenance

- [Frozen Stage59 r2 same-run logs](https://github.com/Hawky11/1/tree/stage59-r2-runtime-writer-attribution-evidence) at commit `d2ce2444b569e439db66e076f8428ac4dfc611cc`.
- [Stage59 r2 runtime analysis](https://github.com/Hawky11/1/blob/main/Stage59_Runtime/R2_RUNTIME_ANALYSIS.md): Stage55 early `AE6E0` has 1,930/1,930 net-X differences attributed to post-EIP `1E70DE`; `ACDF0` has 719/719 differences attributed to 379 native `ACC57`, 50 Stage56 detoured `ACF2A`, and 290 `126A9A`/`126B49` from nested tail. These counts belong only to the 22:07 session and are not new Stage60 runtime.
- Original *private* AALib.exe was inspected locally, never committed: SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, PE32 x86, preferred ImageBase `0x00400000`. Stage60 disassembly used `objdump -D -Mintel` and independent raw-byte/rel32 checks. All addresses below are **RVA**, not absolute runtime addresses.
- [Offline SHA-pinned verifier](tools/verify_stage60.py) checks 10 exact direct CALL targets, 5 original `FSTP` stores and 9 input-read byte signatures without injecting into the process. Local run against the fingerprinted original passed all 24 checks. It cannot prove dynamic execution of unobserved branches.

## Verified call graph

```text
 A119B -> AE6E0         EARLY object-loop update
            |
            +-- AE70C -> 1E66E0 (per-object; also five other direct call sites)
                              |
                              +-- ... 1E70DB: FSTP [ESI+7C] (X)
                              +-- 1E711A: FSTP [ESI+80] (Y)

 A11A0 -> ACDF0         PAIR / CONTACT adjustment
            |
            +-- ACE3D -> ACB80 (candidate-pair first pass)
            |     +-- ACBF2 -> 1E43A0
            |     +-- ACC02 -> 1E43A0
            |     +-- ACC57: FSTP [EBX+7C] (X)
            +-- ACE56 -> ACB80 (second pass)
            +-- ACF2A: FSTP [EAX+7C] (X), conditional on B86A0
            +-- ACFD6 -> 15CEC0 (secondary adjustment)
                          +-- 15CF5B -> 12D520 (record loop)
                                         +-- 12D58A -> 126580 (record type 7 branch)
                                                        +-- 126A9A: FSTP [ESI+7C] (X)
                                                        +-- 126B49: FSTP [ESI+7C] (X)
            +-- ACFDB -> 15B8A0 (other tail, no tracked net X in Stage59 run)
```

The early and pair stages are consecutive in the original coordinator (`A119B`, `A11A0`, `A11A5`), not independent COM-attack selector callsites.

## Raw dataflow and guard analysis

### 1. Early integration: `1E66E0`, writer `1E70DB`

- Call `AE70C -> 1E66E0` happens as `AE6E0` iterates a linked list of objects (using `ESI` as object / `ECX` as this). The function reads `[ESI+7C]` (X) and `[ESI+80]` (Y) at entry.
- The immediate X computation before `1E70DB` uses `[ESI+88]`, `[ESI+150]`, `[ESI+D4]`, a sign derived from `[ESI+24]`, local floats and a QWORD constant at preferred VA `0x007E86C8`. Relevant anchored operations include `1E70B1 FLD [ESI+7C]`, `1E70BD FLD [ESI+150]`, `1E70CF FLD [ESI+D4]`, then `1E70DB FSTP [ESI+7C]`. `1E711A` separately stores Y.
- This directly shows a position-integration calculation, but does **not** identify the ultimate writers of `+88,+150,+D4`, establish a movement-command source, or prove AI causality. It may run on both human and COM actors.

### 2. Pre-tail adjustment: `ACDF0 -> ACB80`, writer `ACC57`

- `ACDF0` invokes `ACB80` **twice** (`ACE3D` and `ACE56`) while iterating object pairs. `ACB80` inspects player-related object pointers and `[player+3F2C]`, calls `1E43A0` twice with distinct selector arguments, compares/clamps x87 scalars into locals, then computes a delta.
- At `ACC47 FLD [EBP+08]` -> `ACC4B FSUB [EBP-08]` -> `ACC51 FLD [EBP+08]` -> `ACC54 FADD [EBX+7C]` -> `ACC57 FSTP [EBX+7C]`, the adjusted object X is derived from bounded local terms plus its existing X. The confirmed Stage59 post-EIP is `ACC5A`.
- `ACF2A` is another original inline X store within `ACDF0`: `ACF23 FLD [EBP+ECX*4-5C]`, `ACF27 FADD [EAX+7C]`, `ACF2A FSTP [EAX+7C]`, reached only after a set of object/flag checks including `ACF0F CALL B86A0; ACF17 TEST AL,AL; ACF19 JNE ACF2D`. Stage59 recorded 50 changed Stage56-detour writes in this separate run. The detoured post-EIP outside AALib must not be called a native RVA without a module mapping.
- `ACB80` is **not** an independent COM action selection routine on present evidence; it manipulates object coordinates and constraints.

### 3. Tail pair resolver: `15CEC0 -> 12D520 -> 126580`

- The `ACFD6` native call to `15CEC0` reaches `12D520` at `15CF5B`. The record processor iterates a buffer at global `[0x0E1AE840]`; at `12D58A` it invokes `126580` only for a record whose first DWORD is **7** (`12D583 CMP ESI,7; 12D586 JNE 12D594`). This is a **record type value**, not an attack/action ID.
- `126580` reads pointers from record offsets `+0x40` / `+0x60`, compares `[object+DC]`, reads `[object+88]`, derives sign from `[object+24]` and performs x87 comparisons/clamping. It computes local displacement terms `[EBP-4]` and `[EBP-C]` with further bounds involving `[object+1B8]` and `[object+1BC]`.
- The first guarded X correction is `126A94 FLD [ESI+7C]; 126A97 FADD [EBP-4]; 126A9A FSTP [ESI+7C]`. The second is `126B43 FLD [ESI+7C]; 126B46 FADD [EBP-C]; 126B49 FSTP [ESI+7C]`. Only certain object-pair/ordering conditions reach these branches.
- This call graph and operand dataflow are consistent with a spatial/contact constraint solver; the precise game-specific mechanics (hurtboxes, pushboxes, collision type 7, etc.) **remain hypotheses**, not established facts.

## Stage59 dynamic cross-check

| Original store RVA | post-write RVA | Stage59 raw changed events overall | Strict Stage55 attribution |
|---|---|---:|---|
| `1E70DB` | `1E70DE` | 1,985 | 1,930 changed AE6E0 windows |
| `ACC57` | `ACC5A` | 379 | 379 pre-tail changed ACDF0 windows |
| `ACF2A` | outside-module detour EIP | 50 in 288 outside-module events | 50 pre-tail changed ACDF0 windows, nested in Stage56 |
| `126A9A` | `126A9D` | 148 | 145 changed ACDF0/15CEC0 windows |
| `126B49` | `126B4C` | 148 | 145 changed ACDF0/15CEC0 windows |

Stage59 captured **no unexplained net-X change** in the sampled complete parent windows. Static confirmation does not imply that every possible writer or every scenario was covered.

## Decision / next investigation

**Stop instrumenting these final X stores**: they are sufficiently attributed for the tested session and no direct AI action decision is established at them. Replacing their output with a guessed displacement could corrupt engine position/collision and would not prove smarter AI.

**Stage61 proposal (static-first):** determine the *upstream producers* of the early integrator's `object+88`, `object+150` and `object+D4` terms and the command/action producer chain already identified at `F6AD7` and the later command/queue observers. Search for exact native writers, classify routine ownership and call contexts, and find same-actor evidence linking a COM decision to a movement term. Only once a specific decision boundary is identified should a narrow passive hook be developed with preserved ABI and no competing DR ownership. Any later behavioral changes require replayable COM-vs-COM A/B evidence.

**No Stage60 runtime artifact is required**: the current Windows DLL remains Stage59 r2 (1,153,024 bytes), and no extra Stage55–59 logs need uploading for this static task. Preserve original PE private and do not upload the executable.
