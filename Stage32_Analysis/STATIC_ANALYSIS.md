# Stage32 — native type0/payload0 reactivation and downstream dispatch

Date: 2026-11-09. **Status:** original EXE analysed and SHA-pinned; Stage32 diagnostic source prepared, **NOT yet built with Windows MSVC or tested in game**.

## Source of truth and checks

- Prior Stage31 runtime: [evidence](https://github.com/Hawky11/1/blob/stage31-alternate-path-runtime-evidence/Stage31_Runtime/RUNTIME_ANALYSIS.md), branch `stage31-alternate-path-runtime-evidence` @ `0a9ae75cd952f9ec6e2d8588a3c44ce0974cb466`. Source-only Stage31 @ `e905316`.
- Original archive is **already available** in ChatGPT Library `/AALib.zip`. Member `AALib.exe`, size 175,556,608 bytes, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Original PE32 x86 ImageBase `0x400000`; all addresses below are **RVA**, not absolute VA.
- A new read-only Python verifier `Stage32_Analysis/tools/verify_stage32.py` checks the pinned SHA, PE header, 21 exact instruction signatures and 5 direct E8 relative call targets. Executed successfully against the original binary extracted from Library. No AALib.exe or full disassembly published.

## Critical correction to interpretation of Stage31

Stage31 observed six `kind=3` nested calls to `F2DE0`, type=0 payload=0, with `before=(guard=0,active=0,type=FFFF,payload=FFFF)` and `after=(0,0,0,0)`. This confirms **the tuple is deposited**, not that the event has been discarded.

### Original F2DE0 insertion rules

Disassembly RVA `F2DE0..F2E1C`:

- `F2DE6`: if `queue+0xF != 0`, skip the write entirely.
- `F2DEF..F2DF9`: if type is `1` or `2`, set `queue[0]=1` (active).
- `F2DFF..F2E08`: if type is `0` **and** payload is `0x1E`, also set active=1.
- `F2E0B..F2E17`: copy payload to `queue+4` (WORD), type to `queue+2` (WORD), clear `queue+6` and `queue+1`. **For type0/payload0 the active byte is deliberately not set here**.
- Do not conflate the caller's register `EAX/AL` with a queue-acceptance return: `F2DE0` is a void-like routine, `ret` at F2E1C.

### Second-stage native comparison and activation

The original game already contains this code path:

```text
EE5E0 -> EE604 CALL EE480
              EE4AB CALL F2E20  (valid current tuple != history)
              [if changed] EC360 ...
              EE57D CALL F2E50  (current -> history; current -> FFFF/FFFF)
              EE589: queue.active = 1
              EE5B6: if queue.active != 0
              EE5C6 CALL EE200  (dispatch)
         EE609 checks queue.active again in outer function
```

- `F2E20` at `F2E2A,F2E34` rejects invalid current type/payload (`FFFF`), then compares `queue+2/+4` with `queue+8/+A` (history) at `F2E3A,F2E40`. Returns true if both are valid and at least one field differs.
- `EE480` calls this comparison at `EE4AB`. On the changed branch it performs other game logic (including `EC360`), transfers the current tuple into history at `EE57D→F2E50`, **sets active at `EE589`**, and checks the active byte before calling dispatcher `EE200` at `EE5C6` (return `EE5CB`). This is a code-path result, not observation of each Stage31 event being consumed.
- At the Stage31 BEGIN/END snapshots, the saved history `P+3A34` had type1 with payloads listed below. The new current tuple type0/payload0 **differs** from that history in all six cases. Therefore `F2E20` would pass **if nothing overwrites the fields before the next processing call**.

| Stage31 episode | Side | old history WORD(type), WORD(payload) | new current pair |
|---:|---|---|---|
| 1 | P2 | 1, 0x5E | 0, 0 |
| 2 | P1 | 1, 0x25 | 0, 0 |
| 3 | P2 | 1, 0x5C | 0, 0 |
| 4 | P1 | 1, 0x4A | 0, 0 |
| 5 | P2 | 1, 0x58 | 0, 0 |
| 6 | P1 | 1, 0x0C | 0, 0 |

### What the existing dispatcher log already shows

The Stage31 upload's `ArcanaAI_EventDispatchTrace.log` includes aggregated `EVENT_DISPATCH` lines for **both P1 and P2**, `eventTypeArg=0`, `payloadArg=0`, `callerRVA=EE5CB`. Example aggregate entries report `queueActiveBefore=1`, `queueCurrentBefore=(FFFF,FFFF)`, `queueHistoryBefore=(0,0)`. This is exactly the shape predicted after `F2E50` + `EE589`.

**Important**: those are grouped counters (with `tid(first)` and timestamp at log-flush granularity), **not** event-by-event QPC. They **cannot** prove that the six specifically nested Stage31 events each reached EE200. Distinguish static possibility, general observed dispatch and per-episode causality.

## Bit16: writer and separate clearer

- At `E6C66` the original `E6C60` routine executes `OR DWORD [player+3B84],0x10000`. Stage31 six episodes see `0x20000 -> 0x30000` over the helper's lifetime. That is consistent with but not direct-site proof of this write.
- Another routine at `E9E66` executes `AND DWORD [player+3B84],0xFFFEFFFF`, clearing bit16, after a conditional `F2ED0(queue,0,0)` query at `E9E4E` for an unrelated state update. **Do not assume bit16 remains set indefinitely or give it a gameplay name.**
- Many original callsites test `BYTE [player+3B86] & 1` (bit16). A lifecycle interpretation requires both same-event runtime data and the precise caller/state; reading one flag alone is insufficient.
- Stage29 `ED19F` clears bit17 and bit18 via mask `FFF9FFFF`, leaving bit16 untouched; its 0x30000→0x10000 shadow is consistent with this.

## Stage32 source: smallest available passive test

`Stage32_Overlay/src/AIQueueTrace.cpp` derives from the **tested Stage31 overlay**. The only substantive changes are:

1. Add `EnableStage32Type0DispatchDetail` mode (default disabled in code, **enabled in Stage32 INI**).
2. Extend the **existing** `EventDispatchHook` detail predicate from only type1 to type1 **or type0/payload0**. The original `g_originalEventDispatch` is still called once with identical arguments.
3. Emit build/config markers in **existing** `ArcanaAI_EventDispatchTrace.log`: `STAGE32_TYPE0_DISPATCH_BUILD=2026-11-09-r1`.
4. Reuse `EVENT_DISPATCH_DETAIL` with exact QPC before/after, caller RVA, side, player, queue active, current tuple, history tuple and guard. Existing type1 detail behavior, Stage31 and Stage29 observers remain unchanged. No new hook/ABI wrapper and no new gameplay writes.

Existing ring capacity1024 / 1000ms flush, with explicit `WARNING: event dispatch detail buffer full` on overflow. No per-episode identity is fabricated in Stage32 records: pair with Stage31 by QPC, side, player and lack of intervening queue changes. Distinguish close temporal association from strict proof of identity.

**Validation:** JS source invariant checks confirmed exactly one original dispatch call in hook body and intact Stage31 marker; matching source/INI flags and marker were inserted. **No MSVC compile or in-game Stage32 run yet.** The 21+5 native verification is independent of compiler.

## Outcome / next decision

- We have **disproved the inference** “type0/payload0 stays active=0 immediately after F2DE0, therefore cannot be processed”. Native later processing explicitly activates changed tuples and dispatches.
- We have **not proven** that each particular Stage31 episode reaches EE200, nor that a dispatched action has the intended gameplay effect.
- Next natural COM-vs-COM run should provide Stage32 per-event dispatch QPC alongside Stage31/Stage29/ProducerCallTrace/QueueTrace, after a successful Windows x86 build. If the six episodes correlate, investigate downstream type0 action semantics, not forced type1 duplication. If some do not, first determine whether their tuple was overwritten between Stage31 END and EE480. Preserve passive-only research.
