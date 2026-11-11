# Stage75 — native event-record handoff to callback `158570` with argument `0x11`

**2026-11-11 — STATIC VERIFIED.** This is an analysis of the original private \`AALib.exe\` with SHA-256 \`28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba\`, grounded against the **existing Stage74 user capture** (commit [211f088](https://github.com/Hawky11/1/commit/211f0882e6b8e220877e4c15b072e5a7b8a7b067)). [Reproducible read-only verifier](tools/verify_stage75.py) passes **35 exact native opcode ranges, 8 decoded relative CALL edges, 158E90's direct E8 caller inventory and the 0x11 dispatch-table entry**. **No new DLL, process hook, game test, memory/debug-register edits, COM decision edits, or user uploads.** Stage74 already proved native Stage38 callback snapshots, not the producer EIP of the selector.

## 1. Previously missing original callback registration: source of key 11

The original source for the callback **parameter** is not a hard-coded \`push 11h\` inside the registration function.

\`\`\`asm
AALib+158E90   push ebp
               mov ebp,esp
               mov eax,[ebp+08]                  ; P
AALib+158E96   movsx ecx,word ptr [eax+3A36]     ; read P+3A36, prior event code
AALib+158E9D   push ecx                          ; saved callback parameter
AALib+158E9E   add eax,3ED0h                     ; pointer to callback descriptor
AALib+158EA3   push 00558570h                    ; callback AALib+158570
AALib+158EA8   push eax
AALib+158EA9   call AALib+0F7E70                 ; setter only
\`\`\`

The exact original \`0F7E70\` helper does two synchronous DWORD stores, **\`P+3ED0=function pointer\`** and **\`P+3ED4=argument\`**. It does not itself execute the callback. A direct relative-\`CALL\` scan of every byte in the original \`.text\` found only **one direct \`CALL 158E90\`: \`EE643\`**. This is a bounded direct-E8 inventory, not a proof that no indirect caller/alias exists.

**Consequent dataflow:** on this original registration route, the **Stage74 \`f4=P+3ED4=0x11\` callback argument must come from a sign-extended \`WORD P+3A36=0x0011\`** when \`158E90\` ran. It does **not** come from Stage71's \`P+3F50\` selector, and \`0x11\` is not an alternative representation of selector 1.

## 2. Native caller and branch conditions

\`\`\`asm
AALib+EE5E0   ... player P from [EBP+08], object O=[P+4]
AALib+EE602   xor edi,edi                       ; EDI=0
AALib+EE604   call AALib+EE480                 ; event-record state update
AALib+EE609   cmp byte ptr [P+3A2C],0
AALib+EE610   je  EE780                         ; skip record path
AALib+EE621   movsx eax,word ptr [P+3A34]      ; previous event mode
AALib+EE628   sub eax,edi                       ; EDI=0
AALib+EE62A   je  EE642                         ; mode 0
...
AALib+EE642   push esi                           ; P
AALib+EE643   call AALib+158E90
\`\`\`

Mode 1 selects a different helper at \`EE63B\` (target \`12EEE0\`); mode 2 selects \`1584C0\` from \`EE633\`. Therefore, **the direct \`EE643→158E90\` path requires a nonzero event-record flag and previous event mode 0**, plus native caller reachability. This is original static control flow, not a runtime EIP hit from Stage74.

## 3. Where P+3A36 comes from — current→previous event-record transfer

The record starts at \`P+3A2C\`, not \`P+3ED0\`:

| Byte offset in event record | Player field | Native observed purpose |
|---|---|---|
| \`+0\` | \`P+3A2C\` | record active/status byte |
| \`+2\` | \`P+3A2E\` | *current* event mode, set by \`F2DE0\` |
| \`+4\` | \`P+3A30\` | *current* event code, set by \`F2DE0\` |
| \`+8\` | \`P+3A34\` | *previous/transferred* event mode |
| \`+A\` | \`P+3A36\` | *previous/transferred* event code (callback-registrar input) |

Original \`0F2DE0\` writes current \`[record+04]\` from its event-code argument and current \`[record+02]\` from its mode argument, subject to status gating. Different helper **\`0F2E50\`** checks validity and whether current pair differs from previous; on the transfer path it performs:

\`\`\`asm
0F2E6A  mov word ptr [record+0Ah],cx   ; previous code <- current code
0F2E72  mov word ptr [record+08h],dx   ; previous mode <- current mode
0F2E7D  mov word ptr [record+04h],dx   ; invalidate current code (-1)
...
0EE576  lea esi,[P+3A2Ch]
0EE57D  call 0F2E50                    ; record current-to-previous helper
\`\`\`

Thus a concrete original route exists **event mode/code → transfer → P+3A34/P+3A36 → \`158E90\` → descriptor**. The transfer is **conditional**; direct writes (e.g. at original \`108E9C\`, \`1BBCE1\`, \`1BBCFB\`) and aliases can also affect \`P+3A36\`. **Stage74 never sampled the record fields**, so we cannot label a particular current-code writer or \`0F2E50\` execution as proven in those five transitions.

## 4. Why these native writes can occur BETWEEN Stage38 windows

Stage38 PRE is after original \`156C00\`; its POST is before original \`FC60\`, not the end of the whole per-player frame. The original loop continues:

\`\`\`asm
EE7BE  call edi              ; callback itself; Stage38 PRE already captured
EE7C7  call FC60             ; Stage38 POST hook is BEFORE this native call
EE7D1  call EE480            ; event-record update
EE7D6  call E8DA0            ; further player state update
...
EE7E4  test cl,1             ; prior SHR by 17h from P+3B7C
EE7E7  je EE7F2
EE7E9  push esi
EE7EA  call EE5E0            ; gated after-callback event handling
\`\`\`

Once reached, \`EE5E0\` calls \`EE480\` again at \`EE604\`, tests record fields, and can invoke \`158E90\` at \`EE643\` to prepare **the next** callback. This establishes a *specific native after-callback registration path that was outside Stage74's boundary sampling*. It does **not** establish that any particular Stage74 selector change was caused by those instructions; the callback-registration path sets \`P+3ED0/P+3ED4\`, not \`P+3F50\`.

All **five** Stage74 between-window selector transitions had the same next PRE callback \`158570\`, parameter \`11h\`, follow-on callback \`155160\`, and \`f31=P+3F54=0\`. This makes the original \`EE5E0→158E90\` path a useful structural explanation for their callback registration, **not a runtime EIP proof of that route**.

## 5. Callback 0x11 consumes the selector, doesn't select it

Original callback \`158570\` copies its argument from the prepared context \`[frame+0C]\` to EBX at \`158578\` and switches through jump table VA \`00558DC4\`. Its native entry **index \`0x11\` = VA \`00558ACB\` (RVA \`158ACB\`)**. In this switch case:

\`\`\`asm
158ACB  mov eax,[edi+3B7C]
158AD1  shr eax,0Fh
158AD4  test al,1
158AD6  je  158B7D              ; gate bypasses selector logic
158ADC  mov eax,[edi+3F50]     ; **READ** original selector
158AE4  test eax,eax
158AE6  jne 158B18             ; selects handling by already-set 0/1/2
\`\`\`

The exact selector-read instruction is not independently sampled by Stage74, so this is a **static possible execution path**. Its presence is not the missing writer EIP.

## 6. Remaining source problem and narrow next step (Stage76)

The **numeric selector** still has the bounded Stage71/73 inventory: literal-one store \`155D29\`, ten register-derived stores that can yield 1/2, nine literal-zero sites. No Stage74 sampled callback changed f30 internally; five changes appeared between snapshots. The event code \`0x11\` explains the *callback dispatch key*, not the COM selector decision.

If precise producer attribution is worth the added instrumentation, **Stage76 should examine/observe the after-callback segment** in this order: (a) after \`FC60\` / before \`EE480\`, (b) between the existing \`EE480\` and \`E8DA0\` calls, (c) just before conditional \`EE5E0\`, (d) around the \`EE5E0\` call. Record existing \`P+3F50\`, \`P+3F54\`, current/previous event mode and code, P/O/TID/QPC and the callback descriptor; log-read failures and reject mismatched actors. Such boundary windows would locate a transition without attempting eleven expensive writer hooks or taking DR0/DR1/DR2/DR3. **Any new hook still needs a separate original-opcode, ABI, stack, x87/SSE, concurrency, install-order, rollback audit and Windows runtime testing.** This Stage75 branch does not contain a new hook or DLL and does not claim actual writer IP, smarter AI, changed attacks, movement, damage or gameplay.

**Rollback:** Stage74 runtime-verified user DLL **1,893,376 bytes**; previous Stage70 r2 **1,892,864 bytes**. Frozen raw user logs from \`211f088\` and \`main\` remain unchanged. No new user logs or build required for Stage75.
