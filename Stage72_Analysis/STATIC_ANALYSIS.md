# Stage72 — native callback dispatch and deferred writer: original switch + frozen runtime correlation

**Date:** 2026-11-11. **STATIC VERIFIED / EXISTING RUNTIME-LINKED CALLBACK CHAIN VERIFIED; selector writer IP still UNOBSERVED.** No new DLL, hook, gameplay run, binary modification, AI change or user log upload. Branch based on [Stage71 selector source](../Stage71_Analysis/STATIC_ANALYSIS.md), retaining [Stage70 r2 raw user capture](../Stage70_R2_Runtime/RUNTIME_ANALYSIS.md). Original private \`AALib.exe\` SHA256 \`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba\`. All 23 Stage72 critical opcode ranges and switch case entry were independently validated on that exact PE32 image.

## 1. Important correction to the callback "scheduler" concept

Two functions previously described loosely as scheduling/dispatching are in fact simple, synchronous field writers.

\`\`\`asm
AALib+0F7E70  push ebp
              mov ebp,esp
              mov eax,[ebp+08]      ; destination descriptor D
              mov ecx,[ebp+0C]      ; new callback pointer
              mov edx,[ebp+10]      ; callback argument/parameter
              mov [eax],ecx        ; D+0 = callback
              mov [eax+04],edx     ; D+4 = parameter
              pop ebp
              ret
\`\`\`

In this native path D=\`P+3ED0\`. Hence **\`P+3ED0\` is the callback function pointer** and **\`P+3ED4\` is its saved parameter**, not generic action IDs. The original call site \`156BCE\` installs a callback through this two-DWORD setter; \`15679A\` installs the follow-on callback after Stage70's deferred \`O+68\` store.

Execution happens **later**, inside the original COM loop at \`AALib+EE78D\` (\`MOV EDI,[P+3ED0]\`), via the indirect native \`EE7BE: CALL EDI\` after preparing a callback frame with \`156C00\`. In \`156C00\`, \`P+3ED4\` becomes the callback context \`[frame+0C]\`. This is precisely what Stage38 captures.

**The other "event" is a separate structure:** in Stage71 \`155D27 PUSH 21h\` and \`155D3C CALL 0F2DE0\`, the original \`0F2DE0\` writes two WORD fields of an event record at **\`P+3A2C\`**: **code \`0x21\` at \`P+3A30\`** (record+4) and **mode \`0\` at \`P+3A2E\`** (record+2), subject to a record status byte check. This helper writes a record; this alone does NOT prove external event dispatch or queue processing. That **event code 21** is distinct from the **callback dispatch parameter 22** found below. Do not merge the fields/channels because their codes are numerically adjacent.

## 2. New exact native switch decoding: key 22 -> callback setup

\`\`\`asm
AALib+158578  mov ebx,[esi+0C]       ; prepared callback frame+0C = P+3ED4
AALib+1585C5  cmp ebx,30h
AALib+1585CE  jmp dword [ebx*4+00558DC4]   ; original-file jump table
             ; entry index 0x22 (34 decimal): 00558CE0
AALib+158CE0  push esi
AALib+158CE1  call AALib+156A60
...
AALib+156B82  push -1
AALib+156B84  push AALib+1566B0      ; choose native deferred callback
AALib+156BC7  add ecx,3ED0h
AALib+156BCE  call AALib+0F7E70      ; D.callback = 1566B0, D.param = -1
\`\`\`

The original **0x22 jump-table entry** is \`00558CE0\`, verified by reading the exact \`.text\` table DWORD \`[00558DC4+0x22*4]\`. Original CALL \`158CE1→156A60\`, original callback literal \`156B84\`, and setter CALL \`156BCE→0F7E70\` match bytes and relative targets.

\`156A60\` also has other branches: e.g. \`P+3F54==4\` routes to the **\`1560B0\`** descriptor, some state==6 or active-action conditions may do the same, while special action/code/flag predicates can route to **\`1566B0\`**. Thus the switch-key-22 path **has a concrete possible route** to \`1566B0\`; which internal conditional branch actually fired during this frame was not observed by a branch-IP instrument. The callback transition in the log matches the route.

## 3. New existing-runtime evidence in Stage38 (same Stage70 session)

Raw source: \`ArcanaAI_Stage38CallbackWindowTrace.log\` user upload at [98ff59b](https://github.com/Hawky11/1/commit/98ff59b871e90fb757c787b472d5d5322941ce07); Stage38 adapter field map from \`Stage67_Overlay/src/Stage38CallbackWindow.inl\`: **\`f3=P+3ED0\`**, **\`f4=P+3ED4\`**, **\`f20=O+68\`**. Source \`Stage38Prepared\` logs a PRE snapshot after native \`156C00\`, before \`EE7BE CALL EDI\`; POST is before the native \`FC60\` cleanup.

| Stage38 | Same P1/O/TID QPC interval | Original callback observed | f3 callback before -> after | f4 argument | f20 deferred |
|---|---|---|---|---|---|
| **seq4090** | QPC \`1518394805874\`→[see raw log] | \`158570\` | **\`158570→1566B0\`** | **\`22→FFFFFFFF\`** | \`FFFFFFFF→FFFFFFFF\` |
| seq4092 through seq4114, alternating P1 frames | QPC \`1518394908779\` through \`1518396004030\` | \`1566B0\` | \`1566B0→1566B0\` | \`FFFFFFFF→FFFFFFFF\` | \`FFFFFFFF→FFFFFFFF\` |
| **seq4116** | QPC **\`1518396105803→1518396105825\`** | **\`1566B0\`** | **\`1566B0→1560B0\`** | \`FFFFFFFF→FFFFFFFF\` | **\`FFFFFFFF→25\`** |
| **seq4118** | QPC \`1518396204014→1518396204022\` | **\`1560B0\`** | \`1560B0→1560B0\` | \`FFFFFFFF→FFFFFFFF\` | \`25→25\` |

Actor in all these rows: P1 \`P=0E4A1F78\`, \`O=2A3A3020\`, TID \`20812\`. Observed runtime image base \`00800000\`: original \`AALib+158570\` corresponds to callback \`00958570\`; \`1566B0\` to \`009566B0\`, \`1560B0\` to \`009560B0\`.

The **same actual native frame** Stage38 seq4116 fully contains:
- Stage65 seq319 \`156769\` immediate request \`2F\`, QPC \`1518396105810→5813\`;
- Stage70 seq1 actual original-store replay from \`156787\`, QPC \`1518396105821→5822\`: **\`O+68:FFFFFFFF→25\`**, selector \`1→1\`, full \`07/07\` masks;
- Stage38 POST QPC \`1518396105825\`: \`f3\` now \`1560B0\` and \`f20=25\`.

**Source-path connection is now stronger than Stage70 alone**: Stage38 independently captured the *entire original callback invocation* and the **two distinct native-side effects** (deferred ID + callback transition) that are both explicitly present in original \`1566B0\` instructions \`156787\` and \`156794/15679A\`. Stage38's next invocation at seq4118 observes the **new callback actually executing**, not merely being stored.

## 4. What is **not** demonstrated about selector P+3F50

The old Stage38 field snapshot captures \`P+3ED0/P+3ED4\` but **does not include \`P+3F50\` or \`P+3F54\`**. Therefore:
- The Stage38 seq4090 callback registration **does not** prove who set selector=1 or which branch of \`156A60\` ran.
- \`P+3F50=1\` is observed by Stage70 only when its original deferred producer consumed it at QPC \`1518396105821\`.
- Stage71's literal-one store \`155D29\` remains one static candidate among it and the ten register-derived stores that can produce 1 or 2, plus other possible aliased writes. Its execution IP was **not captured**.
- Stage38 seq4090's callback-parameter code **0x22** is **not the same field** as the Stage71 setter's event-code **0x21**; no causal assertion joins these two record families.

**Stage72 conclusion:** we have resolved the origin/dispatch of the **\`1566B0\` callback** for this capture (static switch 22 routes to \`156A60\`, runtime shows 158570→1566B0) and confirmed its **1566B0→1560B0** continuation with direct nested Stage65/70 effects. We have **not resolved the earlier writer IP of selector=1**. Both statements should stay distinct.

## 5. Stage73 next step — no automatic patch

Since the source of deferred \`25\` itself has already been runtime-proven, the remaining question is the *earliest selector assignment* and what conditions cause it. Review Stage71's twenty selector writers and the paths that could set selector before Stage38 seq4090. The original Stage38 trace lacks the selector field. If exact attribution matters, plan a safe multi-writer watch (or a small number of opcode-guarded native call boundaries), but do not claim it is possible with a single \`155D29\` hook. Stage59 already owns DR0/DR1; avoid unexamined DR2/DR3/VEH contention. Never modify/force selector, request, deferred ID, or AI behavior.

**No Stage72 DLL, game test, or new user logs needed.** Preserve tested Stage70 r2 1,892,864-byte DLL and Stage67/65/59 rollback builds. Full reproducibility is provided in [verify_stage72.py](tools/verify_stage72.py), operating on the private SHA-pinned original EXE and optionally the four exact frozen log files.
