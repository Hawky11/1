# Stage51 Windows x86 runtime — verified passive AE220, X divergence occurs earlier

Date: 2026-11-10. Source: user-uploaded main branch commits [b411d18](https://github.com/Hawky11/1/commit/b411d18a1c22a3f1f2ddd5c27b08cfb03f1f4894) and [0341856](https://github.com/Hawky11/1/commit/034185644dd587f5e3fc1eaf725e24d0f8b93c6f). This evidence branch freezes the exact uploaded logs at base commit `034185644dd587f5e3fc1eaf725e24d0f8b93c6f`.

## Build and hook integrity

- `Build.log` starts 2026-11-10 16:14:35 local; **BUILD SUCCEEDED**, x86 MSVC DLL `dinput8.dll` **1,089,024 bytes**. Existing nonfatal C4754 warning at AICommandSiteTrace.cpp:785; no compile/link failures.
- `ArcanaAI.log` confirms COMvsCOM active at 16:14:58 (CPU control DWORDs forced to 1).
- `ArcanaAI_Stage51GlobalCoordinateTrace.log`: `STAGE51_GLOBAL_COORD_BUILD=2026-11-10-r1`; `STAGE51_INSTALLED 1/1 AF119->AE220`; wrapper reports copied original arg1, one native call, original caller cleanup unchanged.
- Stage50 `STAGE50_INSTALLED 1/1 AF136->AE100` and Stage49 r2 `STAGE49_INSTALLED 2/2 AF13D/AF1AA->1E9170` remain installed. Stage47 `INSTALLED 2/2`.
- Installation is proved by in-game log markers, not inferred from build success.

## Full blob verification

Fetch **Git blobs** rather than path content for >1 MB logs (the contents wrapper can return an empty content body for large files):

| Log | Git blob SHA | P1/P2 windows | Other findings |
|---|---|---:|---|
| Stage51 | `76f91158cfb3ba1fcf1ac7d25b1de553215e1697` | **6,560** (3,280 each) | seq 1..6560, no gaps, pre/post masks 1FFF, all arg=1/read=1/callRVA=AF119; changedMask=0000 for **all** |
| Stage50 | `398534c5ff7efab466054d45f9fb431f75a9f22f` | **6,558** (3,279 each) | seq 1..6558, masks 07FF, changedMask=0000 and gateCandidate=0 all |
| Stage49 | `da51e25ecc90bc0589d7f46cb5260e5f0bc72270` | **6,556** (3,278 each) | seq 1..6556, masks 07FF, changedMask=0000 and staticGateCandidate=0 all; AF13D tracked |
| Stage47 | `fd83ee83c0f845d367a7ef998b0990e19a5f73a8` | **6,494** (3,247 each) | seq 1..6494, masks 1FFF, **4,037** windows with changedMask nonzero |
| Stage48 | `d573a25eb276d1f4e71007d9b80c1eda8bb89c8f` | **1,783** binder windows | Previous Stage48 hook still running |
| Stage46 | `57d3b5014a95af59c416ab9aaf93674c28a67050` | **1,783** definition-motion windows | No code change requested |
| Stage44 | `6ce3146edfafbb40484e0b3c67e9050d2207398f` | **1,783** binding windows | No error line |
- Stage38 has **5,942** callback windows.
- Stage51, Stage50, Stage49 and Stage47 each report **31 HEALTH** lines. Stage51 every HEALTH has `missingObjects=0 dropped=0 overflow=0 mismatch=0 badCaller=0 badArg=0`; Stage50/49/47 similarly have zero loss, overflow and mismatch. No ERROR/WARNING in these four logs.
- Stage51 had zero pre/post differences in any of its thirteen tracked raw DWORD fields; specifically no writes to O+7C/+80/+AC/+B0 from the **observed call AF119->AE220**.

## Correlation and native dataflow

Events matched by `side`, `object`, and monotonic `qpcAfter < next qpcBefore`. For Stage47->51, the same-frame matched subset is restricted to **QPC gap <= 5,000**; this yields precisely all 6,494 Stage47 events. The remaining **66** Stage51 rows have no contemporary Stage47 event and are excluded from same-frame conclusions.

| Edge | Matched pairs | Exact raw 32-bit comparisons |
|---|---:|---|
| Last Stage47 POST -> Stage51 PRE | **6,494** | **435** differences in X (Stage47 f5 POST O+7C vs Stage51 f1 PRE O+7C); **0** differences in Y (Stage47 f6 POST vs Stage51 f2 PRE); and **6,494 / 6,494** Stage47 X POST == Stage51 O+AC PRE (Stage51 f3) |
| Stage51 POST -> Stage50 PRE | **6,558** | **6,558 / 6,558** X/Y/AC/B0 bits all match (Stage51 f1/f2/f3/f4 vs Stage50 f0/f1/f2/f3) |
| Stage50 POST -> Stage49 PRE | **6,556** | **6,556 / 6,556** X/Y/AC/B0 bits all match (corresponding f0/f1/f2/f3) |

There are **435** Stage51 rows with O+7C != O+AC in the same-frame matched subset, precisely the count of Stage47-X-to-Stage51-X changes. Earlier total `438` obtained from an unconstrained "latest prior Stage47" search is **not** a valid same-frame count: it includes stale tail matches beyond the end of Stage47. Similarly the five apparent Y differences in the unconstrained join are stale tail matches, not same-frame changes.

Interpretation: a writer updates live X O+7C while leaving anchor O+AC on some frames **after Stage47 and before AF119->AE220**. The actual AE220 native call is observationally inert for these fields, so drilling into AE220 subcalls is not warranted on these data. Stage50 AE100 and Stage49 1E9170 also have no P1/P2 coordinate deltas in this run. These are **observed-call scopes**, not universal properties of each native routine or other caller.

## Next diagnostic boundary — Stage52 source pending

The known SHA-pinned original EXE corridor is:
```text
AF10D CALL AEF10
AF112 CALL AED90
AF117 PUSH 1
AF119 CALL AE220      (Stage51)
AF136 CALL AE100      (Stage50)
AF13D CALL 1E9170     (Stage49)
```

Next, bracket **both** earlier native calls `AF10D->AEF10` and `AF112->AED90` with passive before/after P1/P2 snapshots O+7C/+80/+AC/+B0, reuse the Stage51 event/QPC and health methods, and identify which boundary explains the 435 discrepancies. Confirm exact physical bytes and runtime-safe ABI/signatures before modifying any callsite. If neither call writes, locate the prior writer between Stage47 and AF10D or another intervening native path.

Preserve the tested Stage49 **r2**, Stage50 **r1**, Stage51 **r1**, original game execution and installed lower stages. All new hooks must call the native original **exactly once** and must not write gameplay state. There is **no proof** from these raw numerical snapshots of on-screen motion, animation, hit detection, damage, or improved CPU AI.
