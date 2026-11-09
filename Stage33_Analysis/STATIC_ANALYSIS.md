# Stage33 — type0/payload0: native consumer inside EE200

Date: 2026-11-09. **Status:** read-only static analysis completed against original SHA-pinned AALib.exe; Stage33 observational overlay prepared. **Windows x86/MSVC build and in-game Stage33 runtime NOT yet verified.**

## Provenance and reproducibility

- Stage32 verified runtime: [evidence and 21 QPC pairs](https://github.com/Hawky11/1/tree/stage32-type0-dispatch-runtime-evidence/Stage32_Runtime) @ `a6ed15cce970d92e690f349a8dd7a2cf2e5e5f97`. Three Stage31 alternate episodes (P1 once, P2 twice) reached native `EE200` type0/payload0, with bit17 clear during each EE200 call.
- Original EXE is already in ChatGPT Library `/AALib.zip`, member `AALib.exe`, length **175,556,608 bytes**, SHA-256 **`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`**. PE32 Intel x86, preferred ImageBase 0x400000. Below, every function/instruction label is a **RVA**. Data table addresses are expressly labelled preferred VAs.
- [Stage33_Analysis/tools/verify_stage33.py](tools/verify_stage33.py) uses correct PE section mapping (not RVA-as-file-offset), rejects wrong SHA before any RVA operations, and checks **24 exact instruction signatures, 5 direct CALL targets and 3 native table entries**. Execution against the Library binary **PASS**. Wrong-binary rejection tested against the ZIP; it failed as intended. No executable/ZIP/full disassembly is committed.
- This verification is evidence of *original program code and table contents*, not native Windows DLL compilation or gameplay success.

## 1. Branch and parameter flow for type0/payload0

```text
EE200 native dispatcher(ECX=player, stack type=0,payload=0)
  EE210: O=[P+4]; O+184 <- 0
  EE216: if type==0, fall through EE221
  EE221: push payload(0)
  EE222: CALL 158560                 ; returns table VA 6521F80 + 8*payload
  EE227: ECX = BYTE [table+4] = 0
  EE22B: EDX = BYTE [table+2] = 0
  EE22F: EDI = BYTE [table+3] = 0
  EE2C0: P+3F64 <- ECX(0)
  EE2C6: CALL 158EC0(P, EDX=0)      ; table-driven flags at P+3B70
  EE306: CALL F57D0(P+3A08, EDI=0, P+3F30, P+3F34)
  EE315: CALL EC4F0(P, type=0, payload=0)
  EE31A: test EDI (0)
  EE322..EE32F: payload=0 skips payload4/5 and payload0x23 special branches
  EE37E: payload=0 skips payload0x1F special branch
  EE45F: payload=0 skips payload0x25/0x26 branch
  EE34F..EE352: O+08 &= 0xFFFFFBFF
  EE35D: ret 8
```

The table at preferred **VA `0x6521F80`**, indexed by payload0, begins **`0F 00 00 00 00 00 00 00`**. The first byte is **0x0F**, not zero; *the relevant output fields at +2,+3,+4* are zero. `158560` itself is a simple eight-byte-index address calculation; no game action is performed by the lookup.

## 2. Concrete downstream writes and tables

**`158EC0(P,0)`:** preferred VA `0x652222C` first byte `01`. It sets `P+3B70` bit 10 (`OR 0x400`) and clears bits 5,6,7,8,9 according to table bits. The function is a table-controlled bitmask setter; the term *attack enabled* would be an unverified gameplay interpretation.

**`F57D0` with id 0:** preferred VA `0x4DDF188` has index0 three-byte descriptor **`01 00 FF`**. The observed code writes selected id 0 into `P+3A08` and, for descriptor flag bit0, zeros `P+3A0C/+3A10` then sets bit0 of `P+3A0C`. Additional descriptor bit1 is absent for index0. These are concrete controller/map writes, not proof of a rendered attack.

**`EC4F0(P,0,0)`:** This is a large state-reset/coordination function, not an empty return. Its native code, among other work:

- Clears `P+3B70` bits with `AND 0x7FDF8FFF` at `EC50A`, and conditionally alters other P+3B7x/3B8x flags.
- Calls `F7EB0` on controller `P+3A3C` slots 1, 3 and 0x39 (unconditional early portion), and also slot 0x45 on certain event variants.
- Masks multiple bits on `O+08`, `O+0C`, `O+10`, `O+18`, `O+1C`, and clears O bytes at `O+27C..27F`. Other object/player writes and conditional helper calls follow.
- On type0/payload0, the `type==0 && payload==0x1E` special exclusion at `EACD1..EACE4` is false, so the function can run additional reset logic. Some operations depend on state or callbacks; do **not** assign final-state guarantees to every intermediate write.

**Exact bit17 clear code and validation:**

At `ED188..ED193`, still **inside the EC4F0 routine**, the code pushes args `(queue=P+3A2C, type=1, payload=0x1E)`, then calls `F2ED0`. That helper compares **history WORDs at queue+8/+A**, not a gameplay success flag. On AL0 (`ED19B`), the instruction at **`ED19F`** executes:

```asm
AND DWORD PTR [player+0x3B84], 0xFFF9FFFF
```

The mask clears bits17/18 and **preserves bit16**. Stage32 EE200 entry snapshots for all three scoped events had `history=(0000,0000)` after type0/payload0 activation, so their history does **not** equal `(1,0x1E)`; the native comparison therefore returns AL0 unless another helper modifies history between entry and this call. Runtime Stage29 recorded `0x30000→0x10000` at post-write RVA `ED1A9` **inside each matched EE200 QPC interval**. Together, code and matching timing strongly identify this writer. The fourth P2 cycle from Stage32 also clears bit17 inside **unscoped** type0 dispatch without bit16.

The bit17 clear is an event-history-dependent **native state transition**, *not* proof that the game executed a move or that AI made a successful tactical choice.

## 3. What has and has not been established

**Established:** type0/payload0 chooses table index0; native EE200 performs observable bitmask/controller/object operations; it invokes EC4F0; EC4F0 has an exact conditional bit17/18 clear gated by F2ED0 history comparison; Stage32's three correlated dispatches show precisely the expected bit17 clear during that dispatcher call.

**Unknown:** whether those state changes produce an intended attack, guard, movement, animation, or target acquisition; which downstream callbacks run under the exact per-episode states; whether there is an actual AI failure needing a code change. Do not synthesize type1/payload0x1E or force bit17 based on this evidence.

## 4. Next minimal runtime test — passive Stage33 overlay

[Stage33_Overlay/src/AIQueueTrace.cpp](../Stage33_Overlay/src/AIQueueTrace.cpp) is based on Stage32's working queue/dispatch source. **No new code patch site**; the original `EE200` EventDispatchHook remains installed and calls the game's native dispatcher **once**, preserving the Stage32 type1/type0 detail mode and Stage31 observer. New setting `EnableStage33Type0StateTrace=1` produces a **read-only before/after field snapshot** around the existing `EE200` call only for type0/payload0.

The new `STAGE33_EFFECT_DETAIL` record in `ArcanaAI_EventDispatchTrace.log` shares the Stage32 `EVENT_DISPATCH_DETAIL` `seq`, `qpcBefore`, `qpcAfter`, side, player, TID; it adds 12 raw DWORD values, two 12-bit read masks and no fake success bool. Field index map:

| index | raw field | static reason |
|---:|---|---|
| 0 | P+3B70 | 158EC0 flags + EC4F0 mask |
| 1 | P+3B84 | observed bit16/bit17 lifecycle |
| 2 | P+3F64 | EE2C0 zero output from table |
| 3 | P+3A08 | F57D0 descriptor ID |
| 4 | P+3A0C | F57D0 first bitmask DWORD |
| 5 | P+3A10 | F57D0 second bitmask DWORD |
| 6 | [P+4] | object pointer used in same snapshot |
| 7 | O+08 | EC4F0 and EE352 masks |
| 8 | O+184 | EE210 zero |
| 9 | P+3F54 | conditional EC4F0 field clear |
| 10 | P+3F74 | conditional EC4F0 reset |
| 11 | P+3F78 | conditional EC4F0 reset |

When read mask bit is 0, a displayed zero is a placeholder and **not** a proven game value. Each before/after snapshot is **not atomic**. Only net changes at EE200 boundaries are visible; intermediate writes can be overwritten. Existing worker buffered flush and drop warnings remain in effect.

**Source-only validation:** marker `STAGE33_TYPE0_CONSUMER_BUILD=2026-11-09-r1`; original dispatcher call retained **once**; pre/post reads exist, Stage31+Stage32 markers and INI settings preserved. No x86 native MSVC compile or in-game test performed here.

## Next handoff

Windows user copies *only* `Stage33_Overlay/src/AIQueueTrace.cpp` and `Stage33_Overlay/ArcanaAI.ini` into `D:\INCUBATOR\ArcanaAIOverhaul`, **retains Stage29 AITrace.cpp**, builds with existing x86 `Build.bat`, installs DLL/INI, archives old logs, runs natural COM-vs-COM and uploads `Build.log`, fresh `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage31AlternatePathTrace.log`, `ArcanaAI_Stage29Bit17Transitions.log`, `ArcanaAI_QueueTrace.log` and `ArcanaAI_ProducerCallTrace.log`.

Next analyst: validate Build.log/marker/hook, then join Stage31 episode → Stage32 `EVENT_DISPATCH_DETAIL` → Stage33 `STAGE33_EFFECT_DETAIL` by seq/QPC/side/player. Check the 12 read masks and actual net changes, compare bit17 and object/controller updates to static predictions. **Never equate net state mutation with gameplay success.** Keep historical branches unchanged and don't request original AALib.exe again.
