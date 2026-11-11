# Stage87 r1 — селектор → исходный mapping-код перед F57D0 (SOURCE ONLY)

**Статический анализ оригинального EXE завершён, Windows x86 build и COM-vs-COM runtime для Stage87 НЕ выполнялись.** Никаких AI-улучшений, команд, анимаций, атак или движений Stage87 не создаёт. Это один пассивный диагностический перехват.

## Для чего

В пятираундовом прогоне коммита [`828994d`](https://github.com/Hawky11/1/commit/828994d13e0ac3e254a7413c7c64acdba2192846) Stage79 и Stage80 синхронно увидели **17** изменений `P+3F50`: **15** точно выполнялись в Stage84 (`F3930`) и **2** в Stage81 (`F7150`). Редкие P2 `0→1` — Stage80 seq4814 (Stage81 `F7D38`) и seq7023 (Stage81 `F7D22`). В обоих случаях Stage82 изменил только companion `P+3F54→2`, Stage85 JOIN ещё видел selector=0, Stage86 `E8560` не менял selector, Stage81 затем непосредственно записал 1. Все stage79/80/82/84/85/86 QPC-сопоставления одного прогона согласованы. HEALTH counters имеют нули по `dropped/overflow/mismatch/readFail`.

Теперь нужно проверить, **какой оригинальный код потребляет selector и во что его преобразует**, а не продолжать поиск этих двух MOV.

## Статические факты по SHA-проверенному оригиналу

Original `AALib.exe` SHA-256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.

В оригинальном обработчике `EE200`:

- `EE2D0 CMP EDI,2C` выбирает специальную ветку только при descriptor `EDI=0x2C`.
- `EE2D5 MOV ECX,[ESI+3F50]` считывает selector.
- До `EE2F0` вычисляется исходный `EAX` по `selector=0→0x0E`, `1→0x0F`, `2→0x10`.
- `EE2F0` — безопасно выделенная **6-байтовая** оригинальная инструкция `8B 8E 34 3F 00 00` (`MOV ECX,[ESI+3F34]`) непосредственно перед кодом, который передаёт `EAX` в `EE306 CALL F57D0`. Stage24 уже наблюдает EE306, поэтому **второго перехвата EE306 НЕ делаем**.
- Другие оригинальные чтения selector в `124674`, `156BA1` через таблицы `VA 05B8ED5C/06317664` используют индексы `[26,27,28]` и соответствующие request-коды `[0x24,0x25,0x26]`, которые затем могут поступать к E5BF0. Это **условные пути**, а не утверждение, что каждый вызов F7150 вызывает E5BF0.
- Чтения `F5817/F586D` при табличном значении `0x26` также выбирают индекс бита на основании selector, но прямого исполнения этих чтений в новом пятираундовом прогоне ещё не измеряли.

Проверка всех 12 оригинальных сигнатур и обоих массивов таблиц выполнена в `tools/verify_stage87.py` на исходном EXE из пользовательской ChatGPT Library, результат `PASS_STATIC_SOURCE_ONLY`.

## Что добавляет Stage87

Единственная новая точка **EE2F0** заменяет исходный `MOV ECX,[ESI+3F34]` на `CALL Stage87EE2F0Hook; NOP`, в обработчике:

1. Сохраняет EFLAGS, GPR, x87/SSE и stack shape; для случаев `EDI=0x2C` и точных P1/P2 снимает read-only snapshot (selector, companion, player mode, object, EAX и QPC).
2. **Один раз** исполняет оригинальный `MOV ECX,[ESI+3F34]` внутри хука и возвращается на оригинальный `EE2F5 NOP → EE2F6`.
3. Только worker-поток записывает буфер в `ArcanaAI_Stage87SelectorConsumerTrace.log`; game-data и debug-регистры не изменяются. Следующие инструкции и оригинальный `EE306→F57D0` остаются нативными.
4. До установки проверяет x86/SSE, PE fingerprint, Stage86 installed, точную live-сигнатуру `EE2D0/EE2D5/EE2F0/EE2F6`, свободный EIP-диапазон у остановленных потоков, установочный mutex; при ошибке **отказывает в патче**. При изменении исходных байтов неизвестной стороной — не устанавливается.

**Внимание:** как и предыдущие Stage*, даже пассивный CALL-site перехват изменяет тайминг и вводит новый stack frame на время диагностики. До Windows runtime проверки не считать его гарантированно безопасным.

## Установка на проверенный Stage86 r1

1. **Закрой игру**, сделай отдельную копию `build\dinput8.dll` Stage86 (**1 965 568 байт**) и текущих `src\Stage81SelectorWriter.inl`, `src\Stage86E8560CallBoundary.inl`.
2. Распакуй ZIP и выполни из него:

   ```powershell
   powershell -ExecutionPolicy Bypass -File .\ApplyStage87.ps1
   ```

   Путь по умолчанию — `D:\INCUBATOR\ArcanaAIOverhaul`. Скрипт **сверяет SHA-256** установленного Stage86-адаптера и Stage86-хука, затем копирует **только** `src\Stage81SelectorWriter.inl` и добавляет **только** `src\Stage87SelectorConsumer.inl`. До успешной сверки ничего не пишет. Создаёт резервную копию `Stage81SelectorWriter.inl.stage86r1.bak`. Все остальные исходники, INI и DLL остаются нетронутыми.
3. Убедись, что в **рабочем** `ArcanaAI.ini` `EnableStage36ControlToCommandTrace=0`. Новая диагностика включена по умолчанию, либо явно `EnableStage87=1` в секции `[AIResearch]`; `EnableStage87=0` отключает новый перехват. Менять прежние настройки не требуется.
4. Запусти штатный `Build.bat` для **MSVC x86**. Нужен `BUILD SUCCEEDED`. Размер новой DLL пока неизвестен, выдумывать его нельзя.
5. Один короткий COM-vs-COM тест. В логе должно быть:

   ```text
   STAGE87_SITE_AUDIT rva=0EE2F0 ... exact=1
   STAGE87_INSTALLED 1/1 rva=0EE2F0 ... replayMOV=1
   STAGE87_CONSUMER seq=... side=P... selector=... mappedEax=... expected=... match=1
   STAGE87_HEALTH ... dropped=0 mismatch=0 readFail=0 ...
   ```

   Не обязательно увидеть все три состояния за один короткий бой. Отсутствие `STAGE87_CONSUMER` при наличии `INSTALLED` означает, что фильтр `EDI=0x2C` не сработал в этом тесте, **не** ошибку установки.
6. Загрузи **логи одной сессии**: новый Stage87, Stage79/80/81/82/84/85/86, `Build.log`. Сохранить QPC-идентичность P/O/TID; не смешивать предыдущие забеги.

## Статический и runtime offline-тест

```powershell
python .\tools\verify_stage87.py 'D:\путь\AALib.zip'
python .\tools\correlate_stage87.py '.\ArcanaAI_Stage87SelectorConsumerTrace.log' --stage80 '.\ArcanaAI_Stage80F7150LateTrace.log' --output '.\stage87_results.json'
```

Важно: `correlate_stage87.py` показывает последние ранее наблюдавшиеся переходы только как **временной контекст**. Это не причинная связь и не доказательство выполненной атаки. Для связи с движением/ударом позднее нужна независимая same-run трасса конкретного исполнительного consumer.

## Откат

Если появились `ERROR`, `exact=0`, Health failures, `match=0`, проблемы поведения или сборки — закрыть игру, вернуть сохранённую Stage86 DLL и выполнить:

```powershell
powershell -ExecutionPolicy Bypass -File .\RollbackStage87.ps1
```

Откат тоже сверяет SHA-256 и не перезаписывает неизвестно изменённые файлы. Установка Stage87 **не является улучшением AI** и не содержит синтезированных команд.
