// Stage90 r1 SOURCE ONLY — capture all CPU P1/P2 EE306 -> native F57D0
// through the EXISTING Stage24 bridge. No new patch, breakpoint, or game write.
// Stage88's narrow 0x2C join is preserved; Stage90 also logs all other selectors
// to locate the actual writer that clears the 0x200/0x400/0x800 command bits.
#ifndef _M_IX86
#error Stage90 requires Windows MSVC x86
#endif
namespace arcana_stage90 {
using arcana_stage81::g_gameBase;
constexpr DWORD kCapacity=8192u, kDepth=16u, kBitmapOffset=0x3A08u;
constexpr DWORD kSpecialBits=0x00000E00u, kDescriptorTableRva=0x049DF188u;
struct Event {
 DWORD seq,stage87Seq,side,player,tid,tick,outputPtr,argSelector,arg3,arg4;
 DWORD dispatchType,dispatchPayload,dispatchMask,tableRaw,tableMask;
 DWORD pre[3],post[3],preMask,postMask;
 DWORD returnEax,returnFlags,clearMask,setMask,anyChanged,tableClearsBitmap;
 ULONGLONG qpcBefore,qpcAfter;
};
struct Pending { Event event; bool valid; };
__declspec(thread) Pending g_pending[kDepth]{};
__declspec(thread) DWORD g_depth=0u;
SRWLOCK g_lock=SRWLOCK_INIT;
Event g_ring[kCapacity]{};
DWORD g_read=0u,g_write=0u,g_count=0u;
volatile LONG g_enabled=0,g_logAll=1,g_seq=0,g_preCalls=0,g_tracked=0,g_completed=0;
volatile LONG g_filtered=0,g_readFail=0,g_mismatch=0,g_depthOverflow=0,g_dropped=0;
volatile LONG g_specialCleared=0,g_specialSet=0,g_changed=0,g_noChange=0,g_readMaskIncomplete=0;
void Log(const char* message){
 wchar_t path[MAX_PATH+1]{};
 if(!arcana_stage81::GetPath(path,_countof(path),L"ArcanaAI_Stage90AllF57D0Trace.log"))return;
 HANDLE file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
    nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(file==INVALID_HANDLE_VALUE)return;
 DWORD bytes=0u;
 WriteFile(file,message,static_cast<DWORD>(std::strlen(message)),&bytes,nullptr);
 WriteFile(file,"\r\n",2u,&bytes,nullptr);CloseHandle(file);
}
bool ReadDword(DWORD address,DWORD& value){
 if(address==0u)return false;
 __try{value=*reinterpret_cast<volatile DWORD*>(static_cast<uintptr_t>(address));return true;}
 __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&g_readFail);return false;}
}
ULONGLONG Qpc(){LARGE_INTEGER p{};QueryPerformanceCounter(&p);return static_cast<ULONGLONG>(p.QuadPart);}
void Snapshot(Event& e,bool after){
 DWORD* values=after?e.post:e.pre;
 DWORD& mask=after?e.postMask:e.preMask;
 for(DWORD i=0u;i<3u;++i)if(ReadDword(e.outputPtr+4u*i,values[i]))mask|=1u<<i;
}
// Called from original Stage88Stage24Pre() BEFORE Stage88 consumes the
// Stage87 same-thread handoff. Never consume/change that handoff ourselves.
void Pre(const DWORD* frame){
 if(!frame||InterlockedCompareExchange(&g_enabled,0,0)!=1)return;
 InterlockedIncrement(&g_preCalls);
 const DWORD depth=g_depth++;
 if(depth>=kDepth){InterlockedIncrement(&g_depthOverflow);return;}
 Pending& pending=g_pending[depth];pending=Pending{};
 const DWORD ptr=frame[0];
 if(ptr<kBitmapOffset){InterlockedIncrement(&g_filtered);return;}
 const DWORD player=ptr-kBitmapOffset;
 const DWORD side=arcana_stage81::Side(player);
 if(side==0u){InterlockedIncrement(&g_filtered);return;}
 Event& e=pending.event;
 e.side=side;e.player=player;e.outputPtr=ptr;e.tid=GetCurrentThreadId();
 e.tick=GetTickCount();e.argSelector=frame[1];e.arg3=frame[2];e.arg4=frame[3];
 e.qpcBefore=Qpc();
 if(arcana_stage87::g_stage88Available){
    const arcana_stage87::Stage88Handoff& h=arcana_stage87::g_stage88Last;
    if(h.player==player&&h.tid==e.tid&&h.eax==e.argSelector)
       e.stage87Seq=h.seq;
 }
 if(frame[5]){
   if(ReadDword(frame[5]+8u,e.dispatchType))e.dispatchMask|=1u;
   if(ReadDword(frame[5]+12u,e.dispatchPayload))e.dispatchMask|=2u;
 }
 if(e.argSelector<=0xFFu){
   DWORD raw=0u;
   if(ReadDword(g_gameBase+kDescriptorTableRva+e.argSelector*3u,raw)){
       e.tableRaw=raw;e.tableMask=1u;
       e.tableClearsBitmap=((raw&0xFFu)&1u)?1u:0u;
   }
 }
 Snapshot(e,false);
 pending.valid=true;
 InterlockedIncrement(&g_tracked);
}
// Stage24 assembly passes savedContext from PUSHFD/PUSHAD after F57D0;
// saved[3] is the pre-PUSHAD ESP. Reconstruct its 0x48-byte private frame.
void Post(const DWORD* saved){
 if(!saved||InterlockedCompareExchange(&g_enabled,0,0)!=1)return;
 if(g_depth==0u){InterlockedIncrement(&g_mismatch);return;}
 const DWORD depth=--g_depth;
 if(depth>=kDepth)return;
 Pending& pending=g_pending[depth];
 if(!pending.valid)return;
 Event e=pending.event;pending.valid=false;
 const DWORD esp=saved[3];
 if(esp>0xFFFFFFFBu){InterlockedIncrement(&g_mismatch);return;}
 const DWORD* frame=reinterpret_cast<const DWORD*>(static_cast<uintptr_t>(esp+4u));
 if(frame[0]!=e.outputPtr||frame[1]!=e.argSelector||e.tid!=GetCurrentThreadId()){
    InterlockedIncrement(&g_mismatch);return;
 }
 e.qpcAfter=Qpc();e.returnEax=saved[7];e.returnFlags=saved[8];
 Snapshot(e,true);
 if(e.preMask!=7u||e.postMask!=7u)InterlockedIncrement(&g_readMaskIncomplete);
 if((e.preMask&2u)&&(e.postMask&2u)){
   e.clearMask=(e.pre[1]&~e.post[1])&kSpecialBits;
   e.setMask=(~e.pre[1]&e.post[1])&kSpecialBits;
   if(e.clearMask)InterlockedIncrement(&g_specialCleared);
   if(e.setMask)InterlockedIncrement(&g_specialSet);
 }
 if(e.preMask==7u&&e.postMask==7u){
   e.anyChanged=(e.pre[0]!=e.post[0]||e.pre[1]!=e.post[1]||e.pre[2]!=e.post[2])?1u:0u;
   if(e.anyChanged)InterlockedIncrement(&g_changed);else InterlockedIncrement(&g_noChange);
 }
 InterlockedIncrement(&g_completed);
 if(InterlockedCompareExchange(&g_logAll,0,0)!=1&&
    !e.clearMask&&!e.setMask&&!e.anyChanged)return;
 e.seq=static_cast<DWORD>(InterlockedIncrement(&g_seq));
 AcquireSRWLockExclusive(&g_lock);
 if(g_count>=kCapacity)InterlockedIncrement(&g_dropped);
 else{g_ring[g_write]=e;g_write=(g_write+1u)%kCapacity;++g_count;}
 ReleaseSRWLockExclusive(&g_lock);
}
void Start(){
 wchar_t ini[MAX_PATH+1]{};
 if(arcana_stage81::GetPath(ini,_countof(ini),L"ArcanaAI.ini")){
    if(GetPrivateProfileIntW(L"AIResearch",L"EnableStage90AllF57D0Trace",1,ini)==0){
      Log("STAGE90_DISABLED EnableStage90AllF57D0Trace=0");return;
    }
    if(GetPrivateProfileIntW(L"AIResearch",L"EnableStage24ResolverConsumerTrace",0,ini)==0){
      Log("STAGE90_ERROR requires existing Stage24 EE306 hook enabled; no patch");return;
    }
    InterlockedExchange(&g_logAll,GetPrivateProfileIntW(L"AIResearch",L"Stage90LogAllF57D0",1,ini)?1:0);
 }
 if(InterlockedCompareExchange(&arcana_stage88::g_enabled,0,0)!=1){
    Log("STAGE90_ERROR Stage88 bridge inactive; no patch");return;
 }
 InterlockedExchange(&g_enabled,1);
 Log("STAGE90_READY r1 SOURCE_ONLY existing Stage24 EE306->F57D0 bridge; "
     "all P1/P2 selectors; bounded ring; noNewHook=1 nativeOnce=1 gameWrites=0 DRwrites=0");
}
void Flush(){
 for(DWORD i=0u;i<kCapacity;++i){
   Event e{};
   AcquireSRWLockExclusive(&g_lock);
   const bool has=g_count!=0u;
   if(has){e=g_ring[g_read];g_read=(g_read+1u)%kCapacity;--g_count;}
   ReleaseSRWLockExclusive(&g_lock);
   if(!has)break;
   char message[1250]{};
   sprintf_s(message,
    "STAGE90_F57D0 seq=%lu stage87Seq=%lu side=P%lu player=%08lX tid=%lu tick=%lu "
    "qpcBefore=%I64u qpcAfter=%I64u argSelector=%08lX arg3=%08lX arg4=%08lX "
    "dispatchType=%08lX dispatchPayload=%08lX dispatchMask=%lX "
    "tableRaw=%08lX tableMask=%lX tableClearsBitmap=%lu "
    "pre=%08lX,%08lX,%08lX post=%08lX,%08lX,%08lX preMask=%lX postMask=%lX "
    "clearMask=%08lX setMask=%08lX anyChanged=%lu returnEax=%08lX returnFlags=%08lX",
    e.seq,e.stage87Seq,e.side,e.player,e.tid,e.tick,
    e.qpcBefore,e.qpcAfter,e.argSelector,e.arg3,e.arg4,
    e.dispatchType,e.dispatchPayload,e.dispatchMask,
    e.tableRaw,e.tableMask,e.tableClearsBitmap,
    e.pre[0],e.pre[1],e.pre[2],e.post[0],e.post[1],e.post[2],
    e.preMask,e.postMask,e.clearMask,e.setMask,e.anyChanged,
    e.returnEax,e.returnFlags);
   Log(message);
 }
}
void Health(){
 if(InterlockedCompareExchange(&g_enabled,0,0)!=1)return;
 char message[650]{};
 sprintf_s(message,
   "STAGE90_HEALTH preCalls=%ld tracked=%ld completed=%ld filtered=%ld "
   "specialCleared=%ld specialSet=%ld changed=%ld noChange=%ld "
   "readMaskIncomplete=%ld depthOverflow=%ld mismatch=%ld readFail=%ld dropped=%ld "
   "logAll=%ld ring=8192 noNewHook=1 gameWrites=0 DRwrites=0",
   InterlockedExchange(&g_preCalls,0),InterlockedExchange(&g_tracked,0),
   InterlockedExchange(&g_completed,0),InterlockedExchange(&g_filtered,0),
   InterlockedExchange(&g_specialCleared,0),InterlockedExchange(&g_specialSet,0),
   InterlockedExchange(&g_changed,0),InterlockedExchange(&g_noChange,0),
   InterlockedExchange(&g_readMaskIncomplete,0),
   InterlockedExchange(&g_depthOverflow,0),InterlockedExchange(&g_mismatch,0),
   InterlockedExchange(&g_readFail,0),InterlockedExchange(&g_dropped,0),
   InterlockedCompareExchange(&g_logAll,0,0));
 Log(message);
}
} // namespace arcana_stage90
