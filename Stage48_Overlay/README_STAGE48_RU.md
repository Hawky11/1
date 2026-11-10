# Stage48 — установка и runtime-прогон (Windows x86)

**Stage47 runtime подтверждён; старый Stage47 повторять отдельно не нужно.**
Stage48 SOURCE READY; настоящие Windows-сборка и игровой запуск Stage48 пока НЕ проверены.

## Локальные файлы

В `D:\\INCUBATOR\\ArcanaAIOverhaul\\src` замени **два** файла и добавь **один**:

1. ЗАМЕНИ `Stage38CallbackWindow.inl` из [src/Stage38CallbackWindow.inl](src/Stage38CallbackWindow.inl).
2. ЗАМЕНИ `Stage44ActionDefinition.inl` из [src/Stage44ActionDefinition.inl](src/Stage44ActionDefinition.inl).
3. ДОБАВЬ `Stage48VariantBinder.inl` из [src/Stage48VariantBinder.inl](src/Stage48VariantBinder.inl).

**ОБЯЗАТЕЛЬНО СОХРАНИ** актуальные Stage46 `Stage45DefinitionResolver.inl` и Stage47 `Stage47MotionConsumer.inl`. Stage43 `Stage42ActivationHook.inl`, Stage41 `Stage40RequestConsumer.inl`, Stage39, AIQueueTrace.cpp, Stage35/36, Stage29 AITrace.cpp, Build.bat, INI тоже не меняй. Не откатывай Stage45/46, иначе нарушится вложенная трассировка.

Три Stage48 файла рассчитаны на ту же уже работавшую конфигурацию в `D:\\INCUBATOR\\ArcanaAIOverhaul`.

## Прогон

Архивируй старые журналы, запусти x86 `Build.bat`.
Устанавливай свежую `build\\dinput8.dll` **только** при `BUILD SUCCEEDED`.
Сделай обычный бой COM-vs-COM с естественными сменами приёмов/кадров, без принудительных ID и записей в память. После выхода оставь не менее 2 секунд для flush worker.

## Загрузка на GitHub

Залей в `main` свежие `Build.log`, новый
**`ArcanaAI_Stage48VariantBindingTrace.log`**, и Stage47/46/45/44/43/42/41/40/39/38/37/36, Queue, EventDispatch, ParentFlow, FinalCommand и Stage35 по наличию.

Проверка: STAGE48_INSTALLED 2/2, корректный callRVA (`001E8DA8`, `001E908F`, прежний `001E9E2D`), pre/post variant и definition, nested Stage45 resolver, дальнейшие Stage47 consumer calls, и нулевые health counters.
Если один путь не встретился, это не автоматическая ошибка: проверь, выполнялась ли нужная ветка игры. Не заявляй реальную анимацию или качество ИИ по этим данным.

Полный [статический анализ](../Stage48_Analysis/STATIC_ANALYSIS.md), [проверочный скрипт](../Stage48_Analysis/tools/verify_stage48.py).
