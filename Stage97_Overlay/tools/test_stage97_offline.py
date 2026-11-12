#!/usr/bin/env python3
"""Static source/PE/tests for Stage97; not a substitute for MSVC or live gameplay."""
import hashlib, importlib.util, pathlib, sys, tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
N=0

def ck(ok,why):
    global N
    if not ok: raise AssertionError(why)
    N+=1

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def run(aalib):
    exe=pathlib.Path(aalib)
    ck(exe.stat().st_size==175556608,'Original AALib.exe size')
    ck(sha(exe)=='28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba','Original fingerprint')
    data=exe.read_bytes()
    ck(data[:2]==b'MZ','PE MZ')
    def b(rva,n):return data[rva-0xC00:rva-0xC00+n]
    checks={0xF77D4:'83 7d f8 ff 8b 75 d4',0xF77DB:'0f 85 bd 00 00 00',
            0xF7181:'89 4d d4',0xF7714:'c7 45 d4 1f 00 00 00',
            0xF7756:'89 45 d4',0xF7749:'8b 55 cc 8d 04 13',
            0xF7756:'89 45 d4',0xF789E:'8b cf e8 8b f5 fe ff',
            0xF7A94:'89 75 e4 eb 42',0xF79F0:'8b f0 75 0c 83 fe 11'}
    for r,h in checks.items():ck(b(r,len(bytes.fromhex(h)))==bytes.fromhex(h),f'Native {r:x}')
    ck(0xF77DB+6+int.from_bytes(b(0xF77DD,4),'little',signed=True)==0xF789E,'JNE branch target exact')
    ck(b(0xF77D4,7)==bytes.fromhex('83 7d f8 ff 8b 75 d4'),'No instruction split F77D4')
    ck(0xF77D4+7==0xF77DB,'Resume at JNE')
    stage81=(ROOT/'src/Stage81SelectorWriter.inl').read_text()
    witness=(ROOT/'src/Stage97InheritedEsiTrace.inl').read_text()
    apply=(ROOT/'ApplyStage97.ps1').read_text()
    rollback=(ROOT/'RollbackStage97.ps1').read_text()
    ck(sha(ROOT/'src/Stage81SelectorWriter.inl') in apply,'New Stage81 hash gated')
    ck(sha(ROOT/'src/Stage97InheritedEsiTrace.inl') in apply,'New Stage97 hash gated')
    ck('7ea9938883f8a7b5e3db6142d64750902d51a49f9ce6603ba617f9f7e1644f81' in apply,'Parent Stage96 loader SHA')
    ck('d3defa5befc6cb8a1fc1b1d51ab72d3c8445e319ac40762a14679bc74e4f8516' in apply,'Parent Stage96 observer SHA')
    for t in ['.stage96_source_backup/manifest.json','.stage97_source_backup','Get-Process AALib','Stage97 package source SHA','Stage96 source SHA','Stage96 rollback manifest','STAGE97 SOURCE INSTALLED']:
        ck(t in apply,f'Install guard: {t}')
    for t in ['.stage97_source_backup','Get-Process AALib','Sha $stage81','Sha $stage97','Sha $old81','STAGE97 ROLLED BACK']:
        ck(t in rollback,f'Rollback guard: {t}')
    for t in ['namespace arcana_stage97','arcana_stage97::Install();','arcana_stage97::Flush();','arcana_stage97::Health();','#include "Stage97InheritedEsiTrace.inl"']:
        ck(t in stage81,f'Loader wiring: {t}')
    for t in ['Stage96OwnerOK','arcana_stage96::kSites','Stage96Hook7924','Stage96Hook7A8F','SignatureOK','SuspendAll','ReleaseMutex(mutex)','Resume(peers)','FlushInstructionCache','VirtualProtect','EnableStage97InheritedEsiTrace','originalOnce=1','DRwrites=0','gameWritesExtra=0']:
        ck(t in witness,f'Native safety: {t}')
    ck('if(!ok&&touched)rollback=Patch(base+kRva,kNative,kLength);' in witness,'Native rollback')
    ck(witness.count('_emit 0x83')==1 and witness.count('_emit 0x7D')==1 and witness.count('_emit 0xF8')==1 and witness.count('_emit 0xFF')==1 and witness.count('_emit 0x8B')==1 and witness.count('_emit 0x75')==1 and witness.count('_emit 0xD4')==1,'Exact native instruction replay only once')
    ck('jmp dword ptr [g_Stage97Resume77DB]' in witness,'Jump to original JNE')
    ck('fxsave [esp]' in witness and 'fxrstor [esp]' in witness,'FPU/SSE state preserved')
    ck('pushfd' in witness and 'popfd' in witness and 'pushad' in witness and 'popad' in witness,'GPR and flags preserved')
    ck('readFail' in witness and 'dropped' in witness and 'mismatch' in witness,'Health counters')
    # Tests of same-run last-writer attribution logic. A later FFFFFFFF supersedes any older matching writer.
    spec=importlib.util.spec_from_file_location('stage97join',ROOT/'tools/correlate_stage97.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    ctx=('AAA','100','BBB')
    def rec(qpc,value,seq,**kw):return dict(context=ctx,qpcN=qpc,nativeNextEsi=value,valid='1',seq=str(seq),**kw)
    bypass=lambda qpc,value='0000002C':dict(context=ctx,qpcN=qpc,chosenEsi=value,seq='9')
    out,_=module.join([rec(100,'0000002C',1,localD4='0000002C')],[],[bypass(105)])
    ck(out.get('F77D4_LOCAL')==1,'Positive inherited ESI match')
    out,_=module.join([rec(100,'0000002C',1)],[rec(102,'0000002B',2,siteRVA='0F79F0')],[bypass(105)])
    ck(out.get('mismatch')==1,'Later Stage96 different value supersedes inherited')
    out,_=module.join([rec(100,'0000002C',1)],[rec(102,'FFFFFFFF',2,siteRVA='0F7924')],[bypass(105)])
    ck(out.get('mismatch')==1,'FFFFFFFF writer also supersedes inherited')
    out,_=module.join([rec(100,'0000002C',1)],[],[bypass(100000)])
    ck(out.get('no_recent_writer')==1,'Stale candidate is not a match')
    out,_=module.join([],[],[bypass(105)])
    ck(out.get('no_recent_writer')==1,'Missing writer remains unattributed')
    out,_=module.join([rec(100,'0000002C',1)],[],[bypass(105,'FFFFFFFF')])
    ck(not out,'Nonfinite bypass ignored')
    out,_=module.join([rec(100,'0000002C',1,localD4='0000002C')],[],[dict(bypass(105),context=('AAA','101','BBB'))])
    ck(out.get('no_recent_writer')==1,'Wrong TID does not join')
    ck(b(0xF77D4,7)!=b'\xe9\0\0\0\0\x90\x90','Native unmodified')
    ck('Stage36' not in witness or 'Stage36 OFF' in witness,'No Stage36 enabled')
    print(f'Stage97 offline checks: {N}/{N} PASS (original EXE + source + simulated joins). Windows/game UNTESTED.')

if __name__=='__main__':
    if len(sys.argv)!=2:raise SystemExit('Usage: python test_stage97_offline.py /path/to/AALib.exe')
    run(sys.argv[1])
