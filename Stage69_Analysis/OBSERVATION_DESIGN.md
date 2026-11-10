# Stage69 — scoped deferred-slot writer provenance (DESIGN ONLY)

Date: 2026-11-11. Base: `stage68-deferred-origin-static-evidence`. This is an **observation-design checkpoint**, not a newly verified native writer or a new DLL. Original uploaded logs, Stage67 DLL and `main` unchanged.

## Immutable evidence and distinct transitions

Original AALib.exe SHA-256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. The Stage67 capture is the 2026-11-11 01:30:28 session, P1 O=`2A393020`, P2 O=`2A393C50`, TID `10788`.

1. **Deferred ID origin (P1):** Stage67 seq4412 POST `QPC=1467958766094` has `O+68=FFFFFFFF`; Stage41 seq278 PRE `QPC=1467958862984` already has `O+68=25`. The producer's IP **is unknown**. Stage65 seq276 request `2F` is a separate write to `P+3F4C` and has not been shown to cause `25`.
2. **Arming (P1):** Stage67 seq4434 POST `QPC=1467959867548` has `O+30=0`; seq4436 PRE `QPC=1467959966659` has `O+30=1`. Native instruction `1E8FEB: C6 46 30 01` is a **static candidate only**.
3. **Promotion:** Stage67 seq4436 directly confirms `O+68 25→FFFFFFFF`, `O+64 2F→25`, `O+60 2F→25`; the consumer is not the unknown positive-ID producer.
4. **P2 control:** seq3465 sees `O+68=26` while action `33` activates. A source test must observe **both** sides; it cannot hardcode ID `25` as an action selection.

## Decision before any new instrument

**Do not install additional DR0/DR1 watches:** Stage59 already uses these registers. Do not assume DR2/DR3 are free without inspecting runtime ownership, thread propagation, exception-dispatch precedence, and teardown. The existence of free hardware slots does not prove that a new per-object watch coexists safely with Stage59.

Preferred investigation order:

1. **Static inventory of true positive writes.** Examine *all executable instructions that can store into the same object field*, including register-offset stores (`[base+68]`), derived bases (`[base+offset]` where base points into O), memcpy/struct copies, indexed writes, and indirect helper calls. Filter by object identity and caller context, **not** the numeric displacement alone. Record candidate RVA, full original opcode bytes, enclosing function, pointer provenance, callsites, and evidence class. Distinguish `FFFFFFFF` clears (e.g. `1E9F51`, `1E9F11`, `0EDC41`, `1904EE`) and initialization `1E55F9` from positive-ID stores.
2. **Read-only DR/ABI coexistence audit.** From the exact Stage67 working source, enumerate all uses of DR0–DR3, VEH/SEH handler order, per-thread instrumentation and cleanup; identify whether nested handlers and concurrent P1/P2 writes can be observed without changing the original behavior. Treat insufficient evidence as **NO-GO** for a new hook.
3. **If safe and justified**, design a tightly scoped test with one-shot or per-object write observation of `O+68` and a separately gated `O+30`. The logger should record TID, side, actual O pointer, instruction pointer (original RVA or explicit external attribution), old/new value, source register/stack if safely readable, QPC and sequence. Emit a record only when the watched object matches the Stage67-discovered P1/P2 native object identity and a value actually changes. Do not change requested action IDs, movement, damage, AI decisions, stack/flags or branch behavior.
4. **Correlate same run only.** A qualifying proof of an `O+68` positive-ID writer requires an actual *instruction-level* write and matching old/new value on the same actor/TID/object, timestamped within a bracket, with no intervening unaccounted write. A `O+30` writer requires a separately observed byte transition `0→1`, not just an IP that may execute. Respect capture boundaries; do not mix QPC from old sessions.
5. **Stop conditions:** original opcode mismatch; unsupported thread hook propagation; VEH contention; DR overlap; changed source DLL behavior; log overflow; mismatched object; incomplete pre/post mask; or capture lacking the relevant transition. Such a run cannot justify causality.

## Minimal future result matrix

| Field | Required observation | Current status |
|---|---|---|
| `O+68` P1 positive ID `25` | original writer IP + `FFFFFFFF→25`, same object | **UNRESOLVED** |
| `O+68` P2 positive ID `26` | original writer IP + preceding value→`26`, same object | **UNRESOLVED** |
| `O+30` P1 arming | original writer IP + `0→1` | **STATIC CANDIDATE 1E8FEB; runtime unresolved** |
| `O+68→O+64` promotion | entry/exit and nested activation | **RUNTIME VERIFIED Stage67** |

## Operational guardrails and next handoff

Stage69 currently adds **no hook**, **no DLL build**, **no request for new gameplay logs**, and **no new factual attribution**. First complete original-binary alias/callsite inventory and inspect the exact installed source's DR/handler ABI. Only after passing those gates should a narrowly limited Stage70 code change be considered. Preserve known-good DLLs: Stage67 `1,887,232` bytes; Stage65 `1,879,552`; Stage59 r2 `1,153,024`. The original EXE is private and must not be committed. Maintain `PROJECT_HISTORY.md` and `NEXT_SESSION.md` when Stage69 becomes a completed evidence stage.
