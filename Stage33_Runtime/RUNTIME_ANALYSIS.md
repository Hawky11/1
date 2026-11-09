# Stage33 — полный анализ реального runtime-прогона (2026-11-09)

## Источники, сборка и достоверность

- GitHub user upload: runtime `main` коммит [`7ca41fae`](https://github.com/Hawky11/1/commit/7ca41fae5a4b3e5af9699c55845f497e683ae8e9) и отдельный `Build.log` коммит [`40eb5bb1`](https://github.com/Hawky11/1/commit/40eb5bb1b6a3afee635571b9b201d625858d25dc); сделана копия **21 оригинального .log + Build.log** под `Stage33_Runtime/` **без изменения Git blob SHA**, плюс три CSV.
- Рабочая исходная ветка Stage33 [stage33-type0-consumer-state-trace](https://github.com/Hawky11/1/tree/stage33-type0-consumer-state-trace) @ `774ddbbf3a8fec3d6ed09afc31232ade3731c2da`; оригинальный AALib.exe остаётся в ChatGPT Library `/AALib.zip`, эталонный SHA-256 `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
- **Компиляция подтверждена:** `Build.log` 09.11.2026 23:02:48, x86 VS18/MSVC 14.51.36231, `vcvarsall x86` exit0, `cl.exe` и `link.exe` обнаружены, все 8 C++ модулей прошли, `BUILD SUCCEEDED`, `dinput8.dll` 249856 bytes. **Одно предупреждение C4754** в `src/AICommandSiteTrace.cpp:785`, с отсылкой к `:782`, не блокирует сборку; русский текст предупреждения в логе не в UTF-8. Не называть сборку полностью warning-free.
- В `ArcanaAI_EventDispatchTrace.log` обнаружены `STAGE32_TYPE0_DISPATCH_BUILD=2026-11-09-r1`, `STAGE33_TYPE0_CONSUMER_BUILD=2026-11-09-r1`, `STAGE33_CONFIG`; исходный EE200 hook установлен. Stage31 `F6A8C→130A10` установлен; Stage29 watchpoint DR0/DR1 активен; F2DE0 queue hook установлен. Все записи одного свежего COM-vs-COM сеанса 23:03:36–23:04:24.
- Stage31 `seq=1..54`: 3 BEGIN, 3 END, 48 kind3. Stage29 `seq=1..6`: три set/clear цикла bit17. Stage32 `EVENT_DISPATCH_DETAIL seq=1..176` (104 type0/payload0, 72 type1). Stage33 `STAGE33_EFFECT_DETAIL`: **104 записи**, каждая однозначно и точно совпала по `seq, QPC before/after, side, TID, player` с отдельной Stage32 type0/payload0 detail. **104/104 preMask=postMask=0xFFF**, все 12 чтений в обоих снимках успешны. Нет дыр в Stage31, Stage29, Stage32 seq; Stage33 использует seq Stage32 и не должен быть непрерывным. В основных логах нет явных ERROR/nenнулевых dropped/overflow.
- Stage33 тип0/payload0 — подмножество runtime событий, а не полный наблюдатель type0 с иными payload.

## Полный результат изменений состояния

Из 104 полных pre/post EE200 снимков в **37** имеется хотя бы один конечный изменившийся DWORD; в **67** все двенадцать полей равны до/после (это **не** означает отсутствия промежуточных записей или игровой активности).

| Индекс | Поле | Сколько записей с изменением |
|---:|---|---:|
| 0 | `P+3B70` | 29 |
| 1 | `P+3B84` | 4 |
| 2 | `P+3F64` | 25 |
| 3 | `P+3A08` | 34 |
| 4 | `P+3A0C` | 33 |
| 5 | `P+3A10` | 16 |
| 6 | `[P+4]` | 0 |
| 7 | `O+08` | 1 |
| 8 | `O+184` | 3 |
| 9 | `P+3F54` | 4 |
| 10 | `P+3F74` | 0 |
| 11 | `P+3F78` | 0 |

См. [все 104 валидных снимка](stage33_effect_records.csv) с полной маской и 12 парами raw DWORD. В частности объектный указатель [P+4] не менялся; O+08 менялся 1 раз, O+184 — 3 раза. Значение 0 в неизменной паре — только факт читаемого равенства, не доказательство отсутствия временных эффектов.

## Три родительских эпизода Stage31: два типа0/0, один типа0/1

**Критическая находка:** Stage31 `episode=2` (P2) вставляет **type0/payload1**, а не type0/payload0. Нативная функция 156DA0 может выбирать payload 0/1/4/5 в зависимости от состояния. Существующий Stage32+Stage33 фильтр перехватывает индивидуальные QPC/поля только для type0/**payload0**, поэтому эпизод 2 нельзя записывать в «потерянные» и нельзя ему выдумывать same-seq запись.

| Episode | Side | Stage31 BEGIN/QUEUE/END | type/payload | Stage32 detail | Stage33 effect | END→dispatch |
|---:|---|---|---|---|---|---|
| 1 | P1 | 10/11/12 | `0/0` | 47 | 47 | 12 QPC |
| 2 | P2 | 30/31/32 | `0/1` | not observed per event | not observed (filter) | — |
| 3 | P2 | 47/48/49 | `0/0` | 161 | 161 | 9 QPC |

При этом Stage29 у всех трёх эпизодов фиксирует `bit17: 0→1` перед Stage31 и `1→0` после END, `0x30000→0x10000`, post-write RVA `ED1A9`, один поток TID=14016. Для episode2 QPC: insert `515844913581`, Stage31 END `515844913931`, bit17 clear `515844914498`. Старый агрегированный `EVENT_DISPATCH` в 23:04:12 зарегистрировал P2 type0/payload1 с `active=1/history=(0,1)`; но это **групповой счётчик без per-event QPC**, не доказанная передача конкретного ep2 до EE200.

### Два полных совпадения Stage31 → Stage32 → Stage33

**Episode 1 / P1, Stage32/33 seq47:** Stage31 END QPC 515727356260 → EE200 entry 515727356272 (Δ=12 ticks), EE200 exit 515727357128, Stage29 bit17 clear 515727356857 **inside dispatch**. pre/post P+3B84 `00030000→00010000`. Значимые net поля: `P+3B70` 00020640→00020400; `P+3B84` 00030000→00010000; `P+3F64` 00000015→00000000; `P+3A08` 00000029→00000000; `P+3A0C` 00000000→00000001; `P+3A10` 00000010→00000000. Все 12 пар с обеими масками 0xFFF.

**Episode 3 / P2, Stage32/33 seq161:** Stage31 END QPC 515932026741 → EE200 entry 515932026750 (Δ=9 ticks), EE200 exit 515932027571, Stage29 bit17 clear 515932027308 **inside dispatch**. pre/post P+3B84 `00030000→00010000`. Значимые net поля: `P+3B70` 00020250→00020410; `P+3B84` 00030000→00010000; `P+3F64` 00000016→00000000; `P+3A08` 00000029→00000000; `P+3A0C` 00000000→00000001; `P+3A10` 00000010→00000000; `O+08` 04014000→04004000; `O+184` 0000001E→00000000; `P+3F54` 00000001→00000000. Все 12 пар с обеими масками 0xFFF.

Соответствие осуществляется по тем же QPC, side, TID, player, типу и payload; у двух эпизодов `EE200` начинается через 12 и 9 QPC-тиков после Stage31 END. Этот evidence подтверждает **конечное изменение управляющих полей**, но **не** успешное выполнение конкретной команды атаки/движения.

## Внешние (episode=0) записи и очередь

Из 48 Stage31 kind3 `type0` payload распределение: **0:31, 1:9, 4:5, 5:3**. Из **31** Stage31 type0/payload0 вставок (включая два вложенных) **31/31** имеют уникальные последующие Stage32+Stage33 совпадения в том же side/TID, без разрыва наблюдаемого ряда; Δ от окончания F2DE0 до входа EE200: **6..392 QPC ticks**. [CSV с полной корреляцией 31 вставки](stage31_stage33_type0_pairs.csv).

**Важное ограничение:** Stage33 и Stage32 не записывают индивидуальный type0/payload1/4/5 в этом режиме; их отсутствие в таблицах не означает отсутствие вызовов диспетчера. Не суммировать агрегаты `EVENT_DISPATCH.totalHits` как независимые события.

## Stage15 и прямая ветвь

В ProducerCallTrace **7** реальных `F6A68_PLAYER_BIT17_BRANCH`: три `bit17=1 branchTaken=0`, точно соответствующие Stage31 episodes; четыре `bit17=0 branchTaken=1` (P1 два, P2 два) пошли через `F6AD7` с `type1/payload0x1E`. Stage21 classifier для четырёх type1/0x1E вернул AL0, fallback slot3 остаётся 0. Это нормальный контрольный маршрут, отдельный от type0; не уравнивать их.

## Вывод и следующий этап

**Stage33 завершён для type0/payload0:** Windows x86 билд и игровой runtime успешны, 104/104 валидных net snapshots, 31/31 подтверждённых type0/payload0 очередей с индивидуальным EE200 QPC; два из трёх Stage31 alternates получили прямые same-event Stage33 снимки с bit17 clear и управляющими полями. **Незакрытый пробел** — единственный третий scoped episode `type0/payload1`, который отфильтрован по design, а не потерян.

**Stage34 (при продолжении):** сначала статически проверить, как table index1/4/5 отличается от index0 в `158560/158EC0/F57D0`, затем **пассивно расширить существующий Stage32/33 EE200 detail predicate на type0/payload {0,1,4,5}**, сохранив type1 и Stage29/31, без новых хуков, без изменения игровых данных. Не делать выводов об успехе действия из net флагов. Оригинальный `AALib.exe` уже в ChatGPT Library `/AALib.zip`, SHA pinned — повторно не просить. 
