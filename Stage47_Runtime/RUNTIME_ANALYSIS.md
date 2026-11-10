# Stage47 — native motion-consumer runtime VERIFIED

Date: 2026-11-10. Fresh upload [main e517667](https://github.com/Hawky11/1/commit/e5176670e76396174cf58986fe3bf62c50c47178).
This evidence branch was created at the exact user upload commit **before** adding analysis, retaining every original Build.log and runtime log blob unchanged.
Source: [stage47-native-motion-consumer-trace](https://github.com/Hawky11/1/tree/stage47-native-motion-consumer-trace), Stage47 overlay unchanged.
Full replayable [5,614-row join](stage47_consumer_binding_join.csv) and [336-row binding summary](stage47_binding_summary.csv).

## Build, installation, loss counters

Windows x86 Build.log dated 2026-11-10 13:32:18 local: **BUILD SUCCEEDED**, dinput8.dll **1,063,424 bytes**, only existing AICommandSiteTrace.cpp warning C4754.
Stage47 log begins with `STAGE47_MOTION_CONSUMER_BUILD=2026-11-10-r1` and
`STAGE47_INSTALLED 2/2 callsites=AE70C,AE93E native=1E66E0`.
Stage44 and Stage45 installed 1/1; Stage46 enabled. Stage42 and Stage40 remain installed.

The entire original Stage47 Git blob (SHA 5fd7d499a27383e5f375717449f1efe2a0df07c6; 3,202,722 bytes) was fetched and parsed, not merely its header. **5,614** complete consecutive records seq1..5614; P1=2,807, P2=2,807. All 5,614 have callerRVA **000AE70C**, zero native argument, preMask=postMask=1FFF, extraMask=F, expected changedMask and monotonic QPC. Source callsite AE93E installed but no P1/P2 observation at AE93E during this run, so branch coverage of that path remains untested.

Stage47 28 HEALTH records: calls **49,315** = 43,701 other objects + **5,614** P1/P2. All reported dropped=overflow=mismatch=badCaller=0; no truncated rows and file ends with newline.
Stage42 HEALTH calls=1,876 otherObjects=1,578, matched P1/P2=298; Stage44 calls=5,631 other=5,292, matched=339; Stage45 calls=47,443 other=47,104, matched=339. All three stages dropped/overflow/mismatch=0.
Full Stage36/37 and Stage39–47 text inspected (Stage47 through blob API; each smaller file through contents API), no fatal/errors or truncation signatures. Stage35/38 huge raw logs retained but not full-text verified in this analysis.

## Correlation to Stage46

Stage46 has 339 complete binding/definition records (P1 169, P2 170), and Stage44 and Stage45 have exactly 339. All 339 Stage46 records join Stage44 339/339 and Stage45 resolver EAX/ID/variant 339/339. The Stage45 resolver uses the normal table path and no fallback. Exactly 298 Stage42 activations each contain one matching Stage44 binding; 41 additional bindings occur outside captured Stage42 windows and do not imply losses.
Stage40 ↔ Stage41 **363/363** requests. All **296** Stage41 0→1 bit0 transitions match distinct Stage42 activations, 67 Stage41 windows do not newly set bit0. The two additional Stage42 activations (seq3 P2 ID 0506→0; seq4 P1 ID 0500→0) have known AD75C caller and no new Stage41 bit0 transition; this does not establish a missing command. Stage42 ↔ Stage43 all 298/298 known caller AD75C.

For every Stage47 event we found the nearest preceding Stage46 bind for the *same side and object* (using QPC). Every one of the 5,614 observations has the same active action ID as that binding.

**Key new runtime invariant (5,614 / 5,614):**

```text
Stage47 O+34 definition pointer =
    preceding Stage46 definitionAfter
    + 0x5C * (Stage47 O+6C variant - Stage46 POST O+6C variant)
```

The nearest preceding Stage46 variant is zero in this capture, and Stage47 variant ranges from 0 to 37. Definition pointer delta is always nonnegative, with **1,581** observations at base definition and **4,033** at positive multiples of 0x5C. Zero violations of the invariant, and no Stage47 consumer invocation changes O+34 within its own PRE/POST window. This is strong evidence that the consumer sees successive indexed 0x5C-byte definition rows while the active ID stays the same. **Do not** infer that Stage47 native 1E66E0 writes the definition pointer: it does not change within the measured call.

Of the 339 Stage46 bindings, 336 have at least one subsequent Stage47 window as the nearest preceding binding. 320 first subsequent consumers match *all eight* Stage46 POST float snapshots exactly at their Stage47 PRE and have the base definition unchanged. The remaining earliest consumers may already have different indexed definition rows; they are not necessarily data corruption. Multiple Stage47 update windows per binding are normal.

## Real native changes during consumer update

Of 5,614 Stage47 windows, **3,703** change at least one of eight tracked raw IEEE754 float fields; 1,911 show no float-field change. Per-field change counts (overlapping):

| Field | Changed consumer windows |
|---|---:|
| O+7C (f5) | 2,248 |
| O+80 (f6) | 2,415 |
| O+88 (f7) | 795 |
| O+8C (f8) | 3,173 |
| O+90 (f9) | 0 |
| O+94 (f10) | 11 |
| O+150 (f11) | 88 |
| O+154 (f12) | 20 |

Mode byte (definition+30 low byte) distribution of consumer windows: 0=4935, 1=185, 2=26, 3=326, 4=87, 5=55. Float changes occur in all six observed modes; mode zero can still have downstream consumer changes because Stage47 is not the Stage46 initial nine-way dispatch.
Concrete P1 seq18 O+7C/O+88/O+8C all changed in the native function; P2 updates of O+80/O+8C likewise observed.
Raw data support **executed native numerical updates**, not direct claims about on-screen positions, rendered animation, collision/hit, damage or AI quality.

## Original executable: new concrete variant-binding callsites

Reused exact Library `AALib.exe`, original length 175,556,608, SHA256
`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
Disassembled .text with GNU objdump (PE32 image base 400000). Besides existing
Stage44 CALL **1E9E2D → 1E8490**, two *additional* direct native CALLs target
the same action-definition binder:

- **1E8DA8 → 1E8490**, bytes `E8 E3 F6 FF FF`, within native routine **1E8D60**. At 1E8DA1 the code compares new O+6C versus saved value, and only when changed sets ECX=ESI and calls 1E8490. RET 4 at 1E8DB0.
- **1E908F → 1E8490**, bytes `E8 FC F3 FF FF`, within native routine **1E8DC0**. This routine updates action timing O+78 and conditionally adjusts O+6C; at 1E9088 compares old/new O+6C, then calls native binder only on change.
- Existing Stage44 CALL **1E9E2D → 1E8490** remains separate and was the only earlier Stage44 hook.

The native 1E8490 binder resolves O+60 active ID and O+6C variant, then writes resolved definition to O+34 (1E84B8). This provides a **static mechanism** for the new observed Stage47 pointer/variant relationship. We have **not yet instrumented runtime caller attribution** at 1E8DA8 or 1E908F; do not claim which site executed each observed variant change. These are ideal passive Stage48 candidates.

## Stage48 next step, not yet verified in-game

Safest next scope: hook original verified CALLs 1E8DA8 and 1E908F, using the existing Stage44 verified S44BindingHook (same native 1E8490 thiscall no stack args) and record callsite return RVA alongside pre/post O+6C, O+34, ID, Stage45 resolver EAX, QPC. Preserve Stage47 consumer trace and the working Stage44/45/46 logic, all native-state/ABI preservation, install both sites transactionally after Stage47. Confirm predicted variant/definition table +0x5C linkage at instruction path and zero drops. First prepare Stage48 overlay and static verifier; no need to repeat the current Stage47 log or ask user for EXE.
