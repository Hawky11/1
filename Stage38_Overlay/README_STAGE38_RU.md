# Stage38 — выполнение обработчика действия

**Исходники готовы; Windows x86-сборка и работа Stage38 в игре ещё не проверены.**

После EE200 игра выбирает обработчик команды и действительно вызывает его через `call edi` в EE5E0. Новая трасса сравнивает состояние непосредственно до/после этого вызова, пишет реальный адрес обработчика, P1/P2, TID, QPC и 30 полей. Нулевой указатель сохраняется отдельной записью с `guardedCall=0`. Вызов обработчика ещё не доказывает видимый удар или попадание.

## Установка

Замени/добавь в `D:\INCUBATOR\ArcanaAIOverhaul` ровно три файла:

| Файл в этой папке | Куда положить |
|---|---|
| [src/AIQueueTrace.cpp](src/AIQueueTrace.cpp) | `src\AIQueueTrace.cpp` — заменить |
| [src/Stage38CallbackWindow.inl](src/Stage38CallbackWindow.inl) | `src\Stage38CallbackWindow.inl` — новый |
| [ArcanaAI.ini](ArcanaAI.ini) | `ArcanaAI.ini` — заменить |

Сохрани рабочие `Stage35ConsumerTrace.inl`, `Stage36ControlPredicate.inl` от Stage36 r2 и `AITrace.cpp` от Stage29. Остальной проект и Build.bat без изменений.

1. Архивируй старые логи.
2. Запусти обычный **x86 Build.bat**. При ошибке пришли Build.log; старую DLL не считать новым прогоном.
3. Установи полученную `build\dinput8.dll` и новый INI.
4. Естественный COM-vs-COM, без принудительных входов/команд. Перед закрытием дай журналам сброситься не менее двух секунд.
5. Загрузи в main Build.log и свежие журналы, особенно `ArcanaAI_Stage38CallbackWindowTrace.log`, Stage37, EventDispatch, Stage36, Queue, ParentFlow, FinalCommand, Stage35.

Ожидаемые маркеры:

```text
STAGE38_CALLBACK_WINDOW_BUILD=2026-11-10-r1
STAGE38_INSTALLED 2/2
STAGE38_CALLBACK_WINDOW seq=... callback=... callbackAfter=... guardedCall=... preMask=0x3FFFFFFF postMask=0x3FFFFFFF ...
```

`WARNING: Stage38 dropped=... pendingOverflow=... pairMismatch=...` означает ограничение достоверности; защиты при ошибке установки не отключать.

[Статическое исследование, поля и проверки](../Stage38_Analysis/STATIC_ANALYSIS.md).
