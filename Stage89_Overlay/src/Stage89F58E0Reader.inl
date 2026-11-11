// Stage89 r1 SOURCE ONLY — four guarded native CALL-site readers of controller bitmap.
// Original F58E0(bitmap=P+3A08,index=0x26) invoked EXACTLY ONCE per call.
// No new hook at EE306, no F58E0 entry detour, no game data / DR writes.
// Stage88 remains the preceding controller bitmap writer; only separate-session
// QPC correlation can locate nearby writes, not prove gameplay action causality.
#ifndef _M_IX86
#error Stage89 requires Windows MSVC x86 inline assembler
#endif
extern "C" {
DWORD g_Stage89F58E0NativeTarget=0;
void __cdecl Stage89Before(const DWORD* saved, DWORD site);
void __cdecl Stage89After(const DWORD* saved, DWORD site);
}
namespace arcana_stage89 {
using arcana_stage81::g_gameBase;
constexpr DWORD kSites[4]={0x9A870u,0x9AA20u,0x9AB00u,0xAAB55u};
constexpr DWORD kCount=4u,kCapacity=8192u,kDepth=16u,kBitmapOffset=0x3A08u;
constexpr BYTE kCall[4][5]={{0xE8,0x6B,0xB0,0x05,0x00},
                            {0xE8,0xBB,0xAE,0x05,0x00},
                            {0xE8,0xDB,0xAD,0x05,0x00},
                            {0xE8,0x86,0xAD,0x04,0x00}};
constexpr BYTE kBefore[4][9]={{0x05,0x08,0x3A,0,0,0x6A,0x26,0x50,0},
                              {0x05,0x08,0x3A,0,0,0x6A,0x26,0x50,0},
                              {0x05,0x08,0x3A,0,0,0x6A,0x26,0x50,0},
                              {0x8D,0x86,0x08,0x3A,0,0,0x6A,0x26,0x50}};
constexpr DWORD kBeforeLength[4]={8u,8u,8u,9u};
constexpr BYTE kAfter[4][8]={{0x83,0xC4,0x08,0x84,0xC0,0x74,0x4E,0},
                             {0x83,0xC4,0x08,0x84,0xC0,0x74,0x36,0},
                             {0x83,0xC4,0x08,0x56,0x84,0xC0,0x74,0x3F},
                             {0x83,0xC4,0x08,0x84,0xC0,0x75,0x11,0}};
constexpr DWORD kAfterLength[4]={7u,7u,8u,7u};
constexpr BYTE kTargetStart[9]={0x55,0x8B,0xEC,0x8B,0x45,0x0C,0x56,0x8B,0x75};
// Keep thread-local records POD: MSVC x86 __declspec(thread) must not need
// a dynamic C++ TLS constructor (C2483 in older toolsets).
struct Event {
 DWORD seq,site,side,player,bitmap,tid,index,returnRva;
 DWORD regsEsi,pre[3],post[3],preMask,postMask;
 DWORD nativeEax,nativeFlags,al,predictedPre,predictedPost;
 DWORD predictionValid,predictionMatch,nextBranchPredictedTaken;
 ULONGLONG qpcBefore,qpcAfter;
};
struct Pending { Event e; bool valid; };
__declspec(thread) Pending g_pending[kDepth]{};
__declspec(thread) DWORD g_depth=0;
SRWLOCK g_lock=SRWLOCK_INIT;
Event g_ring[kCapacity]{};
DWORD g_read=0,g_write=0,g_count=0;
volatile LONG g_installed=0,g_seq=0,g_calls[4]{},g_events=0,g_filtered=0;
volatile LONG g_overflow=0,g_dropped=0,g_mismatch=0,g_readFail=0;
DWORD g_mask=0;
void Log(const char* message){
 wchar_t path[MAX_PATH+1]{};
 if(!arcana_stage81::GetPath(path,_countof(path),L"ArcanaAI_Stage89F58E0ReaderTrace.log"))return;
 HANDLE file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,
     OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(file==INVALID_HANDLE_VALUE)return;
 DWORD written=0;
 WriteFile(file,message,static_cast<DWORD>(std::strlen(message)),&written,nullptr);
 WriteFile(file,"\r\n",2u,&written,nullptr);
 CloseHandle(file);
}
bool ReadDword(DWORD address,DWORD& out){
 __try {out=*reinterpret_cast<volatile DWORD*>(static_cast<uintptr_t>(address));return true;}
 __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&g_readFail);return false;}
}
ULONGLONG Qpc(){ LARGE_INTEGER t{};QueryPerformanceCounter(&t);return static_cast<ULONGLONG>(t.QuadPart); }
void Snapshot(Event& e,bool after){
 DWORD* values=after?e.post:e.pre;
 DWORD& mask=after?e.postMask:e.preMask;
 for(DWORD i=0;i<3u;++i){if(ReadDword(e.bitmap+4u*i,values[i]))mask|=1u<<i;}
}
// F58E0(0x26) checks the three special bits at bitmap+4, falling back to
// generic bit index 38 (bitmap+8 & 0x40) if all three are clear.
DWORD Predict(const DWORD* values){return ((values[1]&0xE00u)||(values[2]&0x40u))?1u:0u;}
void Before(const DWORD* saved,DWORD site){
 if(site>=kCount||!saved)return;
 InterlockedIncrement(&g_calls[site]);
 const DWORD depth=g_depth++;
 if(depth>=kDepth){InterlockedIncrement(&g_overflow);return;}
 Pending& p=g_pending[depth];p=Pending{};
 Event& e=p.e;e.site=site;e.tid=GetCurrentThreadId();
 // PUSHFD/PUSHAD: 9=original CALL return, 10=bitmap, 11=index.
 const DWORD ret=saved[9],ptr=saved[10],idx=saved[11];
 if(ret!=g_gameBase+kSites[site]+5u||idx!=0x26u||ptr<kBitmapOffset){
    InterlockedIncrement(&g_mismatch);return;
 }
 const DWORD player=ptr-kBitmapOffset,side=arcana_stage81::Side(player);
 if(!side){InterlockedIncrement(&g_filtered);return;}
 e.side=side;e.player=player;e.bitmap=ptr;e.index=idx;e.returnRva=kSites[site]+5u;
 e.regsEsi=saved[1];e.qpcBefore=Qpc();Snapshot(e,false);p.valid=true;
}
void After(const DWORD* saved,DWORD site){
 if(!saved||site>=kCount)return;
 if(!g_depth){InterlockedIncrement(&g_mismatch);return;}
 const DWORD depth=--g_depth;
 if(depth>=kDepth)return;
 Pending& p=g_pending[depth];
 if(!p.valid)return;
 Event e=p.e;p.valid=false;
 if(e.site!=site||e.tid!=GetCurrentThreadId()||saved[9]!=g_gameBase+kSites[site]+5u||
   saved[10]!=e.bitmap||saved[11]!=e.index){InterlockedIncrement(&g_mismatch);return;}
 e.qpcAfter=Qpc();e.nativeEax=saved[7];e.nativeFlags=saved[8];e.al=e.nativeEax&0xFFu;
 Snapshot(e,true);
 if((e.preMask&6u)==6u&&(e.postMask&6u)==6u){
    e.predictionValid=1u;e.predictedPre=Predict(e.pre);e.predictedPost=Predict(e.post);
    // Compare only if both snapshots agree: other threads may write between captures.
    if(e.predictedPre==e.predictedPost){
       e.predictionMatch=e.al==e.predictedPre?1u:0u;
       if(!e.predictionMatch)InterlockedIncrement(&g_mismatch);
    }else e.predictionValid=2u; // unstable: no AL mismatch assertion
 }
 // site0..2: JE if AL=0. site3: JNE if AL!=0 (skips E8560).
 e.nextBranchPredictedTaken=site==3u?(e.al!=0u):(e.al==0u);
 e.seq=static_cast<DWORD>(InterlockedIncrement(&g_seq));
 AcquireSRWLockExclusive(&g_lock);
 if(g_count>=kCapacity)InterlockedIncrement(&g_dropped);
 else{g_ring[g_write]=e;g_write=(g_write+1u)%kCapacity;++g_count;}
 ReleaseSRWLockExclusive(&g_lock);
 InterlockedIncrement(&g_events);
}
struct Suspended { HANDLE handles[512]{};DWORD count=0; };
void Resume(Suspended& s){while(s.count){HANDLE h=s.handles[--s.count];ResumeThread(h);CloseHandle(h);}}
bool SuspendAll(Suspended& s,DWORD mask){
 const DWORD pid=GetCurrentProcessId(),self=GetCurrentThreadId();
 HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
 if(snap==INVALID_HANDLE_VALUE)return false;
 THREADENTRY32 t{};t.dwSize=sizeof(t);BOOL more=Thread32First(snap,&t);
 bool ok=more!=FALSE;
 while(more&&ok){
  if(t.th32OwnerProcessID==pid&&t.th32ThreadID!=self){
   if(s.count>=_countof(s.handles)){ok=false;break;}
   HANDLE th=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT,FALSE,t.th32ThreadID);
   if(!th){ok=false;break;}
   if(SuspendThread(th)==static_cast<DWORD>(-1)){CloseHandle(th);ok=false;break;}
   s.handles[s.count++]=th;
   CONTEXT ctx{};ctx.ContextFlags=CONTEXT_CONTROL;
   if(!GetThreadContext(th,&ctx)){ok=false;break;}
   for(DWORD i=0;i<kCount;++i)if(mask&(1u<<i)){
     const DWORD from=g_gameBase+kSites[i];
     if(ctx.Eip>=from&&ctx.Eip<from+5u){ok=false;break;}
   }
  }
  t.dwSize=sizeof(t);more=Thread32Next(snap,&t);
 }
 CloseHandle(snap);
 if(!ok)Resume(s);
 return ok;
}
bool SignatureOK(DWORD mask){
 const BYTE* base=reinterpret_cast<const BYTE*>(static_cast<uintptr_t>(g_gameBase));
 const BYTE special[18]={0x8B,0x4E,0x04,0xF7,0xC1,0x00,0x02,0x00,0x00,
                         0x74,0x05,0xB0,0x01,0x5E,0x5D,0xC3,0xF7,0xC1};
 if(std::memcmp(base+0xF58E0u,kTargetStart,sizeof(kTargetStart))!=0||
    std::memcmp(base+0xF58EFu,special,sizeof(special))!=0)return false;
 for(DWORD i=0;i<kCount;++i)if(mask&(1u<<i)){
    const BYTE* s=base+kSites[i];
    if(std::memcmp(s,kCall[i],5u)!=0||
       std::memcmp(s-kBeforeLength[i],kBefore[i],kBeforeLength[i])!=0||
       std::memcmp(s+5,kAfter[i],kAfterLength[i])!=0)return false;
    std::int32_t rel=0;std::memcpy(&rel,s+1u,4u);
    if(static_cast<DWORD>(kSites[i]+5u+rel)!=0xF58E0u)return false;
 }
 return true;
}
bool PatchBytes(BYTE* site,const BYTE* value,DWORD count){
 DWORD old=0,unused=0;
 if(!VirtualProtect(site,count,PAGE_EXECUTE_READWRITE,&old))return false;
 std::memcpy(site,value,count);
 const bool flush=FlushInstructionCache(GetCurrentProcess(),site,count)!=FALSE;
 const bool restore=VirtualProtect(site,count,old,&unused)!=FALSE;
 return flush&&restore;
}
} // arcana_stage89
extern "C" {
void __cdecl Stage89Before(const DWORD* saved,DWORD site){arcana_stage89::Before(saved,site);}
void __cdecl Stage89After(const DWORD* saved,DWORD site){arcana_stage89::After(saved,site);}
}
// Separate CALL-site thunks (not a global F58E0 detour). Preserve native
// GPR/FLAGS/SSE/x87 around observer; replay original cdecl args exactly once.
extern "C" __declspec(naked) void Stage89Hook0() {
 __asm {
    pushfd
    pushad
    mov ebx,esp
    sub esp,220h
    and esp,0FFFFFFF0h
    fxsave [esp]
    fninit
    cld
    push 0
    push ebx
    call Stage89Before
    add esp,8
    fxrstor [esp]
    mov esp,ebx
    popad
    popfd
    push dword ptr [esp+8]  // native bitIndex, copied first
    push dword ptr [esp+8]  // native bitmap, original argument retained
    call dword ptr [g_Stage89F58E0NativeTarget]
    lea esp,[esp+8]        // preserve native EFLAGS
    pushfd
    pushad
    mov ebx,esp
    sub esp,220h
    and esp,0FFFFFFF0h
    fxsave [esp]
    fninit
    cld
    push 0
    push ebx
    call Stage89After
    add esp,8
    fxrstor [esp]
    mov esp,ebx
    popad
    popfd
    ret
 }
}
extern "C" __declspec(naked) void Stage89Hook1() {
 __asm {
    pushfd
    pushad
    mov ebx,esp
    sub esp,220h
    and esp,0FFFFFFF0h
    fxsave [esp]
    fninit
    cld
    push 1
    push ebx
    call Stage89Before
    add esp,8
    fxrstor [esp]
    mov esp,ebx
    popad
    popfd
    push dword ptr [esp+8]  // native bitIndex, copied first
    push dword ptr [esp+8]  // native bitmap, original argument retained
    call dword ptr [g_Stage89F58E0NativeTarget]
    lea esp,[esp+8]        // preserve native EFLAGS
    pushfd
    pushad
    mov ebx,esp
    sub esp,220h
    and esp,0FFFFFFF0h
    fxsave [esp]
    fninit
    cld
    push 1
    push ebx
    call Stage89After
    add esp,8
    fxrstor [esp]
    mov esp,ebx
    popad
    popfd
    ret
 }
}
extern "C" __declspec(naked) void Stage89Hook2() {
 __asm {
    pushfd
    pushad
    mov ebx,esp
    sub esp,220h
    and esp,0FFFFFFF0h
    fxsave [esp]
    fninit
    cld
    push 2
    push ebx
    call Stage89Before
    add esp,8
    fxrstor [esp]
    mov esp,ebx
    popad
    popfd
    push dword ptr [esp+8]  // native bitIndex, copied first
    push dword ptr [esp+8]  // native bitmap, original argument retained
    call dword ptr [g_Stage89F58E0NativeTarget]
    lea esp,[esp+8]        // preserve native EFLAGS
    pushfd
    pushad
    mov ebx,esp
    sub esp,220h
    and esp,0FFFFFFF0h
    fxsave [esp]
    fninit
    cld
    push 2
    push ebx
    call Stage89After
    add esp,8
    fxrstor [esp]
    mov esp,ebx
    popad
    popfd
    ret
 }
}
extern "C" __declspec(naked) void Stage89Hook3() {
 __asm {
    pushfd
    pushad
    mov ebx,esp
    sub esp,220h
    and esp,0FFFFFFF0h
    fxsave [esp]
    fninit
    cld
    push 3
    push ebx
    call Stage89Before
    add esp,8
    fxrstor [esp]
    mov esp,ebx
    popad
    popfd
    push dword ptr [esp+8]  // native bitIndex, copied first
    push dword ptr [esp+8]  // native bitmap, original argument retained
    call dword ptr [g_Stage89F58E0NativeTarget]
    lea esp,[esp+8]        // preserve native EFLAGS
    pushfd
    pushad
    mov ebx,esp
    sub esp,220h
    and esp,0FFFFFFF0h
    fxsave [esp]
    fninit
    cld
    push 3
    push ebx
    call Stage89After
    add esp,8
    fxrstor [esp]
    mov esp,ebx
    popad
    popfd
    ret
 }
}
namespace arcana_stage89 {
bool Install(){
 Log("STAGE89_BUILD=2026-11-11-r1 SOURCE_ONLY targeted F58E0 CALL readers; nativeOnce=1 noNewEE306=1 gameWrites=0 DRwrites=0");
 wchar_t ini[MAX_PATH+1]{};DWORD mask=15u;
 if(arcana_stage81::GetPath(ini,_countof(ini),L"ArcanaAI.ini")){
    if(GetPrivateProfileIntW(L"AIResearch",L"EnableStage89F58E0ReaderTrace",1,ini)==0){
       Log("STAGE89_DISABLED ini EnableStage89F58E0ReaderTrace=0");return false;
    }
    mask=static_cast<DWORD>(GetPrivateProfileIntW(L"AIResearch",L"Stage89ReaderSiteMask",15,ini));
 }
 if(!mask||(mask&~15u)){Log("STAGE89_ERROR invalid Stage89ReaderSiteMask; expected 1..15, no patch");return false;}
 HMODULE game=GetModuleHandleW(L"AALib.exe");
 if(sizeof(void*)!=4u||!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)||
    !arcana_stage81::Fingerprint(game)||
    InterlockedCompareExchange(&arcana_stage87::g_installed,0,0)!=1||
    InterlockedCompareExchange(&arcana_stage88::g_enabled,0,0)!=1){
    Log("STAGE89_ERROR PE/Stage87/Stage88 prerequisites missing; no patch");return false;
 }
 if(InterlockedCompareExchange(&g_installed,0,0)!=0||!SignatureOK(mask)){
    Log("STAGE89_ERROR live exact signature mismatch or already installed; no patch");return false;
 }
 BYTE* base=reinterpret_cast<BYTE*>(static_cast<uintptr_t>(g_gameBase));
 const void* handlers[4]={reinterpret_cast<const void*>(&Stage89Hook0),
                          reinterpret_cast<const void*>(&Stage89Hook1),
                          reinterpret_cast<const void*>(&Stage89Hook2),
                          reinterpret_cast<const void*>(&Stage89Hook3)};
 BYTE patch[4][5]{};
 for(DWORD i=0;i<kCount;++i)if(mask&(1u<<i)){
    const std::intptr_t delta=reinterpret_cast<std::intptr_t>(handlers[i])-
          reinterpret_cast<std::intptr_t>(base+kSites[i]+5u);
    if(delta<static_cast<std::intptr_t>(INT32_MIN)||delta>static_cast<std::intptr_t>(INT32_MAX)){
       Log("STAGE89_ERROR wrapper outside rel32 CALL range; no patch");return false;
    }
    const std::int32_t disp=static_cast<std::int32_t>(delta);
    patch[i][0]=0xE8u;std::memcpy(patch[i]+1,&disp,4u);
 }
 HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
 if(!mutex){Log("STAGE89_ERROR patch mutex unavailable");return false;}
 const DWORD wait=WaitForSingleObject(mutex,10000u);
 if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
     CloseHandle(mutex);Log("STAGE89_ERROR patch mutex wait failed");return false;
 }
 Suspended peers{};
 bool ok=SuspendAll(peers,mask);
 bool rollbackOK=true;
 DWORD touchedMask=0;
 if(ok){
    ok=SignatureOK(mask);
    if(ok){
       // Native function address is set before any patched thread can resume.
       g_Stage89F58E0NativeTarget=g_gameBase+0xF58E0u;
       for(DWORD i=0;i<kCount;++i)if(mask&(1u<<i)){
          touchedMask|=1u<<i;
          if(!PatchBytes(base+kSites[i],patch[i],5u)){ok=false;break;}
       }
    }
    if(!ok){
       for(DWORD i=0;i<kCount;++i)if(touchedMask&(1u<<i))
          if(!PatchBytes(base+kSites[i],kCall[i],5u))rollbackOK=false;
    }
    Resume(peers);
 }
 ReleaseMutex(mutex);CloseHandle(mutex);
 if(!ok||!rollbackOK){
   Log("STAGE89_ERROR patch refused/rollback attempted; restart and restore Stage88 DLL if needed");return false;
 }
 g_mask=mask;InterlockedExchange(&g_installed,1);
 char msg[300]{};
 sprintf_s(msg,"STAGE89_INSTALLED mask=%lX sites=%u/%u rvas=09A870,09AA20,09AB00,0AAB55 "
   "nativeTarget=0F58E0 originalOnce=1 noNewEE306=1 DRwrites=0 gameWrites=0",
   mask,(mask&1u)+((mask>>1)&1u)+((mask>>2)&1u)+((mask>>3)&1u),kCount);
 Log(msg);return true;
}
void Flush(){
 if(InterlockedCompareExchange(&g_installed,0,0)!=1)return;
 for(DWORD i=0;i<kCapacity;++i){
   Event e{};
   AcquireSRWLockExclusive(&g_lock);
   const bool has=g_count!=0;
   if(has){e=g_ring[g_read];g_read=(g_read+1u)%kCapacity;--g_count;}
   ReleaseSRWLockExclusive(&g_lock);
   if(!has)break;
   char msg[850]{};
   sprintf_s(msg,"STAGE89_F58E0 seq=%lu siteRVA=%06lX side=P%lu player=%08lX bitmap=%08lX tid=%lu "
     "qpcBefore=%I64u qpcAfter=%I64u bitIndex=%08lX returnRVA=%06lX esi=%08lX "
     "pre=%08lX,%08lX,%08lX post=%08lX,%08lX,%08lX preMask=%lX postMask=%lX "
     "nativeEax=%08lX returnAL=%lu returnFlags=%08lX predictedPre=%lu predictedPost=%lu "
     "predictionValid=%lu predictionMatch=%lu nextBranchPredictedTaken=%lu",
     e.seq,kSites[e.site],e.side,e.player,e.bitmap,e.tid,
     e.qpcBefore,e.qpcAfter,e.index,e.returnRva,e.regsEsi,
     e.pre[0],e.pre[1],e.pre[2],e.post[0],e.post[1],e.post[2],
     e.preMask,e.postMask,e.nativeEax,e.al,e.nativeFlags,
     e.predictedPre,e.predictedPost,e.predictionValid,e.predictionMatch,e.nextBranchPredictedTaken);
   Log(msg);
 }
}
void Health(){
 if(InterlockedCompareExchange(&g_installed,0,0)!=1)return;
 char msg[380]{};
 sprintf_s(msg,"STAGE89_HEALTH calls0=%ld calls1=%ld calls2=%ld calls3=%ld "
    "events=%ld filtered=%ld overflow=%ld dropped=%ld mismatch=%ld readFail=%ld "
    "ring=8192 depth=16 mask=%lX gameWrites=0 DRwrites=0",
    InterlockedExchange(&g_calls[0],0),InterlockedExchange(&g_calls[1],0),
    InterlockedExchange(&g_calls[2],0),InterlockedExchange(&g_calls[3],0),
    InterlockedExchange(&g_events,0),InterlockedExchange(&g_filtered,0),
    InterlockedExchange(&g_overflow,0),InterlockedExchange(&g_dropped,0),
    InterlockedExchange(&g_mismatch,0),InterlockedExchange(&g_readFail,0),g_mask);
 Log(msg);
}
} // arcana_stage89
