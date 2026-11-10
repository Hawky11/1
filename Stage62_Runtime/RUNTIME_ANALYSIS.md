# Stage62 — same-session native action request → activation → definition (2026-11-10)

**Status: RUNTIME LOG JOIN VERIFIED (read-only/offline).** This stage introduces **no new native hook, DLL, game write, or gameplay change**. Results below were independently computed from the exact user-uploaded GitHub log contents. The accompanying Python 3 program is a reproduction aid; its Windows execution has **not** been reported or asserted.

## Frozen evidence and provenance

Main snapshot at analysis start: commit [ae8b67757e0f0d4330aa7e2e60bc319a0529450d](https://github.com/Hawky11/1/commit/ae8b67757e0f0d4330aa7e2e60bc319a0529450d). All eight logs start during the **same 2026-11-10 22:07:41–22:07:42 local launch** and contain P1=0E4A1F78/O=2A89A020 and P2=0E4CCCA0/O=2A89AC50; native game thread **15132**; common monotonic QPC. They differ from older 19:56 captures.

| Observer | Rows | Git blob SHA |
|---|---:|---|
| Stage37 type1 event-side effect | 48 | 5ca3c6de9b065dce0f39ff85548edc16b38eff3e |
| Stage39 native callback action output | 44 | 21aca49f78a7fac5fdfb2a946badbe1d9c49cd9b |
| Stage40 raw request consumption | 263 | 155ad7f9e6bf29ace43bc84416dcdc663eff6d80 |
| Stage41 direct pending object commit | 263 | 05e969aad9b893ad538a8e9260ee85ede785e7cf |
| Stage42 native action activation | 202 | afafba91563bcc1b9865942568c6a77c632e0716 |
| Stage43 activation's native caller | 202 | 9b571abbc515c2175b99f39b1c9fa53ade5653c0 |
| Stage44 definition binding | 1,476 | 6d0b44dbebd2c1c9b1d674b7f9b856be29d50a0c |
| Stage46 definition-motion correlation | 1,476 | c973b4593a6de15a045220b39c9b8d010db6c764 |

Stage59 r2 Windows build began **22:07:10**, succeeded (DLL 1,153,024 bytes), and native observers started 22:07:41–42. Stage61 had separately established strict Stage46→47→59 motion/X provenance for **4,616** windows (see ../Stage61_Runtime/RUNTIME_ANALYSIS.md); **those are repeated native motion calls, NOT 4,616 AI decisions**.

## Exact joins and observed results

- **Stage40 ↔ Stage41: 263/263.** Exact stage40Seq, side, player, game thread, QPC interval and request ID. Stage41 follows the raw request in the same E8480 call, observes O+64 pending action and O+08 bit0. All **263** Stage40 rows have path=SELECTOR_SENTINEL; Stage40 actorSeen=0, hence **do not** claim its actor-helper path returned an actor. Stage41's direct [P+4] object snapshots provide the pending-state evidence.
- **Stage40 → Stage42 → Stage43 → Stage44 → Stage46: 202/202 activation chains.** For each Stage42 native activation select only the latest earlier Stage40 request with exact P/TID/side and require matching ID. Each has the exact Stage43 seq/QPC origin, a unique Stage44 binding enclosed by its Stage42 QPC window, and a Stage46 record with same stage44Seq, binding QPC, definition pointer and active ID. Stage43's callerRVA is **000AD75C for every one of the 202**. All Stage42 and Stage44 sampled health counters report dropped/overflow/mismatch=0.
- **61 Stage40 requests** do not correspond to *separate* Stage42 activations. Every one has request ID **00000049**, a repeated/pending duplicate while that request was already present. The total Stage40 0x49 count is **64**; three correspond to separate activations. This does **not** prove missed AI actions or a queue bug. Stage41 can requeue with bit0 already set for some other events; do not classify every unchanged bit0 as duplicate.
- **Stage37 → Stage39 → Stage40 → Stage41 → Stage42 → Stage43 → Stage44 → Stage46: 44/44 distinct upstream chains.** For each Stage39 observer event choose the latest prior same-player/TID Stage37 type1 effect; require same payload; then the *first subsequent* same-player Stage40 request; require its ID equal Stage39 a0 POST; then the previously verified unique activation/definition chain. These 44 events map to **44 distinct** Stage37, Stage40, Stage42 and Stage44 records. Stage37→39 QPC gap is **16–75 ticks** and Stage39→40 gap **23–870 ticks**. QPC ticks are not milliseconds.
- Of **48 Stage37 type1 effects**, four do not participate in these 44 logged Stage39 output chains: dispatchSeq **40, 73, 106, 153**. No claim of lost callbacks/commands: Stage37 is broader than Stage39's action-output scope.
- **158/202 activations** have **no matched Stage39 action-output record** in this capture, even though they have complete Stage40→46 provenance. Do not infer that Stage39 is the only action request producer or that all other 158 are bypassing native callbacks; its instrumentation watches only specific observed type1 callback-induced request changes.

The Stage39 output rows are naturally sparse: 44 events in 22:07:58–22:08:22, whereas Stage40 can observe requests before the first Stage39 event (22:07:56) and after any other uninstrumented writer. No cross-run, ID-only or fixed time-shift correlation was used.

## Native interpretation and limit

**Proven original handoff** (field names raw, not animation labels):

    type1 dispatch effect (Stage37, subset)
      -> callback changes P+3F4C (Stage39, subset)
      -> E8480 consumes P+3F4C (Stage40)
      -> [P+4] object's O+64 pending, O+08 bit0 (Stage41)
      -> 1E9F62 calls 1E9C70; O+64 -> O+60 active ID (Stage42)
      -> original AD75C caller confirmed (Stage43)
      -> 1E9E2D calls 1E8490; O+34 definition bound (Stage44)
      -> Stage46 matching definition and movement-state initialization
      -> existing Stage61 Stage47→Stage59 native motion/X verification separately

The final Stage61 motion/write relation is established in the same session but **this report does not count 44 complete first-choice-to-X causal chains**; that final nested join is not performed here.

**NOT proven:** where the COM's strategic action decision originated, the score/candidate comparisons or rejection reason, whether the activation caused a rendered animation or hit/damage, or that the mod improved AI. Even 202 O+60 changes are action-state activations, not semantic AI choices. These passive hooks preserve original native behavior.

## Next focused task (Stage63 proposal)

Trace **native writer provenance of P+3F4C upstream of Stage40**, starting with callback-to-request producers already verified at Stage39, and separately handle the **158 Stage42 activations lacking Stage39 matches**. Static original-PE xrefs and exact rel32/caller checks come first; distinguish COM strategy candidate selection from general engine-command plumbing. Check existing Stage35/36/37 event paths and guard against attributing every action to the special payload 0x1E. Only consider a passive runtime observer if the static findings identify a minimal, ABI-safe site; Stage59 r2 DLL remains the rollback baseline. **Do not rebuild now.**
