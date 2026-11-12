# Stage111 — Complete same-session Stage94 → Stage104/93 → Stage107 and Stage110 audit

**2026-11-12. Windows x86 build and live instrumentation VERIFIED to the degree shown by logs; zero observed CEBE0 entry. No new source patch, hook, binary, user game test or gameplay-data/DR write performed for Stage111.**

## Source provenance
[User's fresh 11-file upload commit](https://github.com/Hawky11/1/commit/3ed706cb9be5e9d13f9202c0606080f304ba5070). All counts and joins below derive exclusively from the new upload, not from Stage108's previous session.

| File | Git blob SHA |
|---|---|
| Build.log | 60d09673d40e3b0ba33a3bdbb5e5f2e60cd9dbb3 |
| ArcanaAI_Stage104Mode0NativeTrace.log | a1157bb674a9d1e6889613545fb14288d805c150 |
| ArcanaAI_Stage107ConsumerReadTrace.log | 05f4271541b090b630bf75674b8e2e6805627901 |
| ArcanaAI_Stage110Cebe0GuardTrace.log | 36892266b0fee4072e5d094862221cb9dd03b28a |
| ArcanaAI_Stage93FinalCommandQpcTrace.log | f65f11c1b3e263317933781a29b8bb9451dd43bc |
| ArcanaAI_Stage94ProducerGateTrace.log | 0e600b5815a80f607766ff10fc83f08d69d7c55a |
| ArcanaAI_Stage93DirectBit3Trace.log | 51c4791dabfa1399d21e82fcd7a4ffd07430f5d1 (empty) |
| ArcanaAI_Stage95AlternateEsiTrace.log | 23853b827b20b0713468e8b874f566856367ea99 (empty) |
| ArcanaAI_Stage96EsiWriterTrace.log | 2e9978838edcf0cd61f765b3c930bb2dec7747be (empty) |
| ArcanaAI_Stage97InheritedEsiTrace.log | 975b0abe4cdc9b2e24a07f8319612d699c7444df (empty) |
| ArcanaAI_Stage98CommandRouteTrace.log | 799a889200912d1a774b3c692bcd39947fbaf7ce (empty) |

The five empty diagnostic files are not negative evidence that those functions never ran: the respective observers may be disabled, not installed, or have separate capture conditions. Fresh general ArcanaAI.log and ArcanaAI.ini are **not** in this upload; earlier copies may represent a different session.

## Windows build and hook installation
- Build.log started 12.11.2026 19:18:37, x86 MSVC toolchain, **BUILD SUCCEEDED**; resulting **dinput8.dll 2,073,088 bytes**; the preexisting C4754 warning is nonfatal.
- Original Stage104 r2 observer **INSTALLED 2/2** at F7CC0 and F7DA3; prior F2DE0 E9+NOP detour preserved (targetVA 78E42300, owner unidentified).
- Stage107 r1 **INSTALLED 2/2** at F7DA8 and CEC64; Stage110 r1 **INSTALLED 2/2** at CEBE0 entry and CEC57 activity gate.
- Stage104,107,110 and Stage94 each have **47 HEALTH intervals**. Their available dropped/mismatch/readFail counters are universally **zero**. These are logger health observations, not game-wide stability guarantees.
- No Stage111 new compiled binary or runtime test by analyst. Stage36 was required OFF in prior config; this new commit has no fresh INI to independently recheck that exact setting.

## Native producer / gate / downstream joins

1. **Stage94 producer:** 2,044 real STAGE94_PRODUCER event lines; 47 HEALTH sum producer=2,044. Its 214 STAGE94_GATE events split into **171 forwardType1=1** and **43 forwardType1=0** (forward/reject health counters = 171/43). All 214 gates carry valid QPC/thread/player information.
2. **All 171 forwardType1 gates ↔ Stage93 FinalCommand:** strict same actor/player/TID, chosen ESI == final payload ESI, QPC increasing within 1,000 ticks: **171/171 unique joins**, no missing, no ambiguity; gate→final QPC delta **2–49 raw ticks**. All 171 FinalCommand records have type=1.
3. **All 43 forwardType1=0 gates ↔ Stage104 MODE0:** strict same side/player/TID/frame, chosen ESI == native MODE0 ESI, QPC increasing within 1,000 ticks: **43/43 unique joins**, no missing/ambiguity, gate→MODE0 QPC delta **10–29 raw ticks**. MODE0 chosen ESI histogram: **0x10: 7**, **0x0E: 11**, **0x27: 20**, **0x26: 2**, **0x25: 3**.
4. **All 20 0x27 gate events ↔ Stage104 MODE0 0x27:** **20/20 unique joins** under the same strict gate→MODE0 predicates. Stage104 records show **20 actual CEA20 PRE** for these 20 0x27 decisions; 23 other MODE0 events did not take this special CEA20 path.
5. **Stage104 CEA20 PRE ↔ Stage107 F7DA8 POST:** **20/20 unique joins**, matching side/player/TID 18684/frame 0E58F9C4/state (Stage104 ECX vs Stage107 state)/ESI 0x27 and ordered QPC within 1000 ticks. P1 **7** / P2 **13**. Lag **4–15 raw QPC ticks**, median **7**; no unmatched or reused POST.
6. POST all **20 valid=1**, index=0, v10==v18 and v20=0x0000000A. PRE CEA20 arg0 offset from player **0x930 in 17 cases**; **0x954 in 3 cases** (same-run actor details available in raw logs). Neither POST shared-state equality nor 0x0A demonstrates subsequent consumption/accepted attack/hit.
7. Producer attribution for the 20 code 0x27 gates: **16/20** sourceValueMatches=1 and sourceFrameMatches=1 with candidate EAX=0x27; other **4/20** sourceValueMatches=0 with sourceCandidateEax=0. The four cannot be confidently ascribed to F3930 from these logs; **do not** equate missing immediate producer join with proven alternate source without Stage96/98 data.

## Real Stage110 negative finding
- **47 Stage110_HEALTH intervals:** entry=0, guard=0, guardActive=0, guardSkip=0, guardUnknown=0, idMinus1=0, outRange=0, tableFF=0, inactivePred=0, candidatePred=0, unreadable=0, unknownCaller=0, dropped=0, mismatch=0, readFail=0. **No STAGE110_ENTRY or STAGE110_GUARD events.**
- **47 Stage107_HEALTH intervals:** POST=20; nativeRead=0, matched=0, filtered=0, dropped=0, mismatch=0, readFail=0. No READ_CEC64 event. Read counter is pre-filter in original Stage107 source, so this is not due to the 10-second POST matching filter.
- Stage104 HEALTH totals MODE0=43, CEA20=20; Stage94 HEALTH producer=2044, gates=214, forward=171, reject=43, all available error counters zero.
- Scope: Instrumentation **reports no native CEBE0 entry at the patched RVA and no CEC57/CEC64 execution during this real active game interval**. With no positive Stage110 entry hook hit, absolute absence of all CEBE0 calls or proof of perfect Stage110 interception cannot be established. No captured ID means Stage109 ID/table/active early-gate failure is NOT observed or individually diagnosed. As Stage109 static control flow established, F7DA3 CEA20 immediate continuation returns or calls **CBDE0**, not CEBE0 directly.

## Recommended next Stage112
1. Review original **CBDE0** and its actual callers/callees, especially F7DA8 → CBDE0 versus return, plus whether and when the CEBE0 four direct callers (988EA/98948/CAD79/CADDA) are ever reached. Do static CFG and ABI/ownership/byte verification **before** new patch.
2. If genuinely necessary, add **one or two opt-in read-only** counters at a confirmed execution decision/callsite (avoid redundant F2DE0 detour, and avoid logging all iteration-loop events unbounded). Provide SHA-guarded source apply/rollback and a positive hook-firing control to rule out entry instrumentation blind spots.
3. Preserve user's **Windows-tested Stage110 DLL 2,073,088 B**; Stage107 fallback 2,065,920 B; Stage104 r2 fallback 2,058,240 B. No forced AI commands, game/DR writes, hit/animation/AI-quality claims. Stage36 OFF unless a future uploaded INI proves otherwise.
