# ArcanaAI — Stage98 r1: Command Route Predicates F7A00 (SOURCE ONLY)

**Подготовлен только исходный код, Windows MSVC x86 и игру здесь не запускал.** Последняя реально проверенная сборка — Stage97 r1 `dinput8.dll` 2 045 440 байт. Перед тестом сохранить её.

## Цель

Stage97 закрыл наблюдаемый пробел в происхождении ESI: 61/61 finite bypass с точным последним writer. Stage98 исследует следующий неизвестный вопрос: **какие native-предикаты направляют управление на F3930 (producer) или F7A94 (bypass)?** Оригинальный AALib.exe SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba` содержит на F7A00 общую развилку. Подробности в `analysis/STATIC_ANALYSIS.md`.

На F7A00 — **один пассивный hook** и точный replay оригинального `MOV EAX,[EDI+3B78]`, затем оригинальный `F7A06`. Логируем снимки битов 8/15/17 `[EDI+3B78]`, бит18 `[EDI+3B70]`, `[EDI+4]+0x60` (статус действия), `[EDI+3A34]` и ESI. Существующие Stage94/95 логи укажут **реальный** маршрут. Классы 1–4 — локальные предсказания ранних ветвей, класс 5 оставляет дальнейшую функцию недоказанной.

Stage36 OFF. Никаких synthetic commands, gameplay/DR writes, повторного F7A94/F7AD6/F7B0A/F7C5D/EE306 hook.

## Установка Stage98 поверх Stage97

1. Закрыть AALib.exe; сохранить работающий Stage97 `build/dinput8.dll` (2 045 440 B) и рабочие исходники.
2. Распаковать этот ZIP в отдельную папку, в PowerShell из папки архива:

```powershell
powershell -ExecutionPolicy Bypass -File .\ApplyStage98.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'
```

3. Получить `STAGE98 SOURCE INSTALLED`; при SHA/manifest/owner отказе **не форсировать** — прежний Stage97 остаётся рабочей точкой.
4. В `D:\INCUBATOR\ArcanaAIOverhaul` выполнить `Build.bat`, убедиться в `BUILD SUCCEEDED` и сохранить `Build.log`.
5. Короткий COM-vs-COM запуск: `STAGE98_INSTALLED 1/1`, `STAGE97_INSTALLED 1/1`, `STAGE96_INSTALLED 5/5`, `STAGE95_INSTALLED 2/2`, `STAGE94_INSTALLED 2/2` и HEALTH без `dropped/mismatch/readFail`. Если ошибка/вылет — вернуться на Stage97.
6. После нескольких естественных раундов залить свежие полные логи Stage98, Stage97, Stage96, Stage95, Stage94, Stage93 Final QPC и Build.log **из одного запуска**.

Локальная корреляция:

```powershell
python .\tools\correlate_stage98.py --stage98 .\ArcanaAI_Stage98CommandRouteTrace.log --stage94 .\ArcanaAI_Stage94ProducerGateTrace.log --stage95 .\ArcanaAI_Stage95AlternateEsiTrace.log
```

## Откат Stage98 → Stage97

При закрытой игре:

```powershell
powershell -ExecutionPolicy Bypass -File .\RollbackStage98.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Откат проверяет SHA и восстанавливает **только** Stage97 loader, удаляет новый Stage98 inl и свой backup, предыдущие стадии не трогает. Затем заново собрать DLL или восстановить сохранённый Stage97 DLL.

Offline source+PE+synthetic тест: `python tools/test_stage98_offline.py /path/to/AALib.exe`. Не заменяет сборку/тест в Windows.
