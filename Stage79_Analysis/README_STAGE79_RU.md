# Stage79 — одна общая точка наблюдения для 10 вычисляемых записей selector

**11 ноября 2026. SOURCE READY / оригинальный PE статически проверен. Windows x86 BUILD и игровой RUNTIME Stage79 ещё НЕ выполнены.**

## Что нашли

Десять оригинальных инструкций, записывающих в `P+3F50` значения **1/2**, находятся в четырёх функциях: `0F3930` (4), `0F4360` (2), `0F46F0` (2), `0F7150` (2). Все четыре входят в проверенное дерево прямых вызовов из `0E89E0`. Единственный найденный прямой `CALL 0E89E0` находится по RVA **`0EFFE8`**. В исходном `AALib.exe` подтверждены SHA256 и **11** ребер CALL, а также десять шестибайтовых MOV, все `SETNE/SETE` и прямые caller sets восьми функций. Полное описание — [STATIC_ANALYSIS.md](STATIC_ANALYSIS.md).

Вместо десяти опасных хуков Stage79 наблюдает **один** оригинальный `CALL 0EFFE8→0E89E0` до и после. Если здесь изменится selector, мы получим конкретный интервал вызова общего нативного семейства и игрока. Это **ещё не точный EIP отдельной MOV**.

## Установка — только два файла

1. Сохрани рабочую Stage78 `dinput8.dll` **1 911 808 байт** и текущий `src\Stage38CallbackWindow.inl`.
2. Замени в `D:\INCUBATOR\ArcanaAIOverhaul\src` файл `Stage38CallbackWindow.inl` на [Stage79 adapter](../Stage79_Overlay/src/Stage38CallbackWindow.inl).
3. Добавь рядом новый файл [Stage79SelectorFamilyBoundary.inl](../Stage79_Overlay/src/Stage79SelectorFamilyBoundary.inl).
4. **НЕ** заменяй `Stage78LiteralOneWriter.inl`, `Stage76AfterCallbackPhases.inl`, другие inl, `ArcanaAI.ini` и файлы игры. Уже существующие наблюдатели Stage38/59/65/70/76/77/78 сохраняются.

Дополнительная проверка исходного оригинального бинарника на скопированной ветке (ничего не изменяет):

```powershell
py -3 .\Stage79_Analysis\tools\verify_stage79.py "D:\PATH\TO\AALib.zip"
```

Запусти `Build.bat`. Требуется `BUILD SUCCEEDED`. Размер DLL Stage79 заранее неизвестен.

## После успешной сборки

Один обычный короткий **COM-vs-COM** бой/прогон. Сначала убедись в `Stage79SelectorFamilyTrace.log`:

- `STAGE79_GUARD ... stage38=1 stage76=3 stage78=1 stage59Started=0 nativeOK=1`;
- `STAGE79_INSTALLED 1/1 callRVA=0EFFE8 targetRVA=0E89E0 originalsOnce=1`;
- `STAGE79_COMMON_WINDOW` с `f0=old>new`, игроком P1/P2, `qpcBefore/qpcAfter` и чтением полных масок `1FFF/1FFF`;
- `STAGE79_HEALTH ... dropped=0 overflow=0 mismatch=0 readFail=0`.

При нормальном тесте также должны остаться `STAGE38_INSTALLED 2/2`, `STAGE77_ENABLED`, `STAGE76_INSTALLED 3/3`, `STAGE78_INSTALLED 1/1`, Stage59 `WATCH_ARMED`. Старая Stage78 DLL — откат, **не** удалять.

Залей с **одной игровой сессии** `Build.log`, `ArcanaAI_Stage79SelectorFamilyTrace.log`, `ArcanaAI_Stage38CallbackWindowTrace.log`, `ArcanaAI_Stage76AfterCallbackTrace.log`, `ArcanaAI_Stage78LiteralOneWriterTrace.log`. Stage59/Stage65/Stage70 добавь, если возникнут их ошибки или соответствующий deferred event.

## Как читать результат

`STAGE79_COMMON_WINDOW f0=00000000>00000002` доказывает: зафиксирован **эффект внутри выполнения `0E89E0`**, одного из четырёх потомков которого могло достичь выполнение. Но **не доказывает** EIP конкретной инструкции, поскольку функция может обращаться к другим помощникам, а MOV может вызываться условно. Нулевое изменение внутри окна не исключает временные записи, альтернативные пути, иную сессию или alias.

**При зависании/краше/изменении поведения, ошибках пар, чтения или переполнении** немедленно вернуть два Stage79 файла к Stage78 и предыдущую DLL. Не включать DR2/DR3 и не изменять боевую логику AI. Stage79 — диагностический эксперимент, не улучшение ИИ; корректность ABI всё ещё требует фактического Windows-прогона.