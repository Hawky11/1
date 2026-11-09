# Stage32 — type0/payload0, проверка передачи в диспетчер

**Статус: выполнен SHA-pinned статический анализ оригинального EXE; исходники диагностического расширения подготовлены. MSVC x86 сборка и проверка внутри игры пока НЕ выполнены.**

Основа: точный `AIQueueTrace.cpp` из Stage31 (Git SHA `6eb585a89142c5df74eb71fb6b7f91d0220bcb31`), INI Stage31 и работающий Stage29 `AITrace.cpp`. Сохранены все прежние Stage21–25/31 наблюдатели и прочие файлы проекта. Новых точек перехвата Stage32 **не добавляет**.

## Установка

1. Скопировать [Stage32_Overlay/src/AIQueueTrace.cpp](src/AIQueueTrace.cpp) в `D:\INCUBATOR\ArcanaAIOverhaul\src\AIQueueTrace.cpp`.
2. Скопировать [Stage32_Overlay/ArcanaAI.ini](ArcanaAI.ini) в `D:\INCUBATOR\ArcanaAIOverhaul\ArcanaAI.ini`.
3. **Не трогать `src/AITrace.cpp` от Stage29** и остальные исходники.
4. Обычный x86 `Build.bat`; проверить `BUILD SUCCEEDED` в полном `Build.log`. Установить новую DLL и INI.
5. До запуска перенести предыдущие `ArcanaAI*.log` в архив, чтобы новый прогон был чистым.
6. Естественный COM-vs-COM, несколько эпизодов с установками bit17 у обоих игроков, минимум 2 сек на выгрузку буфера перед выходом.
7. Загрузить `Build.log` и все свежие логи. Особенно: `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage31AlternatePathTrace.log`, `ArcanaAI_Stage29Bit17Transitions.log`, `ArcanaAI_QueueTrace.log`, `ArcanaAI_ProducerCallTrace.log`.

## Что искать

В `ArcanaAI_EventDispatchTrace.log`:

```text
STAGE32_TYPE0_DISPATCH_BUILD=2026-11-09-r1
STAGE32_CONFIG type0_payload0=1 ...
CPU event dispatch hook installed at RVA=0x000EE200 ...
EVENT_DISPATCH_DETAIL seq=... qpcBefore=... qpcAfter=... side=P... callerRVA=0x000EE5CB ... eventType=0x00000000 payload=0x00000000 ... before=(active:01 current:FFFF,FFFF history:0000,0000 ...) ...
```

Новый `EnableStage32Type0DispatchDetail=1` добавляет детализацию **только type0/payload0** к прежним type1 записам, используя уже установленный `EventDispatchHook`. Именно последовательность Stage31 END → последующий dispatcher QPC/side/player/queue будет проверяться. Старые сгруппированные `EVENT_DISPATCH`-строки и `tid(first)` недостаточны для точного связывания.

## Критерий анализа

- Stage31 BEGIN / вложенный kind3 / END должны оставаться целыми; Stage29 set/clear привязаны к тем же игрокам и QPC.
- Stage32 event detail подтверждает **вызов** EE200 после изменения type0/payload0, а не удачный игровой удар/движение.
- Если несколько type0 событий между вставкой и диспетчером, не приписывать отдельный dispatch эпизоду без доказательств. Сравнивать историю/текущую пару, QPC и последующие вставки.
- Ошибка/отсутствие hook, dropped или переполнение буфера делает вывод об отсутствии dispatch ненадёжным.
- Проверка оригинального EXE (при необходимости): `py -3 Stage32_Analysis\tools\verify_stage32.py --binary "G:\Arcana Heart 3 - LS XTEND\AALib.exe" --source "Stage32_Overlay\src\AIQueueTrace.cpp" --ini "Stage32_Overlay\ArcanaAI.ini"`.

[Полный статический разбор](../Stage32_Analysis/STATIC_ANALYSIS.md). Исходный EXE уже в ChatGPT Library `/AALib.zip` — повторно запрашивать не нужно.
