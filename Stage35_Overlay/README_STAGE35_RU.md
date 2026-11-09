# Stage35 — контроллер → потребитель → флаги (только наблюдение)

**Статус 2026-11-10:** оригинальный `AALib.exe` статически проверен; исходники наблюдателя подготовлены. **Windows MSVC x86 build и настоящий COM-vs-COM runtime Stage35 ещё НЕ подтверждены.** Данные Stage34 — реальный проверенный baseline.

## Установка: заменить только три файла из Stage35_Overlay

Источник: https://github.com/Hawky11/1/tree/stage35-controller-consumer-trace/Stage35_Overlay

1. `src/AIQueueTrace.cpp` → `D:\INCUBATOR\ArcanaAIOverhaul\src\AIQueueTrace.cpp`
2. `src/Stage35ConsumerTrace.inl` → **в ту же папку** `D:\INCUBATOR\ArcanaAIOverhaul\src\Stage35ConsumerTrace.inl` (новый необходимый файл).
3. `ArcanaAI.ini` → `D:\INCUBATOR\ArcanaAIOverhaul\ArcanaAI.ini`

**Сохранить `src\AITrace.cpp` от Stage29, остальные .cpp/.h и Build.bat не менять.** Скомпилировать штатным **x86 `Build.bat`**, проверить `BUILD SUCCEEDED` и заголовок `Build.log`, затем установить собранную DLL и INI. Старые runtime-логи заранее архивировать. Провести естественный COM-vs-COM, не подменять события, payload5, bit17 или игровые флаги. Перед завершением дать >=2 сек worker flush.

Новый файл `ArcanaAI_Stage35ControllerConsumerTrace.log`. Также нужны свежие `Build.log`, `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage31AlternatePathTrace.log`, `ArcanaAI_Stage29Bit17Transitions.log`, `ArcanaAI_QueueTrace.log`, `ArcanaAI_ProducerCallTrace.log` и остальные логи нового прогона.

## Что делает Stage35

INI флаг `EnableStage35ControllerConsumerTrace=1` в `[AIResearch]`. Stage35 не добавляет новых игровых действий: **две проверяемые 5-байтные точки CALL** в оригинальной per-side update `ACA90`, с атомарным совместным patch-install, общим Stage31 mutex, приостановкой остальных потоков, повторной проверкой исходных bytes и относительного адреса CALL, и отказом от модификации при mismatch:

- `ACABE` → исходная `F60C0(P)` (метка `phase=TIMER`): потребление слотов таймера, в том числе `P+3A6C`.
- `ACAC4` → исходная `F5BD0(P)` (метка `phase=FLAGS`): чтение P+3A6C и вычисление player/object flags.

Нативная функция каждого CALL вызывается **один раз** с оригинальным аргументом. Сохраняются GPR/EFLAGS и x87/MMX/XMM/MXCSR; считываются только два подтверждённых адреса P1/P2. Данные пишутся в ограниченный ring на игровых потоках; файловый I/O — лишь во фоновом worker. Ring=2048, depth=8, переполнение/dropped выдаёт `WARNING`.

### Лог и маски чтения

Ожидаются:

```text
STAGE35_CONTROLLER_CONSUMER_BUILD=2026-11-10-r1
STAGE35_CONFIG read-only TIMER ACABE->F60C0 and FLAGS ACAC4->F5BD0 ...
STAGE35_INSTALLED callsite=ACABE->F60C0 phase=TIMER callsite=ACAC4->F5BD0 phase=FLAGS ...
STAGE35_CONSUMER seq=... phase=TIMER side=P1 player=0x... tid=... qpcBefore=... qpcAfter=... preMask=0xFFF postMask=0xFFF f0=...>... ... f11=...>...
STAGE35_CONSUMER seq=... phase=FLAGS side=P1 player=0x... tid=... qpcBefore=... qpcAfter=... preMask=0xFFF postMask=0xFFF f0=...>... ... f11=...>...
```

`preMask/postMask` are 12-bit successful-read masks; **if bit i is zero, value 00000000 is placeholder, not observed data**. Mapping:

| index | Native raw field |
|--:|--|
| f0 | `P+3A08` controller selector |
| f1 | `P+3A0C` controller bitmap DWORD0 |
| f2 | `P+3A10` controller bitmap DWORD1 |
| f3 | `P+3A6C` controller timer slot 11 |
| f4 | `P+3B78` derived flags |
| f5 | `P+3B70` controller flags |
| f6 | `P+3ABC` another conditional state |
| f7 | `P+3F64` last dispatcher output |
| f8 | `P+3A3C` timer-array base DWORD |
| f9 | `P+3B84` bit17/18 lifecycle |
| f10 | `O+08`, O read from `[P+4]` |
| f11 | `O+10`, O read from `[P+4]` |

`seq` is monotonically increasing **within Stage35 only**, not an EE200 or Stage31 ID. Correlate on **player pointer, side, TID and QPC ordering**, preserving uncertainty if other events intervene.

### Checking natural runtime

Compare `Stage34 EE200 exit` to subsequent `Stage35 phase=TIMER` and `FLAGS` with same P1/P2 + QPC. Verify actual TIMER f3 transitions: values may decrement by the **current header DWORD f8 (`P+3A3C`)**, clear immediately or remain unchanged when branch conditions skip the update. Evaluate FLAGS f4/f10/f11 changes but **do not** infer that F5F55 branch ran from f4 alone: `F5C0C` first ORs `0x380` but `F5C71` later clears bit7 before `F5F4C` on the linear path; other branches/writes can also influence the final flags. Neither f3 nor flags are a proven visible attack, move or hit result.

For true per-branch evidence in the next stage, consider a carefully validated observer at `F5F4C` and, independently, `F58E0` bitmap reads (e.g. `EB7D9` call), **only after** interpreting Stage35 runtime. No reason to change AI gameplay semantics yet.

**Original EXE** already in persistent ChatGPT Library `/AALib.zip`; do not ask for reupload. [Static findings and limitations](../Stage35_Analysis/STATIC_ANALYSIS.md) · [SHA-pinned checker](../Stage35_Analysis/tools/verify_stage35.py).
