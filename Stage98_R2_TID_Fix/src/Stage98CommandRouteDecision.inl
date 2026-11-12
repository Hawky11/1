// Stage98 r1 — passive command-route predicate snapshot at AALib.exe RVA F7A00.
// Source-only, MSVC x86. Exactly replay original MOV EAX,[EDI+3B78] once.
// No F3930/F7A94/F7AD6/F7B0A/FinalCommand/EE306 hook collision.
// No new gameplay/DR writes; Stage36 MUST remain off. Provenance != AI improvement.
#ifndef _M_IX86
#error Stage98 requires MSVC Windows x86 native inline assembly.
#endif
extern "C" {
 DWORD g_Stage98Resume7A06 = 0;
 void __cdecl Stage98Observe(const DWORD* saved);
}
namespace arcana_stage98 {
 constexpr DWORD kRva = 0xF7A00u, kLength = 6u, kCapacity = 8192u;
 constexpr BYTE kOriginal[6] = {0x8B,0x87,0x78,0x3B,0x00,0x00};
 constexpr BYTE kFlagTest[8] = {0x8B,0xD0,0xC1,0xEA,0x08,0xF6,0xC2,0x01}; // F7A06..0D
 constexpr BYTE kJe[6] = {0x0F,0x84,0x85,0x00,0x00,0x00}; // F7A0E -> F7A99
 constexpr BYTE kFlag18[12] = {0x8B,0x8F,0x70,0x3B,0x00,0x00,0xC1,0xE9,0x12,0xF6,0xC1,0x01}; // F7A14..1F
 constexpr BYTE kJne[2] = {0x75,0x77}; // F7A20 -> F7A99
 // preClass: 1 early PRODUCER (F7A99); 2 status bypass (F7A94);
 // 3 bit15-clear bypass; 4 bit17-set bypass; 5 further conditions incl CALL525CD0.
 // 0 invalid/incomplete read. These are PRE-branch predictions only.
 struct Event {
  DWORD seq,side,player,tid,frame,preEsi,preEax,flags78,flags70,flags84;
  DWORD ptr4,state60,mode34,localM8,branchClass;
  DWORD r78,r70,r84,rPtr,rState,rMode,rM8,valid;
  ULONGLONG qpc;
 };
 SRWLOCK g_lock = SRWLOCK_INIT;
 Event g_ring[kCapacity]{};
 DWORD g_read = 0,g_write = 0,g_count = 0;
 volatile LONG g_installed=0,g_seq=0,g_hits=0,g_filtered=0,g_dropped=0,g_readFail=0,g_mismatch=0;
 void Log(const char* line) {
  wchar_t path[MAX_PATH+1]{};
  if(!arcana_stage81::GetPath(path,_countof(path),L"ArcanaAI_Stage98CommandRouteTrace.log"))return;
  HANDLE h=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(h==INVALID_HANDLE_VALUE)return;
  DWORD n=0;WriteFile(h,line,static_cast<DWORD>(std::strlen(line)),&n,nullptr);
  WriteFile(h,"\r\n",2,&n,nullptr);CloseHandle(h);
 }
 bool Read32(DWORD addr,DWORD& v) {
  if(!addr)return false;
  __try{v=*reinterpret_cast<volatile DWORD*>(static_cast<uintptr_t>(addr));return true;}
  __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&g_readFail);return false;}
 }
 bool Read16(DWORD addr,DWORD& v) {
  if(!addr)return false;
  __try{v=*reinterpret_cast<volatile WORD*>(static_cast<uintptr_t>(addr));return true;}
  __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&g_readFail);return false;}
 }
 DWORD PredClass(const Event& e) {
  if(!e.r78 || !e.r70 || !e.rState)return 0u;
  if(!(e.flags78 & (1u<<8)))return 1u;
  if(e.flags70 & (1u<<18))return 1u;
  if(e.state60>=0x816u && e.state60<=0x81Au)return 2u;
  if(!(e.flags78 & (1u<<15)))return 3u;
  if(e.flags78 & (1u<<17))return 4u;
  return 5u;
 }
 void Push(const Event& e) {
  AcquireSRWLockExclusive(&g_lock);
  if(g_count==kCapacity)InterlockedIncrement(&g_dropped);
  else{g_ring[g_write]=e;g_write=(g_write+1u)%kCapacity;++g_count;}
  ReleaseSRWLockExclusive(&g_lock);
 }
 void Observe(const DWORD* saved) {
  InterlockedIncrement(&g_hits);
  if(!saved){InterlockedIncrement(&g_mismatch);return;}
  Event e{};e.player=saved[0];e.side=arcana_stage81::Side(e.player);
  if(!e.side){InterlockedIncrement(&g_filtered);return;}
  e.preEsi=saved[1];e.frame=saved[2];e.preEax=saved[7]; // pushad EAX slot 7; ESP slot 3
  e.tid=GetCurrentThreadId();
  LARGE_INTEGER q{};QueryPerformanceCounter(&q);e.qpc=static_cast<ULONGLONG>(q.QuadPart);
  e.seq=static_cast<DWORD>(InterlockedIncrement(&g_seq));
  if(Read32(e.player+0x3B78u,e.flags78))e.r78=1u;
  if(Read32(e.player+0x3B70u,e.flags70))e.r70=1u;
  if(Read32(e.player+0x3B84u,e.flags84))e.r84=1u;
  if(Read32(e.player+4u,e.ptr4))e.rPtr=1u;
  if(e.rPtr && e.ptr4 && Read32(e.ptr4+0x60u,e.state60))e.rState=1u;
  if(Read16(e.player+0x3A34u,e.mode34))e.rMode=1u;
  if(e.frame>=8u && Read32(e.frame-8u,e.localM8))e.rM8=1u;
  e.valid=e.r78&&e.r70&&e.r84&&e.rPtr&&e.rState&&e.rMode&&e.rM8;
  e.branchClass=PredClass(e);Push(e);
 }
 struct Suspended{HANDLE h[512]{};DWORD count=0;};
 void Resume(Suspended& s){while(s.count){HANDLE h=s.h[--s.count];ResumeThread(h);CloseHandle(h);}}
 bool SuspendAll(Suspended& s) {
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
 bool Stage97OwnerOK(const BYTE* b) {
  if(!b || InterlockedCompareExchange(&arcana_stage97::g_installed,0,0)!=1 ||
     InterlockedCompareExchange(&arcana_stage96::g_installed,0,0)!=1 ||
     InterlockedCompareExchange(&arcana_stage95::g_installed,0,0)!=1 ||
     InterlockedCompareExchange(&arcana_stage94::g_installed,0,0)!=1 ||
     !arcana_stage97::Stage96OwnerOK(b))return false;
  const DWORD bypass=0xF7A94u;
  if(b[bypass]!=0xE9u)return false;
  std::int32_t bypassRel=0;std::memcpy(&bypassRel,b+bypass+1u,4u);
  if(static_cast<DWORD>(arcana_stage81::g_gameBase+bypass+5u+bypassRel)!=
        static_cast<DWORD>(reinterpret_cast<uintptr_t>(&Stage95BypassHook)) ||
     g_Stage95BypassResume!=arcana_stage81::g_gameBase+0xF7ADBu)return false;
  const DWORD site=0xF77D4u;
  if(b[site]!=0xE9u || b[site+5]!=0x90u || b[site+6]!=0x90u)return false;
  std::int32_t rel=0;std::memcpy(&rel,b+site+1,4);
  const DWORD target=static_cast<DWORD>(arcana_stage81::g_gameBase+site+5u+rel);
  return target==static_cast<DWORD>(reinterpret_cast<uintptr_t>(&Stage97Hook77D4))&&
         g_Stage97Resume77DB==arcana_stage81::g_gameBase+0xF77DBu;
 }
 bool SignatureOK() {
  const BYTE* b=reinterpret_cast<const BYTE*>(static_cast<uintptr_t>(arcana_stage81::g_gameBase));
  if(!Stage97OwnerOK(b))return false;
  if(std::memcmp(b+kRva,kOriginal,sizeof(kOriginal))!=0 ||
     std::memcmp(b+0xF7A06u,kFlagTest,sizeof(kFlagTest))!=0 ||
     std::memcmp(b+0xF7A0Eu,kJe,sizeof(kJe))!=0 ||
     std::memcmp(b+0xF7A14u,kFlag18,sizeof(kFlag18))!=0 ||
     std::memcmp(b+0xF7A20u,kJne,sizeof(kJne))!=0)return false;
  std::int32_t rel=0;std::memcpy(&rel,b+0xF7A10u,4u);
  if(static_cast<DWORD>(0xF7A14u+rel)!=0xF7A99u)return false;
  return static_cast<DWORD>(0xF7A22u+static_cast<std::int8_t>(b[0xF7A21u]))==0xF7A99u;
 }
 void Diagnostic() {
  const BYTE* b=reinterpret_cast<const BYTE*>(static_cast<uintptr_t>(arcana_stage81::g_gameBase));
  if(!Stage97OwnerOK(b))Log("STAGE98_SIGFAIL exact Stage97 detour owner mismatch");
  if(std::memcmp(b+kRva,kOriginal,sizeof(kOriginal))!=0)Log("STAGE98_SIGFAIL F7A00 native original bytes changed");
  if(std::memcmp(b+0xF7A06u,kFlagTest,sizeof(kFlagTest))!=0 ||
     std::memcmp(b+0xF7A0Eu,kJe,sizeof(kJe))!=0 ||
     std::memcmp(b+0xF7A14u,kFlag18,sizeof(kFlag18))!=0 ||
     std::memcmp(b+0xF7A20u,kJne,sizeof(kJne))!=0)
     Log("STAGE98_SIGFAIL F7A06-F7A21 conditional branch chain changed");
 }
 bool Patch(BYTE* ptr,const BYTE* bytes,DWORD len) {
  DWORD old=0,unused=0;
  if(!VirtualProtect(ptr,len,PAGE_EXECUTE_READWRITE,&old))return false;
  std::memcpy(ptr,bytes,len);
  const bool flushed=FlushInstructionCache(GetCurrentProcess(),ptr,len)!=FALSE;
  const bool restored=VirtualProtect(ptr,len,old,&unused)!=FALSE;
  return flushed&&restored;
 }
} // namespace arcana_stage98
extern "C" void __cdecl Stage98Observe(const DWORD* saved){arcana_stage98::Observe(saved);}
extern "C" __declspec(naked) void Stage98Hook7A00(){
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
  call Stage98Observe
  lea esp,[esp+4]
  fxrstor [esp]
  mov esp,ebx
  popad
  popfd
  // Replay exactly original MOV EAX,[EDI+3B78].
  _emit 0x8B
  _emit 0x87
  _emit 0x78
  _emit 0x3B
  _emit 0x00
  _emit 0x00
  jmp dword ptr [g_Stage98Resume7A06]
 }
}
namespace arcana_stage98 {
 bool Install() {
  Log("STAGE98_BUILD=2026-11-12-r1 SOURCE_ONLY F7A00 branch-predicate witness; originalOnce=1 gameWritesExtra=0 DRwrites=0");
  wchar_t ini[MAX_PATH+1]{};
  if(arcana_stage81::GetPath(ini,_countof(ini),L"ArcanaAI.ini") &&
      GetPrivateProfileIntW(L"AIResearch",L"EnableStage98CommandRouteTrace",1,ini)==0){
    Log("STAGE98_DISABLED EnableStage98CommandRouteTrace=0");return false;
  }
  HMODULE game=GetModuleHandleW(L"AALib.exe");
  if(sizeof(void*)!=4u || !IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) ||
      !arcana_stage81::Fingerprint(game) ||
      InterlockedCompareExchange(&arcana_stage97::g_installed,0,0)!=1){
    Log("STAGE98_ERROR missing Stage97/fingerprint/x86; no patch");return false;
  }
  if(InterlockedCompareExchange(&g_installed,0,0)!=0){Log("STAGE98_ERROR already installed; no patch");return false;}
  if(!SignatureOK()){Diagnostic();Log("STAGE98_ERROR native signature changed/not exact; no patch");return false;}
  BYTE* base=reinterpret_cast<BYTE*>(static_cast<uintptr_t>(arcana_stage81::g_gameBase));
  const std::intptr_t delta=reinterpret_cast<std::intptr_t>(&Stage98Hook7A00)-
                            reinterpret_cast<std::intptr_t>(base+kRva+5u);
  if(delta<static_cast<std::intptr_t>(INT32_MIN)||delta>static_cast<std::intptr_t>(INT32_MAX)){
    Log("STAGE98_ERROR rel32 out of range; no patch");return false;
  }
  BYTE patch[kLength]{0xE9u,0,0,0,0,0x90u};
  const std::int32_t rel=static_cast<std::int32_t>(delta);std::memcpy(patch+1,&rel,4u);
  HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
  if(!mutex){Log("STAGE98_ERROR mutex creation failed");return false;}
  const DWORD wait=WaitForSingleObject(mutex,10000u);
  if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){CloseHandle(mutex);Log("STAGE98_ERROR mutex wait failed");return false;}
  Suspended peers{};const bool suspended=SuspendAll(peers);
  bool ok=suspended,rollback=true,touched=false;
  if(suspended){
    ok=SignatureOK();
    if(ok){
      g_Stage98Resume7A06=arcana_stage81::g_gameBase+0xF7A06u;
      touched=true;ok=Patch(base+kRva,patch,kLength);
    }
    if(!ok && touched)rollback=Patch(base+kRva,kOriginal,kLength);
    Resume(peers);
  }
  ReleaseMutex(mutex);CloseHandle(mutex);
  if(!ok||!rollback){Log("STAGE98_ERROR install refused/rollback attempted; restore tested Stage97 DLL on instability");return false;}
  InterlockedExchange(&g_installed,1);
  Log("STAGE98_INSTALLED 1/1 rva=0F7A00 resume=0F7A06 Stage97Owner=1 originalOnce=1 nativeBranch=1 gameWritesExtra=0 DRwrites=0");
  return true;
 }
 void Flush() {
  if(InterlockedCompareExchange(&g_installed,0,0)!=1)return;
  for(DWORD i=0;i<kCapacity;++i){
    Event e{};AcquireSRWLockExclusive(&g_lock);
    const bool has=g_count!=0;
    if(has){e=g_ring[g_read];g_read=(g_read+1u)%kCapacity;--g_count;}
    ReleaseSRWLockExclusive(&g_lock);if(!has)break;
    char msg[750]{};
    sprintf_s(msg,"STAGE98_ROUTE seq=%lu side=P%lu player=%08lX tid=%lu frame=%08lX qpc=%I64u "
      "preEsi=%08lX preEax=%08lX flags3B78=%08lX flags3B70=%08lX flags3B84=%08lX "
      "ptr4=%08lX state60=%08lX mode3A34=%04lX localM8=%08lX branchClass=%lu "
      "r78=%lu r70=%lu r84=%lu rPtr=%lu rState=%lu rMode=%lu rM8=%lu valid=%lu",
      e.seq,e.side,e.player,e.tid,e.frame,e.qpc,e.preEsi,e.preEax,e.flags78,e.flags70,e.flags84,
      e.ptr4,e.state60,e.mode34,e.localM8,e.branchClass,e.r78,e.r70,e.r84,e.rPtr,e.rState,e.rMode,e.rM8,e.valid);
    Log(msg);
  }
 }
 void Health() {
  if(InterlockedCompareExchange(&g_installed,0,0)!=1)return;
  char msg[350]{};
  sprintf_s(msg,"STAGE98_HEALTH route=%ld filtered=%ld dropped=%ld mismatch=%ld readFail=%ld "
    "ring=8192 gameWritesExtra=0 DRwrites=0",
    InterlockedExchange(&g_hits,0),InterlockedExchange(&g_filtered,0),
    InterlockedExchange(&g_dropped,0),InterlockedExchange(&g_mismatch,0),
    InterlockedExchange(&g_readFail,0));
  Log(msg);
 }
}
