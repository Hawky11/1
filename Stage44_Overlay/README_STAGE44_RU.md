# Stage44 — native action-definition binding (SOURCE ONLY; Windows x86 pending)

## Проверенный Stage43

Свежий [Stage43 runtime 09526f3](https://github.com/Hawky11/1/commit/09526f3edebd3057bad4a524cbaa75509a7fbfd6): Windows x86 BUILD SUCCEEDED, DLL 1041408 байт; Stage43 ENABLED, Stage42 INSTALLED 1/1. 232/232 записей Stage43 совпали с Stage42 по seq/side/player/object/TID/QPC/ID. Каждая имела nativeReturnRVA=1E9F67, callerRead=1, knownCaller=1, а original callerRVA=AD75C (P1 114, P2 118). Stage42 232/232 переносов ожидающего O+64 в текущий O+60; 30 HEALTH сообщений: calls=2090, otherObjects=1858, dropped=overflow=mismatch=0. Stage41 295 окон: 232 NEW/REQUEUE и 63 DUPLICATE; все 232 новых команды получили ровно одну активацию Stage42, а дополнительных активаций в этом прогоне не было.

Отчёт и оригинальные пользовательские логи находятся в [stage43-runtime-activation-origin-evidence](https://github.com/Hawky11/1/tree/stage43-runtime-activation-origin-evidence).

## Что проверит Stage44

Исходный AALib.exe SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba проанализирован по оригинальным PE байтам:
- RVA 1E9E10 записывает O+64 в O+60 (активный ID);
- RVA **1E9E2D** вызывает native **1E8490** (CALL bytes E8 5E E6 FF FF, callee prologue 55 8B EC 83 EC 0C 53 56 57 8B F1);
- RVA 1E84B3 внутри 1E8490 вызывает 1DF2D0 с O+60 (текущий ID) и O+6C (вариант). Native 1DF2D0 возвращает адрес описания действия или fallback и очищает два аргумента RET 8;
- RVA 1E84B8 сохраняет указатель в **O+34**, RVA 1E84BB устанавливает byte **O+26D=1**, RVA 1E84D5 переносит поле из адреса O+34 плюс 2C в **O+178**.

Stage44 добавляет **ОДИН** строго проверяемый CALL wrapper на 1E9E2D->1E8490, выполняющий native ровно один раз и сохраняющий GPR/EFLAGS/x87/SSE. Он использует существующий Stage38 worker, не создает новые потоки, не подменяет ID, inputs, таблицы и game-state. Логирует только O=[P1+4]/[P2+4], остальные объекты только учитывает в HEALTH.

Новые поля до/после native: O+60 active ID, O+34 action-definition pointer, O+26C DWORD с байтом готовности +26D, O+178 metadata type, O+6C variant, O+08/+0C flags, O+48 class; также безопасные masked reads definition+20/+2C/+48 по указателю O+34. Не путать определение действия с реальной анимацией, попаданием или уроном.

## Установка — ДВА файла

В D:\INCUBATOR\ArcanaAIOverhaul\src:

1. **Заменить** [Stage38CallbackWindow.inl](src/Stage38CallbackWindow.inl) версией Stage44.
2. **Добавить** [Stage44ActionDefinition.inl](src/Stage44ActionDefinition.inl).

**Все остальные исходники оставить из работающего Stage43:** Stage42ActivationHook.inl с Stage43 origin log, Stage40RequestConsumer.inl (Stage41), Stage39ActionOutput.inl, AIQueueTrace.cpp, AITrace.cpp Stage29, Stage35/36 inls, ArcanaAI.ini, Build.bat.

Архивировать предыдущие логи. Запустить Windows x86 Build.bat. Только если Build.log содержит BUILD SUCCEEDED, установить свежую build/dinput8.dll. Провести естественный COM-vs-COM без искусственных ID, флагов или записей в память. Дать не менее двух секунд для worker flush. Залить в main новые Build.log, ArcanaAI_Stage44ActionDefinitionTrace.log и Stage43/42/41/40/39/38/37/36 + Queue, EventDispatch, ParentFlow, FinalCommand, Stage35.

## Критерии runtime

Ожидаются STAGE44_ACTION_DEFINITION_BUILD=2026-11-10-r1 и STAGE44_INSTALLED 1/1 callsite=1E9E2D native=1E8490, далее STAGE44_BINDING_WINDOW. Сверить поля и время Stage44 внутри Stage42 одного и того же object/ID, полные masks=FF, definition metadata masks, изменение O+34, +26D и O+178, нулевые dropped/overflow/mismatch в STAGE44_HEALTH. Не требовать одинакового seq между Stage42 и Stage44: это разные счетчики. Указатель может вести в fallback, и ненулевой адрес НЕ доказывает полноценного действия.

Stage44 пока SOURCE ONLY. Проверены оригинальные SHA-прикреплённые инструкции и статическая структура исходника; Windows x86 компиляция/установка/runtime ещё не проверены. Исходный EXE уже находится в Library; повторно его не просить. Исторические ветки сохранить.
