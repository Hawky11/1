# Stage44 original native action-definition binding — static analysis

2026-11-10. This stage follows the proven Stage43 232/232 original caller provenance (all callerRVA AD75C), Stage42 active-ID 232/232 promotions, Stage41 232/232 new/requeue activations. Earlier Stage42 run had six extras, but they did not recur in Stage43; do not assert their upstream O+64 writer was found. Current evidence and exact logs are in stage43-runtime-activation-origin-evidence. All historical stages remain unchanged.

## Verified original AALib.exe

Original binary from ChatGPT Library /AALib.zip, previously materialized and SHA-checked (SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba), size 175556608 bytes, PE32 i386, preferred base 00400000. LLVM disassembly and local Python raw PE signature checker confirm:

- RVA 1E9E10, bytes 89 4E 60: native 1E9C70 writes new current ID O+60. At RVA 1E9E2D bytes E8 5E E6 FF FF, native CALL target RVA 1E8490. No intervening conditional branch between that write and this call (there are data/flag updates). The caller enters with ECX=O and no extra stack args.
- RVA 1E8490, native prologue 55 8B EC 83 EC 0C 53 56 57 8B F1 (standard thiscall), stores O in ESI. Calls native 1E4800 then 1E0120 for action lookup context.
- RVA 1E84A9 reads O+60 and RVA 1E84A6 reads O+6C; passes two stack arguments ID,variant with ECX=context to native 1DF2D0 through RVA 1E84B3, call bytes E8 18 6E FF FF. Its original prologue RVA 1DF2D0 is 55 8B EC 8B 45 08, and return at 1DF2F7 is C2 08 00 (thiscall with two cleaned stack args).
- Resolver RVA 1DF2D0 checks a mode/ID lookup table and returns either table-indexed action-definition pointer or a fallback object pointer. Consequently nonzero O+34 alone cannot prove correct action binding; resolve against fallback if exact values are available.
- RVA 1E84B8 stores returned EAX into O+34 (bytes 89 46 34). RVA 1E84BB sets O+26D byte to 1 (bytes C6 86 6D 02 00 00 01). RVA 1E84CC reads metadata type byte from [O+34]+2C; RVA 1E84D5 stores that value into O+178. Other action flag/timing initialization follows, but actual animation, hitbox, damage and combat effectiveness remain UNPROVEN.

## Stage44 passive hook

Source: Stage44_Overlay/src/Stage44ActionDefinition.inl, included via the new Stage44_Overlay/src/Stage38CallbackWindow.inl following Stage42ActivationHook.inl. The user keeps the working Stage43-flavored Stage42ActivationHook.inl and existing Stage41/40/39 files.

Only one new guarded native CALL rel32 patch at original RVA 1E9E2D -> S44BindingHook. It validates both five original CALL bytes, decoded target 1E8490, and 11 original callee prologue bytes before patching, and again inside stopped-thread patch install. On failure it logs and leaves Stage43/42/41/40 installed. Native call is performed exactly once, no stack arguments, original ECX untouched, EAX and EFLAGS/GPR restored after bookkeeping. Existing FXSAVE/FXRSTOR patterns preserve x87/SSE before/after real native 1E8490 call. No game-state writes beyond the one diagnostic CALL redirect; no injected values or altered INI.

Before/after snapshots for exact P1/P2 O=[P+4] capture 8 DWORDs with masks:
f0 O+60 current ID; f1 O+34 definition pointer; f2 O+26C DWORD containing ready-byte O+26D; f3 O+178 action-definition type; f4 O+6C variant; f5 O+08 flags; f6 O+0C flags; f7 O+48 class. Three metadata DWORDs with independent masks read action-pointer+20/+2C/+48. The original fallback metadata pointer may be readable and nonzero; do not treat every value as a valid action definition without checking metadata.

One SRW ring capacity 2048, TLS depth 8, existing Stage38 worker calls FlushStage44 after Stage42; per-interval STAGE44_HEALTH native calls, other objects, dropped, overflow, mismatch. New ArcanaAI_Stage44ActionDefinitionTrace.log includes STAGE44_ACTION_DEFINITION_BUILD r1, STAGE44_INSTALLED 1/1, per-window P1/P2 object/QPC/fields/masks and health.

Test expectations: Windows x86 BUILD SUCCEEDED, original Stage43/42 and Stage40/41 remain installed, Stage44 1/1; nested P1/P2 Stage44 windows chronologically inside Stage42 native 1E9C70 before/after window for same object; f0 same current ID, f1 O+34 nonzero or fallback, f2 byte O+26D usually 1, f3 metadata type consistent with selected definition+2C low byte. Health counters zero lost/overflow/mismatch. Stage44 Windows x86 compilation and runtime are NOT yet verified; static-only original PE validation is not a substitute.

Original EXE is in Library, do not request re-upload. Preserve all old source/evidence branches.
