# Stage50 — пассивная проверка источника изменений координат до Stage49

**Stage49 r2 установлен и проверен**: свежий Windows x86 прогон 15:32, 6 818 событий, все счётчики потерь нулевые. При этом сама Stage49 функция ни разу не изменила поля в P1/P2: `staticGateCandidate=0` во всех событиях.

Сопоставление Stage47→Stage49 обнаружило **1 201** различие в `O+7C` между Stage47 POST и Stage49 PRE, при полном совпадении активного ID со Stage48. Следующий оригинальный код в том же списке объектов `E2E02C8` вызывает **AF136 → AE100** непосредственно перед AF13D → 1E9170. Статически AE100 действительно содержит `FST/FSTP` в `O+7C/+80/+AC/+B0`, условная ветка зависит от `O+3C==4` или `O+1A4==0x32`. Stage50 измерит эти записи **без изменения поведения игры**.

## В D:\\INCUBATOR\\ArcanaAIOverhaul\\src выполни ровно две операции

1. **ЗАМЕНИ** [Stage38CallbackWindow.inl](src/Stage38CallbackWindow.inl) новым адаптером Stage50.
2. **ДОБАВЬ** [Stage50PreLinkedCoordinate.inl](src/Stage50PreLinkedCoordinate.inl).

**ОСТАВЬ** уже работающий `Stage49LinkedMotion.inl` **из Stage49 r2** (не заменять на r1!). Оставь остальные файлы Stage48/47/46/45/44, Stage43/41, 39/36/35/29, AITrace.cpp, AIQueueTrace.cpp, INI и Build.bat. Не копируй исходники старых веток и не стирай прежние evidence-ветки.

## Компиляция и runtime

1. Архивируй старые логи, выполни обычный Windows x86 `Build.bat`. Если не `BUILD SUCCEEDED`, ничего не устанавливай и загрузи только Build.log.
2. При успешной сборке установи `build\\dinput8.dll`, проведи естественный COM-vs-COM бой, без ручной модификации памяти/действий/режимов.
3. После завершения боя подожди не менее двух секунд на flush. Залей на GitHub в `main` **Build.log**, **ArcanaAI_Stage50PreLinkedCoordinateTrace.log**, и свежие `ArcanaAI_Stage49LinkedMotionTrace.log`, Stage48/47/46/45/44, Stage43/42/41/40/39/38/37/36/35 и Queue/EventDispatch/FinalCommand по наличию.

## Что проверять

- `STAGE50_PRELINKED_COORD_BUILD=2026-11-10-r1` и **`STAGE50_INSTALLED 1/1 callsite=AF136 native=AE100`**.
- Старый маркер Stage49 **r2** и `STAGE49_INSTALLED 2/2`; не допускай отката на r1.
- `STAGE50_PRELINKED_WINDOW`: P1/P2, `callRVA=AF136`, `preMask/postMask=07FF`, `gateCandidate`, `changedMask`, `f0..f10`.
- `f0/f1/f2/f3` соответствуют `O+7C/+80/+AC/+B0`; `f4=O+3C`, `f5=O+1A4`, `f6=O+44`, `f7=O+5C`, `f8=O+10`, `f9=O+08`, `f10=O+24`. Сравни с Stage49 PRE непосредственно после этого вызова по стороне, объекту, QPC и ID Stage48.
- `STAGE50_HEALTH dropped=0 overflow=0 mismatch=0 badCaller=0`; также Stage49/47/44 loss counters нулевые.

**Важные ограничения:** source-only подготовка не подтверждает Windows x86 сборку, установку новых перехватов или runtime. Сам факт изменения числовых float-полей ещё не доказывает экранное движение, попадания, урон или более качественный AI.

[Статический разбор](../Stage50_Analysis/STATIC_ANALYSIS.md) · [SHA-проверка](../Stage50_Analysis/tools/verify_stage50.py) · [Результат Stage49 r2](https://github.com/Hawky11/1/blob/stage49r2-runtime-linked-motion-evidence/Stage49_Runtime/RUNTIME_ANALYSIS.md).
