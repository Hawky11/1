# Stage78 — прямой observer инструкции selector=1

**Статус 11.11.2026: SOURCE READY / original PE STATIC VERIFIED. Windows x86 BUILD и runtime ещё НЕ проверены.**

## Цель

Stage77 [runtime](https://github.com/Hawky11/1/blob/stage77-runtime-selector-positive-callback-evidence/Stage77_Runtime/RUNTIME_ANALYSIS.md) показал P2 callback 1560B0, selector 0→1, state 1→3 и последующий event 0x21. По оригинальному PE это соответствует цепи 1560B9→155B70→155D74→155D29→0F2DE0, но точная инструкция ещё не наблюдалась непосредственно.

Stage78 инструментирует **ровно один native writer**: RVA 155D29, MOV DWORD PTR [ESI+3F50h],1 (10 байт). Точка установки проверяет родные байты и соседние инструкции, временно заменяет их на JMP rel32 + NOPs. В диагностической обёртке выполняется **этот же MOV ровно один раз**, затем JMP обратно к оригинальному коду RVA 155D33. Остальные игровые значения и debug registers не должны изменяться.

Файл ArcanaAI_Stage78LiteralOneWriterTrace.log включает STAGE78_GUARD, STAGE78_INSTALLED 1/1, записи STAGE78_LITERAL_ONE и STAGE78_HEALTH. В каждой записи: P1/P2, P/O/TID, native writer RVA 155D29, returnRVA из EBP+4 (ожидается 1560BE или 1566F5 на прямых путях), stackTop=21, QPC, pre/post mask, selector f0, state f1, object f2, callback f3, arg f4, event record f5.

## Установка (два файла)

1. Сохрани рабочую Stage77 dinput8.dll **1 905 664 байта** и старый src/Stage38CallbackWindow.inl.
2. Замени D:\INCUBATOR\ArcanaAIOverhaul\src\Stage38CallbackWindow.inl файлом [Stage78 adapter](../Stage78_Overlay/src/Stage38CallbackWindow.inl).
3. Добавь D:\INCUBATOR\ArcanaAIOverhaul\src\Stage78LiteralOneWriter.inl из [Stage78 writer](../Stage78_Overlay/src/Stage78LiteralOneWriter.inl).
4. Оставь текущий Stage76AfterCallbackPhases.inl, остальные src, INI и игровые файлы без изменений.

Для дополнительной статической проверки, из checkout этой ветки:

    py -3 .\Stage78_Analysis\tools\verify_stage78.py "D:\PATH\TO\AALib.zip"

Запусти Build.bat. Требуется **BUILD SUCCEEDED**. Размер Stage78 DLL неизвестен до сборки.

## Проверка в игре

Только после успешной сборки: один обычный COM-vs-COM запуск. Проверить Stage38 INSTALLED 2/2, Stage77 ENABLED, Stage76 INSTALLED 3/3, Stage70 INSTALLED 1/1, Stage78 GUARD nativeOK=1 и Stage78 INSTALLED 1/1. Stage59 должен продолжить WATCH_ARMED без изменений DR0/DR1. В логах Stage78 HEALTH overflow/mismatch/readFail/dropped=0.

Если исполнится native RVA 155D29, появится STAGE78_LITERAL_ONE с selector f0=old>1. **0→1 подтверждает фактический переход от данного сайта**, 1→1 лишь подтверждает его повторное исполнение. Отсутствие STAGE78_LITERAL_ONE не исключает другие десять register-derived writers (или alias).

Залей на GitHub **из одной сессии**: Build.log, ArcanaAI_Stage78LiteralOneWriterTrace.log, ArcanaAI_Stage38CallbackWindowTrace.log и ArcanaAI_Stage76AfterCallbackTrace.log. Stage65 и Stage70 нужны лишь при observed deferred request; Stage59 — при его аномалиях.

При краше, зависании, ошибках счётчиков или изменении поведения — откат обоих Stage78 файлов и DLL до сохранённой Stage77. Не включать DR2/3, не менять игровые параметры или AI.

**Ограничение:** это source-only инструмент, не готовая runtime-верифицированная DLL. Сохранение GPR/EFLAGS/x87/SSE запланировано, но не подтверждено Windows-прогоном; инструментирование добавляет нагрузку и риск гонок. Никаких улучшений AI/боевой логики не заявляем.