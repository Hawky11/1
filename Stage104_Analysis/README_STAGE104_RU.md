# Stage104 — проверка native Mode0 (source-only)

Оригинальный путь `F7CC0 → F2DE0(mode 0)`, далее `F7DA3 → CEA20` теперь покрыт двумя подготовленными пассивными CALL-перехватами. Оригинальный Stage93 наблюдал только mode1 в F7C5D. Важный момент: перед F7CC0 оригинальный код прибавляет к EDI `0x3A2C`; новый наблюдатель корректно вычисляет исходный player.

**Windows DLL не собиралась, в игре хуки не устанавливались.** Исходник и PowerShell-установщик находятся в доставленном через чат ZIP `ArcanaAI_Stage104_Mode0_CEA20_Passive_SourceOnly.zip` (SHA256 `d55f025ee38c4b066c37a076f086654d63006a75f16940d9702367fe8ed51b7e`). ZIP включает `ApplyStage104.ps1`, `RollbackStage104.ps1`, два .inl, Python verifier/correlator/tests и подробную RU-инструкцию. Бинарник AALib.exe в ZIP и репозиторий не добавлялся.

Порядок: сохранить рабочую Stage98 r2 DLL (2051584 байта); закрыть игру; запустить `powershell -ExecutionPolicy Bypass -File .\ApplyStage104.ps1 -Root "D:\INCUBATOR\ArcanaAIOverhaul"`; собрать x86 через прежний Build.bat; проверить Build.log. При несовпадении SHA не применять вручную. В игре искать `STAGE104_INSTALLED 2/2`, `STAGE104_MODE0`, `STAGE104_CEA20`, HEALTH с нулевыми ошибками. Затем сохранить в GitHub логи одного запуска: Stage104, Stage94, Stage93, Stage98, ArcanaAI.log и Build.log. Не делать искусственных команд и не включать Stage36. Следующий шаг — анализ фактического runtime.

Оригинальный образ подтверждён SHA, 39/39 opcode+CFG; исходники 25/25; офлайн-тесты 25/25. **Это не проверка сборки под MSVC и не доказательство улучшения AI.**