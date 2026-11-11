#!/usr/bin/env python3
"""Audit native Stage89 reader trace; optional time-local predecessor Stage88 joins.

Always distinguish temporal proximity from causal event identity. Stage89 is NOT
nested in Stage88 F57D0, and a nearest earlier writer is not a command identity.
"""
import argparse,bisect,collections,json,re
from pathlib import Path
PAT=re.compile(r'(?:^|\s)([A-Za-z][A-Za-z0-9]*)=([^\s]+)')
SITES={0x9a870,0x9aa20,0x9ab00,0xaab55}

def rows(path,marker):
    return [dict(PAT.findall(line)) for line in Path(path).read_text(encoding='utf-8-sig',errors='replace').splitlines() if marker in line]
def d(x,key):return int(x[key],10)
def h(x,key):return int(x[key],16)
def three(x,key):return tuple(int(z,16) for z in x[key].split(','))
def expected(values):return int(bool((values[1]&0xe00) or (values[2]&0x40)))

def analyze(path89,path88=None):
    rs=rows(path89,'STAGE89_F58E0')
    hs=rows(path89,'STAGE89_HEALTH')
    inst=rows(path89,'STAGE89_INSTALLED')
    failures=[];by_site=collections.Counter();by_side=collections.Counter();al=collections.Counter()
    seen=set();cases=[]
    for r in rs:
        try:
            seq=d(r,'seq');site=h(r,'siteRVA');pre=three(r,'pre');post=three(r,'post')
            if seq in seen:failures.append(f'duplicate seq {seq}')
            seen.add(seq)
            checks={
                'site':site in SITES,
                'index':h(r,'bitIndex')==0x26,
                'pointer':h(r,'bitmap')==h(r,'player')+0x3a08,
                'return':h(r,'returnRVA')==site+5,
                'qpc':d(r,'qpcBefore')<=d(r,'qpcAfter'),
                'mask':h(r,'preMask')==h(r,'postMask')==7,
                'al':d(r,'returnAL') in (0,1) and (h(r,'nativeEax')&0xff)==d(r,'returnAL'),
                'predict':d(r,'predictedPre')==expected(pre) and d(r,'predictedPost')==expected(post),
                'valid':d(r,'predictionValid') in (1,2),
                'match':d(r,'predictionValid')==2 or d(r,'predictionMatch')==int(d(r,'returnAL')==expected(pre)),
                'branch':d(r,'nextBranchPredictedTaken')==(int(bool(d(r,'returnAL'))) if site==0xaab55 else int(not bool(d(r,'returnAL')))),
            }
            # In the AAB55 JNE case branch taken iff AL != 0; otherwise JE iff AL == 0.
            for key,ok in checks.items():
                if not ok:failures.append(f'seq={seq} failed {key}')
            by_site[hex(site)]+=1;by_side[r['side']]+=1;al[str(d(r,'returnAL'))]+=1
            cases.append(r)
        except (KeyError,TypeError,ValueError,IndexError) as e:
            failures.append(f'malformed record: {e}')
    totals={key:sum(d(x,key) for x in hs) for key in ('calls0','calls1','calls2','calls3','events','filtered','overflow','dropped','mismatch','readFail')}
    for key in ('overflow','dropped','mismatch','readFail'):
        if totals[key]:failures.append(f'STAGE89_HEALTH {key}={totals[key]}')
    if totals['events']!=len(rs):failures.append(f'Health event total {totals["events"]} != lines {len(rs)}')
    if not inst:failures.append('Missing STAGE89_INSTALLED')
    near={'available':False}
    if path88:
        writes=rows(path88,'STAGE88_F57D0')
        by=collections.defaultdict(list)
        for w in writes:
            key=(w.get('side'),w.get('player'),w.get('tid'))
            by[key].append(w)
        times={key:sorted(((d(w,'qpcAfter'),d(w,'seq'),w) for w in seqs), key=lambda x:(x[0],x[1])) for key,seqs in by.items()}
        temporal=[]
        for r in cases:
            key=(r.get('side'),r.get('player'),r.get('tid'))
            matches=times.get(key,[])
            if not matches:continue
            target=d(r,'qpcBefore')
            i=bisect.bisect_right([x[0] for x in matches],target)-1
            if i<0:continue
            qw,_,w=matches[i];gap=target-qw
            temporal.append({'stage89_seq':d(r,'seq'),'stage88_seq':d(w,'seq'),
               'same_player_tid':True,'qpc_delta_ticks':gap,
               'stage88_expected_bit':w.get('expectedBit'),'stage89_al':r.get('returnAL')})
        near={'available':True,'stage88_rows':len(writes),'readers_with_prior_writer':len(temporal),
          'candidate_links':temporal[:50],
          'caution':'Nearest previous Stage88 same P/TID is time context only, NOT proven causality'}
    return {'stage89_rows':len(rs),'installed':bool(inst),'health':totals,'by_site':dict(by_site),
       'by_side':dict(by_side),'return_al':dict(al),'failures':failures[:100],
       'full_pass':not failures and len(rs)>0, 'stage88_temporal_context':near,
       'gameplay_proof':False}

def main():
    p=argparse.ArgumentParser();p.add_argument('stage89');p.add_argument('--stage88');p.add_argument('--output',default='stage89_results.json')
    a=p.parse_args();out=analyze(a.stage89,a.stage88)
    Path(a.output).write_text(json.dumps(out,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
    print(json.dumps(out,indent=2,ensure_ascii=False))
    return 0 if out['full_pass'] else 1
if __name__=='__main__':raise SystemExit(main())
