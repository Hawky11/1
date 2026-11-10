# Stage46 — применение описания к полям движения

Исходники готовы. Windows x86 сборка и runtime ещё не проверены.

В `D:\INCUBATOR\ArcanaAIOverhaul\src` заменить ровно два файла из
`Stage46_Overlay/src`:

1. `Stage44ActionDefinition.inl`
2. `Stage45DefinitionResolver.inl`

Оставить Stage45-версию `Stage38CallbackWindow.inl` и все остальные рабочие
файлы, включая Stage43-версию Stage42ActivationHook.inl, Stage41-версию
Stage40RequestConsumer.inl, AIQueueTrace.cpp, Stage29 AITrace.cpp, INI и Build.bat.

Архивировать прежние логи. Запустить x86 Build.bat; только после BUILD SUCCEEDED
установить свежий build\dinput8.dll. Естественный COM-vs-COM прогон, затем дать
рабочему потоку минимум 2 секунды для сброса журналов.

Загрузить Build.log, новый **ArcanaAI_Stage46DefinitionMotionTrace.log**, свежие
Stage45/44/43/42/41/40/39/38/37/36, Queue/EventDispatch/ParentFlow/FinalCommand
и Stage35 в main.

Новых hooks нет: расширены снимки уже работающих Stage44/45. Новый журнал
показывает поля объекта после поиска описания и после его обработки движком.
Проверяем выбор ветки по definition+30, изменения float-полей и переход O+70.
Это пока не подтверждение видимой анимации, попаданий или улучшения ИИ.

Ожидаются STAGE46_DEFINITION_MOTION_BUILD=2026-11-10-r1, STAGE46_ENABLED,
resolverCount=1, маски 1FFF/1FFF и F. Потери проверяются по общему HEALTH
Stage44 и Stage45. Если какой-либо параметр отличается, нужен анализ журнала,
а не автоматический повтор прогона.
