#!/usr/bin/env python3
"""Cross-check Stage91 and Stage92 same-actor, thread, QPC, and tick-linked helper data.
Does not infer native FinalCommand/attack from temporal proximity.
"""
import argparse,bisect,collections,json,re
from pathlib import Path
PAT=re.compile(r'\b([A-Za-z][A-Za-z0-9]*)=([^\s]+)')
def rows(path,token):
 out=[]
 for line in Path(path).open('r',encoding='utf-8-sig',errors='replace'):
  if token not in line:continue
  row=dict(PAT.findall(line));row['line']=line.strip();out.append(row)
 return out
def number(r,key,default=0,hexadecimal=False):
 v=r.get(key)
 if v is None:return default
 return int(v,16 if hexadecimal else 10)
def key(r):return(r.get('side'),r.get('player'),r.get('tid'))
def check(reader_rows,stage92_rows):
 stage91=collections.defaultdict(list)
 for r in reader_rows:
  stage91[key(r)].append((number(r,'qpcBefore'),number(r,'qpcAfter'),r))
 for v in stage91.values():v.sort(key=lambda x:x[0])
 helpers=collections.defaultdict(list);ticks=[]
 for r in stage92_rows:
  if 'STAGE92_TICK' in r['line']:ticks.append(r)
  elif 'STAGE92_HELPER' in r['line']:helpers[number(r,'tick')].append(r)
 counts=collections.Counter();examples=[];errors=[]
 for t in ticks:
  id=number(t,'tick');k=key(t);lo=number(t,'qpcBefore');hi=number(t,'qpcAfter')
  if not(hi>=lo):errors.append(f'tick{id}: invalid QPC');continue
  matching=[r for a,b,r in stage91[k] if lo<=a<=b<=hi]
  index=[number(r,'bitIndex',hexadecimal=True) for r in matching]
  al=[number(r,'returnAL') for r in matching]
  seen=helpers.get(id,[])
  set_h=[h for h in seen if h.get('kind')=='SET_9ED20' and key(h)==k and lo<=number(h,'qpc')<=hi]
  clear_h=[h for h in seen if h.get('kind')=='CLEAR_9ED60' and key(h)==k and lo<=number(h,'qpc')<=hi]
  if len(seen)!=(len(set_h)+len(clear_h)):errors.append(f'tick{id}: helper actor/thread/QPC mismatch')
  if number(t,'helperSet')!=len(set_h) or number(t,'helperClear')!=len(clear_h):errors.append(f'tick{id}: counter mismatch')
  expected='guard-or' if not matching else 'unresolved'
  if matching:
   if index[0]!=9:errors.append(f'tick{id}: first bit index not 9')
   if len(index)>3 or any(a!=9+i for i,a in enumerate(index)):
    errors.append(f'tick{id}: call order invalid {index}')
   if al[0]==1:expected='direct-or'
   elif len(al)>1 and al[1]==1:expected='direct-or'
   elif len(al)>2:expected='set' if al[2]==1 else 'clear'
  if expected in ('direct-or','guard-or'):
   if set_h or clear_h:errors.append(f'tick{id}: helper executed on direct-OR branch')
  if expected=='set' and (len(set_h)!=1 or clear_h):errors.append(f'tick{id}: expected native 9ED20 SET')
  if expected=='clear' and (len(clear_h)!=1 or set_h):errors.append(f'tick{id}: expected native 9ED60 CLEAR')
  counts[expected]+=1
  b=number(t,'flagBefore',hexadecimal=True);a=number(t,'flagAfter',hexadecimal=True)
  if number(t,'valid')==1:
   if expected in ('direct-or','set') and (a&8)==0:errors.append(f'tick{id}: expected bit3=1 POST')
   if expected=='clear' and (a&8)!=0:errors.append(f'tick{id}: expected bit3=0 POST')
   if (b&8)==0 and (a&8)!=0:counts['bit3_0_to_1']+=1
   if (b&8)!=0 and (a&8)==0:counts['bit3_1_to_0']+=1
  if expected=='set' and len(set_h)==1 and len(examples)<5:
   examples.append(dict(tick=id,player=t.get('player'),tid=t.get('tid'),bit11_AL1=True,
      helper='9ED20',flag_before=t.get('flagBefore'),flag_after=t.get('flagAfter')))
 observed={number(t,'tick') for t in ticks}
 orphans=[h for hid,rr in helpers.items() if hid not in observed for h in rr]
 if orphans:errors.append(f'{len(orphans)} helper rows outside recorded enclosing tick')
 return dict(status='PASS' if not errors else 'REVIEW',ticks=len(ticks),
   helpers=sum(map(len,helpers.values())),stage91_rows=len(reader_rows),path_counts=dict(counts),
   errors=errors[:30],error_total=len(errors),examples=examples,final_command_proven=False)
def main():
 ap=argparse.ArgumentParser()
 ap.add_argument('stage91_log');ap.add_argument('stage92_log');ap.add_argument('--output')
 a=ap.parse_args()
 reads=rows(a.stage91_log,'STAGE91_F58E0 ')
 s92=rows(a.stage92_log,'STAGE92_TICK ')+rows(a.stage92_log,'STAGE92_HELPER ')
 result=check(reads,s92)
 payload=json.dumps(result,indent=2)+'\n';print(payload,end='')
 if a.output:Path(a.output).write_text(payload,encoding='utf-8')
 if result['error_total']:raise SystemExit(2)
if __name__=='__main__':main()
