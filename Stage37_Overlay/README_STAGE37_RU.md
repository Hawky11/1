# Stage37 — установка read-only трассы type1 → controller bitmap / player-object state

**Статус:** исходники Stage37 готовы, 22 оригинальные сигнатуры + 7 CALL проверены на настоящем `AALib.exe`. **Новая Windows x86-сборка и работа Stage37 в игре пока НЕ проверены**.

Исходники: [Stage37_Overlay](https://github.com/Hawky11/1/tree/stage37-type1-controller-object-commit-trace/Stage37_Overlay). Обоснование и RVA: [Stage37 static analysis](../Stage37_Analysis/STATIC_ANALYSIS.md). Проверенный предыдущий этап: [Stage36 r2, 7/7 цепочек](../Stage36_R2_Runtime/RUNTIME_ANALYSIS.md).

## Замени только два файла в `D:\INCUBATOR\ArcanaAIOverhaul`

| Новый файл в GitHub | Путь на Windows |
|---|---|
| [src/AIQueueTrace.cpp](src/AIQueueTrace.cpp) | `D:\INCUBATOR\ArcanaAIOverhaul\src\AIQueueTrace.cpp` |
| [ArcanaAI.ini](ArcanaAI.ini) | `D:\INCUBATOR\ArcanaAIOverhaul\ArcanaAI.ini` |

**НЕ трогать:** `src\AITrace.cpp` (**оставить Stage29**), `src\Stage35ConsumerTrace.inl`, `src\Stage36ControlPredicate.inl` (оставить рабочие Stage36 r2), другие исходники и `Build.bat`. Если эти два `.inl` файла уже есть после успешного Stage36 r2, дополнительных копий не требуется.

`[AIResearch]` содержит новый ключ:

```ini
EnableStage37Type1CommitTrace=1
```

Также сохранены обязательные `EnableCpuEventDispatchTrace=1`, `EnableCpuEventDispatchDetailTrace=1` и рабочие прежние Stage35/36 опции. При их отключении Stage37 выведет явную ошибку конфигурации и не будет писать эффект-записи.

## Порядок прогона

1. **Архивируй предыдущие логи.** Не смешивай записи Stage36 r2 и нового Stage37 в одних файлах.
2. Обычный **x86 `Build.bat`**. При ошибке компиляции DLL не устанавливать, загрузить свежий `Build.log`. Старый warning C4754 в `AICommandSiteTrace.cpp:785` допускается.
3. Установи новый `build\dinput8.dll` вместе с новым `ArcanaAI.ini`. Естественный **COM-vs-COM**, никаких инъекций payload, переменных, битов/флагов, событий `0x1E`, кадров ввода и принудительных действий. Как обычно дай рабочему потоку не менее 2 секунд на сброс лога перед закрытием.
4. Залей в **GitHub main**: `Build.log`, **`ArcanaAI_Stage37Type1CommitTrace.log`**, `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage36ControlToCommandTrace.log`, `ArcanaAI_FinalCommandTrace.log`, `ArcanaAI_QueueTrace.log`, `ArcanaAI_ParentFlowTrace.log`, `ArcanaAI_Stage35ControllerConsumerTrace.log`, `ArcanaAI_Stage31AlternatePathTrace.log`, `ArcanaAI_Stage29Bit17Transitions.log` и остальные новые runtime-журналы.

## Ожидаемый новый журнал

`ArcanaAI_Stage37Type1CommitTrace.log`:

```text
STAGE37_TYPE1_COMMIT_BUILD=2026-11-10-r1
STAGE37_CONFIG uses existing EE200 EventDispatchHook; ...
STAGE37_ENABLED existing EE200 dispatch hook shared with Stage32/33/34; type1 capture scheduled.
STAGE37_TYPE1_EFFECT dispatchSeq=... qpcBefore=... qpcAfter=... side=P1 tid=...
    player=0x... callerRVA=0x... type=1 payload=0x...
    preMask=0x3FFFFFFF postMask=0x3FFFFFFF changedMask=0x... fields=0:...>...,1:...>...,...,29:...>...
```

Записи `STAGE37_TYPE1_EFFECT` физически лежат **каждая в одной строке**; перенос выше только для чтения. `dispatchSeq`, `qpcBefore`, `qpcAfter` и side точно совпадают с существующим `EVENT_DISPATCH_DETAIL` того же события, поскольку оба сообщения выходят из **одной и той же bounded ring record**. Для корректной диагностики событие должно быть `eventType=1`. Независимые маски чтения: полный набор из 30 полей — `0x3FFFFFFF`, а ноль в маске означает невозможность прочитать поле, **не нулевое состояние**.

Наблюдаемые изменения определяются только для полей, успешно прочитанных **и до, и после EE200**; это `changedMask`. После завершения функции могут быть изменения `P+3A0C/10`, `P+3F64`, `[P+4]+184`, флагов player/object и других raw DWORD. **Не трактовать разницу как готовую анимацию, удар или ошибку ИИ.**

## Что будем сопоставлять

Построим точное соответствие `Stage36 positive F4360 → Queue F7C62 (type1) → EE200 (type1) → Stage37 state changes` по **P1/P2, player pointer, tid, payload, QPC и dispatchSeq**. Сначала проверим, как часто каждый ранее выбранный код действительно обновляет контроллер и объект; отдельно проверим нулевые и неизменившиеся поля. Повторяющиеся ID и события с другими авторами должны считаться отдельно.

Если появится ошибка/отсутствие Stage37-строк, анализировать новый Build.log, маркер Stage37 и целостность `EventDispatchTrace`; **не отключать защиту и не форсировать игровые команды**.

На этом этапе новой установки хуков Stage37 **нет** — используется уже подтверждённый native EE200 trampoline. Для последующей проверки конкретной анимации/удара нужен другой, ещё не доказанный animation/action executor boundary.
