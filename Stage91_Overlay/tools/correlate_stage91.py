#!/usr/bin/env python3
"""Audit native F58E0 indices 9/10/11. Joins to Stage90 are temporal, not action identity."""
import argparse
import collections
import json
import re
from pathlib import Path

FIELDS=re.compile(r'(?:^|\s)([A-Za-z][A-Za-z0-9]*)=([^\s]+)')
SITES={0xF5A4D:9,0xF5A5C:10,0xF5A76:11}

def rows(path,prefix):
 if path is None:return []
 return [dict(FIELDS.findall(line)) for line in Path(path).read_text(encoding='utf-8-sig',errors='replace').splitlines() if line.startswith(prefix+' ')]
def D(x,k):return int(x[k],10)
def H(x,k):return int(x[k],16)
def triplet(x,k):return tuple(int(v,16) for v in x[k].split(','))

def analyze(stage91,stage90=None,stage88=None):
 rs=rows(stage91,'STAGE91_F58E0')
 hs=rows(stage91,'STAGE91_HEALTH')
 installed=rows(stage91,'STAGE91_INSTALLED')
 failures=[];seqs=set();per_site=collections.Counter();per_bit=collections.Counter()
 positive=[];records=[]
 for e in rs:
  try:
   seq=D(e,'seq');site=H(e,'siteRVA');index=H(e,'bitIndex');bit=1<<index
   pre=triplet(e,'pre');post=triplet(e,'post');al=D(e,'returnAL')
   if seq in seqs:failures.append(f'duplicate seq={seq}')
   seqs.add(seq)
   checks={
    'site_index':SITES.get(site)==index,
    'return_rva':H(e,'returnRVA')==site+5,
    'side':e['side'] in ('P1','P2'),
    'bitmap_offset':H(e,'bitmap')==H(e,'player')+0x3A08,
    'qpc_order':D(e,'qpcBefore')<=D(e,'qpcAfter'),
    'full_masks':H(e,'preMask')==7 and H(e,'postMask')==7,
    'predictedPre':D(e,'predictedPre')==int(bool(pre[1]&bit)),
    'predictedPost':D(e,'predictedPost')==int(bool(post[1]&bit)),
    'prediction':D(e,'predictionValid') in (1,2),
    'return_AL':al in (0,1),
    'guard_mask':H(e,'guardMask')==1,
    'branch':D(e,'nextBranchPredictedTaken')==(int(al!=0) if index==9 else int(al==0)),
   }
   if pre[1]&bit == post[1]&bit:
    checks['predictionMatch']=D(e,'predictionMatch')==int(al==int(bool(pre[1]&bit)))
   for key,ok in checks.items():
    if not ok:failures.append(f'seq={seq} {key}')
   per_site[f'0x{site:06X}']+=1
   per_bit[f'0x{bit:X}']+=1
   if al==1:positive.append(seq)
   records.append(e)
  except (KeyError,ValueError,IndexError) as er:failures.append(f'malformed Stage91 row: {er}')
 health_keys=['calls0','calls1','calls2','events','filtered','overflow','dropped','mismatch','readFail']
 health={k:sum(int(h.get(k,'0'),10) for h in hs) for k in health_keys}
 if not installed:failures.append('Stage91 installed marker missing')
 if health['events']!=len(records):failures.append('health event count != log count (dropped/truncated?)')
 for n in ['mismatch','readFail','dropped','overflow']:
  if health[n]:failures.append(f'health {n}={health[n]}')
 temporal=[];writer_rows=rows(stage90,'STAGE90_F57D0');writer88=rows(stage88,'STAGE88_F57D0')
 for r in records:
  nearest=[w for w in writer_rows if w.get('player')==r.get('player') and w.get('tid')==r.get('tid')
           and D(w,'qpcAfter')<=D(r,'qpcBefore')]
  w=max(nearest,key=lambda x:D(x,'qpcAfter')) if nearest else None
  index=H(r,'bitIndex');bit=1<<index
  recent88=[q for q in writer88 if q.get('player')==r.get('player') and q.get('tid')==r.get('tid')
            and D(q,'qpcAfter')<=D(r,'qpcBefore')]
  q=max(recent88,key=lambda x:D(x,'qpcAfter')) if recent88 else None
  temporal.append({'stage91_seq':D(r,'seq'),'site':f'0x{H(r,"siteRVA"):X}',
   'bit':f'0x{bit:X}','al':D(r,'returnAL'),'special_bit_pre':bool(triplet(r,'pre')[1]&bit),
   'nearest_same_player_tid_stage90_seq':D(w,'seq') if w else None,
   'nearest_stage90_post':w.get('post') if w else None,
   'last_stage90_post_equals_reader_pre':triplet(w,'post')==triplet(r,'pre') if w else None,
   'qpc_gap_from_stage90':D(r,'qpcBefore')-D(w,'qpcAfter') if w else None,
   'nearest_same_player_tid_stage88_seq':D(q,'seq') if q else None,
   'nearest_stage88_selector':q.get('selector') if q else None,
   'qpc_gap_from_stage88':D(r,'qpcBefore')-D(q,'qpcAfter') if q else None})
 return {'status':'PASS_OFFLINE_AUDIT' if not failures else 'FAIL_OFFLINE_AUDIT',
 'installed':bool(installed),'observations':len(records),'health':health,'by_site':dict(per_site),'by_bit':dict(per_bit),
 'native_AL1_records':positive,'temporal_context':temporal,'errors':failures[:100],
 'warning':'An AL=1 proves this native bit predicate only; QPC neighbors are temporal context, NOT a final command/attack/hit proof.'}

def main():
 p=argparse.ArgumentParser()
 p.add_argument('stage91');p.add_argument('--stage90');p.add_argument('--stage88');p.add_argument('--output',default='stage91_results.json')
 a=p.parse_args();r=analyze(a.stage91,a.stage90,a.stage88)
 Path(a.output).write_text(json.dumps(r,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
 print(json.dumps(r,indent=2,ensure_ascii=False))
 return 0 if r['status']=='PASS_OFFLINE_AUDIT' else 1
if __name__=='__main__':raise SystemExit(main())
