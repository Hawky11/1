# Stage49 r2 — исправление relocation-aware guard и следующий Windows x86 прогон

Статус: **SOURCE READY, WINDOWS BUILD/RUNTIME PENDING**. Предыдущий [Stage49 r1](https://github.com/Hawky11/1/tree/stage49-linked-motion-state-propagation) сохраняется неизменным. Этот этап **не Stage50**: предыдущий Stage49 не установил ни одного hook.

## Что показал загруженный r1 прогон (2026-11-10 14:37)

- [Сборка](https://github.com/Hawky11/1/commit/b7e0fbedf493a2939ecf2ef96e9098ff80836db5) — `BUILD SUCCEEDED`, dinput8.dll 1,074,176 байт, старый C4754 warning.
- [Runtime-логи](https://github.com/Hawky11/1/commit/1dde176a9cf02285cea12de7f6cbd2252807690e) — `STAGE49_LINKED_MOTION_BUILD=2026-11-10-r1`, затем **`ERROR: Stage49 root/CALL/linked-field/ABI signature mismatch; no patches.`**. Установка Stage49 **не состоялась**; Stage49 окон и HEALTH не появилось.
- Другие хуки исправны: Stage47 `STAGE47_INSTALLED 2/2`, 4,664 P1/P2 consumer windows, 2,899 изменённых окон, суммарные health dropped/overflow/mismatch/badCaller равны 0. Stage48 `STAGE48_INSTALLED 2/2`, 1,318 native binds, Stage44 1,318; Stage38 4,277 и Stage35 9,336 P1/P2 событий.
- Предпочтительная база оригинального PE32 = **0x00400000**; текущий процесс: `ret=0x008F6ADC` на `RVA 0xF6ADC`, т.е. база **0x00800000**. Исходный r1 guard сравнивал байты абсолютных `MOV ESI,[abs32]` по `AF11E/AF19E` с диском, хотя загрузчик Windows должен применить base relocation (+0x00400000). Это **вероятная причина**, но r1 не сообщил идентификатор несовпавшего guard; не считать доказанным до r2.

## Исправление r2

В `D:\INCUBATOR\ArcanaAIOverhaul\src` **ЗАМЕНИ ТОЛЬКО ОДИН ФАЙЛ**:

- [Stage49LinkedMotion.inl](src/Stage49LinkedMotion.inl) из ветки **stage49r2-relocation-aware-root-guard**.

**НЕ ЗАМЕНЯЙ** работающий `Stage38CallbackWindow.inl` (у тебя установлен адаптер Stage49 r1), а также Stage44/45/46/47/48 и другие .inl/.cpp, INI, Build.bat.

Новый root guard проверяет неизменные `8B 35` и **точный перенесённый** 32-битный адрес: `expectedPreferredAbs + (g_gameBase - 0x00400000)`. Для базы `0x00800000` ожидается `AF11E->0x0E6E02C8` и `AF19E->0x0E6E02C0`. Другие машинные сигнатуры и обе оригинальные CALL bytes/rel32 targets проверяются строго как раньше. Если что-либо не совпадёт, лог теперь покажет конкретный RVA / индекс байта / expected / actual, без установки патчей.

## Прогон

1. Архивируй старые журналы; в Windows запусти обычный x86 `Build.bat`. Требуй `BUILD SUCCEEDED`; при ошибке DLL не устанавливать.
2. Установи новую `build\dinput8.dll`. Проведи естественный COM-vs-COM бой без ручных изменений состояния; после остановки подожди >=2 секунд для flush.
3. Загрузи в GitHub main новый **Build.log**, **ArcanaAI_Stage49LinkedMotionTrace.log**, свежие Stage48/47/46/45/44 и остальные runtime-журналы (по наличию).

Проверить `STAGE49_LINKED_MOTION_BUILD=2026-11-10-r2` и **`STAGE49_INSTALLED 2/2`**. Только после установки можно делать выводы по `STAGE49_LINKED_WINDOW` и `STAGE49_HEALTH`. При повторном отказе новый конкретный `ERROR: Stage49 root...` или `ERROR: Stage49 native signature RVA...` необходим для следующего исправления. Не утверждать, что наблюдались реальные игровые перемещения/хиты/урон или улучшение AI.
