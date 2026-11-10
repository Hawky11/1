# Stage51 — наблюдение глобального обновления перед Stage50

**Stage50 runtime подтверждён**: 10 ноября 2026, BUILD SUCCEEDED, 5 792 событий P1/P2, STAGE50_INSTALLED 1/1. Ни в одном событии AE100 не меняла координаты (gateCandidate=0). Stage50 POST и Stage49 PRE совпадают по X/Y/AC/B0 во всех 5 790 сопоставленных парах. В 1 257 случаях O+7C уже отличается от предыдущего Stage47 POST, но O+AC ещё содержит предыдущее значение; значит, writer находится **перед AE100**, не внутри AE100/1E9170.

## Установка Windows x86 (только два изменения)

В `D:\INCUBATOR\ArcanaAIOverhaul\src`:
1. **ЗАМЕНИ** [Stage38CallbackWindow.inl](src/Stage38CallbackWindow.inl) — адаптер Stage51.
2. **ДОБАВЬ** [Stage51GlobalCoordinate.inl](src/Stage51GlobalCoordinate.inl).

**Не заменяй** работающий `Stage49LinkedMotion.inl` (строго r2!), `Stage50PreLinkedCoordinate.inl` и существующие Stage48/47/46/45/44/43/41/39/36/35, AITrace.cpp, AIQueueTrace.cpp, .ini, Build.bat. Никаких изменений этих файлов не требуется.

## Что будет измеряться

В оригинальном AALib.exe непосредственная последовательность перед Stage50:

- `AF10D CALL AEF10`
- `AF112 CALL AED90`
- **`AF117 PUSH 1; AF119 CALL AE220`** — глобальный native update с одним DWORD аргументом, очистка оригинальным вызывающим кодом на AF124
- `AF11E MOV ESI,[list C8]`; затем `AF136 CALL AE100` (Stage50), `AF13D CALL 1E9170` (Stage49).

Stage51 перехватывает **только AF119** с оригинальным 5-байтовым CALL, копирует входной аргумент, вызывает AE220 **ровно один раз** с оригинальными регистрами/флагами и оставляет исходную очистку стека нетронутой. До/после AE220 читает P1/P2 object offsets: `f0=O+60`, `f1=O+7C`, `f2=O+80`, `f3=O+AC`, `f4=O+B0`, `f5=O+3C`, `f6=O+1A4`, `f7=O+18`, `f8=O+24`, `f9=O+2F0`, `f10=O+88`, `f11=O+8C`, `f12=O+10`. Отдельный log, 4096-slot ring, штатный worker Stage38, никакой записи в игровой state.

## Запуск

1. Архивируй старые логи. Запусти штатный Windows x86 `Build.bat`; только при `BUILD SUCCEEDED` ставь новую `build\dinput8.dll`. При ошибке сборки загрузи только Build.log.
2. Проведи естественный COM-vs-COM бой, без ручных переключений/записи в память. После боя подожди >=2 секунд для flush.
3. Загрузи в GitHub `main` **Build.log**, **ArcanaAI_Stage51GlobalCoordinateTrace.log**, свежие `ArcanaAI_Stage50PreLinkedCoordinateTrace.log`, `ArcanaAI_Stage49LinkedMotionTrace.log`, Stage47/48/46/44, остальные Stage35–45 и Queue/EventDispatch/FinalCommand по наличию.

Ожидаем в runtime:

```text
STAGE51_GLOBAL_COORD_BUILD=2026-11-10-r1
STAGE51_INSTALLED 1/1 AF119->AE220
```

В `STAGE51_GLOBAL_WINDOW`: ровно P1 и P2, `callRVA=000AF119`, `arg=00000001`, полные маски `1FFF` (13 полей), raw `f0..f12`, `changedMask`, QPC PRE/POST. `STAGE51_HEALTH dropped=0 overflow=0 mismatch=0 badCaller=0 badArg=0`. Сравнить Stage47 POST → Stage51 PRE/POST → Stage50 PRE → Stage49 PRE для того же P1/P2 и объекта по упорядоченному QPC.

**Статус: SOURCE READY**, локальная проверка исходников и оригинальных машинных сигнатур, **не** собран/протестирован под Windows x86. Отдельные изменения raw float не доказывают экранное движение, анимацию, коллизии, урон или качество AI.

[Статический анализ](../Stage51_Analysis/STATIC_ANALYSIS.md) · [verifier](../Stage51_Analysis/tools/verify_stage51.py) · [Stage50 runtime](https://github.com/Hawky11/1/blob/stage50-runtime-prelinked-coordinate-evidence/Stage50_Runtime/RUNTIME_ANALYSIS.md).
