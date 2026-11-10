#!/usr/bin/env python3
"""Synthetic Stage70 correlator regression tests; no AALib.exe or game needed."""
import importlib.util
import tempfile
from pathlib import Path

TARGET = Path(__file__).with_name("correlate_stage70.py")
spec=importlib.util.spec_from_file_location("stage70_join", TARGET)
assert spec and spec.loader
m=importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)

HEADER="[2026-11-11 01:30:28] "
ROWS={
  65: "STAGE65_WRITER_WINDOW seq=1 side=P1 player=00001000 object=00002000 tid=123 qpcBefore=100 qpcAfter=110 callerCallRVA=00156769 postRequest=0000002F",
  70: "STAGE70_DEFERRED_WRITE seq=1 side=P1 player=00001000 object=00002000 tid=123 qpcBefore=140 qpcAfter=142 siteRVA=156784 originalStoreRVA=156787 valueEDX=00000025 eaxAfter=00002000 edxAfter=00000025 preMask=07 postMask=07 deferred=FFFFFFFF>00000025 selector=00000001>00000001 objectPtr=00002000>00002000",
  41: "STAGE41_OBJECT_WINDOW stage40Seq=1 side=P1 player=00001000 objectBefore=00002000 objectAfter=00002000 tid=123 qpcBefore=150 qpcAfter=155 request=0000002F o4=00000025>00000025",
  67: "STAGE67_NATIVE_WINDOW seq=1 kind=0 side=P1 player=00001000 object=00002000 tid=123 qpcBefore=200 qpcAfter=210 v1=00000001>00000000 v4=00000025>FFFFFFFF",
}
MARKER={70:"STAGE70_INSTALLED 1/1",67:"STAGE67_INSTALLED 2/2"}


def write(folder, row_overrides=None, header_overrides=None):
    row_overrides = row_overrides or {}
    header_overrides = header_overrides or {}
    for stage, filename in m.FILES.items():
        head=header_overrides.get(stage,HEADER)
        rows=[
            head+"STAGE%d_BUILD=synthetic"%stage,
            head+MARKER[stage] if stage in MARKER else head+"STAGE%d_STARTED"%stage,
            head+row_overrides.get(stage,ROWS[stage]),
            head+f"STAGE{stage}_HEALTH dropped=0 overflow=0 mismatch=0 readFail=0",
        ]
        (folder/filename).write_text("\n".join(rows)+"\n",encoding="utf-8")


def must_fail(folder, message):
    try:
        m.audit(folder)
    except (ValueError, KeyError, AssertionError):
        return
    raise AssertionError("expected rejection: "+message)


def main():
    with tempfile.TemporaryDirectory() as tmp:
        p=Path(tmp)
        write(p)
        result=m.audit(p)
        c=result["counts"]
        assert c["observedNativeSiteWrites"]==1
        assert c["ID25writes"]==1
        assert c["latestStage65from156769"]==1
        assert c["sameActorRequestDeferredCommit"]==1
        assert c["laterNativePromotions"]==1
        write(p,{70:ROWS[70].replace("valueEDX=00000025","valueEDX=00000026")})
        must_fail(p,"wrong source EDX")
        write(p,{70:ROWS[70].replace("preMask=07","preMask=03")})
        must_fail(p,"incomplete read masks")
        write(p,header_overrides={65:"[2026-11-11 01:20:00] "})
        must_fail(p,"mixed capture")
        write(p,{70:ROWS[70].replace("originalStoreRVA=156787","originalStoreRVA=156788")})
        must_fail(p,"unknown store")
        write(p,{70:ROWS[70].replace("tid=123","tid=124")})
        must_fail(p,"alien thread")
        write(p,{65:ROWS[65].replace("callerCallRVA=00156769","callerCallRVA=0015676A")})
        result=m.audit(p)
        assert result["counts"].get("latestStage65from156769",0)==0
        assert result["counts"].get("sameActorRequestDeferredCommit",0)==0
    print("PASS: 1 positive join + 5 strict negatives + 1 nonclaim case")


if __name__=="__main__":
    main()
