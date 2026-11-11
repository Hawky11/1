# Stage88 r1 — Stage87 selector -> F57D0 controller bitmap (SOURCE ONLY)

**Статическая проверка пройдена на точном оригинальном `AALib.exe`. Windows MSVC x86 BUILD и игровой COM-vs-COM тест Stage88 ещё НЕ выполнялись.** Никакого изменения искусственного интеллекта, команд, скорости, битов игрока или эмуляции ввода нет.

## Что уже подтверждено

- Stage87 был проверен в runtime коммита [`0315870`](https://github.com/Hawky11/1/commit/031587014a6f3be7c4ec3bfef0195106dfe8e689): `STAGE87_INSTALLED 1/1`, 221/221 совпадений преобразования `P+3F50` (`0→0x0E`, `1→0x0F`, `2→0x10`), `mismatch=readFail=dropped=0`. Это **прошлый** этап, не результат Stage88.
- Оригинальная функция `EE306→F57D0` получает cdecl четыре аргумента: `output=P+3A08`, `selectorCode=EAX`, `arg3=P+3F30`, `arg4=P+3F34`; Stage24 уже устанавливает единственный CALL-hook на `EE306`.
- Для `selectorCode=0x0E/0x0F/0x10` таблица F57D0 (`RVA 049DF188+code*3`) содержит **`01 26 FF`**. Это сбрасывает выходные DWORD `P+3A0C/10`, устанавливает `P+3A08=code` и через особый `0x26` выбирает бит 9/10/11 по **текущему** `P+3F50` (ожидаемые маски `0x200/0x400/0x800`) в `P+3A0C`. Статическая таблица и native MOV/OR подтверждены, но без Stage88 runtime не доказано, что в отдельном событии конкретный выходной бит был записан.
- Существующая функция логирования Stage24 фильтрует `dispatchType=1,payload=0x1E`, поэтому старые `STAGE24_RESOLVER_CONSUMER` не охватывают все случаи `EDI=0x2C`. Stage88 расширяет **наблюдение**, но не меняет фильтр исторических записей Stage24.

## Изменения в исходниках

1. `src/Stage87SelectorConsumer.inl`: добавляет один TLS handoff-контекст к уже существующей Stage87 записи (`stage87Seq`, player, TID, QPC, selector и EAX). Установщик Stage87 и оригинальная инструкция `EE2F0 MOV ECX,[ESI+3F34]` не изменены.
2. `src/Stage88Stage24Join.inl`: **не устанавливает ни одного хука**. `Stage88Stage24Pre(frame)` и `Stage88Stage24Post(savedContext)` получают снимки на входе/выходе **существующего Stage24**. Stage87 event consumed **один раз** тем же потоком, плюс проверяется соответствие P, EAX, аргументов и QPC окно не более 50 мс. Только P1/P2 EDI=0x2C. Реальный F57D0 выполняется один раз в исходном Stage24 wrapper.
3. `src/Stage81SelectorWriter.inl`: подключает `.inl`, запускает/периодически сбрасывает Stage88 ring из уже существующего Stage81 worker. Без нового worker, без дополнительных DR/VEH-хуков. `ArcanaAI_Stage88Stage24JoinTrace.log` создаётся worker-потоком.
4. `ApplyStage88.ps1`: проверяет SHA-256 **обоих исходников Stage87 r1** до записи; копирует два новых `.inl`, добавляет **две диагностические строки вызова + два prototype** в текущий `src/AIQueueTrace.cpp`, без замены целиком на старую версию. Предварительно проверяет единственность Stage24-якорей и создаёт backup каждого изменяемого исходника. Никаких изменений в `AALib.exe`, `dinput8.dll` или `ArcanaAI.ini` скрипт не делает.
5. `RollbackStage88.ps1`: восстанавливает три предыдущих исходника, только если их текущие SHA совпадают с зафиксированным установленным Stage88; удаляет новый `.inl`. После rollback нужно пересобрать и поставить Stage87 DLL.

## Как поставить

1. **Закрыть игру**. Сохранить работающую Stage87 `dinput8.dll` (из предыдущего теста 1 970 176 байт) и необходимые исходники.
2. Распаковать этот пакет. Выполнить из папки пакета в PowerShell:

   ```powershell
   powershell -ExecutionPolicy Bypass -File .\ApplyStage88.ps1
   ```

   По умолчанию рабочий проект: `D:\INCUBATOR\ArcanaAIOverhaul`. При другом расположении указать `-Root 'путь'`. Если хэши Stage81/Stage87 не совпадают — скрипт откажет без модификации.
3. В **рабочем** `ArcanaAI.ini` секция `[AIResearch]`:

   ```ini
   EnableStage24ResolverConsumerTrace=1
   EnableStage36ControlToCommandTrace=0
   EnableStage87=1
   EnableStage88=1
   ```

   `EnableStage87/88` по умолчанию 1; `EnableStage24ResolverConsumerTrace` должен быть явно включён. Убедиться, что старые хук-установщики Stage24/Stage87 не отказывают.
4. Собрать **Windows x86 MSVC** штатным `Build.bat`, проверить `BUILD SUCCEEDED` и размер DLL. Старое предупреждение C4754 не считать ошибкой, если оно единственное. Если ошибка компиляции — откат Stage88, предыдущую DLL не заменять.
5. Установить DLL обычным способом, выполнить 3–5 раундов COM-vs-COM (нативное поведение, без синтетики), завершить корректно с запасом для worker flush.
6. Загрузить в GitHub **из одного запуска** `Build.log`, `ArcanaAI_Stage87SelectorConsumerTrace.log`, `ArcanaAI_Stage88Stage24JoinTrace.log`, `ArcanaAI_Stage24ResolverConsumerTrace.log`, `ArcanaAI_FinalCommandTrace.log` и, по возможности, `ArcanaAI_EventDispatchTrace.log` + Stage79/80/81/82/84/85/86. Проверять результаты по **одному QPC домену** и одинаковым P/TID.

## Ожидаемый лог Stage88

```text
STAGE88_READY r1 SOURCE_ONLY Stage87 EE2F0 -> existing Stage24 EE306/F57D0; needs compiled Stage24 Pre/Post bridge; newHook=0 ...
STAGE88_F57D0 seq=... stage87Seq=... side=P1 player=... tid=... tick=... qpc87=... qpcBefore=... qpcAfter=... selector=... mappedEax=... argSelector=... arg3=... arg4=... dispatchType=... dispatchPayload=... dispatchMask=... tableRaw=... tableMask=... expectedBit=... bitBefore=... bitAfter=... pre=... post=... preMask=7 postMask=7 returnEax=... returnFlags=... match=1
STAGE88_HEALTH ... stale=0 mismatch=0 readFail=0 dropped=0 ...
```

Цель: количество `STAGE88_F57D0` должно совпасть с количеством `STAGE87_CONSUMER` по `stage87Seq`, а аргумент F57D0 с `mappedEax`, и после вызова должен быть установлен соответствующий `expectedBit`. Несовпадение допускается как **новое наблюдение**, а не признак для исправления игры без проверки.

Оффлайн:

```powershell
python .\tools\verify_stage88_static.py 'D:\путь\AALib.zip'
python .\tools\correlate_stage88.py .\ArcanaAI_Stage87SelectorConsumerTrace.log .\ArcanaAI_Stage88Stage24JoinTrace.log --final .\ArcanaAI_FinalCommandTrace.log --output stage88_results.json
```

`FinalCommandTrace` содержит `GetTickCount`, но не QPC. Даже единственное совпадение player/TID/payload в секундном окне — только *временной кандидат*, не доказанная причинная связка и не подтверждение выполненной атаки. Для следующего этапа понадобится точное наблюдение нативного action executor / animation queue.

## Откат

При сбое сборки, установке Stage24/87 с ошибкой, `mismatch/readFail/dropped`, зависаниях или нежелательном поведении игры закрыть игру, вернуть сохранённую Stage87 DLL, затем:

```powershell
powershell -ExecutionPolicy Bypass -File .\RollbackStage88.ps1
```

Никаких выводов об улучшении COM AI до runtime нет. Исторические Stage24/Stage87 ветки и логи не переписывать.