# Stage57 static analysis — ACDF0 early and final native helper boundaries

**SOURCE READY, Windows x86 build and game runtime PENDING — 2026-11-10.**

## Why the next probe is needed

[Stage56 runtime](https://github.com/Hawky11/1/blob/main/Stage56_Runtime/RUNTIME_ANALYSIS.md) verified **6,194 same-frame Stage47→Stage54 chains** with **1,337 raw X differences, zero Y** — all inside whole native ACDF0. Of **593** tracked native direct-store writer events, **272** changed X at original **ACF2A**, none executed ACFA3. Only **225** changed native windows were completely explained by the observed pair. **64 changed windows** had matching ACDF0 PRE and ACF2A PRE X, but changed again **after** ACF2A before ACDF0 POST. Another **1,048** changed native windows had no tracked Stage56 writer. These are *same-run* counts, not proof of any specific helper.

Stage57 passively observes **three further original CALL boundaries inside ACDF0** to distinguish early updates, final updates, and writes between them without modifying AI state.

## Original SHA-pinned binary and CALL targets

Private original AALib.exe: 175,556,608 bytes; SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`; PE32 x86 ImageBase `00400000`.

| Native CALL site RVA | Exact original five bytes | Decoded callee RVA | Native prologue (verified) | Native RET |
|---|---|---|---|---|
| **ACE11** | `E8 2A 5E FF FF` | **A2C40** | `55 8B EC B8 60 14 00 00 E8 83 88 16 00` | A2D75 |
| **ACFD6** | `E8 E5 FE 0A 00` | **15CEC0** | `55 8B EC 51 56 57 E8 A5 7D 08 00` | 15CF65 |
| **ACFDB** | `E8 C0 E8 0A 00` | **15B8A0** | `55 8B EC 81 EC C8 03 00 00 53 56 57` | 15BB2B |

Original objdump shows **five-byte complete direct CALL instructions**, all **without caller-supplied stack arguments**, and each native uses a plain RET. A2C40 is called early inside ACDF0. 15CEC0 and 15B8A0 are two consecutive final calls just before ACDF0's native epilogue. **Their RVA is 15CEC0/15B8A0, not 55CEC0/55B8A0** (the latter are image-base-added original VAs).

Independent static inspection verified **original file SHA/size and raw bytes**, rel32 targets, complete native prologues and a PE relocation scan covering 212,051 relocation entries: **zero overlaps** with all six checked CALL/prologue regions. On-disk absolute addresses elsewhere in these natives still relocate normally; there is no assumption that the entire function lacks relocations.

## Instrumentation

New `Stage57_Overlay/src/Stage57ACDF0HelperBoundary.inl` reuses the already runtime-tested Stage55 three-call **original-once** wrapper architecture: three complete, checked direct CALL replacements, GPR/EFLAGS + FXSAVE/FXRSTOR around read-only snapshots, original native called once, TLS pending stack of depth 8, ring of 8,192 samples, both P1 and P2 objects, 13 raw fields with `f1=O+7C X`, `f2=O+80 Y`, `f3=O+AC anchor X`, `f4=O+B0 anchor Y`, QPC and per-site/health counters.

Hook installation requires Stage56 2/2 and Stage55 3/3 already installed and verifies all signatures before and during thread suspension. Any mismatch returns without Stage57 patching; earlier stages are not deliberately uninstalled or altered. It includes exactly **three additional CALL hooks** at ACE11, ACFD6 and ACFDB. The original game AI logic/coordinate values are not intentionally written by these probes.

Adapter `Stage57_Overlay/src/Stage38CallbackWindow.inl` differs from Stage56 adapter only in: adding the Stage57 include, chaining Stage57 installation after successful Stage56, and flushing Stage57 at the existing stage flush point. All other Stage47–56 includes and source remain unchanged.

The wrappers do add an indirect call frame and logging timing. Synthetic equivalence of wrappers with complex native game execution and exceptional x87 pointer metadata has **not been established** for Stage57. No Windows MSVC build and no game runtime test have been run.

## Validation / next check

```powershell
py -3 Stage57_Analysis\tools\verify_stage57.py --aalib "D:\path\to\AALib.exe" --repo-root "D:\path\to\stage57-checkout"
```

The Python stdlib verifier checks SHA-pinned original PE CALL/prologue signatures and relocation non-overlap, then compares the exact three adapter edits and required source symbols. It is **not** a native unit test or substitute for an actual Windows build.

After successful compilation, require `STAGE57_INSTALLED 3/3`, retained Stage56 2/2 / Stage55 3/3, all masks 1FFF, zero missingObjects/dropped/overflow/mismatch/badCaller and unique contiguous Stage57 sequences.

Compare **Stage55 ACDF0 PRE→Stage57 early ACE11 PRE/POST→Stage56 direct writer if present→Stage57 final ACFD6 PRE/POST→Stage57 ACFDB PRE/POST→Stage55 ACDF0 POST** using side/player/object/thread and nested QPC. Count changed X at each sub-boundary, including 64 previously demonstrated after-ACF2A change paths (fresh capture count may differ) and changed windows lacking Stage56 writer. Do **not** assume the root helper's X effects occur only at the direct CALL boundaries or that a same-number count proves identity.

If the two final native boundaries do not account for the remaining X changes, inspect pre-final code and nested helpers in the newly narrowed interval. No claim about visible animation, collision, damage, attacks or smarter AI follows from raw X bits.
