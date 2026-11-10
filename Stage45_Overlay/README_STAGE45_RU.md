# Stage45 — поиск описания действия: таблица или fallback

Исходники готовы. Windows x86 сборка и runtime ещё НЕ проверены.

## Установка

В `D:\INCUBATOR\ArcanaAIOverhaul\src` замените/добавьте ровно два файла:

1. Заменить `Stage38CallbackWindow.inl` файлом из `Stage45_Overlay/src`.
2. Добавить `Stage45DefinitionResolver.inl` из той же папки.

Сохранить рабочий `Stage44ActionDefinition.inl`, Stage43-версию
`Stage42ActivationHook.inl`, Stage41-версию `Stage40RequestConsumer.inl`,
`Stage39ActionOutput.inl`, `AIQueueTrace.cpp`, Stage35/36 inl, Stage29 `AITrace.cpp`,
`ArcanaAI.ini` и `Build.bat`.

Архивировать старые логи, запустить обычный x86 `Build.bat`. Только при
`BUILD SUCCEEDED` установить свежий `build\dinput8.dll`. Провести естественный
COM-vs-COM прогон, дать рабочему потоку минимум 2 секунды на сброс журналов.

Загрузить `Build.log`, новый `ArcanaAI_Stage45DefinitionResolverTrace.log`,
логи Stage44/43/42/41/40/39/38/37/36, Queue, EventDispatch, ParentFlow,
FinalCommand и Stage35 в main.

## Что проверяем

Новый пассивный hook `1E84B3 -> 1DF2D0` фиксирует ID/вариант, таблицу поиска
и фактический результат. Он связан с соответствующей записью Stage44 через
`bindingQpcBefore`. Маркер `STAGE45_DEFINITION_RESOLVER_BUILD=2026-11-10-r1`,
установка `STAGE45_INSTALLED 1/1`. Здоровье: dropped/overflow/mismatch = 0.

Путь `path=3` — расчёт по таблице, `path=1/2` — условия fallback,
`path=0` — недостаточно данных для предсказания. Сверяем `expected` с
`nativeEAX`, а затем с Stage44 O+34. Ненулевой указатель сам по себе
не является доказательством правильности выбранного действия или улучшения ИИ.

Точная проверка EXE уже показала: в прошлом Stage44 прогоне ни один из 321
результатов не совпал с резервным указателем 0E6E49E0. Stage45 проверяет
сам процесс поиска и позволяет разбирать будущие fallback/несовпадения.
29 привязок вне Stage42 пока не имеют установленного upstream caller.
