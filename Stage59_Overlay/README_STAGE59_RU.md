# Stage59 r2 — исправление раннего тайм-аута ожидания объектов

**Причина r1 подтверждена:** 10.11.2026 Stage59 стартовал **21:53:34**, завершил 5-секундное ожидание **21:53:39**, а первые живые Stage55 P1/P2 object snapshots появились **21:53:53–21:53:54**. Поэтому r1 не установил ни одного DR0/DR1 watchpoint; отсутствие X_WRITE записей **не является** доказательством отсутствия записей X.

В r2 время ожидания живых объектов увеличено с **5 до 600 секунд**, с записями `STAGE59_WAITING` каждые ~15 секунд и `STAGE59_OBJECTS_READY` после обнаружения двух читаемых объектов и их X DWORD. Все прочие диагностики и адреса неизменны. Это **source ready, Windows x86 compile/runtime ещё не проверены**.

## Установка поверх уже установленного Stage59 r1

**Замени ровно ОДИН файл:** `D:\INCUBATOR\ArcanaAIOverhaul\src\Stage59XWriterWatch.inl` на [исправленный Stage59 r2](./src/Stage59XWriterWatch.inl) из ветки `stage59-r2-wait-for-combat-objects`.

НЕ меняй существующий Stage59 r1 `src\Stage38CallbackWindow.inl`, Stage55–58, другие исходники или Build.bat. Оставь `ArcanaAI.ini` как для Stage59:

```ini
[AIResearch]
EnableControlFieldTrace=0
EnablePlayerBit17WriteTrace=0
EnableStage59XWriterWatch=1
```

Сначала сохрани работающую r1 DLL (**1 152 512 байт**, сборка 21:53:04) и предыдущую Stage58 DLL (**1 145 344 байта**) для отката. Выполни `Build.bat`. Если сборка неуспешна, не устанавливай DLL. После успешной сборки запусти игру и COM-vs-COM, дождись появления персонажей. Проверяй `STAGE59_WATCH_ARMED` и минимум один `STAGE59_THREAD_INSTALLED`; запись `STAGE59_OBJECTS_READY` — только готовность объектов, **ещё не доказательство установки watchpoint**. Если `STAGE59_HEALTH` показывает ошибки или есть просадка FPS/вылет — остановись и откатись.

Для следующего анализа залей те же **шесть файлов одного запуска**: `Build.log`, `ArcanaAI_Stage59XWriterWatchTrace.log`, `ArcanaAI_Stage58ACDF0ConditionalCallTrace.log`, `ArcanaAI_Stage57ACDF0HelperBoundaryTrace.log`, `ArcanaAI_Stage56ACDF0WriterTrace.log`, `ArcanaAI_Stage55EarlierMotionCoordinateTrace.log`. Если Stage59 снова не стартует, хватит Build.log + Stage59 trace.

---

# Stage59 — как установить и какие логи загрузить

**Текущий статус: только исходники. Windows x86 BUILD и реальный COM-vs-COM runtime Stage59 ещё не проверены.**

Цель — определить реальную инструкцию, которая записывает X в \`[player+4]+0x7C\`. Stage59 не меняет алгоритм ИИ и не должен намеренно менять координаты.

## 1. Резервная копия и два файла

Рабочая DLL Stage58: **1 145 344 байта**. Сохрани её отдельно перед любой заменой.

Из ветки [stage59-dynamic-x-write-watchpoint](https://github.com/Hawky11/1/tree/stage59-dynamic-x-write-watchpoint):

1. **Замени** \`D:\INCUBATOR\ArcanaAIOverhaul\src\Stage38CallbackWindow.inl\` файлом [Stage59 adapter](./src/Stage38CallbackWindow.inl).
2. **Добавь** \`D:\INCUBATOR\ArcanaAIOverhaul\src\Stage59XWriterWatch.inl\` из [нового наблюдателя](./src/Stage59XWriterWatch.inl).

Остальные файлы **не трогай**: Stage58/57/56/55 и все предыдущие исходники, \`Build.bat\`.

## 2. Настройка ArcanaAI.ini — ОБЯЗАТЕЛЬНО

В секции \`[AIResearch]\` выстави:

\`\`\`ini
[AIResearch]
EnableControlFieldTrace=0
EnablePlayerBit17WriteTrace=0
EnableStage59XWriterWatch=1
\`\`\`

Эти строки нужно **установить в существующей секции**, а не создать вторую \`[AIResearch]\`. Все остальные настройки оставь как есть.

Почему: Stage16 использует аппаратные DR0/DR1. Stage59 намеренно **откажется запускать watchpoint**, пока включён хотя бы один режим Stage16. Чужие занятые DR0/DR1 тоже не перезаписываются; DR2/DR3 сохраняются.

## 3. Сборка и тест

По желанию проверь статические условия на оригинальном **приватном** \`AALib.exe\`:

\`\`\`powershell
py -3 Stage59_Analysis\tools\verify_stage59.py --aalib "D:\путь\к\AALib.exe" --repo-root "D:\путь\к\checkout-stage59"
\`\`\`

Это проверяет исходники и бинарь, **но не заменяет сборку в Windows**.

Запусти \`Build.bat\`. Если **BUILD SUCCEEDED** отсутствует — **не ставь** новую DLL. После успешной сборки установи её и проведи обычный непродолжительный COM-vs-COM тест (30–60 секунд); дай логам закончить запись.

В новом файле \`ArcanaAI_Stage59XWriterWatchTrace.log\` должны появиться:

- \`STAGE59_X_WRITER_WATCH_BUILD=2026-11-10-r1\` и \`STAGE59_WORKER_STARTED\`;
- \`STAGE59_WATCH_ARMED\` с реальными адресами P1/P2;
- минимум один \`STAGE59_THREAD_INSTALLED\` — без него **нет подтверждения, что watchpoint действительно работал**;
- \`STAGE59_X_WRITE\` — события записи, если они случились;
- \`STAGE59_HEALTH\` с \`dropped=0\`, \`readFail=0\`, желательно \`ownershipConflicts=0\` и \`installFailures=0\`.

Важные нюансы: EIP/RVA в логе — адрес **после** записи; \`oldShadow\` — только приблизительная предыдущая величина. Возможны пропущенные записи из недоступных для отслеживания потоков. Если есть вылет, сильная просадка FPS или конфликт аппаратных регистров — прекрати проверку и верни сохранённую DLL Stage58.

## 4. После удачного теста загрузи ровно шесть файлов из ОДНОГО запуска

1. \`Build.log\`
2. \`ArcanaAI_Stage59XWriterWatchTrace.log\`
3. \`ArcanaAI_Stage58ACDF0ConditionalCallTrace.log\`
4. \`ArcanaAI_Stage57ACDF0HelperBoundaryTrace.log\`
5. \`ArcanaAI_Stage56ACDF0WriterTrace.log\`
6. \`ArcanaAI_Stage55EarlierMotionCoordinateTrace.log\`

Старые логи, остальные стадии и сам оригинальный AALib.exe загружать **не нужно**. Если установка не удалась, для первичной диагностики достаточно Build.log и Stage59 trace.

После проверки сопоставим точные QPC событий записи с окнами Stage55–58 и определим, что именно записывает координату и на каком потоке. Это ещё не утверждение об улучшении ИИ.
