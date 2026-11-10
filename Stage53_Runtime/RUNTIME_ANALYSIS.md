# Stage53 — parent AF0A0 coordinate boundary runtime VERIFIED (2026-11-10)

## Source and evidence provenance

- Frozen source base: `stage53-parent-af0a0-coordinate-boundary`, commit `c5ebdb9070701c82383918b16b025919015889ee`.
- Exact user uploads on main: `ac8b4baca7479be511a9137af6b1cffbdac24998` and `2bb84f00f6f8702ba8334a9958ae41f0fa381a74`.
- This evidence snapshot carries source overlay/analysis together with the *unmodified blob SHAs* of all main root logs and Build.log from commit `2bb84f0`; no rerecorded or inferred runtime entries.
- Windows x86 `Build.log`: BUILD SUCCEEDED, started 2026-11-10 17:18:05, `dinput8.dll` size 1,105,408 bytes. Preexisting nonfatal MSVC warning C4754 in AICommandSiteTrace.cpp; no compile/link failure.
- COMvsCOM enabled at 17:18:32 for P1/P2. Stage53 marker `STAGE53_PARENT_COORD_BUILD=2026-11-10-r1`, installed `STAGE53_INSTALLED 1/1 A2B90->AF0A0`. Earlier Stage52/51/50/49 retained.

## Exact observations

| Trace | Matched tracked rows | Comments |
|---|---:|---|
| Stage47 | 5,904 (2,952 P1 + 2,952 P2) | Current 17:18 run |
| Stage53 parent A2B90→AF0A0 | 6,024 (3,012 each side) | contiguous seq 1..6024 |
| Stage52 AEF10 + AED90 | 12,048 (6,024 each native) | current same run |
| Stage53 health records | 28 | missingObjects, dropped, overflow, mismatch, badCaller all zero |
| Stage53 full-window changedMask!=0 | 2 | only first P1/P2 `f7 = O+18: 0→4`; no X/Y change |

All Stage53 P1/P2 rows have `preMask=postMask=1FFF`, `callerRead=1`, `callRVA=000A2B90`, and show no changes in `O+7C` (X), `O+80` (Y), `O+AC` (anchor X), `O+B0` (anchor Y) within the parent call.

Same-side/object strict chronological QPC join:
- Stage47 POST → first following Stage53 PRE within 5000 ticks: **5,904/5,904** valid unique pairs, QPC gap **355–1,742 ticks**; **1,091 X differences** (P1 **403**, P2 **688**), **zero Y differences**.
- Stage47 POST X equals Stage53 PRE anchor X in **5,904/5,904** pairs.
- Stage53 PRE → enclosed Stage52 AEF10 PRE: **6,024/6,024** pairs, **zero X or Y differences**.
- Stage53 PRE→POST: **zero X/Y changes** across 6,024 windows.
- 120 Stage53 tail rows lack Stage47 preceding windows and **are excluded** from Stage47→Stage53 denominator, not treated as no-change.

Example P1: Stage47 seq124 POST X `4450802B` vs Stage53 seq123 PRE X `4450402B`, Stage53 anchor X `4450802B`, gap 393 QPC ticks.
Example P2: Stage47 seq123 POST X `445E8000` vs Stage53 seq124 PRE X `445EC000`, Stage53 anchor `445E8000`, gap 406 QPC ticks.

## Interpretation and next step

The tracked live-X difference **precedes** `A2B90 → AF0A0`. The native AF0A0 wrapper contains no observed P1/P2 X/Y changes and therefore is not the responsible writer in this run. The next sequential sibling no-argument calls are `A2B81→A1B80`, `A2B86→CFB80`, and `A2B8B→BAB60`, between Stage47 and Stage53; passive before/after snapshots can isolate the first observed change. The drift could also occur before the first sibling call, which must be distinguished from a change inside it.

Causality/semantics remain unproven: raw pointer/coordinate deltas cannot by themselves establish rendered movement, dashes, cancels, hits, damage, or enhanced AI. Existing COMvsCOM DWORD forcing is a separate intentional behavior change. Stage53 passive wrappers do not intentionally write gameplay state. Source branch previously remained untested; **only this exact Windows Build.log and runtime log** verify it.

## Preservation

Keep Stage53 source and these exact log blobs frozen, and preserve prior Stage47–Stage52 source/runtime evidence. Do not alter the game's decision or coordinate data in the follow-up investigation.
