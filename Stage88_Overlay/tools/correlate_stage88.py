#!/usr/bin/env python3
"""Offline same-run Stage87 -> Stage88 join audit; optional FinalCommand candidates.
Stage87/88 use QPC; FinalCommand uses GetTickCount and has weaker attribution.
"""
import argparse
import collections
import json
import re
from pathlib import Path

KEYS = re.compile(r'(?<!\S)([A-Za-z][A-Za-z0-9]*)=([^\s]+)')

def records(path, marker):
    lines=[]
    for line in Path(path).read_text(encoding='utf-8-sig',errors='replace').splitlines():
        if marker not in line: continue
        fields=dict(KEYS.findall(line))
        if marker=='FINAL_COMMAND' and 'payloadArg' not in fields: continue
        lines.append(fields)
    return lines

def D(d,key): return int(d[key],10)
def H(d,key): return int(d[key],16)
def triplet(d,key): return [int(x,16) for x in d[key].split(',')]

def main():
    p=argparse.ArgumentParser()
    p.add_argument('stage87');p.add_argument('stage88')
    p.add_argument('--final', help='same-session ArcanaAI_FinalCommandTrace.log (temporal candidates only)')
    p.add_argument('--output',default='stage88_results.json')
    a=p.parse_args()
    xs=records(a.stage87,'STAGE87_CONSUMER')
    ys=records(a.stage88,'STAGE88_F57D0')
    assert len(xs)>0,'Stage87 events missing'
    by87=collections.defaultdict(list)
    for x in xs:by87[D(x,'seq')].append(x)
    by88=collections.defaultdict(list)
    for y in ys:by88[D(y,'stage87Seq')].append(y)
    failures=[]
    sides=collections.Counter();selectors=collections.Counter()
    valid=0
    for k,entries in by87.items():
        if len(entries)!=1: failures.append(f'duplicate Stage87 seq {k}');continue
        x=entries[0]; links=by88.get(k,[])
        if len(links)!=1:
            failures.append(f'Stage87 seq {k} has {len(links)} Stage88 records');continue
        y=links[0]
        try:
            selector=H(x,'selector'); mapped=H(x,'mappedEax'); expected=0x0e+selector
            pre=triplet(y,'pre');post=triplet(y,'post')
            bit=1<<(9+selector)
            conditions={
              'side':x['side']==y['side'],
              'player':H(x,'player')==H(y,'player'),
              'tid':D(x,'tid')==D(y,'tid'),
              'qpc':D(x,'qpc')==D(y,'qpc87') and D(y,'qpcBefore')>=D(y,'qpc87') and D(y,'qpcAfter')>=D(y,'qpcBefore'),
              'mapping':selector in (0,1,2) and mapped==expected==H(y,'mappedEax')==H(y,'argSelector'),
              'descriptor':H(y,'tableMask')==1 and (H(y,'tableRaw') & 0xffffff)==0xff2601,
              'masks':H(x,'readMask')==15 and H(y,'preMask')==H(y,'postMask')==7 and D(y,'match')==1,
              'bit':H(y,'expectedBit')==bit and D(y,'bitBefore')==int(bool(pre[1]&bit)) and D(y,'bitAfter')==int(bool(post[1]&bit)) and (post[1]&bit)!=0,
              'controller':post[0]==mapped,
            }
            bad=[name for name,passed in conditions.items() if not passed]
            if bad: failures.append(f'Stage87 seq {k} mismatched: {bad}');continue
            valid+=1;sides[x['side']]+=1;selectors[selector]+=1
        except (KeyError,ValueError,IndexError) as e:
            failures.append(f'Stage87 seq {k} malformed Stage88 row: {e}')
    unmatched88=sorted(set(by88)-set(by87))
    for k in unmatched88:failures.append(f'Stage88 refers to absent Stage87 seq {k}')
    final_summary={'available':False}
    if a.final:
        fs=records(a.final,'FINAL_COMMAND')
        possible=0; unique=0
        for y in ys:
            if H(y,'dispatchType')!=1 or H(y,'dispatchMask')!=3: continue
            tick=D(y,'tick')
            matches=[f for f in fs if f.get('side')==y.get('side') and f.get('player','').lower() == '0x'+y['player'].lower()
                and f.get('tid')==y.get('tid') and f.get('type')=='0x00000001'
                and H(f,'payloadArg')==H(y,'dispatchPayload')
                and 0<=((tick-D(f,'tick'))&0xffffffff)<=1000]
            if matches:possible+=1
            if len(matches)==1:unique+=1
        final_summary={'available':True,'records':len(fs),'temporal_candidates':possible,'unique_temporal_candidates':unique,
            'warning':'FinalCommand tick-window matches are NOT event identity or proof of movement/hit'}
    result={'stage87_events':len(xs),'stage88_events':len(ys),'one_to_one_valid':valid,'full_coverage':valid==len(xs)==len(ys),
      'sides':dict(sides),'selectors':{str(k):v for k,v in sorted(selectors.items())},
      'failed_count':len(failures),'failures':failures[:100], 'final_command':final_summary,
      'warning':'F57D0 controller bitmap result is not gameplay execution or visible animation'}
    Path(a.output).write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
    print(json.dumps(result,ensure_ascii=False,indent=2))
    return 0 if result['full_coverage'] else 1

if __name__=='__main__':raise SystemExit(main())