#!/usr/bin/env python3
from pathlib import Path
from tempfile import TemporaryDirectory
from correlate_stage90 import analyze

def emit(path,lines):path.write_text('\n'.join(lines)+'\n',encoding='utf-8')
with TemporaryDirectory() as d:
 d=Path(d)
 trace=d/'stage90.log';stage88=d/'stage88.log';stage89=d/'stage89.log'
 emit(trace,[
 'STAGE90_READY r1 SOURCE_ONLY',
 'STAGE90_F57D0 seq=1 stage87Seq=4 side=P1 player=0A000000 tid=100 tick=123 qpcBefore=10 qpcAfter=20 argSelector=0000000E arg3=00000000 arg4=00000000 dispatchType=00000000 dispatchPayload=00000022 dispatchMask=3 tableRaw=01FF2601 tableMask=1 tableClearsBitmap=1 pre=00000010,00000800,00000000 post=0000000E,00000200,00000000 preMask=7 postMask=7 clearMask=00000800 setMask=00000200 anyChanged=1 returnEax=00000000 returnFlags=00000246',
 'STAGE90_HEALTH preCalls=1 tracked=1 completed=1 filtered=0 specialCleared=1 specialSet=1 changed=1 noChange=0 readMaskIncomplete=0 depthOverflow=0 mismatch=0 readFail=0 dropped=0 logAll=1 ring=8192 noNewHook=1 gameWrites=0 DRwrites=0'])
 emit(stage88,['STAGE88_F57D0 seq=4 stage87Seq=4 side=P1 player=0A000000 tid=100 qpcBefore=12 qpcAfter=18 argSelector=0000000E pre=00000010,00000800,00000000 post=0000000E,00000200,00000000'])
 emit(stage89,['STAGE89_F58E0 seq=1 side=P1 player=0A000000 tid=100 qpcBefore=32 qpcAfter=33 pre=00000000,00000000,00000000'])
 r=analyze(trace,stage88,stage89)
 assert r['status']=='PASS_OFFLINE_AUDIT',r
 assert r['stage88_strict_join']['full_strict_containment']
 assert r['special_cleared_mask_calls']=={'0x800':1}
 assert r['special_set_mask_calls']=={'0x200':1}
 assert r['stage89_temporal_context']['nearest_prior_same_player_tid'][0]['snapshots_same']==False
 print('PASS Stage90 correlator: exact nested Stage88 join + bit clear/set + temporal-only Stage89')
 # deliberately malformed bitmask must fail
 broken=trace.read_text().replace('clearMask=00000800','clearMask=00000000')
 trace.write_text(broken)
 assert analyze(trace,stage88,stage89)['status']=='FAIL_OFFLINE_AUDIT'
 print('PASS Stage90 correlator negative test: wrong clearing mask rejected')
