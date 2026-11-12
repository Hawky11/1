// Stage97 r1: passive native selected-command inheritance from [EBP-2Ch] at F77D4.
// AALib.exe SHA256: 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba
// Native bytes: F77D4 CMP [EBP-8],-1; MOV ESI,[EBP-2Ch]; F77DB JNE F789E.
// Only F77D4..F77DA is patched. Original CMP+MOV replay ONCE with original EFLAGS.
// No game data/DR writes, no synthetic commands, no overlapping Stage82/94/95/96 detours.
#ifndef _M_IX86
#error Stage97 requires MSVC Windows x86 inline assembly.
#endif
extern "C" {
 DWORD g_Stage97Resume77DB=0;
 void __cdecl Stage97Observe(const DWORD* saved);
}
namespace arcana_stage97 {
 constexpr DWORD kRva=0xF77D4u, kLength=7u, kCapacity=8192u;
 constexpr BYTE kNative[7]={0x83,0x7D,0xF8,0xFF,0x8B,0x75,0xD4};
 constexpr BYTE kJne[6]={0x0F,0x85,0xBD,0x00,0x00,0x00}; // F77DB -> F789E
 constexpr BYTE kLocalInit[3]={0x89,0x4D,0xD4}; // F7181: [EBP-2C]=-1
 constexpr BYTE kLocalConstant[7]={0xC7,0x45,0xD4,0x1F,0x00,0x00,0x00}; // F7714
 constexpr BYTE kLocalDynamic[3]={0x89,0x45,0xD4}; // F7756 after EBX+[EBP-34]
 struct Event {
  DWORD seq,side,player,tid,frame,preEsi,localD4,localM8,flag,localRead,m8Read,flagRead,valid;
  ULONGLONG qpc;
 };
 SRWLOCK g_lock=SRWLOCK_INIT;
 Event g_ring[kCapacity]{};
 DWORD g_read=0,g_write=0,g_count=0;
 volatile LONG g_installed=0,g_seq=0,g_hits=0,g_filtered=0,g_dropped=0,g_readFail=0,g_mismatch=0;
 void Log(const char* line){
  wchar_t p[MAX_PATH+1]{};
  if(!arcana_stage81::GetPath(p,_countof(p),L"ArcanaAI_Stage97InheritedEsiTrace.log"))return;
  HANDLE h=CreateFileW(p,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(h==INVALID_HANDLE_VALUE)return;
  DWORD n=0;WriteFile(h,line,static_cast<DWORD>(std::strlen(line)),&n,nullptr);
  WriteFile(h,"\r\n",2,&n,nullptr);CloseHandle(h);
 }
 bool Read32(DWORD a,DWORD& value){
  __try{value=*reinterpret_cast<volatile DWORD*>(static_cast<uintptr_t>(a));return true;}
  __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&g_readFail);return false;}
 }
 void Push(const Event& e){
  AcquireSRWLockExclusive(&g_lock);
  if(g_count==kCapacity)InterlockedIncrement(&g_dropped);
  else{g_ring[g_write]=e;g_write=(g_write+1u)%kCapacity;++g_count;}
  ReleaseSRWLockExclusive(&g_lock);
 }
 void Observe(const DWORD* saved){
  InterlockedIncrement(&g_hits);
  if(!saved){InterlockedIncrement(&g_mismatch);return;}
  Event e{};e.player=saved[0];e.side=arcana_stage81::Side(e.player);
  if(!e.side){InterlockedIncrement(&g_filtered);return;}
  e.preEsi=saved[1];e.frame=saved[2];e.tid=GetCurrentThreadId();
  LARGE_INTEGER q{};QueryPerformanceCounter(&q);e.qpc=static_cast<ULONGLONG>(q.QuadPart);
  e.seq=static_cast<DWORD>(InterlockedIncrement(&g_seq));
  if(e.frame>=0x2Cu&&Read32(e.frame-0x2Cu,e.localD4))e.localRead=1u;
  if(e.frame>=8u&&Read32(e.frame-8u,e.localM8))e.m8Read=1u;
  if(Read32(e.player+0x3B70u,e.flag))e.flagRead=1u;
  e.valid=e.localRead&&e.m8Read&&e.flagRead;
  Push(e);
 }
 struct Suspended{HANDLE h[512]{};DWORD count=0;};
 void Resume(Suspended& s){while(s.count){HANDLE h=s.h[--s.count];ResumeThread(h);CloseHandle(h);}}
 bool SuspendAll(Suspended& s){
  const DWORD pid=GetCurrentProcessId(),self=GetCurrentThreadId();
  HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
  if(snap==INVALID_HANDLE_VALUE)return false;
  THREADENTRY32 entry{};entry.dwSize=sizeof(entry);
  BOOL more=Thread32First(snap,&entry);bool ok=more!=FALSE;
  while(more&&ok){
   if(entry.th32OwnerProcessID==pid&&entry.th32ThreadID!=self){
    if(s.count==_countof(s.h)){ok=false;break;}
    HANDLE h=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT,FALSE,entry.th32ThreadID);
    if(!h){ok=false;break;}
    if(SuspendThread(h)==static_cast<DWORD>(-1)){CloseHandle(h);ok=false;break;}
    s.h[s.count++]=h;
    CONTEXT ctx{};ctx.ContextFlags=CONTEXT_CONTROL;
    if(!GetThreadContext(h,&ctx)){ok=false;break;}
    const DWORD site=arcana_stage81::g_gameBase+kRva;
    if(ctx.Eip>=site&&ctx.Eip<site+kLength){ok=false;break;}
   }
   entry.dwSize=sizeof(entry);more=Thread32Next(snap,&entry);
  }
  CloseHandle(snap);if(!ok)Resume(s);return ok;
 }
 bool Stage96OwnerOK(const BYTE* b){
  if(InterlockedCompareExchange(&arcana_stage96::g_installed,0,0)!=1||
     InterlockedCompareExchange(&arcana_stage95::g_installed,0,0)!=1)return false;
  const void* hooks[5]={reinterpret_cast<const void*>(&Stage96Hook7924),
   reinterpret_cast<const void*>(&Stage96Hook79B6),reinterpret_cast<const void*>(&Stage96Hook79D4),
   reinterpret_cast<const void*>(&Stage96Hook79F0),reinterpret_cast<const void*>(&Stage96Hook7A8F)};
  for(DWORD i=0;i<arcana_stage96::kCount;++i){
   const DWORD site=arcana_stage96::kSites[i];
   if(b[site]!=0xE9u)return false;
   std::int32_t rel=0;std::memcpy(&rel,b+site+1u,4u);
   const DWORD target=static_cast<DWORD>(arcana_stage81::g_gameBase+site+5u+rel);
   if(target!=static_cast<DWORD>(reinterpret_cast<uintptr_t>(hooks[i])))return false;
   for(DWORD j=5u;j<arcana_stage96::kLengths[i];++j)if(b[site+j]!=0x90u)return false;
  }
  return g_Stage96Resume7924==arcana_stage81::g_gameBase+0xF7929u&&
         g_Stage96Resume7A8F==arcana_stage81::g_gameBase+0xF7A94u;
 }
 bool SignatureOK(){
  const BYTE* b=reinterpret_cast<const BYTE*>(static_cast<uintptr_t>(arcana_stage81::g_gameBase));
  if(!b||!Stage96OwnerOK(b))return false;
  if(std::memcmp(b+kRva,kNative,sizeof(kNative))!=0||
     std::memcmp(b+0xF77DBu,kJne,sizeof(kJne))!=0||
     std::memcmp(b+0xF7181u,kLocalInit,sizeof(kLocalInit))!=0||
     std::memcmp(b+0xF7714u,kLocalConstant,sizeof(kLocalConstant))!=0||
     std::memcmp(b+0xF7756u,kLocalDynamic,sizeof(kLocalDynamic))!=0)return false;
  std::int32_t displacement=0;std::memcpy(&displacement,b+0xF77DDu,4u);
  return static_cast<DWORD>(0xF77E1u+displacement)==0xF789Eu;
 }
 void Diagnostic(){
  const BYTE* b=reinterpret_cast<const BYTE*>(static_cast<uintptr_t>(arcana_stage81::g_gameBase));
  if(!Stage96OwnerOK(b))Log("STAGE97_SIGFAIL Stage96 native detour ownership mismatch");
  if(std::memcmp(b+kRva,kNative,sizeof(kNative))!=0){
   char old[24]{},now[24]{},msg[240]{};
   for(DWORD j=0;j<kLength;++j){sprintf_s(old+j*2,sizeof(old)-j*2,"%02X",kNative[j]);
    sprintf_s(now+j*2,sizeof(now)-j*2,"%02X",b[kRva+j]);}
   sprintf_s(msg,"STAGE97_SIGFAIL F77D4 expected=%s actual=%s",old,now);Log(msg);
  }
  if(std::memcmp(b+0xF77DBu,kJne,sizeof(kJne))!=0)Log("STAGE97_SIGFAIL F77DB original JNE mismatch");
  if(std::memcmp(b+0xF7181u,kLocalInit,sizeof(kLocalInit))!=0||
     std::memcmp(b+0xF7714u,kLocalConstant,sizeof(kLocalConstant))!=0||
     std::memcmp(b+0xF7756u,kLocalDynamic,sizeof(kLocalDynamic))!=0)
   Log("STAGE97_SIGFAIL local candidate writer fingerprints mismatch");
 }
 bool Patch(BYTE* ptr,const BYTE* bytes,DWORD len){
  DWORD old=0,unused=0;
  if(!VirtualProtect(ptr,len,PAGE_EXECUTE_READWRITE,&old))return false;
  std::memcpy(ptr,bytes,len);
  const bool flush=FlushInstructionCache(GetCurrentProcess(),ptr,len)!=FALSE;
  const bool restored=VirtualProtect(ptr,len,old,&unused)!=FALSE;
  return flush&&restored;
 }
} // namespace arcana_stage97
extern "C" void __cdecl Stage97Observe(const DWORD* saved){arcana_stage97::Observe(saved);}
extern "C" __declspec(naked) void Stage97Hook77D4(){
 __asm {
  pushfd
  pushad
  mov ebx,esp
  sub esp,220h
  and esp,0FFFFFFF0h
  fxsave [esp]
  fninit
  cld
  push ebx
  call Stage97Observe
  lea esp,[esp+4]
  fxrstor [esp]
  mov esp,ebx
  popad
  popfd
  _emit 0x83
  _emit 0x7D
  _emit 0xF8
  _emit 0xFF
  _emit 0x8B
  _emit 0x75
  _emit 0xD4
  jmp dword ptr [g_Stage97Resume77DB]
 }
}
namespace arcana_stage97 {
 bool Install(){
  Log("STAGE97_BUILD=2026-11-12-r1 SOURCE_ONLY native F77D4 inherited ESI witness; originalOnce=1 gameWritesExtra=0 DRwrites=0");
  wchar_t ini[MAX_PATH+1]{};
  if(arcana_stage81::GetPath(ini,_countof(ini),L"ArcanaAI.ini")&&
     GetPrivateProfileIntW(L"AIResearch",L"EnableStage97InheritedEsiTrace",1,ini)==0){
   Log("STAGE97_DISABLED EnableStage97InheritedEsiTrace=0");return false;
  }
  HMODULE game=GetModuleHandleW(L"AALib.exe");
  if(sizeof(void*)!=4u||!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)||
     !arcana_stage81::Fingerprint(game)||
     InterlockedCompareExchange(&arcana_stage96::g_installed,0,0)!=1){
   Log("STAGE97_ERROR missing Stage96/fingerprint/x86 prerequisite; no patch");return false;
  }
  if(InterlockedCompareExchange(&g_installed,0,0)!=0){Log("STAGE97_ERROR already installed; no patch");return false;}
  if(!SignatureOK()){Diagnostic();Log("STAGE97_ERROR native signature changed/not exact; no patch");return false;}
  BYTE* base=reinterpret_cast<BYTE*>(static_cast<uintptr_t>(arcana_stage81::g_gameBase));
  const std::intptr_t delta=reinterpret_cast<std::intptr_t>(&Stage97Hook77D4)-
                             reinterpret_cast<std::intptr_t>(base+kRva+5u);
  if(delta<static_cast<std::intptr_t>(INT32_MIN)||delta>static_cast<std::intptr_t>(INT32_MAX)){
   Log("STAGE97_ERROR rel32 out of range; no patch");return false;
  }
  BYTE patch[kLength]{0xE9u,0,0,0,0,0x90u,0x90u};
  const std::int32_t rel=static_cast<std::int32_t>(delta);std::memcpy(patch+1,&rel,4u);
  HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
  if(!mutex){Log("STAGE97_ERROR mutex creation failed");return false;}
  const DWORD wait=WaitForSingleObject(mutex,10000u);
  if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){CloseHandle(mutex);Log("STAGE97_ERROR mutex wait failed");return false;}
  Suspended peers{};
  const bool suspended=SuspendAll(peers);
  bool ok=suspended,rollback=true,touched=false;
  if(suspended){
   ok=SignatureOK();
   if(ok){
    g_Stage97Resume77DB=arcana_stage81::g_gameBase+0xF77DBu;
    touched=true;
    ok=Patch(base+kRva,patch,kLength);
   }
   if(!ok&&touched)rollback=Patch(base+kRva,kNative,kLength);
   Resume(peers);
  }
  ReleaseMutex(mutex);CloseHandle(mutex);
  if(!ok||!rollback){Log("STAGE97_ERROR install refused/rollback attempted; restore tested Stage96 DLL on instability");return false;}
  InterlockedExchange(&g_installed,1);
  Log("STAGE97_INSTALLED 1/1 rva=0F77D4 originalOnce=1 Stage96Owner=1 nativeJNE=1 gameWritesExtra=0 DRwrites=0");
  return true;
 }
 void Flush(){
  if(InterlockedCompareExchange(&g_installed,0,0)!=1)return;
  for(DWORD i=0;i<kCapacity;++i){
   Event e{};AcquireSRWLockExclusive(&g_lock);
   const bool has=g_count!=0;
   if(has){e=g_ring[g_read];g_read=(g_read+1u)%kCapacity;--g_count;}
   ReleaseSRWLockExclusive(&g_lock);if(!has)break;
   char msg[600]{};
   sprintf_s(msg,"STAGE97_INHERITED seq=%lu side=P%lu player=%08lX tid=%lu frame=%08lX qpc=%I64u "
    "preEsi=%08lX localD4=%08lX nativeNextEsi=%08lX localM8=%08lX predictedJneTo789E=%lu "
    "flag3B70=%08lX localRead=%lu m8Read=%lu flagRead=%lu valid=%lu",
    e.seq,e.side,e.player,e.tid,e.frame,e.qpc,e.preEsi,e.localD4,e.localD4,e.localM8,
    e.m8Read&&e.localM8!=0xFFFFFFFFu?1u:0u,e.flag,e.localRead,e.m8Read,e.flagRead,e.valid);
   Log(msg);
  }
 }
 void Health(){
  if(InterlockedCompareExchange(&g_installed,0,0)!=1)return;
  char msg[330]{};
  sprintf_s(msg,"STAGE97_HEALTH inherited=%ld filtered=%ld dropped=%ld mismatch=%ld readFail=%ld "
   "ring=8192 gameWritesExtra=0 DRwrites=0",
   InterlockedExchange(&g_hits,0),InterlockedExchange(&g_filtered,0),
   InterlockedExchange(&g_dropped,0),InterlockedExchange(&g_mismatch,0),
   InterlockedExchange(&g_readFail,0));
  Log(msg);
 }
} // namespace arcana_stage97
