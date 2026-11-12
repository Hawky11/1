# Stage127 — F30B0 producer prerequisites, candidate comparison CBF50, and record activity: original PE32 static audit

**2026-11-13 — STATIC ONLY. No hooks, new DLL, Windows build, game run or gameplay/DR writes.**

## Source and reproducible independent evidence

Original private user `AALib.exe`, PE32/i386, preferred imagebase `0x00400000`, size **175,556,608 B**, SHA256 **`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`**, independently checked again against original source bytes. The ZIP `ArcanaAI_Stage127_Producer_RecordFlag_Gates_StaticAudit.zip`, **32,935 B**, SHA256 **`eadce91cdcb8b91db701ba738d2c34438438779ec128223c3ccbb4384c42efa1`**, **13 files**, CRC verified. User-facing archive includes a **full Russian analysis**, self-contained pure-stdlib original EXE verifier, negative/positive tests and results, **752** pinned original x86 instruction byte-sites, **154** independently decoded Jcc/JMP destinations, **31** direct E8 callsites across nine selected targets with a full `.text` byte-scan and separate original GNU objdump reconciliation, **28** labelled decision sites, and all original-byte source contexts. **1,096/1,096 Python unittest PASS**, including tampered bytes, destinations, and CALL targets. No private EXE, DLL or executable patch is included. Full file/csv/json catalog is in the ZIP, not this branch.

Direct original E8 callsites in this scoped target census: `F30B0=2` (`F3654,F3CBC`), `F3930=3` (including `F7AD1`), `CBE00=2` (`F3216,F3234`), `CBF50=7` (including `F3B55`), `CBFD0=1`, `CBDE0=12`, `CBE90=1`, `CBF20=1`, `F38D0=2`; **31** total. These are static distinct original callsites, **not** dynamic invocation counts or all possible indirect call paths.

## 1. F30B0 only returns numeric 0x27 under a long precondition chain

Two possible results inside original `F30B0`:
- **`F325A MOV EAX,0x27`**: requires *all* native preliminary checks, then no active records in **40 + 176 = 216** entries of stride **0x24 bytes**, tested through `CBE00(record+0x10)` at `F3216/F3234`. On this path two caller output DWORDs are zeroed at `F3254/F325F`.
- **`F326A MOV EAX,[EBP+0x0C]`**: a different return path reached if **any** earlier check fails or **any** of the scanned records is active. This returns the *input candidate*, not necessarily 0x27.

Validated important prerequisite checks at original RVA `F30D6/F30E0` (both output pointers non-null), `F30E9` (first output DWORD has `-1` sentinel), `F30F6` (object byte `+0x1A9` nonzero), `F311A` (`CC330` positive), `F3133/F3146/F3159` (`F58E0` returns false for `0x1D/1E/1F`), `F316D` (`C99F0` positive), `F3183` (`CB9C0` positive), `F3194` (indexed original code byte is exactly `0x27`), `F31B0/F31C8` (one of two `C9B40` checks positive), `F31FE` (the `C9A30` check must not reject when both object bitfields contain the same chosen bit). The full loop checks 16 bit positions, before the two active-record groups.

The exact geometry is confirmed by original `F3210 CMP ESI,0x28` (40 first-group records), `F3223 ADD EBX,0x24`, `F3244 CMP ESI,0xB0` (176 second-group records) and `F3241 ADD EDI,0x24`. The full native control flow includes additional checks and earlier exits. **No Stage123 log measured these F30B0 prerequisites or the F325A branch**. Therefore observing MODE0 code 0x27 does not establish that these 216 records were all inactive.

## 2. New additional gate in F3930: CBF50 can bypass record+0x10 test

Original producer `F3930` builds a candidate `[ESI + code*0x24 + 0x420]`, and calls `CBF50` at **`F3B55`** with mode flag **1**. At `F3B5C` it skips the candidate when `CBF50` returns AL=0. This is a candidate **comparison/filter** predicate, NOT an attack/gameplay execution ACK.

Inside original `CBF50`:
```asm
CBF53 CMP BYTE [ECX+8],0      ; candidate comparison context
CBF58 MOV BL,[EBP+10]         ; comparison mode argument
CBF5B JNE CBF61
CBF5D TEST BL,BL
CBF5F JNE CBFB2               ; mode != 0 and context[8]==0 -> AL=1 before active-record test
...
CBF68 CMP BYTE [candidate+10],0
CBF6C JE CBFC6                ; after fastpath: inactive candidate -> AL=0
...
CBF73 JE CBFB2                ; if other record null: AL=1
CBF75 CMP BYTE [other+10],0
CBF79 JE CBFB2                ; if other record inactive: AL=1
...
CBFB2 MOV AL,1
CBFC6 XOR AL,AL
```

When called from F3930 with mode=1 and `BYTE [context+8]==0`, **CBF50 returns 1 before touching candidate activity**. On other paths it checks candidate and second record activity, then `record+0x11`, `+0x04`, `+0x20` and numeric ordering; the comparison result must not be mistaken for action acceptance. This fast-path fact is verified directly by native opcodes and branches, **not inferred from runtime**. Other six static CBF50 callsites may pass other mode flags and require independent analysis.

## 3. F3930 fallback is optional, not every producer invocation

Native `F3930` at **`F3CBC CALL F30B0`** reaches it only after original object/flags/other subfunction guards (notably `F3C6D..F3CAA`). The result is saved back to candidate via `F3CC4`; `F3CC7 CMP EAX,-1` and `F3CCD JNE F4011` choose a subsequent route. `F7AD1 CALL F3930` links to the previously observed Stage94/104 producer, but this does not make F30B0's special `F325A` an obligatory path for Stage123 MODE0 0x27.

Related `CBDE0` clears `BYTE record[+0x10]` at `CBDF7`; `CBE00` reads it. Stage123 Stage110 recorded `CEBE0 entry=0` in reported HEALTH windows, while another native path `CF0FB` could update that same flag if CEBE0 ran in a different state. The selected writer/reader census is not an exhaustive alias-aware inventory.

## Interpretation and next step

**Stage127 resolves the static logic, not the live causal history.** In Stage123, MODE0 `0x27 → CEA20 PRE → POST` was observed **25/25**, but no F30B0, F325A, or CBF50 instrumentation existed; neither "all 216 records were inactive" nor "every action is blocked" nor "the action was committed" follows. Conversely, a CBF50 AL=1 can occur without an active candidate, so a positive comparison predicate is not automatically evidence of a valid executed attack.

**Stage128 option:** if the actual `0x27` origin materially matters for AI behavior, begin with SHA-guarded **passive observer feasibility** for the original `F30B0` entry and `F325A` selected branch, plus one unique same-session/call-scope return join in `F3930`, before adding any new detours. Do not mutate results, patch FA480, force active record flags, rehook F2DE0, write DR/game data, or install broad table hooks. **Stage36 OFF**.

Preserve current Windows-tested DLL Stage122 **2,089,984 B**, Stage115 rollback **2,082,304 B**, Stage113 **2,079,744 B**, Stage110 **2,073,088 B**.

[Stage126 236 handler/static fallback audit](https://github.com/Hawky11/1/blob/stage126-dispatch236-recordflag-fallback27-static-audit/Stage126_Analysis/FULL_STATIC_NATIVE_AUDIT.md).
