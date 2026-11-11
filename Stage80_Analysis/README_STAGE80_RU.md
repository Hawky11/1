# Stage80 — сузить источник selector внутри позднего вызова F7150

**11.11.2026: SOURCE READY. Windows x86 сборка и игра Stage80 ещё не проверены.**

В Stage79 уже есть **три реальных изменения** selector внутри original `0E89E0`: P1 `0→1` (seq2901), P2 `0→1` (seq3404), P2 `1→0` (seq3740). Все сопровождаются native event `0x11`. [Отчёт Stage79](https://github.com/Hawky11/1/blob/stage79-runtime-common-selector-positive-evidence/Stage79_Runtime/RUNTIME_ANALYSIS.md).

По оригинальному `AALib.exe` три события допускают поздний путь `0E8A8E→0F7150`: в F7150 имеются именно три прямых записи selector — **F7D22** (1/2), **F7D38** (1/2) и **F7D43** (0). Проверяем **один CALL**, не устанавливая три новых MOV-хука и не занимая DR2/DR3. Positive window подтвердит интервал F7150, но ещё не EIP конкретной записи.

## Установка двух файлов

Сначала сохрани проверенную Stage79 DLL **1 918 976 байт** и резервную копию нынешнего `src\Stage38CallbackWindow.inl`.

В `D:\INCUBATOR\ArcanaAIOverhaul\src`:

1. Замени **только** `Stage38CallbackWindow.inl` файлом [Stage80 adapter](../Stage80_Overlay/src/Stage38CallbackWindow.inl).
2. Добавь **только** новый [Stage80F7150LateBoundary.inl](../Stage80_Overlay/src/Stage80F7150LateBoundary.inl).

**Сохрани** текущие `Stage79SelectorFamilyBoundary.inl`, `Stage78LiteralOneWriter.inl`, `Stage76AfterCallbackPhases.inl` и остальные `.inl`, `ArcanaAI.ini` и файлы игры. Новая ветка основана на Stage79, а не на пустом проекте.

Для проверки SHA и оригинальных машинных инструкций (нужно иметь checkout ветки, с её `Stage80_Analysis`):

```powershell
py -3 .\Stage80_Analysis\tools\verify_stage80.py "D:\PATH\TO\AALib.zip"
```

После этого запусти `Build.bat`, дождись **BUILD SUCCEEDED**. Размер новой DLL заранее неизвестен.

## Короткий runtime-тест COM-vs-COM

Убедись, что на старте:
- `STAGE79_INSTALLED 1/1`, `STAGE80_GUARD ... stage79=1 stage78=1 stage76=3 nativeOK=1`, `STAGE80_INSTALLED 1/1 callRVA=0E8A8E targetRVA=0F7150`.
- Stage38 `2/2`, Stage77 enabled, Stage76 `3/3`, Stage78 `1/1`, Stage59 `WATCH_ARMED`.
- `STAGE80_HEALTH ... dropped=0 overflow=0 mismatch=0 readFail=0`, `STAGE79_HEALTH` тоже без ошибок.

Новый лог `ArcanaAI_Stage80F7150LateTrace.log` должен содержать `STAGE80_LATE_F7150` с player/side, QPC, полными масками **`0FFF/0FFF`** (печать `FFF`) и `f0=старый>новый`. Ключевое — связать событие с **тем же игроком и QPC** события Stage79.

Залей **из одной сессии** пять файлов: `Build.log`, `ArcanaAI_Stage80F7150LateTrace.log`, `ArcanaAI_Stage79SelectorFamilyTrace.log`, `ArcanaAI_Stage38CallbackWindowTrace.log`, `ArcanaAI_Stage76AfterCallbackTrace.log`. Stage78 добавь, если он вдруг поймает запись, Stage59 — при проблемах с мониторингом.

Если зависание, crash, ошибки HEALTH, нет корректной установки или поведение отличается — верни сохранённые Stage79 адаптер/DLL и убери `Stage80F7150LateBoundary.inl`. Новый native caller может менять временные характеристики и требует проверки Windows; **никаких улучшений COM AI на этом этапе не заявляем**.
