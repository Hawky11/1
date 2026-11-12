#!/usr/bin/env python3
"""Stage98 r2: source-only exact-delta/SHA/preflight/rollback/correlator tests.
This does NOT execute Windows/MSVC or patch the live game process.
"""
from pathlib import Path
import hashlib, importlib.util, re, json
ROOT=Path(__file__).resolve().parents[1]
OLD=ROOT/'tests/Stage98_r1_reference.inl'
NEW=ROOT/'src/Stage98CommandRouteDecision.inl'
APPLY=ROOT/'UpdateStage98r2.ps1'
ROLLBACK=ROOT/'RollbackStage98r2.ps1'
N=0

def check(ok, message):
    global N
    if not ok: raise AssertionError(message)
    N+=1

def sha(b): return hashlib.sha256(b).hexdigest()
old, new=OLD.read_bytes(), NEW.read_bytes()
old_marker=b'e.preEax=saved[7]; // pushad saved EAX is slot 7; slot 3 is ESPe.tid=GetCurrentThreadId();'
new_marker=b'e.preEax=saved[7]; // pushad EAX slot 7; ESP slot 3\n  e.tid=GetCurrentThreadId();'
check(sha(old)=='a2548073f73fd7967ecece70cb278b32c162c0dfd07d9f63be7f132f628d5ba1','Pinned Stage98 r1 source SHA')
check(sha(new)=='a56fe5692b3284fae37822e6f08438e3a32a800accbad5ff4c72d36ea97a1591','Pinned Stage98 r2 source SHA')
check(old.count(old_marker)==1,'Original bug occurs exactly once')
check(new.count(new_marker)==1,'Only one corrected TID assignment')
check(new==old.replace(old_marker,new_marker),'EXACT one-region source delta; all native code unchanged')
check(new.count(b'GetCurrentThreadId();')==old.count(b'GetCurrentThreadId();'),'Same API call count, previously commented')
check(len(re.findall(rb'(?m)^\s*e\.tid=GetCurrentThreadId\(\);',new))==1,'TID assignment is executable, not commented out')
check(not re.search(rb'//[^\r\n]*e\.tid=GetCurrentThreadId',new),'No commented-out TID assignment')
for phrase in [b'_emit 0x8B',b'_emit 0x87',b'_emit 0x78',b'_emit 0x3B',b'Stage98Hook7A00',
               b'jmp dword ptr [g_Stage98Resume7A06]',b'pushad',b'popad',b'pushfd',b'popfd',
               b'fxsave',b'fxrstor',b'Stage97OwnerOK',b'SignatureOK',b'SuspendAll',b'FlushInstructionCache',
               b'EnableStage98CommandRouteTrace',b'STAGE98_INSTALLED 1/1',b'gameWritesExtra=0 DRwrites=0',
               b'0xF7A00u',b'kLength = 6u',b'constexpr BYTE kOriginal[6]',b'if(!ok && touched)rollback=Patch(base+kRva,kOriginal,kLength);']:
    check(phrase in old and phrase in new,'Native/safety source kept: '+repr(phrase))
apply=APPLY.read_text(encoding='utf8')
rollback=ROLLBACK.read_text(encoding='utf8')
for s in ['Get-Process AALib','Get-FileHash -LiteralPath','Stage98 r1','a2548073f73fd7967ecece70cb278b32c162c0dfd07d9f63be7f132f628d5ba1',
          'a56fe5692b3284fae37822e6f08438e3a32a800accbad5ff4c72d36ea97a1591',
          '12f2afbee4cf427f2b13cce7e9baeb7d525fc4e4b23bc42cc1c3d0a86463a192',
          'f368037433b3baaec4f322428875edbbb8f3c12e1c89486c5d9ba8ff49c2f78c',
          '.stage98_source_backup/manifest.json','.stage98_r2_patch_backup','No changes.',
          'STAGE98_R2_SOURCE_UPDATED','ConvertTo-Json -Depth 8','Stage98CommandRouteDecision.inl']:
    check(s in apply,'Updater guard '+s)
