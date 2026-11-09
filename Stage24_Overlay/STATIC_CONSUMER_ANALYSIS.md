# Stage24 — static analysis of the resolver-result consumer

Date: 2026-11-09  
Binary: `AALib.exe`, PE32 x86, ImageBase `0x00400000`, SizeOfImage `0x0E0EB000`, timestamp `0x5FE5AA91`  
SHA-256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`

This stage follows the runtime-confirmed Stage23 result. Stage23 captured resolver record byte +3 = `0x1F` for type-1/payload-`0x1E` in both players. This note identifies the consumer of that value and defines the Stage24 observation point.

## 1. Consumer callsite and arguments

The dispatcher calls helper VA `0x004F57D0` at RVA `0x000EE306`.

Relevant callsite instructions:

```asm
004EE2FC  push ecx
004EE2FD  push edx
004EE2FE  push eax
004EE2FF  lea eax, [esi+3A08h]
004EE305  push eax
004EE306  call 004F57D0
004EE30B  mov edi, [ebp+08h]
004EE30E  add esp, 10h
```

At entry to the CALL, the original cdecl arguments are:

| Stack location | Argument | Source |
|---|---|---|
| `[ESP+04]` | arg1 | `ESI + 0x3A08`, output structure pointer |
| `[ESP+08]` | arg2 | resolver record byte +3, previously observed as `0x1F` |
| `[ESP+0C]` | arg3 | resolver record byte +2, previously observed as `0x03` |
| `[ESP+10]` | arg4 | resolver record byte +4, previously observed as `0x12` |

The callsite prefix signature used by the installer is:

`51 52 50 8D 86 08 3A 00 00 50`

The five-byte CALL is:

`E8 C5 74 00 00`

The 15-byte continuation signature beginning at return RVA `0x000EE30B` is:

`8B 7D 08 83 C4 10 53 57 8B CE E8 D6 E1 FF FF`

The helper entry prologue is:

`55 8B EC 8B 45 14 8B 4D 10`

The installer validates all these byte sequences and verifies that the decoded rel32 CALL target is exactly RVA `0x000F57D0`. It also checks the exact PE image size and timestamp.

## 2. Runtime table address

Static analysis identifies the mapping table's preferred VA as `0x04DDF188`. With preferred image base `0x00400000`, its RVA is:

`0x04DDF188 - 0x00400000 = 0x049DF188`

The observer therefore computes the runtime address as:

`g_gameBase + 0x049DF188 + selector * 3`

For the Stage23 selector `0x1F`, the runtime entry begins at:

`g_gameBase + 0x049DF188 + 0x5D`

For a run with AALib loaded at base `0x005B0000`, this resolves to `0x04F8F1E5`. The runtime base must be taken from the current process; do not treat the preferred VA as an absolute pointer.

Each table entry contains three bytes:
- byte +0: flags;
- byte +1: first mapped selector;
- byte +2: second mapped selector.

The source reads a DWORD for safe bounded access but only reports the first three bytes. The fourth byte belongs to the next record and is not interpreted. This is a snapshot immediately before entering the primary helper. Static control flow shows the primary helper first makes an unconditional call to `0x0049EEA0` before it reads the table entry. The current observer does not instrument that internal call or the exact table-load instruction, so if `0x0049EEA0` were to modify the table, the logged entry bytes could differ from the bytes subsequently consumed. Treat the field as `tableBytesBeforeHelper`, not as a direct capture of the internal load.

## 3. Helper behavior confirmed by static disassembly

The helper receives the output pointer, selector, and two mapped values as ordinary cdecl arguments. Its observed control flow is:

1. It uses arg4 and arg3 in an unconditional helper call at VA `0x0049EEA0`.
2. It loads the selector (arg2) and addresses the three-byte table entry at `0x04DDF188 + selector*3`.
3. It stores the selector in the first DWORD of the output structure.
4. It checks flag bit 0. When set, it clears the output DWORDs at offsets +4 and +8 before applying the mapped fields.
5. It consumes table bytes +1 and +2 to set mapped bits in the output DWORDs at offsets +4 and +8. A mapped byte of `0xFF` is treated as an absent/skip value. The `0x26` selector has a special mapping path influenced by the player's field at `+0x3F50`.
6. It checks flag bit 1. When set, the helper takes a conditional secondary path through VA `0x0049EF00`, passing `-1` and arg4, then sets bit 0 in the DWORD at offset `+0xF8` of the returned object.

The output structure of interest is the three-DWORD prefix at `player+0x3A08`. The helper's wider secondary path is relevant for understanding the flag semantics, but Stage24 does not hook that secondary callsite. Its log fields `flagBit0ClearOutputExpected` and `flagBit1SecondaryCallExpected` are static expectations inferred from the table flags, not proof that a separate helper invocation was captured.

The higher-level semantic label of selector `0x1F` is still unknown. It may be tempting to call it an attack, action, command or animation ID, but neither the current table consumer nor the Stage23 samples prove any such naming.

## 4. Stage24 observer design

The Stage24 wrapper replaces only the five-byte CALL at RVA `0x000EE306` after validating the prefix, CALL bytes, suffix, helper prologue and PE fingerprint.

It:
- copies the original four stack arguments into a private 0x48-byte frame;
- records the original dispatcher EBP and caller return address;
- filters observations to known P1/P2 objects, dispatcher type 1 and payload `0x1E`;
- reads the three runtime table bytes and three output DWORDs immediately before the helper;
- replays the original four cdecl arguments exactly once;
- reads the three output DWORDs again after the helper;
- logs helper result EAX and EFLAGS;
- removes only the private frame and returns to the original continuation, leaving the caller's original four arguments and `ADD ESP,0x10` cleanup intact.

The wrapper surrounds the post-call diagnostic reads with PUSHFD/PUSHAD and restores the helper's real return registers/EFLAGS. It does not write the table, the output structure, or any game state.

The installer uses the same shared patch mutex and thread-suspension strategy as the previous stages, revalidates all bytes while other threads are suspended, flushes the instruction cache, restores original page protection, and resumes the threads.

## 5. Runtime interpretation

The decisive log is `ArcanaAI_Stage24ResolverConsumerTrace.log`. Its fields include:
- original args: output pointer, selector, arg3, arg4;
- runtime table address and read status;
- first three table bytes;
- pre-helper and post-helper three-DWORD output snapshots with independent read masks;
- flag-derived expectations for the two static branches;
- side, player, thread, tick, QPC, caller return RVA, helper EAX and EFLAGS.

Correlate each Stage24 record with Stage23's `resolverByte03` and resolver QPC, then Stage21 classifier/fallback and Stage22 post-AND for the same player/thread/tick. Expect arg2=0x1F, arg3=0x03, arg4=0x12 for the previously observed `0x1E` path; do not hardcode these values into the logger's filter.

If the table flags imply the secondary path, Stage24 establishes that the branch condition was set but not that the secondary call ran. A separate observer is only justified if a runtime pattern or unresolved question requires direct confirmation.

## 6. Build/runtime status and next test

**Source-only.** Stage24 source, INI, README and this static analysis have been committed. No user-side build has yet verified compilation, and no runtime marker or hook installation is claimed.

Expected marker:

`STAGE24_RESOLVER_CONSUMER_BUILD=2026-11-09-r1`

Expected install line:

`Installed Stage24 resolver-consumer observer at callsite RVA 0x000EE306 -> helper RVA 0x000F57D0`

After a successful Release | Win32 build, verify the dedicated marker and installation line before interpreting records. Preserve the exact Build.log and all same-run logs in a separate Stage24 runtime-evidence branch. Keep the two Stage23 runtime-evidence branches immutable.
