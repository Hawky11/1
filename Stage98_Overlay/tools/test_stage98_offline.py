#!/usr/bin/env python3
"""Strict PE/source/branch simulation, NOT Windows compilation or live gameplay proof."""
import hashlib,importlib.util,json,pathlib,sys,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
N=0

def check(ok,msg):
    global N
    if not ok: raise AssertionError(msg)
    N+=1

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def run(aalib):
    p=pathlib.Path(aalib)
    check(p.stat().st_size==175556608,'PE size')
    check(sha(p)=='28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba','Original PE SHA256')
    d=p.read_bytes()
    check(d[:2]==b'MZ','MZ header')
    e_lfanew=int.from_bytes(d[0x3C:0x40],'little')
    check(d[e_lfanew:e_lfanew+4]==b'PE\0\0','PE header')
    def native(rva,n):return d[rva-0xC00:rva-0xC00+n]
    checks={0xF7A00:'8b 87 78 3b 00 00',0xF7A06:'8b d0 c1 ea 08 f6 c2 01',
        0xF7A0E:'0f 84 85 00 00 00',0xF7A14:'8b 8f 70 3b 00 00 c1 e9 12 f6 c1 01',
        0xF7A20:'75 77',0xF7A22:'8b 57 04 8b 4a 60',0xF7A2E:'7c 08',
        0xF7A30:'81 f9 1a 08 00 00',0xF7A36:'7e 5c',0xF7A38:'8b c8 c1 e9 0f f6 c1 01',
        0xF7A40:'74 52',0xF7A42:'c1 e8 11 a8 01',0xF7A47:'75 4b',
        0xF7A49:'66 83 bf 34 3a 00 00 02',0xF7A5E:'e8 6d e2 02 00',
        0xF7A68:'75 2a',0xF7A8A:'e8 41 ba ff ff',0xF7A94:'89 75 e4 eb 42',
        0xF7A99:'8b cf 89 75 e4',0xF7AD1:'e8 5a be ff ff',
        0xF77D4:'83 7d f8 ff 8b 75 d4',0xF77DB:'0f 85 bd 00 00 00'}
    for r,hexbytes in checks.items():
        byt=bytes.fromhex(hexbytes)
        check(native(r,len(byt))==byt,f'Original exact native {r:06x}')
    check(0xF7A0E+6+int.from_bytes(native(0xF7A10,4),'little',signed=True)==0xF7A99,'JZ native destination F7A99')
    check(0xF7A20+2+int.from_bytes(native(0xF7A21,1),'little',signed=True)==0xF7A99,'JNE native destination F7A99')
    check(0xF7A00+6==0xF7A06,'Hook length no instruction split')
    loader=(ROOT/'src/Stage81SelectorWriter.inl').read_text()
    probe=(ROOT/'src/Stage98CommandRouteDecision.inl').read_text()
    installer=(ROOT/'ApplyStage98.ps1').read_text()
    rollback=(ROOT/'RollbackStage98.ps1').read_text()
    check(sha(ROOT/'src/Stage81SelectorWriter.inl') in installer,'Installer new Stage81 checksum')
    check(sha(ROOT/'src/Stage98CommandRouteDecision.inl') in installer,'Installer new Stage98 checksum')
    check('a7d8aef49b68d5b30090b313e3cb2f94cebf51ab6a23d7af40766f01a941fe43' in installer,'Verified Stage97 parent Stage81 checksum')
    check('f368037433b3baaec4f322428875edbbb8f3c12e1c89486c5d9ba8ff49c2f78c' in installer,'Verified Stage97 witness checksum')
    for t in ['.stage97_source_backup/manifest.json','.stage98_source_backup',
        'Stage97 source SHA mismatch','Stage97 backup manifest','Stage98 package source SHA',
        'Get-Process AALib','STAGE98 SOURCE INSTALLED']:
        check(t in installer,'Installer safety '+t)
    for t in ['.stage98_source_backup','Get-Process AALib','Sha $old81','Sha $stage98','Sha $stage97',
        'STAGE98 ROLLED BACK','Sha $stage81']:
        check(t in rollback,'Rollback safety '+t)
    for t in ['namespace arcana_stage98 { bool Install(); void Flush(); void Health(); }',
        'if (arcana_stage97::Install()) arcana_stage98::Install();',
        'arcana_stage97::Flush(); arcana_stage98::Flush();',
        'arcana_stage97::Health(); arcana_stage98::Health();',
        '#include "Stage98CommandRouteDecision.inl"']:
        check(t in loader,'Loader wiring '+t)
    for t in ['namespace arcana_stage98','Stage97OwnerOK','arcana_stage97::Stage96OwnerOK',
        'Stage95BypassHook','Stage97Hook77D4','SignatureOK','SuspendAll','Resume(peers)',
        'FlushInstructionCache','VirtualProtect','EnableStage98CommandRouteTrace',
        'originalOnce=1','gameWritesExtra=0 DRwrites=0','Stage98Hook7A00',
        'branchClass','stage98','Read32(e.player+0x3B78u','Read32(e.player+0x3B70u']:
        check(t in probe,'Probe safety '+t)
    check('if(!ok && touched)rollback=Patch(base+kRva,kOriginal,kLength);' in probe,'Failed installation rollback')
    check(probe.count('_emit 0x8B')==1 and probe.count('_emit 0x87')==1 and
          probe.count('_emit 0x78')==1 and probe.count('_emit 0x3B')==1 and
          probe.count('_emit 0x00')==2,'Original 6 byte MOV replay once')
    check('jmp dword ptr [g_Stage98Resume7A06]' in probe,'Resume original F7A06')
    check(all(t in probe for t in ['pushfd','popfd','pushad','popad','fxsave [esp]','fxrstor [esp]']),'Register/x87/XMM snapshots')
    check('preEax=saved[7]' in probe,'PUSHAD EAX correct offset (ESP is slot3)')
    check('Stage36 MUST remain off' in probe,'Stage36 OFF guard note')
    check('STAGE98_INSTALLED 1/1' in probe,'Runtime installed marker')
    spec=importlib.util.spec_from_file_location('stage98join',ROOT/'tools/correlate_stage98.py')
    mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
    context=('P1','112','EBP')
    def e(q,seq,**kwargs):return dict(context=context,player='P1',tid='112',frame='EBP',q=q,seq=str(seq),valid='1',**kwargs)
    routes=[e(100,1,branchClass='1'),e(200,2,branchClass='2'),e(300,3,branchClass='3'),
            e(400,4,branchClass='4'),e(500,5,branchClass='5')]
    producers=[e(101,11),e(505,15)]
    bypasses=[e(202,12),e(303,13),e(405,14)]
    counts,samples=mod.join(routes,producers,bypasses,max_lag=30)
    check(counts['route_events']==5,'Five synthetic route events')
    check(counts['matched_outcome']==5,'All same-context native join')
    check(counts['exact_prediction_match']==4,'Class1-4 predictions correct')
    check(counts['further_conditional_path']==1,'Class5 deliberately unknown')
    check(not samples,'No false mismatches')
    bads=[e(100,1,branchClass='1'),e(200,2,branchClass='2')]
    counts,_=mod.join(bads,[],[e(103,3),e(204,4)])
    check(counts['prediction_mismatch']==1,'Detect class1 wrong bypass')
    counts,_=mod.join([e(100,1,branchClass='2')],[],[],max_lag=30)
    check(counts['missing_outcome']==1,'Unmatched does not invent route')
    counts,_=mod.join([e(100,1,branchClass='2'),e(102,2,branchClass='2')],[],[e(105,3)],max_lag=30)
    check(counts['matched_outcome']==1 and counts['missing_outcome']==1,'Cannot assign next event to two snapshots')
    check(native(0xF7A00,6)!=bytes.fromhex('e9 00 00 00 00 90'),'Original executable unpatched')
    print(f'Stage98 offline checks: {N}/{N} PASS (original PE, source, installer, synthetic route joins); Windows MSVC and game UNTESTED')
if __name__=='__main__':
    if len(sys.argv)!=2:raise SystemExit('Usage: test_stage98_offline.py /path/to/AALib.exe')
    run(sys.argv[1])
