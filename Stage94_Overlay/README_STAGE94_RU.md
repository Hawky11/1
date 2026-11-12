# Stage94 r1 — F3930 producer result → native type1 gate → real FINAL_COMMAND (SOURCE ONLY)

**СТАТУС:** исходники подготовлены и проверены офлайн. **Windows x86-сборки и запуска Stage94 пока не было.** Рабочий Stage93 DLL **2 019 840 байт** обязателен как резервная копия. Stage36 остаётся **OFF**.

## Зачем этот этап

Stage93 уже подтвердил прямой путь изменения `P+3B70 bit3` и 133/133 точных QPC-записей настоящего `FINAL_COMMAND`. Следующий вопрос: откуда взялось конкретное значение `ESI`, которое `F7C59` передаёт в команду.

Оригинальный EXE SHA256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`. Внутри исходной функции:

- `F7AD1 CALL F3930`, затем `F7AD6 ADD ESP,14h; MOV ESI,EAX` — **один** из путей получения команды;
- `F7B07 CMP ECX,1` → `F7B0A JNE F7C68` — условие прохождения ветки type1;
- `F7C59 PUSH ESI; PUSH 1; PUSH EBX` → существующий `F7C5D CALL F2DE0`, перехват FinalCommand уже существует и **не дублируется**.

Добавляются два точечных **пассивных диагностических перехвата** (с точной проверкой байт, откатом и воспроизведением нативных инструкций/условной ветки). **Никаких синтетических команд, записей игровых данных, DR watchpoints или Stage36.** Инструментация не предназначена для исправления поведения ИИ; она собирает доказательства происхождения конкретных команд.

## Установка на проверенный Stage93 r3

1. Закрой AALib.exe. Сохрани *работающий* `build\dinput8.dll` (2 019 840 байт) и папку исходников Stage93 r3.
2. Распакуй этот ZIP в отдельную папку. PowerShell из этой папки:

   ```powershell
   powershell -ExecutionPolicy Bypass -File .\ApplyStage94.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'
   ```

3. Нормальный результат: **`STAGE94 SOURCE INSTALLED`**. Если строгий SHA источников не совпал — **не обходить проверку**; пришли вывод PowerShell и изменённый файл.
4. Выполни обычный `Build.bat` (MSVC x86), проверь `BUILD SUCCEEDED`. В `ArcanaAI.ini` оставь `EnableStage36ControlToCommandTrace=0`. `EnableStage94ProducerGateTrace=1` по умолчанию (можно вписать явно в `[AIResearch]`).
5. Первый запуск: короткий COM-vs-COM бой. Ждём `STAGE93_INSTALLED 1/1`, затем **`STAGE94_INSTALLED 2/2`**, настоящие `STAGE94_GATE`, `STAGE93_FINAL_QPC` и `STAGE94_HEALTH` с `dropped=mismatch=readFail=0`. `STAGE94_PRODUCER` может не появиться, если этот путь не исполнялся.
6. Если чисто, 3–5 обычных раундов. Залей **Build.log**, `ArcanaAI_Stage94ProducerGateTrace.log`, `ArcanaAI_Stage93FinalCommandQpcTrace.log`, `ArcanaAI_FinalCommandTrace.log`, `ArcanaAI_Stage93DirectBit3Trace.log`, Stage92/Stage91 логи того же запуска и `ArcanaAI_EventDispatchTrace.log`.
7. Для отката при закрытой игре:

   ```powershell
   powershell -ExecutionPolicy Bypass -File .\RollbackStage94.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'
   ```

   После отката требуется **пересборка**. При нестабильности восстанови сохранённый рабочий Stage93 DLL.

## Логи и анализ

- `STAGE94_PRODUCER` — EAX после оригинального `F3930`, предыдущее ESI, actor/EBP/TID/QPC.
- `STAGE94_GATE` — ESI и настоящий ZF после `CMP ECX,1`, попадание в type1, кандидат на связь с предыдущим F3930.
- `STAGE94_HEALTH` — ошибки, пропуски, счётчики.
- Существующий `STAGE93_FINAL_QPC` — payload оригинальной финальной команды.

Коррелятор:

```powershell
python .\tools\correlate_stage94.py 'D:\INCUBATOR\ArcanaAIOverhaul\ArcanaAI_Stage94ProducerGateTrace.log' 'D:\INCUBATOR\ArcanaAIOverhaul\ArcanaAI_Stage93FinalCommandQpcTrace.log'
```

Укажи фактическое расположение файлов (они лежат рядом с запущенным dinput8.dll). QPC — **сырые тики, не секунды**. Близость событий и совпадение payload сами по себе не доказывают причинность: возможны другие писатели ESI, fallback-ветки и переиспользование кадра стека.

**Офлайн:** 38/38 exact source/PE checks PASS; 13/13 тестов, включая отрицательные QPC/actor/payload. Отчёт: `analysis/STATIC_ANALYSIS.md`. Не утверждаем, что Stage94 скомпилирован или проверен на игре, пока нет нового Build.log и реального `STAGE94_INSTALLED 2/2`.
