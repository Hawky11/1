# Stage106 — CEA20 → CEBE0 native downstream static consumer audit

**2026-11-12. Static source-level state-writer/reader connection VERIFIED; same-run post-return consumption and gameplay effect remain UNPROVEN.** Evidence only, no new hook/DLL/gameplay writes/DR/forced commands; Stage36 OFF.

## Provenance

- Original user's AALib.exe from ChatGPT Library AALib.zip: SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, PE32 x86, image base 0x00400000. Native **37/37 exact original byte and rel32 edge checks PASS**; 8/8 read-only offline tests pass.
- Reused the exact Stage105 run from [GitHub upload 24885c9](https://github.com/Hawky11/1/commit/24885c90922251cd065ae812c7f1d30b000c7b66), Stage104 trace blob `7fee1be00c73a367bf72cb4d60dc3a62988bf6d8`, [Stage105 full report](https://github.com/Hawky11/1/blob/stage105-mode0-27-runtime-confirmed/Stage105_Runtime/RUNTIME_ANALYSIS.md). No new user run or general ArcanaAI.log available in Stage106.

## Original native path (all addresses RVA)

1. Original `F7150` derives shared state pointer from `[player+0x3F30]`: `state = VA 0x0433C138 + index*0x2806C`. Exact `F7168/F716E/F7179/F718A` instructions verify source, stride, base and local `[EBP-0x30]`.
2. Original `F7CC0 → F2DE0` writes a mode0/code0x27 record **when** F2DE0's `[record+0x0F]` guard permits; `F2E0B` writes code, `F2E0F` writes mode. Thus entry is not itself proof of a successfully stored value.
3. Status-zero/ESI0x27 test at `F7D96/F7D9A`; `ECX=[EBP-0x30]`, `PUSH EAX`, original `F7DA3 CALL CEA20`. At `F7DA8`, no return-value test for CEA20; later optional CBDE0 resets depend on `[EBX+0x55]`.
4. Original `CEA20` is a **shared indexed-state writer, not an attack dispatcher**. Null argument initializes fields with global time / 0x7fffffff. Non-null argument clears `[arg+0x10]`, checks `[arg+0x18]`, uses index `[arg+0x14]`, and copies into `state+0x28010/0x28018/0x28020` (each indexed by index*4). Both conditional non-null branches contain writes.
5. Original `CEBE0` reads the same state arrays at `CEC64` (0x28020), `CEC6F` (0x28010), `CEC86` (0x28018); predicates can invalidate `[input+0x10]`. Exact direct callers `988EA/98948/CAD79/CADDA` originate from `988A0/988A6` and `CACFE/CAD04` computing the **same global base and 0x2806C stride**. This proves a static shared data structure/reader family, not execution of CEBE0 on any particular 17-event record.

## New reanalysis of the existing Stage104 runtime PRE-call log

Exactly **17/17 CEA20 special27 valid=1**, **17/17 arg0 nonzero**. P1 ECX=0403C138 six events, P2 ECX=040641A4 eleven events, delta exactly 0x2806C. Record `arg0 - player` distribution: **16 at +0x930**, **1 at +0x954**. The single +0x954 belongs to **P2 CEA20 seq44** of the real Stage105 class5 F34D0 changed-ESI chain `route12768 → writer14812 → bypass11195 → gate1755 → MODE0 seq43 → CEA20 seq44`. Its +0x24 relative distance is a neighboring record, not an error or an acceptance signal.

**Unproven:** `[arg+0x18]` runtime branch; return-time CEA20 store values; later CEBE0 execution on the same player/index/QPC record; consumption vs rejection, dispatched move, animation/hit, AI quality or game-wide compatibility. Stage104 hook records **pre-call** state only.

## Reproducibility and next step

The ChatGPT-delivered archive `ArcanaAI_Stage106_CEA20_Downstream_Static_Audit.zip` (SHA256 `c1a889b2b1ba3b148ac9d73810c656b08966fb727c5830d88633204190d2ef62`) includes `tools/audit_stage106.py`, `tools/test_stage106.py`, complete static audit, README_RU and results JSON; original private EXE and raw logs deliberately excluded. Run `python tools/audit_stage106.py --exe AALib.zip --stage104-log ArcanaAI_Stage104Mode0NativeTrace.log` offline.

**Stage107 candidate**: only after exhaustive ABI/ownership/signature review, consider passive **post-CEA20 and CEBE0** state observations and same-run player/index/QPC joins; do not hook the already-detoured F2DE0, force status/commands, write game/DR memory, or claim gameplay improvements. Keep Windows-tested Stage104 r2 DLL 2,058,240 B; Stage98 r2 fallback 2,051,584 B; Stage97 fallback 2,045,440 B; Stage36 OFF.
