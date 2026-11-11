#!/usr/bin/env python3
"""Audit Stage90 F57D0 clears and same-run Stage87/88/89 context.
Nearest QPC neighbors are temporal only, never proof of COM execution.
"""
import argparse
import collections
import json
import re
from pathlib import Path

FIELDS=re.compile(r'(?:^|\s)([A-Za-z][A-Za-z0-9]*)=([^\s]+)')
SPECIAL=0xE00

def rows(path,prefix):
 return [dict(FIELDS.findall(line)) for line in Path(path).read_text(encoding='utf-8-sig',errors='replace').splitlines() if line.startswith(prefix)]
def D(r,k):return int(r[k],10)
def H(r,k):return int(r[k],16)
def triple(r,k):return tuple(int(z,16) for z in r[k].split(','))

def analyze(file90,file88=None,file89=None):
 entries=rows(file90,'STAGE90_F57D0 ')
 health=rows(file90,'STAGE90_HEALTH ')
 ready=rows(file90,'STAGE90_READY ')
 failures=[];seqs=set();by_selector=collections.Counter();cleared=collections.Counter();setmask=collections.Counter()
 cases=[]
 for r in entries:
  try:
   seq=D(r,'seq');pre=triple(r,'pre');post=triple(r,'post');clearmask=(pre[1]&~post[1])&SPECIAL;gain=(~pre[1]&post[1])&SPECIAL
   if seq in seqs:failures.append(f'duplicate Stage90 seq {seq}')
   seqs.add(seq)
   checks={
     'qpc':D(r,'qpcBefore')<=D(r,'qpcAfter'),
     'mask':H(r,'preMask')==H(r,'postMask')==7,
     'bitmap':len(pre)==len(post)==3,
     'clearMask':H(r,'clearMask')==clearmask,
     'setMask':H(r,'setMask')==gain,
     'changed':D(r,'anyChanged')==int(pre!=post),
     'tableClears':(H(r,'tableRaw')&1)==D(r,'tableClearsBitmap') if H(r,'tableMask') else True,
     'arg':H(r,'argSelector')>=0,
     'side':r['side'] in ('P1','P2') and H(r,'player')>0,
   }
   for label,ok in checks.items():
    if not ok:failures.append(f'seq={seq} {label}')
   by_selector[f"0x{H(r,'argSelector'):X}"]+=1
   if clearmask:cleared[f'0x{clearmask:03X}']+=1
   if gain:setmask[f'0x{gain:03X}']+=1
   cases.append(r)
  except (ValueError,KeyError,IndexError) as e:failures.append(f'malformed Stage90 event {e}')
 total={k:sum(D(r,k) for r in health) for k in ('preCalls','tracked','completed','filtered','specialCleared','specialSet','changed','noChange','readMaskIncomplete','depthOverflow','mismatch','readFail','dropped')}
 if not ready:failures.append('Stage90 READY marker absent')
 for k in ('depthOverflow','mismatch','readFail','dropped','readMaskIncomplete'):
  if total[k]:failures.append(f'health {k}={total[k]}')
 if total['tracked']!=total['completed']:failures.append('Tracked != completed')
 if total['specialCleared']!=sum(cleared.values()):failures.append('specialClear health != captured events (logAll must be 1)')
 if total['specialSet']!=sum(setmask.values()):failures.append('specialSet health != captured events (logAll must be 1)')
 if not cases:failures.append('No Stage90 records')
 near88={'available':False}
 if file88:
  w=rows(file88,'STAGE88_F57D0 ');unmatched=[];matched=0;multiple=[]
  for event in w:
   candidates=[r for r in cases if r.get('side')==event.get('side') and r.get('player')==event.get('player')
      and r.get('tid')==event.get('tid') and D(r,'qpcBefore')<=D(event,'qpcBefore')<=D(event,'qpcAfter')<=D(r,'qpcAfter')
      and H(r,'argSelector')==H(event,'argSelector')]
   if len(candidates)==1:
    c=candidates[0]
    if D(c,'stage87Seq')!=D(event,'stage87Seq') or triple(c,'pre')!=triple(event,'pre') or triple(c,'post')!=triple(event,'post'):
     failures.append(f'Stage88 seq={event["seq"]} unexpected Stage90 snapshot')
    matched+=1
   elif len(candidates)==0:unmatched.append(D(event,'seq'))
   else:multiple.append(D(event,'seq'))
  near88={'available':True,'stage88_events':len(w),'unique_nested':matched,'unmatched':unmatched[:50],
   'multiple_matches':multiple[:50],'full_strict_containment':matched==len(w) and not unmatched and not multiple}
  if not near88['full_strict_containment']:failures.append('Stage88 / Stage90 strict containment not full')
 near89={'available':False}
 if file89:
  readers=rows(file89,'STAGE89_F58E0 ');last=[]
  for rd in readers:
   options=[r for r in cases if r.get('side')==rd.get('side') and r.get('player')==rd.get('player') and
    r.get('tid')==rd.get('tid') and D(r,'qpcAfter')<=D(rd,'qpcBefore')]
   if not options:
    last.append({'reader_seq':D(rd,'seq'),'prior_stage90_seq':None})
    continue
   r=max(options,key=lambda e:D(e,'qpcAfter'))
   last.append({'reader_seq':D(rd,'seq'),'prior_stage90_seq':D(r,'seq'),
     'gap_qpc_ticks':D(rd,'qpcBefore')-D(r,'qpcAfter'),
     'prior_post':r.get('post'),'reader_pre':rd.get('pre'),
     'snapshots_same':triple(r,'post')==triple(rd,'pre')})
  near89={'available':True,'reader_count':len(readers),'nearest_prior_same_player_tid':last,
   'warning':'A prior F57D0 is only temporal context. Other writes may occur between samples.'}
 return {'status':'PASS_OFFLINE_AUDIT' if not failures else 'FAIL_OFFLINE_AUDIT',
  'stage90_events':len(cases),'health':total,
  'arg_selectors':dict(by_selector),'special_cleared_mask_calls':dict(cleared),'special_set_mask_calls':dict(setmask),
  'stage88_strict_join':near88,'stage89_temporal_context':near89,'errors':failures[:100],
  'causal_gameplay_proven':False}

def main():
 p=argparse.ArgumentParser();p.add_argument('stage90');p.add_argument('--stage88');p.add_argument('--stage89');p.add_argument('--output',default='stage90_results.json')
 a=p.parse_args();r=analyze(a.stage90,a.stage88,a.stage89)
 Path(a.output).write_text(json.dumps(r,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
 print(json.dumps(r,indent=2,ensure_ascii=False))
 return 0 if r['status']=='PASS_OFFLINE_AUDIT' else 1
if __name__=='__main__':raise SystemExit(main())
