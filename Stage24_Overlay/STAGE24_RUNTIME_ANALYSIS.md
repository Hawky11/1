# Stage24 runtime analysis — 2026-11-09 16:35 run

## Provenance
Upload: stage23-resolver-result-trace commit 96464289b7c41c4f62402b75ad1081d191ae999b (user accidentally uploaded Stage24 logs to Stage23).
Stage24 source/INI snapshot: stage24-resolver-consumer-trace commit 0bca8a6d8e851990563b281c517f01bd1a0c81a2.
Build transcript supplied in chat: 2026-11-09 16:35:12, MSVC 14.51.36231 x86, eight translation units compiled, DLL linked at 233984 bytes; sole warning C4754 at AICommandSiteTrace.cpp:785.
Evidence preserves all 19 uploaded log blobs exactly, including large logs that could not be read through the connector. The source/INI are the published Stage24 inputs, not a separately uploaded local-source attestation.

## Stage24 confirmed
Marker STAGE24_RESOLVER_CONSUMER_BUILD=2026-11-09-r1 and successful installation at CALL RVA 0xEE306 -> helper RVA 0xF57D0, 39 suspended threads.
Two records, both P1=0x0E251F78, tid=7572, outputPtr=0x0E255980 (player+0x3A08).
Both selector=31 (0x1F), arg3=0, arg4=0, tableAddress=0x04F8F1E5, tableReadable=1, tableBytesBeforeHelper=(03,00,10).
Both before/after read masks=7. Result EAX=0x0E27CCA0; do not infer a semantic return value or a captured secondary-helper call from this register alone.

| Tick | Output before (hex DWORDs) | Output after |
|---|---|---|
| 28299578 | (00000006,00001002,00000000) | (0000001F,00010001,00000000) |
| 28307859 | (00000004,00000011,00000000) | (0000001F,00010001,00000000) |

The observed final mask 0x10001 is consistent with mapped selectors 0 and 0x10. Flags=3 imply the two documented flag-controlled paths according to static analysis. Their internal execution is not directly observed; table bytes are a pre-entry snapshot, before the primary helper's unconditional internal call.

## Corrected argument interpretation
Earlier Stage24 documentation predicted arg3=3 and arg4=18 by assuming resolver bytes +2/+4 survive until EE306. This run contradicts that prediction: both actual stack snapshots contain zero/zero while Stage23 still reads bytes 3/18.
Source review: the wrapper copies entry stack +4,+8,+C,+10 to frame[0..3]; pre-state capture initializes frame[6..17], not argument slots; repeated PUSH [ESP+0x0C] replays arg4,arg3,arg2,arg1; logging reads frame[2],frame[3]. No argument-slot overwrite was found in those paths. The intervening dispatcher dataflow must be inspected in full; the old static excerpt contains gaps and does not prove direct byte-to-argument propagation. Do not label this a resolver change.

## Same-event correlation
| Point | Event 1 QPC | Event 2 QPC |
|---|---:|---:|
| Alternate call F6AD7 | 282996031240 | 283078842907 |
| Queue acceptance | 282996031303 | 283078842935 |
| Dispatch before | 282996031316 | 283078842950 |
| Stage23 resolver | 282996031320 | 283078842957 |
| Stage24 post-helper log | 282996033684 | 283078844048 |
| Stage21 classifier | 282996035900 | 283078845579 |
| Stage21 fallback | 282996037027 | 283078846084 |
| Stage22 post-AND | 282996037413 | 283078846448 |
| Dispatch after | 282996037859 | 283078847587 |

Both queues accepted type1/payload 0x1E; dispatch before/after history matches. Both classifier AL=0; fallback index3 slot remains 0->0; controller bit0x400 remains 0->0. No nonzero fallback clear is established. No P2 Stage24 target record in this run. QPC differences include diagnostic overhead.

## Additional gate/setter sample
Two P2 Stage18 gates returned AL=1, key0x19, table byteFA. Both Stage20 object guards returned AL=0 with object+3B70=0x640 (bit2 clear), a different raw object value from the earlier 0x58 sample.
Gate-to-guard deltas 123/121 QPC ticks; guard-to-setter 73/71. Both Stage17 setter calls use index0x71, same player/thread/tick28309390 and matching frame. Snapshots show 3B84=0 at first call and 0x20000 at second. Exact store attribution is not concluded here because the large write trace could not be read.

## Completeness and limits
Read all contents of 17/19 logs, including the 1,279,675-byte StateMapTrace via Git blob endpoint. These files end in newlines; no explicit ERROR/WARNING/DROPPED/OVERFLOW/FAILED markers were found. Stage24/23/22 seq=1,2; StateMap seq ends2290.
PlayerBit17WriteTrace (11,721,014 bytes) and ProducerCallTrace (8,120,267 bytes) exceed connector retrieval capability in this session. Their exact blob SHAs are preserved, but integrity/sequence/overflow and write transitions have NOT been analyzed. Do not confuse fetch_file returning empty content for these large files with an empty source log.

## Next task
First retrieve the two large logs using a binary-capable transport and complete write-trace attribution.
Then inspect the complete dispatcher instruction range EE278..EE306 in the fingerprinted executable to locate where ECX/EDX become zero. Preserve this evidence; do not create an additional hook merely to compensate for an incomplete static excerpt.
Only after that review choose a Stage25 observation point for the internal secondary helper and returned-object flag at +0xF8, if still needed. Stage24 has established an actual output-structure transition, not the gameplay meaning of selector0x1F.
