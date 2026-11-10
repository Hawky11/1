# Stage70 r2 — исправление отказа установки из-за PE-релокации

**11.11.2026.** Разобраны свежие логи пользовательского запуска 02:38. Бинарник успешно собран (1 892 864 байта), но Stage70 r1 **не был установлен**: `ERROR: Stage70 prerequisites or signature mismatch; no patch.`

**Причина:** в исходном `AALib.exe` семь байтов по `AALib+15677D` содержат абсолютный адрес `008254E8` для индексированной загрузки таблицы. PE image base исходного EXE — `00400000`; в запуске модуль находится по `00800000`. В таблице `.reloc` оригинального EXE есть запись **HIGHLOW для RVA 156780**, поэтому Windows меняет адрес инструкции на `00C254E8`. Stage70 r1 сравнивал с неизменёнными файловыми байтами и неизбежно отклонял корректный runtime-код.

**Что исправлено:** Stage70 r2 проверяет постоянные байты `8B 14 8D`, отдельно читает `DWORD` по `gameBase+156780` и сверяет его с `gameBase+4254E8`. Контроль `156784: MOV EAX,[EBP+8]`, `156787: MOV [EAX+68],EDX`, перехода на `15678A`, логирование, инструкции replay и DR0/DR1 остаются без изменений. Добавлен диагностический маркер `STAGE70_GUARD` с каждым условием установки и фактическим runtime-адресом.

[Разбор отказа Stage70 r1](../Stage70_R1_Runtime/FAILURE_ANALYSIS.md) · [исправленный исходник](../Stage70_Overlay/src/Stage70DeferredWriter.inl) · [обновлённый статический тест](../Stage70_Analysis/tools/verify_stage70.py).

## Как исправить локальную сборку

Рабочую Stage67 DLL **1 887 232 байта** и предыдущие сборки сохранить для отката. В уже собранном проекте Stage70 r1 **заменить только один файл**:

`D:\INCUBATOR\ArcanaAIOverhaul\src\Stage70DeferredWriter.inl`

Взять его из текущей ветки `stage70-r2-relocation-aware-guard/Stage70_Overlay/src`. Вторая часть Stage70 overlay (`Stage38CallbackWindow.inl`) **не изменилась**: заново заменять её не требуется, если была правильно установлена в r1. `AALib.exe`, `ArcanaAI.ini`, `Build.bat` не менять.

Скачать/переключить соответствующую ветку и выполнить из корня:

```powershell
py -3 .\Stage70_Analysis\tools\verify_stage70.py "D:\PATH\TO\AALib.exe"
py -3 .\Stage70_Analysis\tools\test_stage70.py
```

Оригинал EXE по SHA `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`; верификатор r2 проверяет реальные HIGHLOW-релокации `156780` и `0E9E5C`, три опкодных диапазона, неизменяемость replay и порядок установки.

**Внимание:** в текущей сессии проверка оригинальной `.reloc` и исходных байтов прошла офлайн, но опубликованный полный Python-верификатор, полный x86-дифференциальный тест обёртки и локальная MSVC x86 сборка r2 не выполнены. Перед тестом в игре нужен обязательный review ABI/сборки. Не называть r2 рабочим до подтверждения.

После проверки собрать `Build.bat` через MSVC x86. В новом `ArcanaAI_Stage70DeferredWriterTrace.log` ожидать:

- `STAGE70_DEFERRED_BUILD=2026-11-11-r2`;
- `STAGE70_GUARD x86=1 xmm=1 stage67=2 stage65=1 stage59Started=0 stage70=0 nativeOK=1 ...`;
- `STAGE70_INSTALLED 1/1`;
- `STAGE70_DEFERRED_WRITE` для одного из P1/P2 с `preMask=07 postMask=07` и фактическим `deferred=...>...` соответствующим `valueEDX`;
- `STAGE70_HEALTH` без ошибок.

При `nativeOK=0` или другом сбое **не продолжать игровой тест в качестве валидной Stage70-проверки**. В этом случае достаточно загрузить новый `Build.log` и небольшой `ArcanaAI_Stage70DeferredWriterTrace.log`, остальные файлы не требуются. Если установка и захват удались, минимальная группа для доказательства: **Build.log + Stage70 + Stage65 + Stage41 + Stage67 + Stage59** из одной сессии, без смешения QPC.

Дополнительное наблюдение из проваленного запуска: P2 Stage65 seq73 с исходным caller `156769` запросил `2E`, Stage41 seq73 уже видел deferred `24`, Stage67 seq1133 активировал `2E`, сохранив deferred `24`. Это подтверждает согласованность старой гипотезы, но не является Stage70 writer event.

**Текущее состояние:** Stage70 r2 RELOCATION GUARD SOURCE FIXED / BUILD AND RUNTIME PENDING. Оригинальная Stage67 DLL остаётся эталоном.