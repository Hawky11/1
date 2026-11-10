# Stage57 — инструкция установки и теста

**Статус: исходники готовы, сборка Windows x86 и runtime ещё не проверены.** Если Stage57 даст сбой, верни резервную рабочую DLL Stage56 (1 127 936 байт).

## Задача

Stage56 обнаружил 1 337 отличий X внутри ACDF0. Прямая запись X по `ACF2A` исполнялась 593 раза и изменила X в 272 событиях; 64 изменённых окна имеют дополнительное изменение после этой записи, 1 048 изменённых окон вообще не содержат Stage56-перехвата. Stage57 наблюдает три вызова внутри ACDF0:

- ранний `ACE11 → A2C40` (до кандидатов прямых записей);
- завершающий `ACFD6 → 15CEC0`;
- непосредственно следующий `ACFDB → 15B8A0`.

Обработчики только снимают сырые P1/P2 значения до/после каждого исходного вызова. Игровые команды, AI-решения и координаты намеренно не меняются.

## Два файла для установки

1. Сохрани текущую **рабочую Stage56** `build\dinput8.dll` отдельно.
2. ЗАМЕНИ **только** `D:\INCUBATOR\ArcanaAIOverhaul\src\Stage38CallbackWindow.inl` на [Stage57 adapter](../Stage57_Overlay/src/Stage38CallbackWindow.inl).
3. ДОБАВЬ `D:\INCUBATOR\ArcanaAIOverhaul\src\Stage57ACDF0HelperBoundary.inl` из [Stage57 observer](../Stage57_Overlay/src/Stage57ACDF0HelperBoundary.inl).
4. **Ничего больше не заменяй:** сохрани `Stage56ACDF0Writer.inl`, `Stage55EarlierMotionCoordinate.inl`, `Stage54PreParentCoordinate.inl` r2 и все Stage47–53 исходники, INI и Build.bat.
5. При наличии исходного приватного EXE можно выполнить проверку из **полной ветки Stage57**:

```powershell
py -3 Stage57_Analysis\tools\verify_stage57.py --aalib "ПУТЬ\К\AALib.exe" --repo-root "ПУТЬ\К\КЛОНУ_Stage57"
```

Оригинальный `AALib.exe` **не загружай в публичный репозиторий**.

6. Запусти обычный `Build.bat`. Только при строке **`BUILD SUCCEEDED`** устанавливай полученную DLL в игру. При ошибке сборки — не ставь.

## Тест COM-vs-COM

Обычный бой ~30–60 секунд. После завершения дай логам сброситься. В новом `ArcanaAI_Stage57ACDF0HelperBoundaryTrace.log` ожидаются:

- `STAGE57_ACDF0_HELPER_BUILD=2026-11-10-r1`;
- `STAGE57_INSTALLED 3/3 ACE11->A2C40 ACFD6->15CEC0 ACFDB->15B8A0`;
- `STAGE57_HELPER_WINDOW`, где `callRVA` равен `000ACE11`, `000ACFD6` или `000ACFDB`, с `f1=...` (X), `f2=...` (Y), `preMask/postMask=1FFF`;
- `STAGE57_HEALTH` без `missingObjects`, `dropped`, `overflow`, `mismatch`, `badCaller`.

Также должны остаться `STAGE56_INSTALLED 2/2`, `STAGE55_INSTALLED 3/3`.

## Залить в GitHub с одного прогона

- `Build.log`;
- **`ArcanaAI_Stage57ACDF0HelperBoundaryTrace.log`**;
- **`ArcanaAI_Stage56ACDF0WriterTrace.log`**;
- **`ArcanaAI_Stage55EarlierMotionCoordinateTrace.log`**;
- `ArcanaAI_Stage47MotionConsumerTrace.log`, `ArcanaAI_Stage54PreParentCoordinateTrace.log`;
- при наличии Stage52/53 и остальные логи этого же запуска.

Мы проверим для каждого Stage55 окна ACDF0, в каком именно из трёх Stage57 вызовов меняется X, и объясняются ли расхождения, не охваченные Stage56. Сопоставление — по стороне, объекту, потоку и вложенным границам QPC, а не по общему числу изменений.

Если перехват Stage57 не установился, лог содержит `ERROR`, игра вылетела или изменилось поведение — прекрати тест с новой DLL, верни Stage56 и загрузи диагностический лог.
