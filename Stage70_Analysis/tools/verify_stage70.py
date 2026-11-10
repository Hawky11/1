#!/usr/bin/env python3
"""Stage70 source-only gate: original opcode bytes, integration, DR and ABI invariants.

Usage: py -3 Stage70_Analysis/tools/verify_stage70.py "D:\PATH\TO\AALib.exe"
This does NOT execute compiled x86 hooks or establish runtime writer provenance.
"""
import argparse
import importlib.util
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STAGE69 = ROOT / 'Stage69_Analysis' / 'tools' / 'verify_stage69.py'
SOURCE = ROOT / 'Stage70_Overlay' / 'src' / 'Stage70DeferredWriter.inl'
ADAPTER = ROOT / 'Stage70_Overlay' / 'src' / 'Stage38CallbackWindow.inl'


def load_stage69():
    spec = importlib.util.spec_from_file_location("verify_stage69", STAGE69)
    if spec is None or spec.loader is None:
        raise ValueError("Stage69 verifier missing")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def verify_source(src, adapter, read_rva):
    expected = {
        ('prev', 0x15677D): bytes.fromhex('8b 14 8d e8 54 82 00'),
        ('site', 0x156784): bytes.fromhex('8b 45 08 89 50 68'),
        ('next', 0x15678A): bytes.fromhex('8b 0f 6a ff 81 c1 d0 3e 00 00'),
    }
    for (key, rva), exact in expected.items():
        match = re.search(r'const BYTE ' + key + r'\[\]=\{([^}]*)\};', src)
        if not match:
            raise AssertionError('source guard not found: ' + key)
        actual = bytes(int(s.strip(), 0) for s in match.group(1).split(','))
        assert actual == exact, (key, 'unexpected embedded guard')
        assert read_rva(rva, len(exact)) == exact, (key, 'original EXE mismatch')

    hook = re.search(
        r'__declspec\(naked\)\s+void\s+S70DeferredWriterHook\(\)\s*'
        r'\{\s*__asm\s*\{(.*?)\}\s*\}',
        src, re.S)
    assert hook, 'hook assembly missing'
    body = hook.group(1)
    normalized = re.sub(r'\s+', ' ', body).lower()
    replay = 'mov eax,dword ptr [ebp+8] mov dword ptr [eax+68h],edx'
    assert normalized.count(replay) == 1, 'native pair not replayed exactly once'
    assert normalized.index('call s70enter') < normalized.index(replay) < normalized.index('call s70leave')
    assert normalized.rfind('jmp dword ptr [g_s70resume]') > normalized.index('call s70leave')
    for op in ('pushfd', 'pushad', 'fxsave [edi]', 'fninit', 'cld',
               'fxrstor [edi]', 'popad', 'popfd'):
        assert body.lower().count(op) == 2, op
    assert normalized.count('mov ebx,esp') == 2
    assert normalized.count('sub esp,210h') == 2
    assert normalized.count('and edi,0fffffff0h') == 2
    assert src.count('mov dword ptr [eax+68h],edx') == 1
    assert 'CONTEXT_DEBUG_REGISTERS' not in src
    assert 'SetThreadContext(' not in src
    assert 'AddVectoredExceptionHandler(' not in src
    assert 'RemoveVectoredExceptionHandler(' not in src
    assert 'CONTEXT_CONTROL' in src
    assert '0x156784u' in src and '0x15678Au' in src
    assert 'const bool stopped=S70SuspendPeers(peers,count);' in src
    assert 'if(writable && S70OriginalOK())' in src
    assert 'FlushInstructionCache(GetCurrentProcess(),a,6)' in src
    assert 'InterlockedCompareExchange(&g_s59Started,0,0)!=0' in src
    assert 'InterlockedCompareExchange(&g_s67Installed,0,0)!=2' in src
    assert 'r.NewDeferred!=r.SourceEdx' in src
    assert 'STAGE70_INSTALLED 1/1' in src
    assert 'STAGE70_DEFERRED_WRITE' in src
    assert 'STAGE70_HEALTH' in src
    assert 'kS70Capacity=4096u' in src

    inc65 = adapter.index('#include "Stage65RequestOrigin.inl"')
    inc67 = adapter.index('#include "Stage67NativePaths.inl"')
    inc70 = adapter.index('#include "Stage70DeferredWriter.inl"')
    assert inc65 < inc67 < inc70
    assert adapter.count('InstallStage70Hooks();') == 1
    assert adapter.count('FlushStage70();') == 1
    install = adapter.index('InstallStage70Hooks();')
    assert adapter.index('InstallStage67Hooks();') < install < adapter.index('StartStage59Watch();')
    assert 'if(stage67Ready) InstallStage70Hooks();' in adapter
    assert adapter.index('FlushStage67();') < adapter.index('FlushStage70();')
    return {'guards': len(expected), 'patchBytes': 6, 'replayedOriginalInstructions': 2,
            'hookedDRSlots': 0, 'sourceIntegration': 'PASS',
            'runtimeVerified': False, 'x86DifferentialVerified': False}


def build_rva_reader(path):
    import struct
    blob = path.read_bytes()
    pe = struct.unpack_from('<I', blob, 0x3c)[0]
    n = struct.unpack_from('<H', blob, pe+6)[0]
    opt_size = struct.unpack_from('<H', blob, pe+20)[0]
    opt = pe+24
    sections=[]
    for i in range(n):
        p=opt+opt_size+i*40
        size,rva,rawsize,ptr=struct.unpack_from('<IIII',blob,p+8)
        sections.append((rva,rawsize,ptr))
    def read(rva,nbytes):
        matches=[ptr+rva-start for start,sz,ptr in sections
                 if start<=rva and rva+nbytes<=start+sz]
        if len(matches)!=1: raise ValueError(f'Unmapped RVA: {rva:08X}')
        o=matches[0]
        return blob[o:o+nbytes]
    return read


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('original_exe',type=Path)
    args=parser.parse_args()
    stage69=load_stage69()
    origin=stage69.inspect(args.original_exe)
    src=SOURCE.read_text(encoding='utf-8')
    adapter=ADAPTER.read_text(encoding='utf-8')
    results=verify_source(src,adapter,build_rva_reader(args.original_exe))
    assert origin['candidate_writer_rva']=='156787'
    print('STAGE69 ORIGINAL EXE:', origin['original_sha256'])
    print('STAGE70 SOURCE STATIC PASS:', results)
    print('NOTE: Windows MSVC x86 build, compiled-wrapper differential tests and in-game runtime are PENDING.')


if __name__=='__main__':
    main()
