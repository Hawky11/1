# Stage36 r2 — исправлена совместимость с уже установленным Stage10 ParentFlow

**Текущая ветка для следующего x86 прогона:** [stage36-control-to-command-r2-stage10-compat](https://github.com/Hawky11/1/tree/stage36-control-to-command-r2-stage10-compat).

Предыдущий прогон r1 10.11.2026 в 00:55:42 **собрался успешно** (DLL 264192 байта), но **Stage36 не установился**. Причина: Stage10 ParentFlow уже перехватывает нативное начало функции `F4360` шестибайтовым JMP, а Stage36 r1 требовал неизменённый пролог оригинального EXE. Сохранён [полный разбор и оригинальные логи r1](../Stage36_FailedRuntime/RUNTIME_ANALYSIS.md). Не переустанавливать и не повторять r1.

## Для r2 достаточно заменить два файла

- [src/AIQueueTrace.cpp](src/AIQueueTrace.cpp) → `D:\INCUBATOR\ArcanaAIOverhaul\src\AIQueueTrace.cpp` (маркер сборки `r2`);
- [src/Stage36ControlPredicate.inl](src/Stage36ControlPredicate.inl) → `D:\INCUBATOR\ArcanaAIOverhaul\src\Stage36ControlPredicate.inl` (**основное исправление**).

`Stage35ConsumerTrace.inl` и `ArcanaAI.ini` **не изменились** относительно предыдущего Stage36 — оставь их установленными. `AITrace.cpp` строго оставь от Stage29. Никакие другие исходники, INI, `Build.bat` и игровые поля не изменяй.

Обычный x86 `Build.bat`, установка новой DLL/того же INI, предварительно архивируй старые логи, естественный COM-vs-COM и не менее двух секунд после последнего боя для flush. Загрузи свежие `Build.log`, **`ArcanaAI_Stage36ControlToCommandTrace.log`**, `ArcanaAI_ParentFlowTrace.log`, `ArcanaAI_QueueTrace.log`, `ArcanaAI_FinalCommandTrace.log`, `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage35ControllerConsumerTrace.log`, `ArcanaAI_Stage31AlternatePathTrace.log`, `ArcanaAI_Stage29Bit17Transitions.log`.

**Ожидаемые маркеры:**

```text
STAGE36_CONTROL_TO_COMMAND_BUILD=2026-11-10-r2
STAGE36_R2_ENTRY F4360=0x... bytes=... mode=STAGE10_OWN_DLL detourShape=... target=...
STAGE36_R2_INSTALLED EB718->F58E0 index=1F EB7D9->F58E0 index=1E F79E4->F4360 candidate; originals called once; 3/3 sites.
STAGE36_RESULT seq=... kind=INPUT_1F|INPUT_1E|CANDIDATE ...
```

Если код Stage10 установлен иначе, чем позволяют проверенные варианты JMP, вместо установки r2 запишет `mode=UNKNOWN` и точные шесть байт. **При таком результате не отключай валидацию**: этот журнал позволит определить точный формат перехвата для следующего безопасного исправления. Marker `STAGE36_R2_INSTALLED` и `STAGE36_RESULT` без нового Windows прогона **не подтверждены**.

---

# Stage36 — установка наблюдателя от controller bitmap к предлагаемой игровой команде

**Статус: Stage36 source-only.** Проверка оригинального `AALib.exe` — PASS (22 точные сигнатуры и 8 прямых CALL), но компиляция MSVC x86, установка хуков и реальный COM-vs-COM для этой ветки **ещё не подтверждены**.

## Скопировать четыре файла

Исходники: [Stage36_Overlay](https://github.com/Hawky11/1/tree/stage36-control-to-command-trace/Stage36_Overlay).

| GitHub | Куда на компьютере |
|---|---|
| [src/AIQueueTrace.cpp](src/AIQueueTrace.cpp) | `D:\INCUBATOR\ArcanaAIOverhaul\src\AIQueueTrace.cpp` — заменить |
| [src/Stage35ConsumerTrace.inl](src/Stage35ConsumerTrace.inl) | `D:\INCUBATOR\ArcanaAIOverhaul\src\Stage35ConsumerTrace.inl` — сохранить или скопировать тот же проверенный Stage35 |
| [src/Stage36ControlPredicate.inl](src/Stage36ControlPredicate.inl) | `D:\INCUBATOR\ArcanaAIOverhaul\src\Stage36ControlPredicate.inl` — **добавить новый обязательный файл** |
| [ArcanaAI.ini](ArcanaAI.ini) | `D:\INCUBATOR\ArcanaAIOverhaul\ArcanaAI.ini` — заменить |

**`src\AITrace.cpp` ОСТАВИТЬ от Stage29**, остальные файлы и `Build.bat` не менять. Исходная Stage35 ветка и архив runtime сохранены отдельно; Stage36 включает оба прежних Stage35 наблюдателя.

1. Сделай копию/архив прежних логов.
2. Обычный **x86 `Build.bat`**; если есть compile/link errors — не устанавливай DLL, загрузи `Build.log`. Предыдущее известное нефатальное warning `C4754` в `AICommandSiteTrace.cpp:785` допускается, но новые ошибки сборки — нет.
3. Установи `dinput8.dll` и `ArcanaAI.ini` как обычно. Запусти **естественный COM-vs-COM**, не форсируй type1/0x1E, payload5, bit17, целевые флаги/битовые поля или игровые действия. Дай worker минимум 2 секунды для flush перед закрытием.
4. Загрузи **`Build.log` и все новые runtime логи**, особенно `ArcanaAI_Stage36ControlToCommandTrace.log`, `ArcanaAI_Stage35ControllerConsumerTrace.log`, `ArcanaAI_FinalCommandTrace.log`, `ArcanaAI_QueueTrace.log`, `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage31AlternatePathTrace.log`, `ArcanaAI_Stage29Bit17Transitions.log`.

## Что должно появиться

Новый ключ `[AIResearch] EnableStage36ControlToCommandTrace=1`.

В `ArcanaAI_Stage36ControlToCommandTrace.log`:

```text
STAGE36_CONTROL_TO_COMMAND_BUILD=2026-11-10-r1
STAGE36_CONFIG EB718 INPUT_1F, EB7D9 INPUT_1E, F79E4 CANDIDATE ...
STAGE36_INSTALLED EB718->F58E0 index=1F EB7D9->F58E0 index=1E F79E4->F4360 candidate; originals called once; 3/3 sites.
STAGE36_RESULT seq=... kind=INPUT_1F side=P... player=0x... tid=... qpcBefore=... qpcAfter=... args=(...) argMask=0x03 nativeEAX=... nativeAL=... preMask=0x3FF postMask=0x3FF ...
STAGE36_RESULT seq=... kind=INPUT_1E side=P... player=0x... tid=... qpcBefore=... qpcAfter=... args=(...) argMask=0x03 nativeEAX=... nativeAL=... preMask=0x3FF postMask=0x3FF ...
STAGE36_RESULT seq=... kind=CANDIDATE side=P... player=0x... tid=... qpcBefore=... qpcAfter=... args=(...) argMask=0x1F nativeEAX=... outMask=0x3>0x3 outputs=...>...,...>... ...
```

`INPUT_1F` и `INPUT_1E` — только при естественном достижении native `EB718`/`EB7D9`. В зависимости от условий игры один из них может не встретиться. `CANDIDATE` — исходный фильтр `F4360` на `F79E4`; это **сырой кандидат**, а не гарантированный итоговый payload. При установке всех трёх хуков в журнале будет явный `STAGE36_INSTALLED ... 3/3`.

Поля `f0..f9`:

| Index | Read-only snapshot |
|---:|---|
| 0 | `P+3A08` controller id |
| 1 | `P+3A0C` first bitmap |
| 2 | `P+3A10` second bitmap |
| 3 | `P+3A6C` timer slot11 |
| 4 | `P+3B78` derived flags |
| 5 | `P+3B70` other flags |
| 6 | `P+3B84` bit17/18 lifecycle |
| 7 | `P+3F64` last dispatcher output |
| 8 | `O+08`, `O=[P+4]` |
| 9 | `O+24`, `O=[P+4]` |

`preMask/postMask`: 10-bit read success; `0x3FF` — все поля прочитаны; биты ноль означают отсутствие успешного чтения, а не нулевое значение. Для `CANDIDATE` `outMask` отдельно указывает успех чтений двух output DWORD до/после. `nativeEAX` — **сырое возвращённое значение**, `nativeAL` — младший байт; для bitmap function AL=0/1, для кандидата не трактовать AL как успешное действие.

## Как анализировать Stage36

- Соедини Stage35 `TIMER/FLAGS`, Stage36 `INPUT_1E/1F` и `CANDIDATE` по точному P/TID и порядку QPC; различай самостоятельные функции/ветки, не придумывай общий parent episode.
- Из уже включённого `ArcanaAI_QueueTrace.log` выдели `QUEUE_EVENT_DETAIL callerRVA=0x000F7C62 eventType=1`: это фактическая **попытка отправки выбранного payload через `F7C5D→F2DE0`**, с QPC, side/player. `FINAL_COMMAND` дополнительно показывает исходный `payloadESI`, P1/P2 и состояние флагов. Не утверждай прямую причинность без однозначной корреляции.
- Найди последующий `EVENT_DISPATCH_DETAIL eventType=1` с тем же payload. Для подтверждения animation/attack/movement нужен **другой native gameplay/output boundary**; queue/EE200/bitmap сами по себе не доказывают видимое действие.

[Статическое исследование](../Stage36_Analysis/STATIC_ANALYSIS.md) · [SHA-pinned verifier](../Stage36_Analysis/tools/verify_stage36.py). Эталонный EXE уже есть в ChatGPT Library `/AALib.zip`, снова просить его не нужно.
