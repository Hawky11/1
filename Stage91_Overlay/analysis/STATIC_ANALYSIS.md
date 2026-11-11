# Stage91 — native direct-special-bit readers before controller bitmap clear

**SOURCE ONLY.** Original `AALib.exe` SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` (from the supplied `AALib.zip`), PE32 RVA mapping independently verified. Source-only static verifier: **68/68 PASS**. No Windows x86 compilation or real-game Stage91 capture yet. Preserve previously tested Stage90 DLL **1,991,168 bytes**.

## The missing read path

Stage88/90 have already established real native `F57D0` output in `P+3A08/0C/10`: special bits `0x200`, `0x400`, and `0x800` appear and are later cleared by other `F57D0` calls. Stage89 only observed calls to `F58E0(bitmap,0x26)` at `AAB55` after these bits had been removed (AL=0). **But `F58E0` has a generic bit-lookup mode for every index except `0x26`.** With `bitmap=P+3A08`, its `F592F AND EBX,[ESI+EAX*4+4]` checks:

- index **9** → `P+3A0C & (1<<9) = 0x200`;
- index **10** → `P+3A0C & (1<<10) = 0x400`;
- index **11** → `P+3A0C & (1<<11) = 0x800`.

Three consecutive, individually guarded callers **inside `F5990`** use this exact interface, unlike Stage89's four index=0x26 sites:

| Native CALL RVA | Original bytes | Native argument | Immediate post-call branch / function effect |
|---|---|---|---|
| `F5A4D` | `E8 8E FE FF FF` | `(P+3A08, 9)` | `F5A55 TEST AL`; `F5A57 JNE F5A68` on AL=1, setting `P+3B70 |= 0x8` |
| `F5A5C` | `E8 7F FE FF FF` | `(P+3A08, 10)` | `F5A64 TEST AL`; `F5A66 JE F5A73` on AL=0; otherwise sets `P+3B70 |= 0x8` |
| `F5A76` | `E8 65 FE FF FF` | `(P+3A08, 11)` | `F5A82 TEST AL`; `F5A84 JE F5A8F` AL=0 → `9ED60`, AL=1 → `9ED20` |

`F5A44 LEA EDI,[ESI+3A08]` establishes the exact bitmap pointer; `F5A3C SHR EDX,2; F5A3F TEST DL,1; F5A42 JNE F5A68` is an *upstream guard* for the first two calls. The F5A4D and F5A5C branches short-circuit later checks. `F5A76` is reached only if both earlier indices were false. Thus *three sites installed ≠ three sites executed per call*.

Native function `F5990` is called from **`ACAB8 CALL F5990`** (`E8 D3 8E 04 00`) in an actor update loop, following `ACAB2 CALL E6EC0` and before `ACABE CALL F60C0`. This places the candidate reader in an update path; it does **not** prove that active Stage88 bits reach these calls or that they issue a move, attack, or hit.

## Stage91 r1 observer design

- Install guarded patches at the three **five-byte CALLs** above. **No global `F58E0` entry detour and no additional `EE306` hook.** Preserve the original native callee's `F58E0` execution **exactly once** per site.
- In a small `__declspec(naked)` wrapper: save flags/GPR and x87/MMX/XMM state around each passive observer; call original F58E0 with identical cdecl args once; restore return EAX/EFLAGS and the original stack. Check PE fingerprint and exact bytes **before and again under peer-thread suspension**, reject EIP collisions, patch inside global mutex, and roll back modified CALL bytes if any patch fails.
- Capture both P1/P2; return `AL`, exact native bitIndex (9/10/11), bitmap triple PRE/POST with read masks, same-thread QPC, `P+3B70` before, and *predicted* next branch (not measured branch execution). In the C++ hook, keep bounded 8192 ring, 16-depth TLS, and worker-only log flushing.
- Installer adds one `.inl` file and replaces only existing `Stage81SelectorWriter.inl` to call Install/Flush/Health; Stage24/87/88/89/90 source and their patches remain unmodified. `ApplyStage91.ps1` pins **four** Stage90-baseline SHA values and refuses if anything changed; `RollbackStage91.ps1` restores source hashes. Back up tested Stage90 DLL and keep Stage36 disabled.

**Critical interpretation:** A Stage91 observed `AL=1` at a callsite is direct native evidence that one specific bit was read as 1 by this predicate. It is *not* a proven final command/action until strict same-session downstream queue/events are linked. The pre/post read could race other writers; mark differing snapshots unstable rather than infer a match.

See the [Stage90 previous runtime evidence](../Stage90_Runtime/RUNTIME_ANALYSIS.md), [Stage91 synthetic audit tests](../Stage91_Overlay/tools/test_correlate_stage91.py), and [Stage91 static results](static_results.json).
