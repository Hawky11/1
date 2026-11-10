# Stage58 — установка и COM-vs-COM проверка

**Статус: исходники готовы; Windows x86 сборка и runtime ещё не проверялись.**

## Зачем Stage58

[Отчёт Stage57](https://github.com/Hawky11/1/blob/main/Stage57_Runtime/RUNTIME_ANALYSIS.md) точно разделил все 744 изменения X внутри `ACDF0`: 420 изменений внутри `15CEC0`, 327 перед этим вызовом, три окна содержат оба изменения. Из 327 изменений Stage56 объяснил 112, ещё **215 (P1 — 9, P2 — 206)** не прошли через наблюдаемую прямую запись `ACF2A`.

Stage58 наблюдает 3 условных вызова в этом промежутке: `ACF0F→B86A0`, `ACF7A→B86A0`, `ACF8A→E9A80`. Замеряет P1/P2 raw X до и после оригинального вызова, QPC, возвращаемый EAX, счётчики целостности. AI и координаты намеренно не изменяются. Это только диагностика.

## Установка — два файла

1. Сделай резервную копию **рабочей Stage57** `D:\INCUBATOR\ArcanaAIOverhaul\build\dinput8.dll` (**1 137 664 байта**).
2. **Замени** `D:\INCUBATOR\ArcanaAIOverhaul\src\Stage38CallbackWindow.inl` содержимым [Stage58 adapter](./src/Stage38CallbackWindow.inl).
3. **Добавь** `D:\INCUBATOR\ArcanaAIOverhaul\src\Stage58ACDF0ConditionalCalls.inl` из [Stage58 observer](./src/Stage58ACDF0ConditionalCalls.inl).
4. Остальные файлы, включая `Stage57ACDF0HelperBoundary.inl`, `Stage56ACDF0Writer.inl`, `Stage55EarlierMotionCoordinate.inl`, не трогай.
5. Выполни привычный `Build.bat`. Убедись, что в `Build.log` есть **BUILD SUCCEEDED**.

[Статический анализ и проверка сигнатур](../../Stage58_Analysis/STATIC_ANALYSIS.md) (не заменяет реальный Windows-тест).

## Проверка в игре

Запусти естественный тест **COM-vs-COM**. В `ArcanaAI_Stage58ACDF0ConditionalCallTrace.log` проверь:

- `STAGE58_INSTALLED 3/3 ACF0F->B86A0 ACF7A->B86A0 ACF8A->E9A80`;
- `STAGE58_CONDITIONAL_WINDOW` — реальные события с `callRVA`, `resultEax`, `qpcBefore/qpcAfter`, `f1=...` и `preMask=1FFF postMask=1FFF`;
- `STAGE58_HEALTH` — `missingObjects=0 dropped=0 overflow=0 mismatch=0 badCaller=0`;
- продолжают выводиться `STAGE57_INSTALLED 3/3`, Stage56 `2/2`, Stage55 `3/3`.

Если сборка не проходит, перехват не устанавливается, игра вылетает либо поведение существенно меняется — **прекрати тест и откати DLL Stage57**. При ошибке загрузить Build.log и Stage58 trace достаточно для первичной диагностики.

## Что загрузить после успешного теста

**Только пять файлов, из одного нового запуска:**

1. `Build.log`
2. `ArcanaAI_Stage58ACDF0ConditionalCallTrace.log`
3. `ArcanaAI_Stage57ACDF0HelperBoundaryTrace.log`
4. `ArcanaAI_Stage56ACDF0WriterTrace.log`
5. `ArcanaAI_Stage55EarlierMotionCoordinateTrace.log`

Остальные старые логи не нужны, если не возникнут ошибки. Не смешивай результаты с предыдущим запуском Stage57.

## Ожидаемая проверка

Мы сопоставим каждый вызов Stage58 с окнами Stage57/55 по **стороне, объекту, потоку и QPC** и выясним, изменяются ли X непосредственно во время B86A0/E9A80, особенно для **215 остаточных изменений**. Если эти вызовы не объяснят остаток, следующий этап будет искать изменения между ними. Сам факт изменения X не является доказательством улучшения ИИ.
