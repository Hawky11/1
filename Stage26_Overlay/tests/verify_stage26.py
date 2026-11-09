#!/usr/bin/env python3
"""Stage26 static consistency check; not a compiler or runtime test."""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
source = Path(sys.argv[1]) if len(sys.argv) > 1 else root / "src" / "AITrace.cpp"
ini = Path(sys.argv[2]) if len(sys.argv) > 2 else root / "ArcanaAI.ini"
s = source.read_text(encoding="utf-8")
t = ini.read_text(encoding="utf-8")

def need(token):
    assert token in s, f"Missing source safety token: {token}"

for token in (
    "STAGE26_OBJECT_BIT0_WRITE_BUILD=2026-11-09-r1",
    "ArcanaAI_Stage26ObjectBit0WriteTrace.log",
    "kStage26TargetOffset = 0xF8u",
    "kStage26BitMask = 0x00000001u",
    "kPlayerBit17WriteTraceDr7 = 0x00DD0005u",
    "dataWatchModes > 1",
    "g_stage26TransitionsOnly",
    "kStage26BitMask) == 0u",
    "record.StackMask",
    "record.Stack[7]",
    "TraceExceptionHandler",
    "GetThreadContext",
    "SetThreadContext",
    "STAGE26 active.",
):
    need(token)

values = dict(re.findall(r"(?m)^\s*([A-Za-z][A-Za-z0-9]*)\s*=\s*([01])\s*$", t))
assert values.get("EnableControlFieldTrace") == "0"
assert values.get("EnablePlayerBit17WriteTrace") == "0"
assert values.get("EnableStage26ObjectBit0WriteTrace") == "1"
assert values.get("Stage26TransitionsOnly") == "1"
assert values.get("EnableStage25SecondaryObjectFlagTrace") == "1"

p1 = 0x0DCA5EA4 - 0x3F2C + 0xF8
p2 = 0x0DCD0BCC - 0x3F2C + 0xF8
assert (p1, p2) == (0x0DCA2070, 0x0DCCCD98)
assert 0x0E27CCA0 + 0xF8 == 0x0E27CD98
print("PASS: Stage26 source/INI invariants and watchpoint addresses; MSVC/runtime untested")
