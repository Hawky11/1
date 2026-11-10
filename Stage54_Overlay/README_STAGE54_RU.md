# Stage54 — три вызова до AF0A0, только диагностика

**Stage54 r2 SOURCE READY. Windows x86 BUILD/runtime r2 пока НЕ проверены.** Stage53 показал 1 091 изменений X до вызова `A2B90→AF0A0`. Stage54 отдельно снимает координаты **до и после** трёх более ранних вызовов `A2B81→A1B80`, `A2B86→CFB80`, `A2B8B→BAB60`. Оригинальные функции вызываются ровно по одному разу, без новых игровых решений или записей в координаты.

## Исправление Stage54 r2

Stage54 r1 собрался (1 113 088 байт), но не установился: сравнивал абсолютный адрес внутри оригинального A1B80 без учёта ASLR/PE relocation. Версия r2 правильно проверяет релокацию `A1B82`, не ослабляя проверку всех трёх CALL. **Если адаптер Stage54 уже установлен, менять его не надо: достаточно заменить только Stage54PreParentCoordinate.inl из ветки r2.**

## Установка в D:\INCUBATOR\ArcanaAIOverhaul\src

1. **Заменить только** [Stage38CallbackWindow.inl](src/Stage38CallbackWindow.inl) адаптером Stage54.
2. **Добавить** [Stage54PreParentCoordinate.inl](src/Stage54PreParentCoordinate.inl).
3. **Сохранить без изменений** Stage53ParentCoordinate.inl, Stage52PreGlobalCoordinate.inl, Stage51GlobalCoordinate.inl, Stage50PreLinkedCoordinate.inl, Stage49LinkedMotion.inl (r2), остальные рабочие исходники, ArcanaAI.ini и Build.bat.

## Проверка и запуск

1. При желании запусти verifier с оригинальным AALib.zip: `py -3 Stage54_Analysis\tools\verify_stage54.py --aalib "D:\path\AALib.zip"`.
2. Запусти существующий Build.bat через MSVC x86 и убедись, что в Build.log **BUILD SUCCEEDED**. При ошибке DLL не устанавливать.
3. После резервной копии предыдущей DLL установить новую `dinput8.dll`; запустить COM-vs-COM бой, дать закончить flush, залить свежие Build.log и все логи Stage47–54, в том числе новый `ArcanaAI_Stage54PreParentCoordinateTrace.log`.
4. При зависании или необычном поведении вернуть Stage53 DLL.

Ожидаемые маркеры:

```text
STAGE54_PRE_PARENT_COORD_BUILD=2026-11-10-r2
STAGE54_ASLR_CHECK ... expectedOperand=0E4A1B5C actualOperand=0E4A1B5C readable=1
STAGE54_INSTALLED 3/3 A2B81->A1B80 A2B86->CFB80 A2B8B->BAB60
STAGE54_SIBLING_WINDOW ... callRVA=000A2B81 ... changedMask=...
STAGE54_SIBLING_WINDOW ... callRVA=000A2B86 ... changedMask=...
STAGE54_SIBLING_WINDOW ... callRVA=000A2B8B ... changedMask=...
STAGE54_HEALTH ... dropped=0 overflow=0 mismatch=0 badCaller=0
```

**Цель** — локализовать первый момент записи в X; Stage54 не является патчем, улучшающим AI. [Технический разбор](../Stage54_Analysis/STATIC_ANALYSIS.md) · [Stage53 проверенный runtime](https://github.com/Hawky11/1/tree/stage53-runtime-parent-af0a0-evidence).
