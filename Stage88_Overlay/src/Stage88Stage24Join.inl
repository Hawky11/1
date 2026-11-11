// Stage88 r1 SOURCE ONLY: same-thread Stage87 EE2F0 -> existing Stage24 EE306.
// NO new hook. AIQueueTrace.cpp's existing Stage24 pre/post capture functions
// call Stage88Stage24Pre/Post. Native F57D0 still executes exactly once.
// Only passive readings of P+3A08/0C/10 and original table descriptors.
#ifndef _M_IX86
#error Stage88 requires Windows MSVC x86
#endif
namespace arcana_stage88 {
using arcana_stage81::g_gameBase;
constexpr DWORD kCapacity=8192u;
constexpr DWORD kTableRva=0x049DF188u;
constexpr DWORD kOutputOffset=0x3A08u;
struct Event {
 DWORD seq,stage87Seq,side,player,object,tid,tickPre;
 DWORD selector,mappedEax,argSelector,arg3,arg4;
 DWORD dispatchType,dispatchPayload,dispatchMask,tableRaw,tableMask;
 DWORD expectedBit,pre[3],post[3],preMask,postMask;
 DWORD nativeReturnEax,nativeFlags,match,bitBefore,bitAfter;
 ULONGLONG stage87Qpc,qpcPre,qpcPost;
};
SRWLOCK g_lock=SRWLOCK_INIT;
Event g_ring[kCapacity]{};
DWORD g_head=0,g_tail=0,g_count=0;
__declspec(thread) Event g_pending;
__declspec(thread) bool g_pendingValid=false;
volatile LONG g_enabled=0,g_seq=0,g_preCalls=0,g_joined=0,g_postCalls=0;
volatile LONG g_filtered=0,g_stale=0,g_mismatch=0,g_readFail=0,g_dropped=0;
void Log(const char* msg){
 wchar_t path[MAX_PATH+1]{};
 if(!arcana_stage81::GetPath(path,_countof(path),L"ArcanaAI_Stage88Stage24JoinTrace.log"))return;
 HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
     nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(f==INVALID_HANDLE_VALUE)return;
 DWORD w=0;WriteFile(f,msg,static_cast<DWORD>(strlen(msg)),&w,nullptr);
 WriteFile(f,"\r\n",2,&w,nullptr);CloseHandle(f);
}
bool Read(DWORD address,DWORD& value){
 __try{value=*reinterpret_cast<volatile DWORD*>(static_cast<uintptr_t>(address));return true;}
 __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&g_readFail);return false;}
}
ULONGLONG Qpc(){LARGE_INTEGER q{};QueryPerformanceCounter(&q);return static_cast<ULONGLONG>(q.QuadPart);}
void Pre(const DWORD* frame){
 if(InterlockedCompareExchange(&g_enabled,0,0)!=1||!frame)return;
 InterlockedIncrement(&g_preCalls);
 // Stage87 is reached only on the EDI=0x2C native branch. Consume its
 // record at the NEXT same-thread Stage24 entry (not nearest-match by time).
 if(!arcana_stage87::g_stage88Available){InterlockedIncrement(&g_filtered);return;}
 const arcana_stage87::Stage88Handoff r=arcana_stage87::g_stage88Last;
 arcana_stage87::g_stage88Available=0;
 g_pendingValid=false;
 Event e{};e.stage87Seq=r.seq;e.side=r.side;e.player=r.player;
 e.object=r.object;e.selector=r.selector;e.mappedEax=r.eax;
 e.stage87Qpc=r.qpc;e.tid=GetCurrentThreadId();e.tickPre=GetTickCount();e.qpcPre=Qpc();
 e.argSelector=frame[1];e.arg3=frame[2];e.arg4=frame[3];
 LARGE_INTEGER freq{};QueryPerformanceFrequency(&freq);
 const ULONGLONG maxGap=freq.QuadPart>0?static_cast<ULONGLONG>(freq.QuadPart)/20u:0;
 if(!maxGap||e.qpcPre<e.stage87Qpc||e.qpcPre-e.stage87Qpc>maxGap){
    InterlockedIncrement(&g_stale);return;
 }
 if(r.mask!=0xFu||r.match!=1u||r.player+kOutputOffset!=frame[0]||
    e.argSelector!=e.mappedEax||e.selector>2u||e.tid!=r.tid){
    InterlockedIncrement(&g_mismatch);return;
 }
 e.match=1u;e.expectedBit=1u<<(9u+e.selector);
 for(DWORD j=0;j<3;++j)
    if(Read(frame[0]+j*4u,e.pre[j]))e.preMask|=1u<<j;
 if(Read(frame[5]+8u,e.dispatchType))e.dispatchMask|=1u;
 if(Read(frame[5]+12u,e.dispatchPayload))e.dispatchMask|=2u;
 if(e.mappedEax<=0xFFu){
    const DWORD table=g_gameBase+kTableRva+e.mappedEax*3u;
    if(Read(table,e.tableRaw))e.tableMask=1u;
 }
 if((e.preMask&2u)!=0u)e.bitBefore=(e.pre[1]&e.expectedBit)?1u:0u;
 g_pending=e;g_pendingValid=true;
 InterlockedIncrement(&g_joined);
}
void Post(const DWORD* saved){
 if(!saved||InterlockedCompareExchange(&g_enabled,0,0)!=1||!g_pendingValid)return;
 Event e=g_pending;g_pendingValid=false;
 const DWORD esp=saved[3]; // PUSHAD saved ESP points to PUSHFD, locals at +4
 if(esp>0xFFFFFFFBu){InterlockedIncrement(&g_mismatch);return;}
 const DWORD* frame=reinterpret_cast<const DWORD*>(static_cast<uintptr_t>(esp+4u));
 if(frame[0]!=e.player+kOutputOffset||frame[1]!=e.argSelector||
    GetCurrentThreadId()!=e.tid){InterlockedIncrement(&g_mismatch);return;}
 e.qpcPost=Qpc();e.nativeReturnEax=saved[7];e.nativeFlags=saved[8];
 for(DWORD j=0;j<3;++j)
    if(Read(frame[0]+j*4u,e.post[j]))e.postMask|=1u<<j;
 if((e.postMask&2u)!=0u)e.bitAfter=(e.post[1]&e.expectedBit)?1u:0u;
 e.seq=static_cast<DWORD>(InterlockedIncrement(&g_seq));
 AcquireSRWLockExclusive(&g_lock);
 if(g_count==kCapacity)InterlockedIncrement(&g_dropped);
 else{g_ring[g_tail]=e;g_tail=(g_tail+1u)%kCapacity;++g_count;}
 ReleaseSRWLockExclusive(&g_lock);
 InterlockedIncrement(&g_postCalls);
}
void Start(){
 wchar_t ini[MAX_PATH+1]{};
 if(arcana_stage81::GetPath(ini,_countof(ini),L"ArcanaAI.ini")){
    if(GetPrivateProfileIntW(L"AIResearch",L"EnableStage88",1,ini)==0){
      Log("STAGE88_DISABLED EnableStage88=0");return;
    }
    if(GetPrivateProfileIntW(L"AIResearch",L"EnableStage24ResolverConsumerTrace",0,ini)==0){
      Log("STAGE88_ERROR Stage24 disabled: set EnableStage24ResolverConsumerTrace=1; no hook added");return;
    }
 }
 if(InterlockedCompareExchange(&arcana_stage87::g_installed,0,0)!=1){
    Log("STAGE88_ERROR Stage87 not installed; bridge disabled");return;
 }
 InterlockedExchange(&g_enabled,1);
 Log("STAGE88_READY r1 SOURCE_ONLY Stage87 EE2F0 -> existing Stage24 EE306/F57D0; "
     "needs compiled Stage24 Pre/Post bridge; newHook=0 gameWrites=0 DRwrites=0");
}
void Flush(){
 for(DWORD i=0;i<kCapacity;++i){
   Event e{};AcquireSRWLockExclusive(&g_lock);
   const bool has=g_count!=0;
   if(has){e=g_ring[g_head];g_head=(g_head+1u)%kCapacity;--g_count;}
   ReleaseSRWLockExclusive(&g_lock);if(!has)break;
   char msg[1250]{};
   sprintf_s(msg,
     "STAGE88_F57D0 seq=%lu stage87Seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu tick=%lu "
     "qpc87=%I64u qpcBefore=%I64u qpcAfter=%I64u "
     "selector=%08lX mappedEax=%08lX argSelector=%08lX arg3=%08lX arg4=%08lX "
     "dispatchType=%08lX dispatchPayload=%08lX dispatchMask=%lX "
     "tableRaw=%08lX tableMask=%lX expectedBit=%08lX bitBefore=%lu bitAfter=%lu "
     "pre=%08lX,%08lX,%08lX post=%08lX,%08lX,%08lX preMask=%lX postMask=%lX "
     "returnEax=%08lX returnFlags=%08lX match=%lu",
     e.seq,e.stage87Seq,e.side,e.player,e.object,e.tid,e.tickPre,
     e.stage87Qpc,e.qpcPre,e.qpcPost,
     e.selector,e.mappedEax,e.argSelector,e.arg3,e.arg4,
     e.dispatchType,e.dispatchPayload,e.dispatchMask,
     e.tableRaw,e.tableMask,e.expectedBit,e.bitBefore,e.bitAfter,
     e.pre[0],e.pre[1],e.pre[2],e.post[0],e.post[1],e.post[2],
     e.preMask,e.postMask,e.nativeReturnEax,e.nativeFlags,e.match);
   Log(msg);
 }
}
void Health(){
 if(InterlockedCompareExchange(&g_enabled,0,0)!=1)return;
 char msg[380]{};
 sprintf_s(msg,"STAGE88_HEALTH preCalls=%ld joined=%ld completed=%ld filtered=%ld "
   "stale=%ld mismatch=%ld readFail=%ld dropped=%ld stage87Total=%ld "
   "ring=8192 newHook=0 gameWrites=0 DRwrites=0",
   InterlockedExchange(&g_preCalls,0),InterlockedExchange(&g_joined,0),
   InterlockedExchange(&g_postCalls,0),InterlockedExchange(&g_filtered,0),
   InterlockedExchange(&g_stale,0),InterlockedExchange(&g_mismatch,0),
   InterlockedExchange(&g_readFail,0),InterlockedExchange(&g_dropped,0),
   InterlockedCompareExchange(&arcana_stage87::g_seq,0,0));Log(msg);
}
} // namespace arcana_stage88
extern "C" void __cdecl Stage88Stage24Pre(const DWORD* frame){arcana_stage88::Pre(frame);}
extern "C" void __cdecl Stage88Stage24Post(const DWORD* saved){arcana_stage88::Post(saved);}