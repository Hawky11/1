# Stage34 — нативные варианты type0/payload0,1,4,5 и пассивная диагностика

Date: 2026-11-09. **Статус: оригинальный EXE проанализирован и независимо проверен; Stage34 source-only overlay подготовлен. Windows MSVC x86 build и Stage34 runtime ещё не выполнялись.**

## Источники и метод

- Реальный Stage33: [архив и полный отчёт](https://github.com/Hawky11/1/tree/stage33-type0-consumer-runtime-evidence/Stage33_Runtime), evidence @ `5eb0d96cd4a20cb713f4add43516857e6ba74389`. Наблюдались три Stage31 episode: ep1=P1 type0/payload0, ep2=P2 **type0/payload1**, ep3=P2 type0/payload0. Stage32+33 фиксировали индивидуальные QPC/net-effect только для payload0; ep2 не потерян, он намеренно отфильтрован.
- Исходное `AALib.exe` из ChatGPT Library `/AALib.zip` (повторной загрузки от пользователя не требуется): 175,556,608 байт, SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`, PE32, preferred ImageBase=0x400000. Здесь **функции и места CALL указаны в RVA**; таблицы ниже — **предпочтительные VA**.
- Проверенный на оригинальном EXE read-only [verify_stage34.py](tools/verify_stage34.py): **15 точных byte signatures, 6 direct E8 CALL targets, 4 полные записи таблицы, 8 производных чтений флагов/descriptor; PASS**. Использует проверку хеша и mapping секций PE. Исходный EXE и полные дизассемблированные файлы не публикуются.
- Предшествующая Stage33 source-only ветка `stage33-type0-consumer-state-trace` @ `774ddbbf3a8fec3d6ed09afc31232ade3731c2da` остаётся неизменной; Stage34 копирует только исходники поверх неё.

## Таблица выбора native type0

Native `EE200` при `type=0` исполняет `EE221: PUSH payload` → `EE222: CALL RVA158560` → из восьмибайтной записи `VA 0x6521F80 + payload*8` читает три байта: `[+2]→EDX` (id для `158EC0`), `[+3]→EDI` (id для `F57D0`), `[+4]→ECX` (конечное `P+3F64`).

| Payload | Запись, 8 байт | EDX:158EC0 id | EDI:F57D0 id | ECX:P+3F64 | Extra |
|---:|---|---:|---:|---:|---|
| 0 | `0F 00 00 00 00 00 00 00` | 0 | 0 | 0 | нет |
| 1 | `0F 80 01 01 01 00 00 00` | 1 | 1 | 1 | нет |
| 4 | `0F 00 00 00 00 28 06 00` | 0 | 0 | 0 | `EE360` |
| 5 | `0F 00 01 01 01 28 06 00` | 1 | 1 | 1 | `EE360` |

Следовательно, нельзя трактовать payload1/5 как синоним payload0/4. Байт `0x80` в `payload1` и байты `28 06` у 4/5 пока не интерпретированы как игровые эффекты: EE200 использует только оффсеты +2/+3/+4 в показанном маршруте.

### Флаги и управляющий идентификатор

`EE2C6 CALL 158EC0(P,EDX)`: native table VA `0x652222C`: id0→`0x01`, id1→`0x03`. По дизассемблированию функции:
- При обоих id: `P+3B70` bit10 (mask `0x400`) устанавливается.
- id0: bit5 (mask `0x20`) очищается; id1: bit5 устанавливается.
- Биты 6,7,8,9 очищаются при id0 и id1, т.к. соответствующие биты флагов в `0x01/0x03` нулевые.

`EE306 CALL F57D0(P+3A08, EDI, P+3F30, P+3F34)`: native controller descriptor table VA `0x4DDF188`, 3 байта на id:
- id0 → `01 00 FF` — начальная очистка DWORD `P+3A0C/+3A10`, запись `P+3A08=0`, установка bit0 в `P+3A0C`.
- id1 → `01 01 FF` — аналогичная очистка и `P+3A08=1`, установка bit1 в `P+3A0C`.
- На входе `F57D0` вызывается дополнительный helper `49EEA0` с другими аргументами. Эти байты не доказывают, что вся логика `F57D0` идентична во всех состояниях.

### Специальная ветка payload4/5

В `EE322`: compare EBX с 4, затем в `EE327` с 5; оба переходят на `EE360`. Там два helper вызова и `EE376 CALL RVA991C0` с
`EAX=P+0x3A3C`, индексом **0x0B** и вычисленным значением. `991C0` масштабирует значение на **120** (`x*15*8`) и записывает DWORD:
`[P+3A3C + 0x04 + 0x0B*4] = [P+0x3A6C]`.

Это **не дополнительная ветка для payload1**. Для 4/5 эффект может быть временным и другие helpers могут перезаписать слот; именно поэтому Stage34 добавляет `P+3A6C` в before/after наблюдение. Мы не приписываем этому слоту смысл «атака» или «приседание» без другого доказательства.

Все четыре варианта также вызывают `EE315 CALL EC4F0(P, type=0, payload)`; предыдущее исследование Stage33 установило conditional bit17/18 clear через `ED193 CALL F2ED0` и `ED19F AND [P+3B84], FFF9FFFF`. Реальный результат зависит от текущих history/state; нельзя гарантировать каждый флаг из одних таблиц.

## Реализация наблюдателя (НЕ gameplay patch)

[Stage34_Overlay/src/AIQueueTrace.cpp](../Stage34_Overlay/src/AIQueueTrace.cpp) наследует точную Stage33 проверенную в игре реализацию перехвата `EE200`. **Новых точек hook нет; оригинальная `EE200` вызывается ровно один раз с прежними аргументами; game-state writes отсутствуют.**

- `EnableStage34Type0VariantsTrace=1` активирует дополнительную per-event детализацию **только** для `type0/payload1,4,5`; всё остальное с type0 в Stage34 дополнительно не захватывается. Старые **type0/payload0** и **type1** работают как в Stage33/32.
- Для 1/4/5 стандартный `EVENT_DISPATCH_DETAIL` идёт в общий существующий `ArcanaAI_EventDispatchTrace.log` и пишет QPC/side/player/queue.
- Новая `STAGE34_EFFECT_DETAIL` использует **тот же seq/QPC/side/TID/player** и добавляет `payload=0x...`, `preMask/postMask`, прежние 12 пар `fields=0..11`, плюс `slot0BMask` (bit0=preReadOK, bit1=postReadOK), `slot0B=XXXXXXXX>YYYYYYYY` для P+3A6C.
- `STAGE33_EFFECT_DETAIL` для payload0 сохраняет **исходный формат Stage33**, никаких инвариантов его полей менять не требуется. Стандартное кольцо событий и worker thread продолжают использоваться; сообщения пишутся с worker, без file I/O внутри EE200, overflow/dropped проверять.
- INI зависимости: `EnableCpuEventDispatchTrace=1`, `EnableStage32Type0DispatchDetail=1`, `EnableStage33Type0StateTrace=1`; если нарушены, Stage34 сообщает ERROR и отключает новые variant records. При неуспешном чтении slot0B нулевой raw value не считается реальным: читать `slot0BMask`.
- Маски 12 основных полей остаются по схеме Stage33 (bit i=удачное чтение), snapshot не атомарен; net differences не доказывают все внутренние записи и не доказывают success игровой команды.

**Проверка исходного кода:** структурная проверка установки predicate на {1,4,5}, старых маркеров Stage31/32/33, наличия новых Stage34 записей, одного native original call, сохранения Stage33 payload0 и type1, pre/post snapshot, INI зависимостей и worker-only записи логов — PASS. Это **не** настоящая компиляция x86/MSVC и не runtime тест.

## Следующий шаг

Заменить у пользователя **только** `D:\INCUBATOR\ArcanaAIOverhaul\src\AIQueueTrace.cpp` и `ArcanaAI.ini` файлами Stage34 Overlay. **Stage29 `AITrace.cpp` не менять**; ничего другого в исходниках не подменять.

Обычный x86 `Build.bat`; раздельный `Build.log`, установка DLL/INI, архив старых журналов, естественный COM-vs-COM (желательно >= 2 случая bit17=1, в том числе type0/payload1), дождаться >=2s flush. Загрузить новые `Build.log`, `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage31AlternatePathTrace.log`, `ArcanaAI_Stage29Bit17Transitions.log`, `ArcanaAI_QueueTrace.log`, `ArcanaAI_ProducerCallTrace.log`.

Следующая сессия: подтвердить Stage34 marker, native hook и сборку, seq непрерывность и masks, QPC Stage31 nested type0/payload1→Stage34 EVENT_DISPATCH_DETAIL+STAGE34_EFFECT_DETAIL, затем сравнить отдельные net поля с таблицами и значением P+3A6C для 4/5. **Не делать прямых выводов о результате удара/движения из обновления состояний.** Не просить AALib.exe заново.
