# Stage99 — class-5 native bypass route is statically deterministic

**2026-11-12 | Original binary SHA256** `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` (user ChatGPT Library /AALib.zip, not committed). **Status:** STATIC VERIFIED; no newly built DLL, hook or runtime session.

## Proven original control flow

Stage98 `branchClass=5` means earlier branches did **not** reach producer `F7A99`, status-range bypass, bit15-clear bypass or bit17-set bypass. Entire native tail `RVA F7A49..F7A97` leads to `F7A94` bypass, never to the producer:
- `F7A49 CMP WORD [EDI+3A34],2; F7A51 JNE F7A6A`. If mode !=2, helper is skipped and `F34D0` is called.
- If mode=2: `F7A53 MOVZX EAX,WORD [EDI+3A36]; F7A5A MOVSX EDX,AX; F7A5E CALL 525CD0` (VA=00525CD0, RVA=00125CD0). `F7A66 TEST AL,AL; F7A68 JNE F7A94` skips F34D0 when AL!=0.
- `525CD0` is an exact small pure filter: returns AL=0 for signed parameter in `{0,1,2,9,10}`, AL=1 otherwise. No game-state writes or calls inside this helper.
- `F7A6A` computes call argument `0x22` if `[EDI+3B70]&4` else `0`; `F7A8A CALL F34D0` (VA=004F34D0, RVA=000F34D0). `F7A8F ADD ESP,14h; MOV ESI,EAX` and `F7A94 MOV [EBP-1C],ESI; F7A97 JMP F7ADB`. Stage96 owns the F7A8F observer and replays ADD/MOV once; Stage95 owns the F7A94 bypass observer. `F7A99` producer cannot be reached from this tail.

Thus **class5 predicts BYPASS regardless of mode and helper return**. The remaining branch question concerns whether F34D0 is called and what it returns, not producer-versus-bypass. This is original static control-flow proof, not a new live helper measurement.

## Existing Stage98 r2 live facts and strict boundary

[Stage98 r2 same-run report](../Stage98_R2_Runtime/RUNTIME_ANALYSIS.md): 14,107/14,107 strict Stage98→Stage94/95 joins, classes1+3=13,646/13,646 correctly predicted, class5=461/461 observed bypass, 49 clean HEALTH snapshots per stage. For those 461 class5 events, Stage98 logged `mode3A34=0000` **208** times and `0001` **253** times (no mode=2). Therefore in all 461 **the static native code forces F34D0 and skips helper 525CD0** (assuming unmodified original flow and same event snapshot). There is no same-run mode=2/AL=1 runtime proof. Whether F34D0 returned finite ESI or produced meaningful attacks is outside Stage99.

## Verification and handoff

Verified private original EXE SHA256, PE32 i386/.text mapping, exact **80-byte** F7A49..F7A98 block, **37-byte** helper, 5 native branch targets and 2 call targets, F7A94 continuation: **10 native region/edge checks PASS**. Synthetic subclass model tested **100/100 PASS** (81 combinations + 19 regressions). Offline runnable Python 3 verifier and synthetic tests distributed in `ArcanaAI_Stage99_Class5_StaticRoute_Audit.zip` (ChatGPT session attachment), without original EXE, DLL or injected gameplay changes. Do not upload original EXE to GitHub.

**Next work, only if justified:** analyze F34D0's **return-value semantics** and naturally observed mode=2 with existing Stage96/95 witnesses; no broad new hooks. Preserve verified Stage98 r2 DLL **2,051,584 bytes**, Stage97 rollback **2,045,440 bytes**, Stage36 OFF. Native route prediction is not proof of better AI/hits.

