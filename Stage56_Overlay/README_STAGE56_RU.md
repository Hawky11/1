# Stage56 — проверка точных записей X внутри ACDF0

**Статус: исходники Stage56 готовы; Windows x86 сборка и работа в игре ещё НЕ проверены.** Рабочая Stage55 DLL должна оставаться у тебя для отката.

## Что исследуем

Stage55 локализовал все 480 X-различий своей сессии внутри native ACDF0. Stage56 ставит два пассивных наблюдателя на исходные пары:

- RVA `ACF27 FADD [EAX+7C]` + `ACF2A FSTP [EAX+7C]`, затем прыжок обратно на `ACF2D`;
- RVA `ACFA0 FADD [EAX+7C]` + `ACFA3 FSTP [EAX+7C]`, затем возврат на `ACFA6`.

Обе пары исполняются ровно один раз при входе в соответствующий детур. Снимаются сырые DWORD X до/после, реальный адрес объекта, P1/P2, поток, QPC и счётчики потерь. Игровые координаты, команды и поведение ИИ специально не изменяются. Реальные хелперы ACDF0 пока не перехватываем.

## Установка в D:\INCUBATOR\ArcanaAIOverhaul

1. **Сохрани отдельно работающую Stage55 DLL** `build\dinput8.dll` (и при необходимости Stage55 исходники).
2. Замени только `src\Stage38CallbackWindow.inl` файлом [Stage56_Overlay/src/Stage38CallbackWindow.inl](../Stage56_Overlay/src/Stage38CallbackWindow.inl).
3. Добавь **новый** `src\Stage56ACDF0Writer.inl` из [Stage56_Overlay/src/Stage56ACDF0Writer.inl](../Stage56_Overlay/src/Stage56ACDF0Writer.inl).
4. **Оставь без изменений** `src\Stage55EarlierMotionCoordinate.inl`, `src\Stage54PreParentCoordinate.inl` (r2), Stage47–53, INI и Build.bat. Не удаляй прежние Stage55/54 файлы.
5. Запусти статическую проверку при наличии оригинального EXE:
   ```powershell
   py -3 Stage56_Analysis\tools\verify_stage56.py --aalib "ПУТЬ\К\AALib.exe" --repo-root "ПУТЬ\К\КЛОНУ\stage56-acdf0-two-fstp-writer-observer"
   ```
   Это опциональная локальная проверка кода и бинарника. Путь `--repo-root` должен указывать на полный checkout ветки Stage56 с `Stage55_Overlay` и `Stage56_Overlay`; бинарник в GitHub **не заливать**.
6. Собери проект привычным `Build.bat`. Устанавливай новую DLL в игру **только** если в `Build.log` есть `BUILD SUCCEEDED`.

## Runtime тест

Натуральный COM-vs-COM бой ~30–60 секунд, дай логам записаться/сброситься; **игровую логику и тип боя не меняй между захватами**. В `ArcanaAI_Stage56ACDF0WriterTrace.log` ожидаем:

- `STAGE56_ACDF0_WRITER_BUILD=2026-11-10-r1`;
- `STAGE56_INSTALLED 2/2 ACF27+ACF2A ACFA0+ACFA3`;
- `STAGE56_WRITER` с `patchRVA`, `storeRVA`, `object`, `rawX=BEFORE>AFTER`;
- `STAGE56_HEALTH` — обязательно проверь `dropped=0 overflow=0 mismatch=0 readFail=0`.

`otherObjects` — количество исполнений пары на объектах НЕ из текущих P1/P2, не ошибка. Ноль строк writer при `calls>0` может означать, что обе пары писали в другие объекты; это тоже важный результат. Число X-изменений в новой сессии может отличаться от старых 480.

## Что загрузить в GitHub после теста

- `Build.log`;
- **`ArcanaAI_Stage56ACDF0WriterTrace.log`**;
- **`ArcanaAI_Stage55EarlierMotionCoordinateTrace.log`**;
- `ArcanaAI_Stage47MotionConsumerTrace.log` и `ArcanaAI_Stage54PreParentCoordinateTrace.log` — фактические имена существующих Stage47/54 логов сохрани как в текущем проекте;
- по возможности Stage52/53/49–51 того же прогона.

Ничего из предыдущих запечатанных коммитов/веток с логами не перезаписывай задним числом.

Если `STAGE56_SIGNATURE_FAILURE`, другой `ERROR`, вылет или явные регрессии — **не продолжай тест с новой DLL**; верни Stage55 DLL, загрузи лог сбоя. Предыдущие Stage55 hook-и должны оставаться функциональными, когда Stage56 просто не установился, но сборочные/исключительные ошибки проверяются отдельно.

## После загрузки

Сравним Stage56 события строго внутри Stage55 ACDF0 окна по **object + thread + QPC**. Увидим, покрывают ли конкретные пары записей все изменения X. Пока это только диагностика, не апгрейд боевого ИИ.
