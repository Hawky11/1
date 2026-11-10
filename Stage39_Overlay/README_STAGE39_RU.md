# Stage39 — наблюдение за нативным состоянием действия

**Готова только исходная версия. Сборка Windows x86 и Stage39 runtime ещё не проверены.**

Stage38 подтвердил реальные вызовы обработчиков команд. Stage39 читает выходные поля трёх реальных обработчиков (`12EB10`, `123AF0`, `1DA490`) непосредственно до и после их выполнения. Это позволяет отличить изменение запроса действия от простого прохождения callback. Хуков больше не станет: используются два проверенных Stage38 wrapper. Stage39 ничего не записывает в память игры.

## Установка

В `D:\\INCUBATOR\\ArcanaAIOverhaul`:

1. **Заменить** `src\\Stage38CallbackWindow.inl` на [новый файл](src/Stage38CallbackWindow.inl).
2. **Добавить** `src\\Stage39ActionOutput.inl` из [файла](src/Stage39ActionOutput.inl).

Оставить **без изменений** `src\\AIQueueTrace.cpp` от Stage38, `src\\Stage35ConsumerTrace.inl`, `src\\Stage36ControlPredicate.inl` (Stage36 r2), `src\\AITrace.cpp` от Stage29, `ArcanaAI.ini` от Stage38 и `Build.bat`. В INI уже должно быть `EnableStage38CallbackWindowTrace=1`; отдельного Stage39 ключа нет, Stage39 активируется только вместе с Stage38.

Старые логи перед новым прогоном архивировать. Обычный Windows x86 `Build.bat`, проверить `BUILD SUCCEEDED`, затем установить актуальную `build\\dinput8.dll`. Запустить естественный COM-vs-COM, **не подменять** команды, payload, флаги и контроллер. Перед выходом дать рабочему потоку минимум 2 секунды на сброс журналов. Загрузить в `main` `Build.log`, `ArcanaAI_Stage39ActionOutputTrace.log`, `ArcanaAI_Stage38CallbackWindowTrace.log`, `ArcanaAI_Stage37Type1CommitTrace.log`, `ArcanaAI_EventDispatchTrace.log`, `ArcanaAI_Stage36ControlToCommandTrace.log`, Queue, ParentFlow, FinalCommand, Stage35.

Ожидаемые строки:

```text
STAGE38_CALLBACK_WINDOW_BUILD=2026-11-10-r1
STAGE38_INSTALLED 2/2
STAGE39_ACTION_OUTPUT_BUILD=2026-11-10-r1
STAGE39_ENABLED piggyback=Stage38 2/2 targetRVA=(12EB10,123AF0,1DA490) ...
STAGE39_OUTPUT_WINDOW stage38Seq=... callbackRVA=... type=... payload=... preMask=... postMask=... changedMask=... a0=...>... ... a29=...>...
```

Не требовать полной маски для полей актёра, если `P+43B4` равен нулю. Строка Stage39 появляется **только для указанных трёх callback**. Если они не встречались в прогоне, файл может содержать только маркеры; это не означает поломку игры. Изменённое поле состояния действия ещё **не доказывает** видимую анимацию, попадание или ущерб здоровью.

[Статический анализ и схема полей](../Stage39_Analysis/STATIC_ANALYSIS.md). Проверка оригинального EXE:
```powershell
py -3 Stage39_Analysis\\tools\\verify_stage39.py "G:\\Arcana Heart 3 - LS XTEND\\AALib.exe" --stage38-inl Stage39_Overlay\\src\\Stage38CallbackWindow.inl --stage39-inl Stage39_Overlay\\src\\Stage39ActionOutput.inl
```
