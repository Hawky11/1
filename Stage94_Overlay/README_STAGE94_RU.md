## Stage94 r3 — exact Stage82 F7AD1 call-chain compatibility (2026-11-12)

**RUNTIME STILL PENDING:** Stage94 r2 printed `STAGE94_SIGFAIL region=F3930_CALL rva=0F7AD1 expected=E85ABEFFFF actual=E87A679B50 gameBase=00100000` and refused installation. **Resolved owner from the preserved exact Stage82 r2 source:** its `Stage82Call1` stub (the already-installed Stage82 hook at F7AD1) calls the true `F3930`, restores native `EAX` and returns to `F7AD6`. The r2 diagnostic's `actualRVA=50AAE250` was a game-base-relative value; **actual target VA is `50BAE250`** (gameBase 00100000), not 50AAE250. The original Stage94 check falsely required only the untouched original `E8 5A BE FF FF`.

**r3** `ProducerCallChainOK` permits *only* (a) the exact native original CALL F3930 bytes and destination or (b) an E8 whose computed target is *exactly the linked address* `&Stage82Call1`, with `arcana_stage82::g_installed==1` and `g_Stage82Target1==gameBase+F3930`. It is NOT a loose accept-any-call-into-dinput8 bypass. All other Stage94 signature checks (F7AD6/F7B0A/neighboring instructions), recheck while other threads suspended, and rollback remain unchanged. Exact Stage82 stub symbol comparison does not acquire the loader lock while peers are suspended. Stage94 **does not touch F7AD1**. New trace: `STAGE94_PRODUCER_CHAIN_OK viaStage82=1 ... targetVA=...`, followed on successful Windows test by `STAGE94_INSTALLED 2/2`.

**Upgrade already-installed Stage94 r1 or r2 source**: preserve last tested Stage93 DLL (2,019,840 bytes); close game; extract ChatGPT ZIP `ArcanaAI_Stage94_r3_Stage82_CallChain_Compatibility_SourceOnly.zip`; execute `powershell -ExecutionPolicy Bypass -File .\UpdateStage94r3.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'`; then Build.bat (MSVC x86) and a short COM-vs-COM run. The SHA-gated updater touches **only** `Stage94Type1ProducerGate.inl` and `.stage94_source_backup/manifest.json`, keeping Stage82/91/92/93 and original Stage94 rollback script intact. Requires exact known source SHA for Stage94 r1/r2, Stage81/92/93 and Stage82's known call-site metadata. **Offline 38/38 tests PASS; Windows MSVC r3 and game not tested.** Keep Stage36 OFF. Upload complete new Build.log + Stage94 log; on success include same-run Stage82/Stage93QPC/FinalCommand.

---

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
