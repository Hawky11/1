# Stage52 — установка двойного пассивного наблюдателя AEF10/AED90

**Статус: SOURCE READY.** Это исходники, а не собранная или испытанная Stage52 DLL. Два новых перехвата не изменяют AI, не отправляют команды и не записывают координаты: оба вызывают оригинальные функции ровно один раз и снимают снимки P1/P2 до/после.

## Что изменять

В `D:\INCUBATOR\ArcanaAIOverhaul\src`:

1. **ЗАМЕНИ ТОЛЬКО** [Stage38CallbackWindow.inl](src/Stage38CallbackWindow.inl) — адаптер от уже работающей Stage51 с подключением и запуском Stage52.
2. **ДОБАВЬ** [Stage52PreGlobalCoordinate.inl](src/Stage52PreGlobalCoordinate.inl) — новый диагностический файл.

**ОСТАВЬ КАК ЕСТЬ** рабочие `Stage49LinkedMotion.inl` **r2**, `Stage50PreLinkedCoordinate.inl` **r1**, `Stage51GlobalCoordinate.inl` **r1**, все Stage35–48, `AITrace.cpp`, `AIQueueTrace.cpp`, `ArcanaAI.ini`, `Build.bat`. Не подменяй всю папку `src`!

## Что измеряем

- `AF10D CALL AEF10` — до/после первой функции.
- `AF112 CALL AED90` — до/после второй функции (оригинальная AED90 содержит записи O+7C/+80/+AC/+B0).
- Следом уже перехваченные `AF119→AE220` (Stage51), `AF136→AE100` (Stage50), `AF13D→1E9170` (Stage49 r2).

Снимки у обеих сторон по 13 DWORD: ID O+60, X O+7C, Y O+80, anchor X O+AC, anchor Y O+B0, O+3C/+1A4/+18/+24/+2F0/+88/+8C/+10. Содержат `site`, `callRVA`, `side`, `object`, QPC, маски `1FFF`, `changedMask`, `f0..f12`. Оба оригинальных вызова без аргументов, return `RET`, x86 FPU/SSE сохраняется.

## Запуск

1. Сохрани старые логи в архив. Если есть локальный оригинальный `AALib.exe` или `AALib.zip`, можешь выполнить статический verifier (необязательно для самой компиляции):

```powershell
py -3 Stage52_Analysis\tools\verify_stage52.py --aalib 'D:\path\AALib.zip'
```

2. Собери обычным Windows x86 `Build.bat`. **Если нет `BUILD SUCCEEDED`, ничего в папке игры не меняй**, а загрузи `Build.log`.
3. Только после успешной сборки установи новый `build\dinput8.dll` на место предыдущей (с резервной копией). Запусти естественный бой COM-vs-COM без ручных манипуляций. По завершении оставь процесс минимум на 2 секунды для сброса логов.
4. Залей в `main`: `Build.log`, **`ArcanaAI_Stage52PreGlobalCoordinateTrace.log`**, свежие Stage51/50/49/47/48/46/44 и прочие диагностические логи.

## Ожидаемые маркеры

```text
STAGE52_PRE_GLOBAL_COORD_BUILD=2026-11-10-r1
STAGE52_INSTALLED 2/2 AF10D->AEF10 and AF112->AED90
STAGE52_BOUNDARY_WINDOW ... site=AEF10 ... preMask=1FFF postMask=1FFF
STAGE52_BOUNDARY_WINDOW ... site=AED90 ... preMask=1FFF postMask=1FFF
STAGE52_HEALTH callsAEF10=... callsAED90=... dropped=0 overflow=0 mismatch=0 badCaller=0
```

Проверим, где именно появляются 435 изменённых X между Stage47 и Stage51 — внутри AEF10, внутри AED90 или до них. **Не ожидаем от этого патча улучшений боевого AI**: задача диагностическая. Если появятся падения, нестабильность или необычное поведение, верни прежний `dinput8.dll` и сохрани логи.

[Статический анализ](../Stage52_Analysis/STATIC_ANALYSIS.md) · [verifier](../Stage52_Analysis/tools/verify_stage52.py) · [Stage51 runtime](https://github.com/Hawky11/1/tree/stage51-runtime-global-coordinate-evidence)
