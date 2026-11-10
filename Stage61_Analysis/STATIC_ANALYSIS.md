# Stage61 — action definition → motion fields → original X store

**Date:** 2026-11-10. **Scope:** original-PE static xrefs + a read-only same-run correlator. **No new Windows DLL, hook or game-state write.**

## Provenance and relationship to existing stages

- Source is the user's private original `AALib.exe`, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. It was verified locally, PE32 i386 at preferred image base `0x00400000`. No binary content is committed.
- [Stage46 static analysis](https://github.com/Hawky11/1/blob/stage46-definition-motion-state-trace/Stage46_Analysis/STATIC_ANALYSIS.md) **already established** definition byte `+0x30` dispatch and definition-driven `object+0x88/+0x90/+0x150` writes; Stage47 previously established six direct `1E66E0` callers. Stage61 **does not claim to rediscover those as new**.
- [Stage59 r2 verified runtime](https://github.com/Hawky11/1/blob/main/Stage59_Runtime/R2_RUNTIME_ANALYSIS.md) established post-write `EIP_after=1E70DE` (original `1E70DB FSTP [ESI+7C]`), and 1,930 early-motion windows with net X changes in that session.
- **New Stage61 work:** independent validation of additional upstream setter callers and same-run Stage46/47/59 correlation tooling that refuses to join different sessions.

## Static structural path

```text
 Stage42 native active ID: O+60           (not necessarily a COM-only decision)
             |
 Stage44–46 native definition binding: O+34, definition+30 mode
             |
   1E8490 definition-mode dispatch:
       1E8B97 / 1E8C34 write O+90
       earlier metadata branches write O+88/O+8C/O+94/O+150/O+154
             |
   other independent motion setter paths:
       152767,1527F8,153675 -> 1E4A00 -> write O+150/+154
       15541F,155479,1554CB,155515 -> 1E4AB0 -> adjust O+150/+154
             |
   AE70C / AE79F / AE7BD / AE849 / AE93E / AEA0E
                       -> 1E66E0 (object integrator)
                internally update O+88, O+150, O+D4
                  then 1E70DB writes O+7C (raw X)
             |
   Stage59 r2 post-write hardware watchpoint: RVA_after 1E70DE
```

The arrows denote **native structural producer/consumer dependencies**, not an uninterrupted observed COM decision-to-movement event. They can be entered through independent game-state paths.

### New PE checks

All **15** direct relative CALL edges and **14** original operand signatures in [verify_stage61_static.py](tools/verify_stage61_static.py) passed against the pinned private original. Notable validated RVAs:

| Writer/reader | Native effect / source |
|---|---|
| `1E4A99`, `1E4AA2` | `FSTP [ECX+150]`, `FSTP [ECX+154]` from the `1E4A00` setter |
| `1E6B70`, `1E6B81`, `1E6C10` | Conditional `FST [ESI+150]` in `1E66E0` |
| `1E6BB2`, `1E6BD4`, `1E6C4D` | Conditional `FST [ESI+D4]` in `1E66E0` |
| `1E70EE` | `FSTP [ESI+88]`, after using `[ESI+90]` |
| `1E8B97`, `1E8C34` | Mode-driven `FSTP [ESI+90]` from action-definition data |
| `1E70BD`, `1E70CF` | `FLD [ESI+150]`, `FLD [ESI+D4]` before original raw-X store |
| `1E70DB` | Original `FSTP [ESI+7C]`, observed by Stage59 post-EIP `1E70DE` |

The fields have **multiple** native write paths. A raw `[register+offset]` scan also sees unrelated object structures. Do not assign all references to the P1/P2 motion object without proving base pointer identity. The upstream setters are not proved to be COM-only branches.

## Existing runtime capture cannot be joined across sessions

The current root repository includes these files, but not from the same run:

| Trace | Existing root timestamp | Observed object example | Game thread |
|---|---|---|---|
| Stage46 | 19:56:41–19:57:25 | `2A35A020` | `10280` |
| Stage47 | 19:56:41–19:57:25 | `2A35A020` | `10280` |
| Stage59 | 22:07:43–22:08:23 | `2A89A020` | `15132` |

The Stage46/47 and Stage59 QPC ranges are unrelated and may reflect separate process starts. **Do not reuse, mix, rebase, subtract or normalize these QPCs into artificial matches.** Current evidence cannot produce a valid Stage46→47→59 triple for the 22:07 capture.

## Stage61 offline same-run correlator

[correlate_stage61.py](tools/correlate_stage61.py) reads three text logs, never modifies them or launches the game:

- Parse Stage46 `STAGE46_MOTION_WINDOW` and latest earlier `definitionAfter`, active O+60 and definition+30 mode.
- Parse Stage47 `STAGE47_CONSUMER_WINDOW`, match by **side, object, TID**, native QPC window, definition pointer and current action state.
- Parse Stage59 `STAGE59_X_WRITE` only at `RVA_after=001E70DE`, derive the object from `watched-0x7C`, require same side, object, TID and strict containment inside Stage47's native QPC window.
- Only choose the **latest preceding** Stage46 binding for that actor. If its definition differs from the Stage47 definition, classify mismatch; never search backward to a stale earlier coincidental match.
- Reject absent actor overlap and disjoint Stage47/Stage59 QPC windows. Report latest-original X raw and Stage47 POST agreement separately from best-effort `oldShadow` PRE agreement. Report mode/active ID distribution only from strict triple matches.
- A positive triple establishes **temporal and structural provenance only**; no evidence of COM-specific decision causality or new AI behavior without separate direct action producer and controlled tests.

Four synthetic tests in [test_stage61.py](tools/test_stage61.py) passed: one positive triple, disjoint QPC rejection, different-object rejection and definition mismatch. Tests contain only invented sample values.

## Next action

**No DLL/source changes in the game are required.** The Stage59 r2 DLL (1,153,024 bytes) retains Stage46, Stage47 and Stage59 instrumentation.

If the user still has `ArcanaAI_Stage46DefinitionMotionTrace.log` and `ArcanaAI_Stage47MotionConsumerTrace.log` from the **same 22:07 game run** as the uploaded Stage59 r2 log, upload **only those two files**, keeping the old Stage59 log unchanged.

If files have already been overwritten, run one new ordinary COM-vs-COM session with the **unchanged Stage59 r2 DLL**; upload **Stage46 + Stage47 + Stage59** logs from that single run. Build.log is not needed for a no-rebuild reuse of the previously verified DLL. Use the correlator on this homogeneous capture and independently check health/startup markers.

For Stage62, if valid triples emerge, examine **the specific action IDs and definition modes present before changed-X samples**, and then join Stage40/42/44 command activation with the same player/object/thread/QPC from that same run. The remaining question is *why a COM controller selected the action*, not whether the engine wrote a coordinate. Do not add more DR owners or perturb motion floats.

**Status:** Stage61 static PE checks VERIFIED and offline correlation script SYNTHETIC-TESTED. Stage61 same-run 46→47→59 dynamic correlation **PENDING NEW OR MISSING SAME-RUN LOGS**. No Stage61 DLL/runtime and no AI change.
