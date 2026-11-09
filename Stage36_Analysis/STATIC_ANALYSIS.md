# Stage36 — от нативных управляющих предикатов к выдаче конкретной команды

**Дата:** 2026-11-10. **Статус:** статическая проверка оригинального EXE **PASS**; пассивный Stage36 x86 source overlay подготовлен. **Windows MSVC сборка и реальный runtime Stage36 ещё не проводились**. Не считать результаты Stage35 доказательством анимации, успешного удара или бага ИИ.

## Проверенная исходная база

Stage35 завершён: [отчёт](../../Stage35_Runtime/RUNTIME_ANALYSIS.md), evidence branch `stage35-controller-consumer-runtime-evidence` @ `091fa2692b33b2e97d19406a330139796e4fa7af`. Прежняя Stage35 source-only ветка `stage35-controller-consumer-trace` @ `3ce68f5a5c49fbda11ffd6ad514ea0c229a790e3` остаётся неизменной.

Оригинальный файл получен **read-only** из ChatGPT Library `/AALib.zip`, SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` (175,556,608 bytes, i386 PE32, preferred ImageBase `0x400000`). `verify_stage36.py` проверяет хеш, PE section mapping, **22 точные сигнатуры** и **8 прямых E8 CALL targets**. Проверка тех же байтов на оригинальном бинарнике: **PASS**. Адреса функций/инструкций ниже — **RVA**, не VA и не file offsets. EXE и его полный дизассемблерный листинг не публикуются.

## 1. Фактические читатели управляющих DWORD

Две точные оригинальные команды вызывают одну и ту же `F58E0` с указателем `P+3A08` и разными индексами:

```asm
EB70C   lea  eax,[edi+3A08h]   ; EDI = player P
EB712   push 1Fh
EB714   push eax
EB718   call F58E0              ; AL = bitmap predicate for index 0x1F
EB71D   add  esp,8
EB720   test al,al
EB722   je   EBAD1              ; skip path on false

