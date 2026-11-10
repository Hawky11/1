# Stage65 — подтверждение runtime и следующий шаг

**Статус:** Stage65 Windows x86 **BUILD SUCCEEDED + STAGE65_INSTALLED 1/1 + 429 живых записей**, проверено по залитым логам 11.11.2026 00:44–00:45. [Полный отчёт](RUNTIME_ANALYSIS.md).

## Что обнаружено

- **429/430** записей Stage40 строго сопоставлены со своими предыдущими вызовами общего setter E5BF0 (Stage65): тот же P1/P2, поток, QPC и исходный ID. Все 429 записей Stage65 имеют свою единственную пару в Stage40.
- **430/430** Stage40 совпадают с Stage41 по seq/QPC/ID. **316/318** Stage42 активаций совпадают с *последним* Stage40 ID; **315** из них одновременно имеют источник Stage65.
- **54** разных исходных native CALL-адреса действительно исполнились (из 296, выявленных статически).
- **114** запросов Stage40 без самостоятельной активации все пришли с одного CALL RVA **153ECA**, с ID 0x49 (96), 0x4A (12), 0x48 (6). Stage41 не поменяла флаги/ожидающий ID: это повторные запросы уже активного действия, а **не 114 потерянных команд**.
- **Один** Stage40 seq320: P1, ID0, нет соответствующей предшествующей Stage65. Альтернативная запись E9E30 возможна, но не подтверждена.
- **Две** активации Stage42 seq39/155: на входе Stage42 ожидается 0x25, тогда как последняя Stage41 видела 0x2F. Нужна проверка оригинальных записей O+64 между этими моментами.
- **61/61** Stage39 output передают выбранный native запрос до Stage40 и Stage42. Остальные **257** активаций не содержат Stage39 output в этом запуске, но 254 из них уже связаны через Stage65.

## Проверка стабильности

Build.log: успешная MSVC x86-сборка, DLL **1 879 552 байта** (прежнее предупреждение C4754). Stage65: 26 HEALTH без dropped/overflow/mismatch/readFail. Stage59: WATCH_ARMED, **18 783** записи X, 28 HEALTH, без dropped/readFail/ownershipConflicts/installFailures/objectDrift. Stage42/44/47/49/56/58 тоже без ошибок health, исследованных в рамках анализа.

## Воспроизведение на ПК — не обязательно

В ветке есть read-only [Python-коррелятор](tools/correlate_stage65.py), который требует **ровно один набор файлов от запуска 00:44**. Он не подключается к игре:

    py -3 .\Stage65_Runtime\tools\correlate_stage65.py --dir D:\INCUBATOR\ArcanaAIOverhaul --json .\Stage65_runtime_join.json

Файлы: ArcanaAI_Stage65RequestOriginTrace.log, ArcanaAI_Stage40ActionConsumerTrace.log, ArcanaAI_Stage41ObjectCommitTrace.log, ArcanaAI_Stage42ActionActivationTrace.log, ArcanaAI_Stage43ActivationOriginTrace.log, ArcanaAI_Stage44ActionDefinitionTrace.log и ArcanaAI_Stage39ActionOutputTrace.log. Новый Python-скрипт опубликован для воспроизведения, но **не заявляется как выполненный на твоём Windows ПК**: реальные цифры получены независимым анализом текстов GitHub.

## Дальше — Stage66

Продолжить **статически** поиск писателей ожидающего ID **O+64** (проблемная пара 0x2F→0x25) и источника единственного ID0 в P+3F4C, не возвращаясь к поиску координат X. Приоритет — проверенные реальные CALL RVA 153ECA, 133569, 1BD65A, 158691, 131CC5 и 131D1C.

**На текущем этапе ничего пересобирать и повторно загружать не надо.** Сохрани успешно протестированную Stage65 DLL 1 879 552 байта и прежнюю Stage59 r2 DLL 1 153 024 байта для отката. Новых изменений поведения COM в Stage65 нет.
