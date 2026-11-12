#!/usr/bin/env python3
"""Join Stage98 pre-branch predicate snapshots to first Stage95 BYPASS or Stage94 PRODUCER.
Same-run only; never infer gameplay improvement from a control-flow join.
"""
import argparse,bisect,collections,json,pathlib,re

def events(path,prefix):
    for line in pathlib.Path(path).read_text(encoding='utf-8-sig',errors='replace').splitlines():
        if not line.startswith(prefix+' '): continue
        d=dict(re.findall(r'\b([A-Za-z0-9_]+)=([A-Za-z0-9]+)',line))
        try:
            d['q']=int(d['qpc'])
            d['context']=(d['player'],d['tid'],d['frame'])
            yield d
        except (KeyError,ValueError):continue

def join(routes,producers,bypasses,max_lag=10000):
    by=collections.defaultdict(list)
    for x in producers:by[x['context']].append(dict(x,outcome='PRODUCER'))
    for x in bypasses:by[x['context']].append(dict(x,outcome='BYPASS'))
    for v in by.values():v.sort(key=lambda z:z['q'])
    arrivals={k:[x['q'] for x in v] for k,v in by.items()}
    decision_order=collections.defaultdict(list)
    for x in routes:decision_order[x['context']].append(x)
    counts=collections.Counter();predictions=collections.Counter();samples=[]
    for k,rs in decision_order.items():
        rs.sort(key=lambda x:x['q'])
        stamps=arrivals.get(k,[]);later=by.get(k,[])
        for i,r in enumerate(rs):
            counts['route_events']+=1
            if r.get('valid')!='1':counts['invalid_snapshot']+=1;continue
            c=int(r.get('branchClass','0'))
            counts['class_'+str(c)]+=1
            index=bisect.bisect_left(stamps,r['q'])
            next_route=rs[i+1]['q'] if i+1<len(rs) else float('inf')
            if index==len(later):counts['missing_outcome']+=1;continue
            out=later[index];lag=out['q']-r['q']
            if lag<0 or lag>max_lag or out['q']>=next_route:
                counts['missing_outcome']+=1;continue
            counts['matched_outcome']+=1
            counts['observed_'+out['outcome']]+=1
            counts['class_'+str(c)+'_'+out['outcome']]+=1
            if c in (1,2,3,4):
                expected='PRODUCER' if c==1 else 'BYPASS'
                if out['outcome']==expected:counts['exact_prediction_match']+=1
                else:
                    counts['prediction_mismatch']+=1
                    if len(samples)<20:samples.append(dict(route_seq=r.get('seq'),outcome_seq=out.get('seq'),expected=expected,
                        actual=out['outcome'],class_value=c,qpc_lag=lag))
            else:counts['further_conditional_path']+=1
    return dict(counts),samples

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--stage98',required=True)
    p.add_argument('--stage94',required=True)
    p.add_argument('--stage95',required=True)
    p.add_argument('--max-qpc-lag',type=int,default=10000)
    a=p.parse_args()
    counts,samples=join(list(events(a.stage98,'STAGE98_ROUTE')),
        list(events(a.stage94,'STAGE94_PRODUCER')),
        list(events(a.stage95,'STAGE95_BYPASS')),a.max_qpc_lag)
    print(json.dumps(dict(summary=counts,exceptions=samples,
        caveat='Only native path-choice correlation. BranchClass=5 needs later checks; no attack/hit causality inferred.'),indent=2))
if __name__=='__main__':main()
