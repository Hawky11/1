# Stage98 r2 — исправление TID в пассивном наблюдателе F7A00 (SOURCE ONLY)

**Причина:** Stage98 r1 Windows MSVC x86 собрался успешно, `STAGE98_INSTALLED 1/1`, 49 чистых HEALTH; обработано 13 926 ROUTE. Однако `tid=0` во всех ROUTE: строка `e.tid=GetCurrentThreadId()` находилась после `//` комментария в `Stage98CommandRouteDecision.inl`. У Stage94/95 в том же запуске реальный TID `11956`. Поэтому исходный correlator, который требует одинаковый `player/TID/frame`, для r1 даёт фиктивные missing_outcome.

**Исправление:** Только одна строка исходника, отделена реальная запись `e.tid=GetCurrentThreadId();` на новую строку от комментария. **Нет изменений** native hook F7A00, инструкций `_emit`, восстановлений регистров/флагов, detour, установки, Stage81 loader, Stage94–97 или игрового состояния. Stage36 остаётся OFF. Оригинальный установленный Stage98 r1 rollback manifest обновляется на SHA r2, но поле `stage='Stage98 r1'` сохраняется для совместимости с исходным `RollbackStage98.ps1`.

## Установка поверх уже установленного Stage98 r1

Закрыть игру `AALib.exe` и сохранить проверенную Stage98 r1 DLL `build/dinput8.dll` (**2 051 584 байта**), а также Stage97 fallback DLL (**2 045 440 байт**). Распаковать ZIP в отдельный каталог и выполнить:

```powershell
powershell -ExecutionPolicy Bypass -File .\UpdateStage98r2.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Если `STAGE98_R2_SOURCE_UPDATED`, запустить обычный `Build.bat` из проекта. Затем сделать короткий COM-vs-COM запуск и проверить:

- `BUILD SUCCEEDED`, `STAGE98_INSTALLED 1/1`, Stage97 1/1, Stage96 5/5, Stage95 2/2, Stage94 2/2;
- в новых `STAGE98_ROUTE` **`tid` не равен 0** и соответствует Stage94/95 для того же игрока;
- `STAGE98_HEALTH dropped=0 mismatch=0 readFail=0`;
- повторить `tools/correlate_stage98.py` из **исходного Stage98 r1 пакета** с полными свежими Stage98/94/95 логами — теперь строгий join по `(player,tid,frame)` должен работать.

Если SHA/manifest fail, не форсировать: исходники могут отличаться, нужна диагностика. **Windows сборка r2/игра ещё не проверены.** Успех Stage98 r1 не доказывает улучшение поведения AI.

## Откат

Ревизия r2 имеет собственную копию исходного r1-файла и манифеста в `.stage98_r2_patch_backup`. При закрытой игре:

```powershell
powershell -ExecutionPolicy Bypass -File .\RollbackStage98r2.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Этот откат возвращает Stage98 r1 (с прежней ошибкой TID), после чего можно использовать старый `RollbackStage98.ps1` для возврата к Stage97; в обоих случаях пересобрать DLL или восстановить backup DLL. При необходимости `RollbackStage98.ps1` с сохранённым полем манифеста совместим и с r2, но лучше сначала использовать реверсивный откат r2.

## Корреляция результатов уже загруженного Stage98 r1

В одном запуске 13 926 Stage98 маршрутов / 1 894 Stage94 PRODUCER / 12 032 Stage95 BYPASS. Только **один** TID `11956` встречается в Stage94 и Stage95, и совпадают player/frame/QPC; поэтому в **этом конкретном запуске** можно восстановить join, исключив сломанное поле TID. 1 894 class1 -> PRODUCER, 11 335 class3 -> BYPASS, 697 class5 -> BYPASS. Все 13 926 события имеют уникальную следующую ветку; предсказания class1/class3 верны **13 229/13 229**. Class5 остаётся **неразрешённым по механизму**, хотя наблюдавшийся исход всегда BYPASS. Не использовать правило исключения TID в последующих запусках без доказательства единственности потока. Старый исторический случай Stage96 seq9461 нельзя сопоставлять по QPC между сеансами.

Статическая проверка r2: `python tools/test_stage98_r2.py` из каталога нового ZIP. Исходный Stage98 r1 статический тест 86/86 был выполнен с эталонным PE; r2 меняет только logger, native проверку не отменяет.