for s in ['Get-Process AALib','Stage98 r1','a2548073f73fd7967ecece70cb278b32c162c0dfd07d9f63be7f132f628d5ba1',
          'a56fe5692b3284fae37822e6f08438e3a32a800accbad5ff4c72d36ea97a1591',
          '.stage98_r2_patch_backup','STAGE98_R2_ROLLED_BACK_TO_R1',
          'Stage98CommandRouteDecision.inl','source_revision','Sha $source','Sha $oldSource']:
    check(s in rollback,'Rollback guard '+s)
check('Stage98 r1' in apply and "Keep m.stage='Stage98 r1'" in apply,'Original Stage98 rollback manifest compatibility')
check('if(Test-Path -LiteralPath $backup)' in apply,'Updater protects existing r2 backup')
check('Remove-Item -LiteralPath $backup -Recurse -Force' in apply,'Updater failure cleanup exists')
check('if(!(Test-Path' not in apply,'Fail-closed preconditions do not skip tests')
# Mirror preflight SHA semantics independent of installed Windows project.
manifest={"stage":"Stage98 r1","installed_stage98_sha":sha(old),
          "installed_stage81_sha":"12f2afbee4cf427f2b13cce7e9baeb7d525fc4e4b23bc42cc1c3d0a86463a192",
          "parent_stage81_sha":"a7d8aef49b68d5b30090b313e3cb2f94cebf51ab6a23d7af40766f01a941fe43",
          "parent_stage97_sha":"f368037433b3baaec4f322428875edbbb8f3c12e1c89486c5d9ba8ff49c2f78c"}
def eligible(source,patch,m):
    return (sha(source)==sha(old) and sha(patch)==sha(new) and m.get('stage')=='Stage98 r1'
            and m.get('installed_stage98_sha')==sha(old) and
            m.get('installed_stage81_sha')==manifest['installed_stage81_sha'] and
            m.get('parent_stage81_sha')==manifest['parent_stage81_sha'] and
            m.get('parent_stage97_sha')==manifest['parent_stage97_sha'])
check(eligible(old,new,manifest),'Exact source and manifest pass')
check(not eligible(new,new,manifest),'No duplicate r2 install')
check(not eligible(old,new+b'wrong',manifest),'Tampered package refused')
check(not eligible(old+b'wrong',new,manifest),'Tampered installed code refused')
for key in ['installed_stage98_sha','installed_stage81_sha','parent_stage81_sha','parent_stage97_sha','stage']:
    m=manifest.copy();m[key]='wrong';check(not eligible(old,new,m),'Invalid manifest refuses '+key)
# Synthetic strict correlation works when Stage98 TID is correctly logged (unlike r1 tid=0).
mod_spec=importlib.util.spec_from_file_location('s98r2join',ROOT/'tools/correlate_stage98.py')
mod=importlib.util.module_from_spec(mod_spec);mod_spec.loader.exec_module(mod)
context=('P1','11956','E58F588')
def e(q,seq,**extra):return dict(context=context,player=context[0],tid=context[1],frame=context[2],q=q,seq=str(seq),valid='1',**extra)
routes=[e(100,1,branchClass='1'),e(200,2,branchClass='3'),e(300,3,branchClass='5')]
producers=[e(110,1)]
bypasses=[e(208,2),e(311,3)]
res,samples=mod.join(routes,producers,bypasses,max_lag=30)
check(res['matched_outcome']==3,'Exact TID joins 3/3')
check(res['exact_prediction_match']==2,'Predicted early native paths 2/2')
check(res['further_conditional_path']==1,'Class5 remains unresolved')
check(len(samples)==0,'No route mismatches')
wrong=[dict(x,context=('P1','0','E58F588'),tid='0') for x in routes]
res,_=mod.join(wrong,producers,bypasses,max_lag=30)
check(res['missing_outcome']==3,'Previous tid=0 bug is detected by strict join')
print(f'Stage98 r2 local offline checks PASS: {N}/{N}. Exact one-region logger fix; no Windows compile/run claimed.')
