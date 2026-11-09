# Stage34 — установка пассивного наблюдателя type0/payload 0,1,4,5

**Stage34 исходники готовы, x86 MSVC сборка и игровой прогон пока НЕ подтверждены.**

Брать два файла из ветки `stage34-type0-variant-state-trace`:

- [src/AIQueueTrace.cpp](src/AIQueueTrace.cpp) → заменить **только** `D:\INCUBATOR\ArcanaAIOverhaul\src\AIQueueTrace.cpp`.
- [ArcanaAI.ini](ArcanaAI.ini) → заменить **только** `D:\INCUBATOR\ArcanaAIOverhaul\ArcanaAI.ini`.

**Оставь `src\AITrace.cpp` от Stage29.** Остальной исходный код не изменять. Stage31/Stage32/Stage33 hooks и детали остаются в Stage34-файле.

1. Сохрани старые логи, запусти обычный **x86 Build.bat** и проверь `Build.log`. Предыдущее Stage33 нефатальное предупреждение C4754 в `AICommandSiteTrace.cpp` возможно; главное — реальный `BUILD SUCCEEDED`, корректная DLL и отсутствие ошибок.
2. Установи собранную `dinput8.dll` и `ArcanaAI.ini`, убедись, что старые логи перенесены в архив и не смешались с новым прогоном.
3. Запусти естественный COM-vs-COM и дай системе зафиксировать несколько циклов bit17. В идеале нужны разные payload 0/1/4/5; их возникновение зависит от игры и **не следует насильно вызывать**. Перед закрытием подожди как минимум две секунды для flush worker.
4. Загрузи `Build.log` и все свежие журналы, особенно `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage31AlternatePathTrace.log`, `ArcanaAI_Stage29Bit17Transitions.log`, `ArcanaAI_QueueTrace.log`, `ArcanaAI_ProducerCallTrace.log`.

## Ожидаемые маркеры и записи

В `ArcanaAI_EventDispatchTrace.log` ожидаются:
```text
STAGE32_TYPE0_DISPATCH_BUILD=2026-11-09-r1
STAGE33_TYPE0_CONSUMER_BUILD=2026-11-09-r1
STAGE34_TYPE0_VARIANTS_BUILD=2026-11-09-r1
STAGE34_CONFIG enabled type0 payload1/4/5 ...
EVENT_DISPATCH_DETAIL seq=... eventType=0x00000000 payload=0x00000001 ...
STAGE34_EFFECT_DETAIL seq=... qpcBefore=... qpcAfter=... side=P... player=... eventType=0 payload=0x00000001 preMask=0xFFF postMask=0xFFF slot0BMask=0x3 slot0B=...>... fields=0:...>..., ...
```

Payload 1/4/5 записываются как **`STAGE34_EFFECT_DETAIL`**, payload0 сохраняет **`STAGE33_EFFECT_DETAIL`** с прежним форматом. Оба вида используют тот же `seq`, что и `EVENT_DISPATCH_DETAIL`. Type1 detail тоже сохранён. Маска `0xFFF` означает удачное чтение всех 12 DWORD; `slot0BMask=0x3` — удачное чтение P+3A6C до/после. При других масках не трактовать нули как реальные данные.

**Задача анализа:** для нового scoped эпизода type0/payload1 проверить цепь Stage31 BEGIN → kind3 payload1 → END → EE200 detail → Stage34 effect, bit17 clear из Stage29 и изменения контроллера `P+3A08/+3A0C`. Для payload4/5 отдельно оценить `slot0B`. Нет доказательств, что поле означает реальную игровую атаку.

[Статическая проверка оригинального EXE](../Stage34_Analysis/STATIC_ANALYSIS.md) · [read-only verifier](../Stage34_Analysis/tools/verify_stage34.py).

Проверка статической таблицы, если нужно локально:
```powershell
py -3 .\Stage34_Analysis\tools\verify_stage34.py --binary "G:\Arcana Heart 3 - LS XTEND\AALib.exe"
```

Эталонный оригинальный EXE уже в библиотеке ChatGPT `/AALib.zip`; просить его снова не нужно.
