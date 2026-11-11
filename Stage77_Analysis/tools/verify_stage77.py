#!/usr/bin/env python3
"""Stage77 source-only consistency check. Not a compiler/runtime verifier."""
from pathlib import Path
import hashlib
import sys

root = Path(__file__).resolve().parents[2]
adapter = (root / "Stage77_Overlay/src/Stage38CallbackWindow.inl").read_text(encoding="utf-8")
stage76_path = root / "Stage76_Overlay/src/Stage76AfterCallbackPhases.inl"
assert stage76_path.is_file(), "Stage76 dependency missing"
stage76 = stage76_path.read_text(encoding="utf-8")
assert '#include "Stage76AfterCallbackPhases.inl"' in adapter
assert 'InstallStage76Hooks();' in adapter
assert 'FlushStage76();' in adapter
assert 'StartStage59Watch();' in adapter
assert adapter.count('call dword ptr [g_stage38PrepareTarget]') == 1
assert adapter.count('call dword ptr [g_stage38FinishTarget]') == 0
assert adapter.count('fxsave [edi]') == 3
assert adapter.count('fxrstor [edi]') == 3
assert adapter.count('pushfd') >= 3 and adapter.count('popfd') >= 3
assert adapter.index('call Stage77BeforeNativePrepare') < adapter.index('call dword ptr [g_stage38PrepareTarget]') < adapter.index('call Stage38Prepared')
assert 'constexpr DWORD kS77Offsets[kS77Fields]={0x3F50u,0x3F54u,0x3ED0u,0x3ED4u,0x04u}' in adapter
assert 'constexpr DWORD kS77Stage38Indices[kS77Fields]={30u,31u,3u,4u,0u}' in adapter
for word in ('STAGE77_ENABLED','STAGE77_NATIVE_PREPARE','STAGE77_HEALTH','Stage77TakeNativePrepare','g_s77Depth'):
    assert word in adapter, word
assert 'STAGE76_INSTALLED 3/3' in stage76
print("SOURCE STRUCTURE PASSED: Stage77 native-before sampler and matching Stage76 dependency")
print("Stage77 adapter SHA256:", hashlib.sha256(adapter.encode("utf-8")).hexdigest())
print("Stage76 source SHA256:", hashlib.sha256(stage76.encode("utf-8")).hexdigest())
print("WARNING: Windows/MSVC x86 build and game runtime NOT verified")
