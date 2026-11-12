# Stage125 — original x86 command candidate, record packet, CEA20 state synchronizer, native consumers

**2026-11-13 · STATIC ONLY. NO DLL / hooks / game-run / game writes.** Original user private `AALib.exe` PE32 x86 175,556,608 bytes, preferred base `0x00400000`, SHA256 **28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba** independently reverified.

**Full reproducible offline artifact** in user's conversation: `ArcanaAI_Stage125_NativeCommandPacket_CEA20_Consumer_StaticAudit.zip`, **23,966 bytes**, SHA256 **c2627add99b5c32debffcf255128614f94fc0b6c45ab3964a403b65721a888a9**, **12 files**, ZIP CRC PASS. It contains complete **Russian** report, 149-anchored-original-opcode JSON, 16 original relative branch edges, 173-line original direct E8 callsite CSV, 50 original field xref CSV, GNU objdump native contexts, pure-stdlib EXE verifier, 50-unit-test suite and outputs. **No original executable or compiled DLL is bundled**.

## Verification

All against original **on-disk** native bytes: **149/149** unique pinned instruction sites, **16/16** branch rel destinations, **173/173** direct E8 CALLs (independent full original .text byte scan reconciled to disassembled sites), **50/50** field operand-reference bytes and mappings. The CALL count by native target: `F2DE0` **137**, `CEA20` **18**, `CBDE0` **12**, `CBF20` **1**, `CBE60` **5**. Original field operand refs: `+0x28010` **11**, `+0x28014` **4**, `+0x28018` **17**, `+0x2801C` **6**, `+0x28020` **8**, `+0x28024` **4** = **50**. The portable unit suite **50/50 PASS** (real signed CALL/Jcc targets, corrupted-site bytes, source PE and catalog negatives). Counts reflect **static instructions**, not runtime events.

## Critical division of responsibility — not an 'attack commit' yet

| Native code | Original machine operations | Evidence boundary |
|---|---|---|
| `F7AD1 CALL F3930`, `F7B0A` | Producer provides candidate and branching to Type1/MODE0 path | Candidate != accepted game action |
| `F7C5D` Type1 and `F7CC0 CALL F2DE0` MODE0 | Populate small record fields: WORD offsets +2/+4, BYTE +0/+1/+6, conditional +0xF eligibility | **At F7CC5 MOV EAX,[EBP-4] the CALL's EAX return is overwritten**; there is no immediate success TEST |
| `F7DA3 CALL CEA20` on special MODE0/ESI=0x27 | Synchronizes selected context field triplet and clears record+0x10; returns via RET4 | **CEA20 has no explicit AL success or ACK**, and a POST event is not action acceptance |
| `F7DA8` | Reads `BYTE [EBX+0x55]`; `JE F7D8D` when 0; if nonzero executes two `CBDE0` auxiliary record resets | Both modes of this **special 0x27 path** return **AL=0** at F7D8F or F7DCB, even after CEA20; this cannot be equated with attack rejected |
| `CEC57/CEC64` and `CF0BF` | Separate **conditional readers** of same state context arrays | Static presence != runtime hit or gameplay execution |

### Exact CEA20 branches

- `record == nullptr` at `CEA2A`: copy original tick `[0x0E100DFC]` into context `+0x28010,+0x28014,+0x28018,+0x2801C`, and `0x7FFFFFFF` into `+0x28020,+0x28024`; `CEA60 RET4`.
- `record != nullptr`: **`CEA66 MOV BYTE [record+0x10],0`**; index is `DWORD [record+0x14]`. If `[record+0x18]!=0`, copy `[record+0x8]` into both indexed `context+0x28018` and `+0x28010`, then `BYTE record[0x11]` into indexed `+0x28020`, return `CEA9A RET4`.
- If `[record+0x18]==0`, copy `DWORD record[0]` to indexed `+0x28010`, `DWORD record[0x8]` to indexed `+0x28018`, `BYTE record[0x11]` to indexed `+0x28020`, return `CEAC3 RET4`.

These are **numeric record/context fields**, not proven gameplay categories like accepted attack/animation/hit. No local bounds check for record index was seen in CEA20; upstream checks unknown.

### Actual downstream branch after Stage107 POST

The Stage123 native observer Stage104 logged **25/25** MODE0(ESI=0x27)→CEA20 PRE and Stage107 **25/25** PRE→POST (P1=8/P2=17). The Stage107 POST hook at **F7DA8 occurs before `CMP [EBX+0x55],0`/Jcc F7DAC**. It does **not** log the flag value or the later chosen branch. Both branches of this special 0x27 suffix return AL=0. There is an unrelated branch returning AL=1 at `F7E3A`; function-level return-value meaning remains unproven.

### Proven readers and live evidence

- `CEC57` checks `BYTE [record+0x10]>0` before `CEC64` compares `BYTE [record+0x11]` against `context[+0x28020+index*4]`, with additional differences against `+0x28010` and `+0x28018`. CEA20 clears that same record byte; this supports a *state sync versus later activation/validation* distinction, but does not prove an action dispatch.
- `CF0BF` is another original compare against `context+0x28020`, `CF0C8` subtracts `+0x28010`. **No runtime counter for CF0BF** exists. Stage123 recorded **Stage110 CEBE0 entry=0 / guard=0** and **Stage107 CEC64 read=0** in its reported health windows. Do not claim that all state readers or all native command consumers are inactive.
- The producer `F3930` independently has a conditional `F3B19 CALL FA480` which may set `[ESI+0x252]` if an additional global is 3, but the producer continues when that condition is false. The monitored Stage113 FA480 call `98886` is a **different** native site.
- The source-original F2DE0 instruction sequence is **not guaranteed to be the loaded runtime implementation**: Stage104 already reported an *existing unknown-owner E9+NOP detour* at F2DE0. Preserve it; do not claim this audit checked third-party code live.

## Real conclusion and Stage126 handoff

This Stage125 is **not** a claim that native action acceptance has been proven. The validated producer→record→CEA20→POST chain is real and Stage123 reached it, but the actual execution/commit/hit destination has **not** been identified. In particular neither `CEA20` nor `Stage107 POST` is a commit flag, and MODE0 suffix AL=0 need not imply a rejected move.

**Next Stage126 STATIC-FIRST:** trace original **dispatch/consumer** paths and actual record hand-off via the function-pointer table preferred VA `0x007F8380`, indirect `CALL EAX/EDX` in `CEB4A/CEE04/CEE31`, and writers/readers of `record[+0x10]`; clarify actual game-side execution gate and owner of F7xx return AL. Only after a specific evidence gap should a **narrow passive positive-controlled runtime observer** be considered. No FA480 override, no broad hooks, DR writes, gameplay writes or Stage36.

Known Windows backups: **Stage122 DLL 2,089,984 B**, Stage115 **2,082,304 B**, Stage113 **2,079,744 B**, Stage110 **2,073,088 B**. The Stage125 ZIP itself **requires no installation or rebuild**.

[Prior Stage124 229 FA480 direct-caller original native audit](https://github.com/Hawky11/1/blob/stage124-fa480-229-callers-static-classification/Stage124_Analysis/FULL_STATIC_CALLER_AUDIT.md) · [Stage123 actual writer runtime evidence](https://github.com/Hawky11/1/blob/stage123-positive-g4-writer-windows-runtime-audit/Stage123_Runtime/FULL_G4_WRITER_SAME_SESSION_AUDIT.md).