EB7D0   lea  eax,[edi+3A08h]
EB7D6   push 1Eh
EB7D8   push eax
EB7D9   call F58E0              ; AL = bitmap predicate for index 0x1E
EB7DE   add  esp,8
EB7E1   test al,al
EB7E3   je   EB7F0              ; alternate path on false
EB7E5   mov  ecx,[edi]
EB7E7   call E5F00              ; additional predicate if AL from F58E0 != 0
```

`F58E0` — **bitmap predicate**, не выдача удара: для общего индекса проверяет `[P+3A08 + 4 + (index/32)*4]` и возвращает AL=0/1. Для индекса `0x26` есть особое правило масок `0x200/0x400/0x800` в первом bitmap DWORD. Ветви после `EB718/EB7D9` действительно используют результат теста; значение 1 значит установленный управляющий бит, **не «выполнено действие»**. `EB7D9` вызывается только при прохождении других предикатов (`EB7BE..EB7CE`), поэтому возможен естественный прогон без этого вызова.

## 2. Выбор кандидата и граница выдачи команды

В другом нативном пути (`F79xx`) ведётся выбор/фильтрация идентификатора:

```asm
F79CF   call F3280              ; EAX -> ESI (candidate)
F79D9   lea  eax,[ebp-4]        ; output pointer #2
F79DC   push eax
F79DD   lea  ecx,[ebp-8]        ; output pointer #1
F79E0   push ecx
F79E1   push esi                ; candidate incoming
F79E2   push ebx                ; input selector
F79E3   push edi                ; player P
F79E4   call F4360              ; original five-arg filter
F79E9   add  esp,14h
F79F0   mov  esi,eax            ; raw candidate result
...
F7C59   push esi                ; eventual event payload (may be changed later)
F7C5A   push 1                  ; event type 1
F7C5C   push ebx                ; queue=P+3A2C (EBX prepared earlier)
F7C5D   call F2DE0              ; proposed command reaches native event queue
```

Функция `F4360` содержит собственные проверки (`F43B1→F2FD0`, selection tables and guards) и может вернуть sentinel `0xFFFFFFFF`, другие коды либо изменить output-pointer DWORD. Между `F79E4` и `F7C5D` существуют **дополнительные ветки и пересчёты кандидата** (например, `F3930`, `F34D0`). Поэтому **результат F4360 нельзя автоматически приравнивать к последующему payload**. Не установлена также общая функция-предок между `EB7D9` и `F79E4`: их события нельзя считать одной причинной цепью по одному совпадению времени.

`F7C5D` — конкретная **граница предложения type1-команды в очередь**, уже наблюдаемая проверенным `FinalCommandTrace` и `QUEUE_EVENT_DETAIL callerRVA=F7C62`, а не точка гарантированного удара/анимации. Позже `EE200` type1 вызывает `12EAC0` (`EE273`), `158EC0` и `F57D0` (`EE306`), что приводит к контроллерным записям. Это гарантирует передачу состояния в нативный обработчик только при наблюдаемой диспетчеризации, но не результат на экране.

## 3. Минимальный passive overlay Stage36

Основа — **точно работавший Stage35**, без изменения его Stage29 `AITrace.cpp` и Stage34/35 точек наблюдения.

- Три **точных проверяемых 5-byte CALL**: `EB718→F58E0` (`INPUT_1F`), `EB7D9→F58E0` (`INPUT_1E`), `F79E4→F4360` (`CANDIDATE`). Сверяются original bytes, prefix/suffix, native prologue, относительные E8 targets; все три устанавливаются только вместе под общим mutex с приостановкой других потоков. Нет gameplay writes или синтеза событий. Сохранение GPR/EFLAGS и x87/MMX/XMM/MXCSR как в Stage35.
- Исходные cdecl аргументы replay **ровно один раз** (два для `F58E0`, пять для `F4360`) и оригинальный native EAX/AL восстанавливается после диагностических вызовов. `CANDIDATE` дополнительно снимет сырые output pointer DWORD до/после `F4360`, с отдельными masks.
- Лог `ArcanaAI_Stage36ControlToCommandTrace.log`, кольцо 2048, depth8, worker-only file I/O; `STAGE36_RESULT` имеет собственный `seq`, фазу/вид, P1/P2, TID, QPC до/после, raw args и маску, `nativeEAX`/`nativeAL`, 10 raw player/object DWORDs и маски, outputs для `F4360`. `preMask/postMask=0x3FF` означает успешные 10/10 чтений; иначе ноль — placeholder. Для CANDIDATE `outMask=0x3>0x3` значит оба output DWORD доступны до и после.
- Не изменяется `AICommandSiteTrace.cpp`: уже существующий `FINAL_COMMAND` (с `tick`) и `QUEUE_EVENT_DETAIL` (с точным `QPC`, `callerRVA=F7C62`, `type=1`, `payload`) позволяют проверять, **появилось ли после кандидата конкретное предложение команды**. Корреляция по side/P/TID/QPC, а не недоказанному прямому parent-ID; считать количество несовпадений и других событий.

## 4. Проверка реального прогона

1. Убедиться в успешном x86 `Build.bat`, маркере `STAGE36_CONTROL_TO_COMMAND_BUILD=2026-11-10-r1`, записи `STAGE36_INSTALLED ... 3/3 sites`, сохранении Stage35 `STAGE35_INSTALLED` и Stage29 наблюдателя.
2. Проверить `seq`, QPC неотрицательные интервалы, AL=0/1 для обычных `F58E0`, точность указателей P1/P2, маски, отсутствие dropped/depthOverflow; отклонить обрезанный/смешанный журнал.
3. Сравнить `INPUT_1E/1F` после соответствующих EE200 output bitmap values с собственным `nativeAL`. Для `INPUT_1E` проверить, что native branch действительно использовал AL на `EB7E1`, но не приписывать результат последующему `E5F00` без его наблюдения.
4. Сравнить `CANDIDATE` nativeEAX и выходные DWORD с ближайшим последующим `QUEUE_EVENT_DETAIL` из `F7C62` для того же P1/P2, TID; сохранить случаи расхождения/дополнительных фильтров. Затем сопоставить type1 `EE200` с таким же payload, исключая промежуточные изменения очереди.
5. Не пытаться вывести анимацию/урон из ID или bitmap; если нужен следующий этап, искать **фактическую animation/attack/object-commit функцию**, и только затем делать изолированный passive observer.

**Stage36 пока source-only.** Для реального build и natural runtime нужны четыре Stage36 Overlay файла, текущий Stage29 AITrace.cpp и старые проверенные остальные исходники. См. [README_STAGE36_RU.md](../Stage36_Overlay/README_STAGE36_RU.md).

## Runtime update: Stage36 r1 failed closed because Stage10 already hooked F4360 (2026-11-10)

The user successfully built x86 Stage36 r1 (`dinput8.dll` 264192 bytes), but the instrumentation was NOT installed. At 00:55:43, the Stage10 ParentFlow log specifically reports `Installed Stage 10 all-caller helper hook F4360 at RVA=0x000F4360 (patch=6 bytes)`. The following Stage36 hook installer demanded the original F4360 10-byte prologue in process memory; it could not still be present after the preexisting Stage10 hook. The original SHA-verified EXE prologue and three CALL-site instructions nevertheless match the static signatures. The failure is therefore a previously missed **inter-observer hook conflict**, not evidence of an incorrect native reverse-engineering RVA.

Full report: [Stage36_FailedRuntime/RUNTIME_ANALYSIS.md](../Stage36_FailedRuntime/RUNTIME_ANALYSIS.md). 24 exact uploaded log/build Git blobs preserved under `Stage36_FailedRuntime/`. Existing Stage10 captured 66 F4360 events and Stage35 captured 13172 complete consumer events, proving old observers continued to work.

The r2 Stage36 overlay (new branch `stage36-control-to-command-r2-stage10-compat`) verifies all three original caller CALL signatures, prefixes/suffixes, E8 targets, and native F58E0 entry. For F4360 only, the validator allows original prologue OR a previously applied 6-byte unconditional jump (`E9 rel32 + 90`, `FF25 absolute indirect`, or `68 imm32; C3`), with resolved destination strictly in executable memory belonging to our own `dinput8.dll` loaded module. In process it calls the already-hooked F4360, which then chains to native via Stage10. Unknown detours refuse installation. r2 prints `STAGE36_R2_ENTRY` and exact callsite mismatch diagnostics; source-only and Windows MSVC runtime unverified until new user logs. **Do not claim that Stage36 r1 recorded native predicate/candidate results.**

One new gameplay-state finding from failed r1's successful older observers: six naturally occurring `EE200 type0/payload5` events, three P1 and three P2, wrote/reloaded slot11 `P+3A6C` (0→F00, F00→F00, E88→F00 per side) with complete masks. This proves a controller timer state transition for payload5, NOT an action/hit. [Exact six-row CSV](../Stage36_FailedRuntime/type0_payload5_first_natural_evidence.csv).

