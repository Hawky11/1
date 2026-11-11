# Stage88 SHA-pinned static analysis (2026-11-11)

- Original AALib.exe SHA256: `28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba`.
- `verify_stage88_static.py` PASS: 14 original PE32 instruction signatures, direct `EE306 E8C5740000 -> F57D0`, three selector descriptor rows.
- `EE2D0 CMP EDI,2C` special mapping; `EE2D5 MOV ECX,[ESI+3F50]`; `EE2DB/EE2E3/EE2ED` map 0/1/2 -> EAX 0E/0F/10; `EE2F0 MOV ECX,[ESI+3F34]`; `EE2F6 MOV EDX,[ESI+3F30]`; `EE2FC...EE306` push ECX, EDX, EAX, P+3A08 and call F57D0.
- `F57D0` accesses mapped code's 3-byte table descriptor at `RVA 049DF188 + code*3`. `code=0E/0F/10` all `01 26 FF`.
- `F57EC` if flags bit0 clears P+3A0C and +3A10; `F5800 MOV [P+3A08],esi`; `F5817..F584F` dynamic `0x26` reads P+3F50 and chooses index 9/10/11; `F5856 OR` sets corresponding bit in P+3A0C. Do not assume resulting bit changes from zero; test actual before/after.
- Table byte3 FF means second OR branch skipped. Bit1 of flags zero means no secondary F58C1 path for these rows.
- Static behavior is *controller buffer state*, not visible action/animation/hit. Stage88 no gameplay writes itself.
- Stage24 original wrapper has already replaced EE306 with one call; it invokes original F57D0 once with original four arguments. Stage88 adds only C helper calls to the existing Stage24 pre/post capture, without new hook on EE306.

## Runtime pending

- MSVC x86 compile: NOT RUN in this environment.
- Stage88 hook/bridge execution: NOT TESTED in-game.
- Native 0/1/2 -> bitmap bit 9/10/11 correlation and full coverage: TO CHECK from fresh logs.
- `FinalCommand` to rendered action: NOT PROVEN.