# Stage43 — происхождение дополнительных нативных активаций (SOURCE ONLY)

Последний [Stage42 runtime](https://github.com/Hawky11/1/commit/a1096e980b6d40c290741693b6e8bc9116af01c8) **успешно собран и установлен** (Windows x86, DLL 810496 байт, Stage42 1/1). Полный отчёт и оригинальные журналы сохранены в [stage42-runtime-activation-evidence](https://github.com/Hawky11/1/tree/stage42-runtime-activation-evidence).

В Stage42: 356/356 корректных переводов O+64 -> O+60 и bit0 1->0; 350/350 новых Stage41 запросов имеют по одному точному Stage42-совпадению; шесть дополнительных активаций: P1 три раза ID 0x25 (согласуется с веткой O+68), P2 ID 0x24C, 0x24D, 0x24C (источник пока неизвестен). HEALTH: 3492 native calls, 3136 otherObjects, 356 P1/P2, нулевые dropped/overflow/mismatch.

## Что делает Stage43

**Новых хуков не добавляет.** Оставляет ранее проверенный Stage42 CALL at RVA 1E9F62->1E9C70, исполняет оригинальную функцию ровно один раз, сохраняет GPR, EFLAGS, x87/SSE и старые Stage42/41/40 журналы.

Внутри уже существующего Stage42 before-hook извлекает из оригинального стека два адреса возврата. Исходный 1E9F40 — leaf thiscall без локального пролога, поэтому при входе в Stage42 после PUSHFD/PUSHAD из сохранённого ESP по смещениям +4 и +8 читаются:
- return на original 1E9F67 (продолжение функции 1E9F40 после вызова 1E9C70);
- return в вызывающий участок исходного EXE, откуда был вызван 1E9F40.

В новом ArcanaAI_Stage43ActivationOriginTrace.log будут строки STAGE43_ORIGIN_WINDOW с тем же stage42Seq, side, player, object, QPC, ID и дополнительными nativeReturnRVA, callerReturnRVA, callerRVA (CALL site = return RVA minus 5), callerRead и knownCaller. Исходный EXE содержит 14 статически найденных E8 CALL into 1E9F40; source отмечает известные CALL-адреса, но НЕ блокирует неизвестные. Также включает O+30 и O+68 непосредственно перед 1E9C70 для проверки альтернативной ветки.

Данные **не гарантируют** окончательное определение писателя O+64: callerRVA показывает, откуда пришёл вызов обработки, а не обязательно где ранее изменили O+64. Следующий анализ должен сопоставить шесть дополнительных активаций с конкретным origin site и оценить, какую точную границу целесообразно наблюдать после этого.

## Установка: ровно один файл

Скопируй [Stage43_Overlay/src/Stage42ActivationHook.inl](src/Stage42ActivationHook.inl) в папку **D:\INCUBATOR\ArcanaAIOverhaul\src** поверх предыдущего Stage42ActivationHook.inl.

Все остальные файлы оставь от успешно работавшего Stage42:
- src/Stage38CallbackWindow.inl (он уже включает Stage42ActivationHook.inl);
- src/Stage40RequestConsumer.inl (Stage41);
- src/Stage39ActionOutput.inl, src/AIQueueTrace.cpp;
- Stage35/36, Stage29 AITrace.cpp, ArcanaAI.ini, Build.bat и остальные исходники.

Архивируй предыдущие логи. Запусти обычный x86 Build.bat, обязательно проверь BUILD SUCCEEDED. Установи свежую build/dinput8.dll, затем проведи естественный COM-vs-COM бой без ручной инъекции команд или флагов. После боя дай рабочему потоку 2 секунды на flush. Залей в GitHub main Build.log, новые ArcanaAI_Stage43ActivationOriginTrace.log, Stage42/41/40/39/38/37/36, QueueTrace, EventDispatch, ParentFlow, FinalCommandTrace, Stage35 и другие диагностические логи.

Ожидаемые маркеры:
- STAGE42_ACTION_ACTIVATION_BUILD=2026-11-10-r1
- STAGE42_INSTALLED 1/1
- STAGE43_ACTIVATION_ORIGIN_BUILD=2026-11-10-r1
- STAGE43_ENABLED piggyback=Stage42 1/1

Критерии нового прогона: Stage43/Stage42 точное соответствие по stage42Seq, side, O, QPC, ID; nativeReturnRVA=1E9F67, callerRead=1, knownCaller=1 у известных CALL-путей, O+60 post==O+64 pre, Stage42 HEALTH dropped=overflow=mismatch=0. Не требовать повторения именно шести событий, если следующий бой содержит другой набор действий.

**Stage43 пока SOURCE ONLY.** Проверки ABI и инструкций исходного EXE сделаны статически. Windows x86 компиляция и Stage43 runtime ещё не выполнены. Исходный AALib.exe SHA-256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba уже есть в библиотеке; повторно не просить.
