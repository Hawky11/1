# ArcanaAI — Stage97 r1: inherited ESI witness F77D4 (SOURCE ONLY)

**Этап готов как архив исходников. Сборка MSVC x86 и запуск игры НЕ проводились.**
Пакет создан по оригинальному AALib.exe из ChatGPT Library, SHA256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.

## Зачем

Stage96: 55/56 Stage95 bypass→Stage94 gate имеют точного native ESI-writer. Исключение: Stage95 seq9461 (P1, ESI=0x2C), тогда последний Stage96 F79F0 был 0x2B примерно 2,606,909 QPC тиков раньше. Нашли ещё один native источник: `F77D8 MOV ESI,[EBP-2C]`, где `[EBP-2C]` в обычном случае формируется F7756 из EBX + `[EBP-34]`. В оригинальном CFG есть путь без пяти Stage96 писателей до Stage95 F7A94. Stage97 проверяет его на новом запуске, а не объявляет старый эпизод доказанным.

## Что меняется

* Один сигнатурный перехват F77D4 (7 байт), с точным исполнением оригинальных CMP+MOV, возврат на нетронутый F77DB JNE F789E.
* `ArcanaAI_Stage97InheritedEsiTrace.log`: `STAGE97_INHERITED` с `localD4`, `nativeNextEsi`, контекстом P1/P2/TID/EBP/QPC и predictedJneTo789E.
* Проверка пяти установленных хуков Stage96 **по точным адресам переходов**, плюс fingerprint AALib и все локальные native сигнатуры. При несовпадении никакой Stage97 hook не ставится.
* Опционально `ArcanaAI.ini` `[AIResearch] EnableStage97InheritedEsiTrace=0` отключает Stage97 (по умолчанию включён).
* **Stage36 OFF**, никаких синтетических команд, патчей EXE на диске, дополнительных записей в gameplay/DR и новых хуков на F7A94/F7AD6/F7B0A/F7C5D.

## Установка

1. Закрыть `AALib.exe`; сохранить текущую **проверенную Stage96** DLL `build/dinput8.dll` (2,039,808 байт) как fallback.
2. Распаковать ZIP в отдельный каталог и выполнить в PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File .\ApplyStage97.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'
```

3. Должно появиться `STAGE97 SOURCE INSTALLED`. Если ошибка SHA или guard — ничего не форсировать; нужны текущие исходники.
4. Запустить привычный `Build.bat` в корне проекта; проверить `BUILD SUCCEEDED`.
5. Один короткий COM-vs-COM запуск. Ожидается `STAGE97_INSTALLED 1/1`, прежние `STAGE96_INSTALLED 5/5`, `STAGE95_INSTALLED 2/2`, `STAGE94_INSTALLED 2/2`, `STAGE93_INSTALLED 1/1`; `HEALTH dropped=0 mismatch=0 readFail=0`.
6. Для статистики в том же запуске дать игре несколько естественных раундов и загрузить в GitHub **полные свежие** `Build.log`, `ArcanaAI_Stage97InheritedEsiTrace.log`, Stage96/95/94/93 companion logs. Старые логи Stage96 не смешивать с новым Stage97 по QPC.

Сверка:

```powershell
python .\tools\correlate_stage97.py --stage97 .\ArcanaAI_Stage97InheritedEsiTrace.log --stage96 .\ArcanaAI_Stage96EsiWriterTrace.log --stage95 .\ArcanaAI_Stage95AlternateEsiTrace.log
```

## Откат исходников

Только при закрытой игре:

```powershell
powershell -ExecutionPolicy Bypass -File .\RollbackStage97.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'
```

Проверяется `.stage97_source_backup/manifest.json` и исходные SHA; после отката **пересобрать** DLL или восстановить сохранённую Stage96 DLL. Если DLL/игра нестабильны — сразу вернуть проверенный Stage96 билд, не ждать следующего этапа.

Локальные offline тесты: `python tools/test_stage97_offline.py /path/to/AALib.exe`. Это проверка статической модели и исходников, **не** Windows build, hook installation и не оценка gameplay AI.
