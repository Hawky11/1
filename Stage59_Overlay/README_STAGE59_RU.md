# Stage59 r2 — исправление ожидания COM-vs-COM объектов

**Stage59 r1 диагностика:** [коммит логов](https://github.com/Hawky11/1/commit/7eb07e6bff9392708720cd3ff76ac713703af1c9). DLL **1 152 512 байт** успешно собралась. Рабочий поток запустился **21:53:34**, но его 5-секундный таймер завершился **21:53:39**; первые подтверждённые Stage55-58 координаты и живые объекты появились **21:53:53–21:53:55**. R1 ни разу не установил аппаратные watchpoint’ы. В r2 ожидание увеличено до 600 секунд, раз в 15 секунд пишется `STAGE59_WAITING`, после получения объектов — `STAGE59_OBJECTS_READY`.

**R2 — исходники готовы. Новая сборка Windows и игровой runtime ещё не проверены.**

## Если Stage59 r1 уже установлен

**Замени только один файл:**

`D:\INCUBATOR\ArcanaAIOverhaul\src\Stage59XWriterWatch.inl`

Возьми новую версию из [Stage59 r2 / src / Stage59XWriterWatch.inl](./src/Stage59XWriterWatch.inl).

**Не заменяй** `src\Stage38CallbackWindow.inl` (тот же Stage59 r1 адаптер), Stage55/56/57/58, Build.bat и другие исходники.

Если Stage59 устанавливается с нуля на чистый Stage58, сначала выполни [первоначальную двухфайловую установку Stage59 r1](https://github.com/Hawky11/1/blob/stage59-dynamic-x-write-watchpoint/Stage59_Overlay/README_STAGE59_RU.md), а затем замени новый наблюдатель версией r2 из этого каталога.

## ArcanaAI.ini — без изменений

В существующей секции `[AIResearch]` должны быть:

```ini
[AIResearch]
EnableControlFieldTrace=0
EnablePlayerBit17WriteTrace=0
EnableStage59XWriterWatch=1
```

Отключение двух старых режимов обязательно: они используют те же DR0/DR1. Stage59 также не должен перезаписывать занятые чужими средствами регистры. DR2/DR3 остаются нетронутыми.

## Проверка

1. Сохрани текущую рабочую DLL r1 (**1 152 512 байт**) и предыдущую Stage58 DLL (**1 145 344 байта**) для отката.
2. Запусти `Build.bat`. При отсутствии `BUILD SUCCEEDED` **не устанавливай новую DLL**.
3. После запуска игры проведи короткий тест COM-vs-COM. Ожидание до появления персонажей происходит в отдельном потоке и не означает, что watchpoint уже активен.
4. В логе `ArcanaAI_Stage59XWriterWatchTrace.log` проверь `STAGE59_X_WRITER_WATCH_BUILD=2026-11-10-r2`, затем `STAGE59_OBJECTS_READY`, `STAGE59_WATCH_ARMED` и минимум один `STAGE59_THREAD_INSTALLED`. Только последние два маркера подтверждают попытку установки watchpoint. `STAGE59_X_WRITE` покажет реальные срабатывания, если они будут.
5. Проверь `STAGE59_HEALTH`: `dropped=0`, `readFail=0`; ненулевые `ownershipConflicts/installFailures` означают неполное покрытие. При вылете или сильном падении FPS сразу откати DLL.

После удачного теста залей **только шесть файлов из одного запуска**: `Build.log` и `ArcanaAI_Stage59XWriterWatchTrace.log`, `ArcanaAI_Stage58ACDF0ConditionalCallTrace.log`, `ArcanaAI_Stage57ACDF0HelperBoundaryTrace.log`, `ArcanaAI_Stage56ACDF0WriterTrace.log`, `ArcanaAI_Stage55EarlierMotionCoordinateTrace.log`. Если снова не стартует, достаточно Build.log + Stage59 log.

Предыдущие исходники и доказательные ветки оставлены без изменений. Stage59 — **только диагностика**, никаких доказанных улучшений ИИ пока нет.
