#!/usr/bin/env python3
"""Read-only Stage67 capture audit. Python stdlib only; no DLL/game changes.
Checks one capture, rejects mixed identities/headers and nonzero health errors.
Joins actual observer windows; does not infer unobserved AI decision semantics.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import re

TAGS={40:'STAGE40_CONSUMER_WINDOW',41:'STAGE41_OBJECT_WINDOW',
      42:'STAGE42_ACTIVATION_WINDOW',43:'STAGE43_ORIGIN_WINDOW',
      59:'STAGE59_X_WRITE',65:'STAGE65_WRITER_WINDOW',67:'STAGE67_NATIVE_WINDOW'}
KV=re.compile(r'(\w+)=([^\s]+)')
ERRORS={'dropped','overflow','mismatch','readFail','ownershipConflicts',
        'installFailures','teardownFailures','objectDrift'}
def actor(r): return (r['side'],r['player'],r['tid'])
def q0(r): return int(r['qpcBefore'])
def q1(r): return int(r['qpcAfter'])
def pair(r,key): return tuple(int(v,16) for v in r[key].split('>'))
def contained(a,b): return q0(a)<=q0(b)<=q1(b)<=q1(a)

def analyze(directory):
    rows={}; health={}; starts=[]
    for stage,tag in TAGS.items():
        paths=list(directory.glob(f'ArcanaAI_Stage{stage}*.log'))
        assert len(paths)==1, (stage,'ambiguous/missing file')
        lines=paths[0].read_text(encoding='utf-8-sig').splitlines()
        headers=[l for l in lines if f'STAGE{stage}_' in l and '_BUILD=' in l]
        assert len(headers)==1, (stage,'missing/concatenated build headers')
        starts.append(headers[0].split(']')[0])
        assert not any(re.search(r'\bERROR\b',l) for l in lines), (stage,'error line')
        rr=[dict(KV.findall(l)) for l in lines if tag+' ' in l]
        seqkey='stage40Seq' if stage==41 else 'stage42Seq' if stage==43 else 'seq'
        assert sorted(int(r[seqkey]) for r in rr)==list(range(1,len(rr)+1)), (stage,'sequence gap')
        hs=[dict(KV.findall(l)) for l in lines if f'STAGE{stage}_HEALTH ' in l]
        for h in hs:
            assert all(int(v)==0 for k,v in h.items() if k in ERRORS),(stage,h)
        health[str(stage)]={'samples':len(hs),'sums':dict(Counter()),
                            'fileOrderReversals':sum(int(a[seqkey])>int(b[seqkey]) for a,b in zip(rr,rr[1:]))}
        rr.sort(key=lambda r:int(r[seqkey]))
        totals=Counter()
        for h in hs:
            for k,v in h.items():
                if v.isdigit() and k not in {'ring','depth','depthPerKind','ownedThreads','totalWrites','totalSeq'}: totals[k]+=int(v)
        health[str(stage)]['sums']=dict(totals)
        health[str(stage)]['last']=hs[-1] if hs else {}
        rows[stage]=rr
        if stage==67: assert any('STAGE67_INSTALLED 2/2' in l for l in lines)
        if stage==59: assert any('STAGE59_WATCH_ARMED' in l for l in lines)
    assert len(set(starts))==1, 'mixed initialization sessions'
    d=rows
    identities={actor(r) for r in d[67]}
    for stage in (40,41,42,43,65):
        assert {actor(r) for r in d[stage]}<=identities,(stage,'mixed actor/thread')
    promotions=[r for r in d[67] if r['kind']=='0']
    zero=[r for r in d[67] if r['kind']=='1']
    assert len(promotions)+len(zero)==len(d[67])
    for r in d[67]:
        assert r['preMask']==r['postMask']==('1F' if r['kind']=='0' else '07')
        assert q0(r)<=q1(r)
    assert len({r['callerReturn'] for r in promotions})==1
    assert int(promotions[0]['callerReturn'],16)&0xFFFFF==0xAD761
    for r in d[65]: assert r['readMask']=='3F'
    for r in d[40]: assert r['preMask']==r['postMask']=='FF'
    objects={(r['side'],r['tid'],r['object']) for r in promotions}
    for r in d[59]:
        assert r['readOK']=='1' and (r['side'],r['tid'],'%08X'%(int(r['watched'],16)-0x7C)) in objects
    for stage in (41,42):
        assert all(r['preMask']==r['postMask']=='1F' for r in d[stage])
    activation_parents={}; promotion_cases=[]
    for r in d[42]:
        matches=[p for p in promotions if actor(p)==actor(r) and p['object']==r['object'] and contained(p,r)]
        assert len(matches)==1, ('activation parent',r['seq'],len(matches))
        p=matches[0];activation_parents[r['seq']]=p['seq']
        assert pair(p,'v2')[1]==pair(r,'o2')[1]
        if pair(p,'v1')[0]!=0 and pair(p,'v4')[0]!=0xFFFFFFFF:
            assert pair(r,'o3')[0]==pair(p,'v4')[0] and pair(r,'o4')[0]==0xFFFFFFFF
            assert pair(p,'v3')[1]==pair(p,'v4')[0] and pair(p,'v4')[1]==0xFFFFFFFF
            prior=[c for c in d[40] if actor(c)==actor(p) and q1(c)<q0(p)]
            last=max(prior,key=q1) if prior else None
            promotion_cases.append({'stage67':p,'stage42':r,'latestStage40':last})
    assert len(set(activation_parents.values()))==len(d[42]), 'multiple activation windows per parent'
    for p in promotions:
        children=[r for r in d[42] if activation_parents[r['seq']]==p['seq']]
        assert (int(p['nativeEAX'],16)&255)==(1 if children else 0)
    for c,o in zip(d[40],d[41]):
        assert c['seq']==o['stage40Seq'] and actor(c)==actor(o) and q0(c)==q0(o) and q1(c)==q1(o)
        assert pair(c,'p0')[0]==int(o['request'],16) and pair(c,'p1')[0]==int(o['objectBefore'],16)
    for a,o in zip(d[42],d[43]):
        assert a['seq']==o['stage42Seq'] and actor(a)==actor(o) and a['object']==o['object']
        assert q0(a)==q0(o) and q1(a)==q1(o) and o['nativeReturnRVA']=='001E9F67'
    # Each consumer's immediately preceding OBSERVED request-changing window,
    # including prior consumers (which clear P+3F4C), must be a matching writer.
    events=[(r,'65',q0(r),q1(r),int(r['postRequest'],16)) for r in d[65]]
    events += [(r,'67',q0(r),q1(r),pair(r,'v0')[1]) for r in zero]
    events += [(r,'40',q0(r),q1(r),pair(r,'p0')[1]) for r in d[40]]
    joins=[]
    for c in d[40]:
        prior=[e for e in events if actor(e[0])==actor(c) and e[3]<q0(c)]
        e=max(prior,key=lambda e:e[3])
        assert not any(actor(v[0])==actor(c) and v[2]<q0(c)<=v[3] for v in events if v[0] is not c), 'overlapping writer'
        assert e[1] in ('65','67') and e[4]==pair(c,'p0')[0], ('unattributed consumer',c['seq'])
        writer=e[0]
        assert writer['object']==('%08X'%pair(c,'p1')[0])
        joins.append({'stage40Seq':int(c['seq']),'stage':int(e[1]),'writerSeq':int(writer['seq']),
                      'side':c['side'],'request':c['p0'].split('>')[0],'qpcGap':q0(c)-e[3]})
    assert len({(r['stage'],r['writerSeq']) for r in joins})==len(joins)
    zero_chains=[]
    for w in zero:
        links=[j for j in joins if j['stage']==67 and j['writerSeq']==int(w['seq'])]
        assert len(links)==1 and int(w['writeValue'],16)==pair(w,'v0')[1]==0
        c=next(r for r in d[40] if int(r['seq'])==links[0]['stage40Seq'])
        after=sorted([r for r in d[42] if actor(r)==actor(c) and q0(r)>q1(c)],key=q0)
        a=after[0]
        assert pair(a,'o2')[1]==0
        assert not any(actor(r)==actor(c) and q1(c)<q0(r)<q0(a) for r in d[40])
        zero_chains.append({'stage67':w,'stage40':c,'stage42':a,'join':links[0]})
    latest_mismatch=[]
    for a in d[42]:
        prior=[c for c in d[40] if actor(c)==actor(a) and q1(c)<q0(a)]
        if prior:
            c=max(prior,key=q1)
            if pair(c,'p0')[0]!=pair(a,'o2')[1]:latest_mismatch.append({'stage42':a['seq'],'stage40':c['seq']})
    result={'sessionInitialization':starts[0]+']','counts':{str(k):len(v) for k,v in d.items()},
            'identities':sorted(identities),'health':health,
            'kindCounts':dict(Counter(r['kind'] for r in d[67])),
            'activationParents':activation_parents,'promotionCases':promotion_cases,
            'requestWriterCounts':dict(Counter(str(j['stage']) for j in joins)),
            'zeroChains':zero_chains,'latestRequestActivationMismatch':latest_mismatch,
            'stage43RawReturns':dict(Counter(r['callerReturnRVA'] for r in d[43])),
            'stage43KnownCaller':dict(Counter(r['knownCaller'] for r in d[43]))}
    return result

if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('logs',type=Path);args=ap.parse_args()
    print(json.dumps(analyze(args.logs),indent=2))
