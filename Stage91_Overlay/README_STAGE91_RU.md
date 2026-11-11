# Stage91 r1 — F58E0 indices 9/10/11: прямые читатели bitmap (SOURCE ONLY)

**СТАТУС:** Оригинальный PE32 проверен по SHA-256, **68/68 static PASS**, синтетические позитивный и 3 негативных теста коррелятора PASS. **На Windows x86 НЕ собран и в игре НЕ протестирован.** Проверенная предыдущая DLL — Stage90, **1 991 168 байт**.

## Зачем

Stage88→Stage90 подтвердил установку/сброс `P+3A0C & 0xE00`, а Stage89 `AAB55` видел `AL=0` уже после очистки. Stage91 исследует **три других точных CALL внутри F5990**, которые вызывают `F58E0(bitmap,9/10/11)` и напрямую тестируют соответствующие биты:

| CALL RVA | Bit index | Бит `P+3A0C` | AL=1 приводит к |
|---|---:|---|---|
| `F5A4D` | 9 | `0x200` | ветке `F5A68` (P+3B70 bit0x8) |
| `F5A5C` | 10 | `0x400` | ветке `F5A68` (P+3B70 bit0x8) |
| `F5A76` | 11 | `0x800` | вызову `9ED20` вместо `9ED60` |

Нативная `F5990` вызывается из `ACAB8` в цикле обновления игрока. Не все три CALL выполняются одновременно: проверки имеют условные переходы. **Не считаем это подтверждением выполнения атаки или команды.**

## Состав пакета

- `src/Stage91DirectSpecialReader.inl`: три индивидуальных сигнатурно защищённых CALL wrapper; оригинальная F58E0 вызывается ровно один раз; сохраняются регистры, FLAGS, x87/MMX/XMM и caller stack. Чтения P1/P2, битов, AL, QPC, P+3B70; 8192-ring, worker-only лог. Нет глобального F58E0 detour, дополнительных EE306-хуков, записи геймплейных полей или DR.
- `src/Stage81SelectorWriter.inl`: **минимальное** дополнение Stage90-проверенного адаптера вызовами Install/Flush/Health для Stage91; Stage87/88/89/90 на месте.
- `ApplyStage91.ps1` и `RollbackStage91.ps1`: SHA-гейт на исходные Stage81/88/89/90 и обратимый overlay без затирания правок.
- `tools/verify_stage91.py`, `tools/correlate_stage91.py`, `tools/test_correlate_stage91.py`, `analysis/STATIC_ANALYSIS.md`, `analysis/static_results.json`.

## Windows-тест

1. Закрыть игру; **сохранить Stage90 DLL 1 991 168 байт** и исходники. Убедиться, что установлен именно Stage90 r1 из прошлого пакета.
2. Запустить из распакованного ZIP:
   `powershell -ExecutionPolicy Bypass -File .\ApplyStage91.ps1 -Root 'D:\INCUBATOR\ArcanaAIOverhaul'`
3. В `[AIResearch]` оставить `EnableStage24ResolverConsumerTrace=1`, `EnableStage88=1`, `EnableStage89F58E0ReaderTrace=1`, `EnableStage90AllF57D0Trace=1`, `Stage90LogAllF57D0=1`. Добавить/оставить `EnableStage91DirectSpecialReaderTrace=1`, `Stage91ReaderSiteMask=7`. **Stage36 выключен:** `EnableStage36ControlToCommandTrace=0`.
4. Собрать обычным Windows x86 `Build.bat`. Не запускать игру, если BUILD FAILED, Stage91 `STAGE91_ERROR`, неправильная маска или не создана резервная копия.
5. Запустить 3–5 раундов COM-vs-COM. Загрузить в GitHub **из того же запуска**: `Build.log`, `ArcanaAI_Stage91F58E0ReaderTrace.log`, `ArcanaAI_Stage90AllF57D0Trace.log`, `ArcanaAI_Stage88Stage24JoinTrace.log`, `ArcanaAI_Stage87SelectorConsumerTrace.log`, `ArcanaAI_Stage89F58E0ReaderTrace.log`, `ArcanaAI_FinalCommandTrace.log`, `ArcanaAI_EventDispatchTrace.log`. Можно добавить Stage79/80/81 при регрессиях.
6. Проверить offline: `python tools\correlate_stage91.py ArcanaAI_Stage91F58E0ReaderTrace.log --stage90 ArcanaAI_Stage90AllF57D0Trace.log --stage88 ArcanaAI_Stage88Stage24JoinTrace.log --output stage91_results.json`.

Ожидаемая `STAGE91_INSTALLED mask=7 sites=3/3`. В HEALTH важны `mismatch=0`, `readFail=0`, `overflow=0`, `dropped=0`. Если вызовов не будет — это **не** доказательство отсутствия чтения в других местах. При сбое сначала `RollbackStage91.ps1`, затем пересборка/возврат проверенной DLL.

**Главная цель:** поймать `returnAL=1` с конкретным битом `P+3A0C` и сопоставить QPC с предыдущим F57D0 и последующими следами команд. Само `AL=1` ещё не означает движение/попадание или улучшение AI.
