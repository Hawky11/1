# Stage47 — инструкция для Windows (COM vs COM)

**Stage46 уже успешно пройден. Его повторять не нужно.** Stage47 пока
подготовлен только как исходники и статические проверки. Новая DLL ещё
не проверена реальной Windows x86-сборкой и запуском игры.

## 1. Файлы для замены

В `D:\\INCUBATOR\\ArcanaAIOverhaul\\src`:

1. **ЗАМЕНИ** ровно `Stage38CallbackWindow.inl` файлом
   [Stage47_Overlay/src/Stage38CallbackWindow.inl](src/Stage38CallbackWindow.inl).
2. **ДОБАВЬ** новый `Stage47MotionConsumer.inl` из
   [Stage47_Overlay/src/Stage47MotionConsumer.inl](src/Stage47MotionConsumer.inl).

**СОХРАНИ** уже работающие Stage46 версии
`Stage44ActionDefinition.inl` и `Stage45DefinitionResolver.inl`.
Не откатывай их на Stage44/45! Сохрани также Stage43
`Stage42ActivationHook.inl`, Stage41 `Stage40RequestConsumer.inl`,
Stage39, `AIQueueTrace.cpp`, Stage35/36, Stage29 `AITrace.cpp`,
`ArcanaAI.ini`, `Build.bat` без изменений.

Проверочные эталоны Stage46 расположены в [Stage46_Overlay/src](https://github.com/Hawky11/1/tree/stage46-definition-motion-state-trace/Stage46_Overlay/src).
Оригинальный `AALib.exe` уже есть в библиотеке ChatGPT; повторно
загружать его не нужно.

## 2. Сборка и запуск

Перед прогоном убери старые логи в архив. Выполни обычный локальный
`Build.bat` (Windows x86). Проверяй `BUILD SUCCEEDED` и реальный
размер новой DLL. **При любой ошибке не устанавливай новую DLL** и
сохрани прежнюю рабочую сборку. После успешной сборки установи свежую
`dinput8.dll` и проведи естественный бой COM-vs-COM с движущимися
персонажами; без принудительных вводов, подмены ID или Cheat Engine.
После завершения подожди не менее 2 секунд для flush worker.

## 3. Выгрузка

Залей на GitHub в `main` новый `Build.log` и
**`ArcanaAI_Stage47MotionConsumerTrace.log`**, вместе с Stage46/45/44,
Stage43/42/41/40/39/38/37/36 и Queue/EventDispatch/ParentFlow/
FinalCommand/Stage35, если файлы появились.

## 4. Что проверять

Требуются:
- `STAGE47_MOTION_CONSUMER_BUILD=2026-11-10-r1`
- `STAGE47_INSTALLED 2/2 callsites=AE70C,AE93E native=1E66E0`
- наличие строк `STAGE47_CONSUMER_WINDOW` у P1 и P2
- `callRVA` равен `000AE70C` или `000AE93E`, `argument=00000000`
- полные `preMask/postMask=1FFF` и `extraMask=F` при доступном definition
- здоровье `dropped=overflow=mismatch=badCaller=0`.

Если `STAGE47_INSTALLED` не появился либо новых окон нет,
НЕ считай это автоматически проблемой игры — зафиксируй маркер/
счётчики/способ запуска. Нельзя объявлять доказанными
анимации/попадания/урон или улучшение ИИ на основании Stage47-снимков.
