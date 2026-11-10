# Stage49 — файлы для Windows x86 и COM-vs-COM

**Stage48 runtime подтверждён**, повторять его отдельно не нужно.
Stage49 пока SOURCE READY — DLL и игровой запуск нового этапа ещё не проверялись.

В `D:\\INCUBATOR\\ArcanaAIOverhaul\\src` выполни **только две операции**:

1. **ЗАМЕНИ** `Stage38CallbackWindow.inl` — [src/Stage38CallbackWindow.inl](src/Stage38CallbackWindow.inl).
2. **ДОБАВЬ** `Stage49LinkedMotion.inl` — [src/Stage49LinkedMotion.inl](src/Stage49LinkedMotion.inl).

Обязательно оставь **от Stage48**:
`Stage44ActionDefinition.inl`, `Stage48VariantBinder.inl`.
Оставь **от Stage47** `Stage47MotionConsumer.inl`,
**от Stage46** `Stage45DefinitionResolver.inl`, а также рабочие Stage43
`Stage42ActivationHook.inl`, Stage41 `Stage40RequestConsumer.inl`,
Stage39, Stage36/35, Stage29 AITrace.cpp, AIQueueTrace.cpp, Build.bat, INI.
Предыдущие файлы не перезаписывай старыми версиями из архивных веток.

Затем архивируй старые логи, запусти обычный x86 `Build.bat`.
При ошибке сборки **не устанавливай** DLL; пришли новый Build.log.
Если `BUILD SUCCEEDED`, установи новую `build\\dinput8.dll`,
проведи естественный бой COM-vs-COM (не менять память или команды),
подожди >=2 секунд для flush worker.

Загрузи на GitHub в `main`:
`Build.log`, **`ArcanaAI_Stage49LinkedMotionTrace.log`**,
свежие Stage48/47/46/45/44/43/42/41/40/39/38/37/36, Stage35 и
Queue/EventDispatch/ParentFlow/FinalCommand по наличию.

Контроль:
- `STAGE49_LINKED_MOTION_BUILD=2026-11-10-r1`
- `STAGE49_INSTALLED 2/2 callsites=AF13D(listC8),AF1AA(listC0)`
- P1/P2 `STAGE49_LINKED_WINDOW`, кол-во на AF13D и AF1AA.
- `preMask/postMask=07FF` для целого O; `linkPreMask/linkPostMask=7`
  только когда связанный объект действительно доступен.
- `staticGateCandidate` и поля `f0..f3, l0..l2`
  для проверки копирования float-координат.
- `STAGE49_HEALTH` без dropped/overflow/mismatch/badCaller.

Путь AF1AA обходит **другой** условный список E2E02C0, поэтому
его нулевая активность у P1/P2 допустима.
Не объявлять доказанной реальную экранную анимацию или улучшение AI только
по изменениям числовых полей.

[Полный статический разбор](../Stage49_Analysis/STATIC_ANALYSIS.md).
