# Stage53 — пассивный снимок родительской функции AF0A0

**SOURCE READY. Windows x86 BUILD и runtime ещё не проверены.** Этот этап не меняет AI: наблюдает объект P1/P2 непосредственно на входе/выходе оригинальной функции AF0A0, вызывая оригинал **один раз**.

## Какие файлы поставить

В `D:\INCUBATOR\ArcanaAIOverhaul\src`:

1. **ЗАМЕНИ ТОЛЬКО** [Stage38CallbackWindow.inl](src/Stage38CallbackWindow.inl) новым адаптером Stage53.
2. **ДОБАВЬ** [Stage53ParentCoordinate.inl](src/Stage53ParentCoordinate.inl).

**НЕ ТРОГАЙ** работающие `Stage49LinkedMotion.inl` (**r2**), `Stage50PreLinkedCoordinate.inl` (**r1**), `Stage51GlobalCoordinate.inl` (**r1**), `Stage52PreGlobalCoordinate.inl` (**r1**), предыдущие Stage35–48, `ArcanaAI.ini`, `Build.bat`, `AIQueueTrace.cpp` и прочие исходники.

## Как работает Stage53

В SHA-проверенном оригинальном `AALib.exe`: `A2B90 CALL AF0A0`, оригинальные байты `E8 0B C5 00 00`. Перед этим стоят `A2B81→A1B80`, `A2B86→CFB80`, `A2B8B→BAB60`; после — `A2B95→CFA90`. Внутри AF0A0 выполняются Stage52 AEF10/AED90 и Stage51/50/49.

Снимаются 13 DWORD объекта (те же индексы, что Stage51/52), QPC до/после, P1/P2 адрес объекта, 4096-slot ring, маски и счётчики ошибок. Проверим, когда появляются 868 отличий X из последнего прогона: ДО функции AF0A0 либо внутри неё до вызова AEF10.

## Тест

1. Архивируй старые логи. Необязательный локальный static verifier (путь к оригинальному AALib.zip/exe подставь свой):

```powershell
py -3 Stage53_Analysis\tools\verify_stage53.py --aalib 'D:\path\AALib.zip'
```

2. Собери обычным `Build.bat` в MSVC x86. Если нет `BUILD SUCCEEDED`, **не ставь новую DLL** и пришли `Build.log`.
3. Только после успешной сборки замени `dinput8.dll` в игре (с резервной копией). Запусти естественный COM-vs-COM бой и подожди минимум 2 секунды для flush.
4. Залей в `main`: **`Build.log`**, **`ArcanaAI_Stage53ParentCoordinateTrace.log`**, свежие Stage52/51/50/49/47/48/46 и остальные логи.

Ожидаемые маркеры:

```text
STAGE53_PARENT_COORD_BUILD=2026-11-10-r1
STAGE53_INSTALLED 1/1 A2B90->AF0A0
STAGE53_PARENT_WINDOW ... preMask=1FFF postMask=1FFF
STAGE53_HEALTH ... dropped=0 overflow=0 mismatch=0 badCaller=0
```

При зависании или странном поведении верни предыдущую рабочую Stage52 DLL. Анимации, хитбоксы, комбо и улучшение AI этим этапом не реализуются.

[Статический разбор](../Stage53_Analysis/STATIC_ANALYSIS.md) · [verifier](../Stage53_Analysis/tools/verify_stage53.py) · [проверенный Stage52 runtime](https://github.com/Hawky11/1/tree/stage52-runtime-dual-native-evidence)
