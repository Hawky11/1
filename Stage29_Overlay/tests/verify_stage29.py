#!/usr/bin/env python3
"""Stage29 static source/INI checks; no game runtime or compiler required."""
import argparse
import pathlib
import re

EXPECTED = {
    "marker": "STAGE29_BIT17_TRANSITIONS_BUILD=2026-11-09-r1",
    "events": '"STAGE29_BIT17_TRANSITION"',
    "log": 'ArcanaAI_Stage29Bit17Transitions.log',
    "mask": "((static_cast<DWORD>(oldValue) ^ newValue) & kPlayerBit17Mask) == 0u",
    "filter_counter": 'g_stage29Suppressed',
    "before_filter_shadow": 'InterlockedExchange(\n            &g_playerBit17ShadowValue[sideIndex]',
    "dropped": "WARNING: Stage29 write-record buffer dropped",
    "startup": "STAGE29 active.",
    "stack": "(g_stage26ObjectBit0WriteTrace || g_stage29Bit17TransitionsOnly)",
    "config": 'ReadIniInt(L"AIResearch", L"EnableStage29Bit17TransitionsOnly", 0)',
}
FLAGS = {
    "EnablePlayerBit17WriteTrace": "1",
    "EnableStage29Bit17TransitionsOnly": "1",
    "EnableStage26ObjectBit0WriteTrace": "0",
    "EnableControlFieldTrace": "0",
    "EnableStage14ProducerCallTrace": "1",
    "EnableStage15BranchTrace": "1",
    "EnableStage17Bit113SetterCallTrace": "1",
    "EnableStage21PayloadConsumerTrace": "1",
    "EnableStage22PayloadPostAndTrace": "1",
    "EnableStage23ResolverResultTrace": "1",
    "EnableStage24ResolverConsumerTrace": "1",
    "EnableStage25SecondaryObjectFlagTrace": "1",
}

def verify(source, ini):
    issues=[]
    for label,pattern in EXPECTED.items():
        if pattern not in source: issues.append("Missing source invariant "+label)
    for name,value in FLAGS.items():
        matches = re.findall(r"(?m)^"+re.escape(name)+r"\s*=\s*([^\s;]+)",ini)
        if matches != [value]: issues.append(name+" expected ["+value+"] got "+repr(matches))
    shadow_pos=source.find("&g_playerBit17ShadowValue[sideIndex]")
    filter_pos=source.find("if (g_stage29Bit17TransitionsOnly && oldKnown")
    sequence_pos=source.find("const LONG sequence = InterlockedIncrement(&g_playerBit17WriteSequence);")
    if not (0<=shadow_pos<filter_pos<sequence_pos):
        issues.append("Shadow must be updated before Stage29 filter and before sequence reservation")
    if source.count("const LONG reported = InterlockedCompareExchange(&g_stage26SuppressedReported")!=0:
        issues.append("C4456 shadowed Stage26 reported variable was not fixed")
    return issues

def main():
    p=argparse.ArgumentParser()
    p.add_argument("--source",type=pathlib.Path,required=True)
    p.add_argument("--ini",type=pathlib.Path,required=True)
    args=p.parse_args()
    errors=verify(args.source.read_text(encoding="utf-8"),
                  args.ini.read_text(encoding="utf-8"))
    if errors: raise SystemExit("\n".join("FAIL: "+s for s in errors))
    print("STAGE29 STATIC CHECKS PASSED: %d code invariants, %d INI flags" %
          (len(EXPECTED),len(FLAGS)))

if __name__ == "__main__":
    main()
