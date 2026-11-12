## Stage94 r2 diagnostic correction — actual live signature mismatch (2026-11-12)

User's Windows run of Stage94 r1 showed `STAGE94_BUILD=2026-11-12-r1`, followed by `STAGE94_ERROR native signature already modified/not exact; no patch`. Stage94 therefore **refused native patching safely**; there is no Stage94 2/2 installation evidence. Fresh original `AALib.exe` reverified: all seven code byte ranges and both relative call/JNE targets match exactly; the mismatch is **in live memory after previous hooks**, at an as-yet unknown RVA. Do not assume which prior stage owns it and **do not remove SignatureOK**.

The diagnostic-only r2 `Stage94Type1ProducerGate.inl` logs `STAGE94_SIGFAIL region=... rva=... expected=... actual=...`, `STAGE94_SIGFAIL_JMP` (when an existing E9 jump is observed, targetVA only; owner remains UNKNOWN), and any changed native relative call/jump target; no additional detours or gameplay writes. The source-only zip `ArcanaAI_Stage94_r2_RuntimeSignatureDiagnostic_SourceOnly.zip` contains `UpdateStage94Diagnostic.ps1`, revised observer and SHA manifest. Existing **Stage94 r1** installations should extract this r2 ZIP and execute **`powershell -ExecutionPolicy Bypass -File .\UpdateStage94Diagnostic.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'`**. This is a strict SHA-gated replacement of *only* Stage94 observer source and `installed_stage94_sha` in the existing rollback manifest. It does not rewrite Stage81/92/93. Backup tested Stage93 DLL (2,019,840 bytes), rebuild MSVC x86, run once, upload complete `ArcanaAI_Stage94ProducerGateTrace.log` and Build.log. Existing `RollbackStage94.ps1` remains functional after the update and fully restores Stage93 source if required. No bypass or force-patching until actual live bytes are diagnosed. **Offline** r2: 15/15 native-signature/integration/ZIP checks PASS; Windows r2 compile and live outcome pending.

**For new installs from current GitHub source:** `ApplyStage94.ps1` now expects r2 observer SHA `80f5d540387c2e0df30898a0cb32d60050a8917104e816e4a63bdd135bec772e`; the originally published r1 ZIP and its installer still expect the old SHA and should not be mixed with changed branch files.

---

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
