#!/usr/bin/env python3
"""Synthetic checks of strong audit invariants; NOT native runtime confirmation."""
import tempfile
from pathlib import Path
from correlate_stage91 import analyze

base91='''STAGE91_INSTALLED mask=7 sites=3/3 rvas=0F5A4D,0F5A5C,0F5A76 nativeTarget=0F58E0\nSTAGE91_F58E0 seq=1 siteRVA=0F5A76 side=P1 player=0DF21F78 bitmap=0DF25980 tid=6820 qpcBefore=2200 qpcAfter=2210 bitIndex=0000000B returnRVA=0F5A7B esi=0DF21F78 guard3B70=00000004 guardMask=1 pre=00000010,00000800,00000000 post=00000010,00000800,00000000 preMask=7 postMask=7 nativeEax=00000001 returnAL=1 returnFlags=00000246 predictedPre=1 predictedPost=1 predictionValid=1 predictionMatch=1 nextBranchPredictedTaken=0\nSTAGE91_HEALTH calls0=0 calls1=0 calls2=1 events=1 filtered=0 overflow=0 dropped=0 mismatch=0 readFail=0 ring=8192 depth=16 mask=7\n'''
base90='''STAGE90_F57D0 seq=31 stage87Seq=18 side=P1 player=0DF21F78 tid=6820 qpcBefore=2100 qpcAfter=2110 argSelector=00000010 pre=00000029,00000000,00000010 post=00000010,00000800,00000000 preMask=7 postMask=7\n'''
base88='''STAGE88_F57D0 seq=18 stage87Seq=18 side=P1 player=0DF21F78 tid=6820 qpcBefore=2101 qpcAfter=2109 selector=00000002 argSelector=00000010 pre=00000029,00000000,00000010 post=00000010,00000800,00000000\n'''
with tempfile.TemporaryDirectory() as t:
 root=Path(t); a,b,c=[root/f for f in ('91.log','90.log','88.log')]
 a.write_text(base91);b.write_text(base90);c.write_text(base88)
 result=analyze(a,b,c)
 assert result['status']=='PASS_OFFLINE_AUDIT',result
 assert result['native_AL1_records']==[1]
 assert result['temporal_context'][0]['last_stage90_post_equals_reader_pre']
 assert result['temporal_context'][0]['nearest_same_player_tid_stage88_seq']==18
 a.write_text(base91.replace('returnAL=1','returnAL=0'))
 assert analyze(a,b,c)['status']=='FAIL_OFFLINE_AUDIT','AL mismatch should fail'
 a.write_text(base91.replace('preMask=7','preMask=3'))
 assert analyze(a,b,c)['status']=='FAIL_OFFLINE_AUDIT','invalid readmask should fail'
 a.write_text(base91.replace('qpcAfter=2210','qpcAfter=2190').replace('qpcBefore=2200','qpcBefore=2200'))
 assert analyze(a,b,c)['status']=='FAIL_OFFLINE_AUDIT','reversed QPC should fail'
 print('PASS_SYNTHETIC_POSITIVE_AND_3_NEGATIVES')
