# Stage65 — пассивный источник записей P+3F4C

**Статус: SOURCE ONLY.** Оригинальный AALib.exe проверен статически, но новый код ещё не собирался в Windows x86 и не запускался в AH3. Это диагностический перехват, который ничего не меняет в логике выбора COM.

[Подробный технический отчёт](../Stage65_Analysis/STATIC_ANALYSIS.md) · [Новый пассивный перехват](src/Stage65RequestOrigin.inl) · [Статическая проверка](../Stage65_Analysis/tools/verify_stage65.py).

## Как устроено

Подменяется ровно одна точка оригинального кода: вход функции AALib+E5BF0. Запросы всех её прямых и косвенных вызывающих участков проходят через общий вход, поэтому не приходится патчить 296 CALL по отдельности. Исходные первые 7 байт выполняются в трамплине; затем выполнение возвращается в оригинальную E5BF7. Оригинальная E5BF0 вызывается **один раз** с теми же двумя аргументами и ECX, что и до перехвата.

В отдельном логе ArcanaAI_Stage65RequestOriginTrace.log сохраняются адрес вызова, ID запроса, mode, player, object, TID, QPC и значения P+3F4C/P+3B7C до и после. Обработчик не добавляет аппаратных DR breakpoint и не мешает DR0/DR1 наблюдателю Stage59.

**Важное ограничение:** альтернативный условный writer E9E30 на этом этапе не перехватывается. Если каких-то запросов нет в Stage65, нельзя считать это потерей команды.

## Установка — строго ДВА файла

В рабочем каталоге D:\INCUBATOR\ArcanaAIOverhaul\src:

1. **Добавить** Stage65_Overlay/src/Stage65RequestOrigin.inl.
2. **Заменить** Stage38CallbackWindow.inl версией из Stage65_Overlay/src/Stage38CallbackWindow.inl, которая основана на проверенном адаптере Stage59 r2.

Всё остальное оставить от текущей рабочей Stage59 r2: Stage59XWriterWatch.inl, Stage58/57/56/55 и остальные Stage42–53 инклюды, AIQueueTrace.cpp, Stage35/36, Stage29 AITrace.cpp, Build.bat, ArcanaAI.ini. Не смешивать этот Stage65 адаптер со старой веткой Stage58/59 r1 и не менять EXE. Сохранить **копию рабочей Stage59 r2 DLL** перед любым тестом.

## Проверка ДО компиляции

С исходным оригинальным AALib.exe локально запустить в PowerShell из корня клона ветки Stage65:

    py -3 .\Stage65_Analysis\tools\verify_stage65.py "D:\PATH\TO\AALib.exe" --source .\Stage65_Overlay\src\Stage65RequestOrigin.inl --adapter .\Stage65_Overlay\src\Stage38CallbackWindow.inl

Программа только читает оригинальный EXE и два файла исходников. Ожидается PASS для SHA/PE32, шести точных участков оригинального кода, структуры перехвата и модельного RET8. **Это не проверка MSVC ABI и не гарантия стабильности DLL.**

## Если переходить к следующей фазе проверки

Только после статического PASS и сохранения рабочего отката запустить существующий x86 Build.bat. Не устанавливать DLL при ошибках компиляции/линковки. Успешная сборка сама по себе не подтверждает работоспособность нового хука.

При коротком обычном COM-vs-COM (без принудительной подмены action ID, входов и флагов) проверять прежде всего:

- Build.log — BUILD SUCCEEDED и фактический размер DLL;
- ArcanaAI_Stage65RequestOriginTrace.log — BUILD, STAGE65_INSTALLED 1/1, реальные P1/P2 STAGE65_WRITER_WINDOW и STAGE65_HEALTH;
- ArcanaAI_Stage40ActionConsumerTrace.log и ArcanaAI_Stage41ObjectCommitTrace.log — те же ID и последовательность;
- ArcanaAI_Stage42ActionActivationTrace.log — последующая активация;
- ArcanaAI_Stage59XWriterWatchTrace.log — прежний Stage59 WATCH_ARMED и HEALTH, без новых конфликтов регистров.

Если новый Stage65 не установился (например, mismatch оригинальных байтов), прежний Stage59 должен стартовать независимо; **не пытаться обходить сигнатуры**. Если возникают падения/заметное замедление, вернуть исходный Stage59 r2 dinput8.dll и сохранить логи ошибок.

В самом Stage65 *нет* ни одного изменения поведения COM. Следующий Stage66 — сопоставление событий writer с активациями и поиск причины выбора команды.
