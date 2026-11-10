// Stage56: passive per-instruction observer for the two ACDF0 X FADD/FSTP pairs.
// Original AALib.exe SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba.
// Patch each *complete* 6-byte [fadd dword ptr eax+7C; fstp dword ptr eax+7C]
// pair with JMP rel32 + NOP. Re-execute BOTH original instructions exactly once
// in each naked detour and jump to the exact original continuation.
// No changes to target arithmetic, AI decisions or coordinate data are intentional.
constexpr DWORD kS56Capacity=8192u,kS56Depth=8u;
struct S56Row {
  DWORD Seq=0,Side=0,Player=0,Object=0,Tid=0,Site=0,Store=0;
  DWORD Before=0,After=0,BeforeRead=0,AfterRead=0;
  ULONGLONG Q0=0,Q1=0;
};
struct S56Pending { S56Row Row{}; };
volatile LONG g_s56Installed=0,g_s56Calls=0,g_s56Rows=0;
volatile LONG g_s56Other=0,g_s56Dropped=0,g_s56Overflow=0,g_s56Mismatch=0;
volatile LONG g_s56ReadFail=0;
DWORD g_s56ResumeA=0,g_s56ResumeB=0;
__declspec(thread) DWORD g_s56Depth=0;
__declspec(thread) S56Pending g_s56Pending[kS56Depth]{};
SRWLOCK g_s56Lock=SRWLOCK_INIT;
S56Row g_s56Ring[kS56Capacity]{};
DWORD g_s56Read=0,g_s56Write=0,g_s56Count=0,g_s56Seq=0;

void S56Log(const char* message)
{
  wchar_t path[MAX_PATH+1]{};
  if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage56ACDF0WriterTrace.log"))return;
  SYSTEMTIME t{};GetLocalTime(&t);
  char line[1200]{};
  const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
    t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,message);
  if(n<=0)return;
  HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
    nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(f==INVALID_HANDLE_VALUE)return;
  DWORD written=0;WriteFile(f,line,static_cast<DWORD>(n),&written,nullptr);
  CloseHandle(f);
}
void __cdecl S56Enter(const DWORD* saved,DWORD site)
{
  InterlockedIncrement(&g_s56Calls);
  const DWORD depth=g_s56Depth++;
  if(depth>=kS56Depth){InterlockedIncrement(&g_s56Overflow);return;}
  S56Row& r=g_s56Pending[depth].Row;r=S56Row{};
  if(site!=0xACF27u&&site!=0xACFA0u){
    InterlockedIncrement(&g_s56Mismatch);return;
  }
  const DWORD target=saved[7]; // PUSHFD/PUSHAD: saved[7] is original EAX.
  const DWORD players[2]={g_playerP1,g_playerP2};
  DWORD objects[2]{};
  bool readable[2]={
    players[0]!=0u&&SafeReadDword(players[0]+4u,&objects[0]),
    players[1]!=0u&&SafeReadDword(players[1]+4u,&objects[1])
  };
  for(DWORD i=0;i<2u;++i){
    if(readable[i]&&objects[i]&&objects[i]==target){
      r.Side=i+1u;r.Player=players[i];break;
    }
  }
  if(!r.Side){InterlockedIncrement(&g_s56Other);return;}
  r.Object=target;r.Tid=GetCurrentThreadId();r.Site=site;r.Store=site+3u;
  r.Q0=Stage31Qpc();
  r.BeforeRead=SafeReadDword(target+0x7Cu,&r.Before)?1u:0u;
  if(!r.BeforeRead)InterlockedIncrement(&g_s56ReadFail);
}
void __cdecl S56Leave(const DWORD* saved,DWORD site)
{
  if(g_s56Depth==0u){InterlockedIncrement(&g_s56Mismatch);return;}
  const DWORD depth=--g_s56Depth;
  if(depth>=kS56Depth)return;
  S56Row& r=g_s56Pending[depth].Row;
  if(!r.Side)return; // Store was on an object other than the tracked P1/P2 objects.
  if(r.Site!=site||r.Object!=saved[7]){
    InterlockedIncrement(&g_s56Mismatch);r.Side=0u;return;
  }
  r.AfterRead=SafeReadDword(r.Object+0x7Cu,&r.After)?1u:0u;
  r.Q1=Stage31Qpc();
  if(!r.AfterRead)InterlockedIncrement(&g_s56ReadFail);
  AcquireSRWLockExclusive(&g_s56Lock);
  r.Seq=++g_s56Seq;
  if(g_s56Count==kS56Capacity)InterlockedIncrement(&g_s56Dropped);
  else {
    g_s56Ring[g_s56Write]=r;
    g_s56Write=(g_s56Write+1u)%kS56Capacity;
    ++g_s56Count;
  }
  ReleaseSRWLockExclusive(&g_s56Lock);
  InterlockedIncrement(&g_s56Rows);
  r.Side=0u;
}
// ACF27: fadd [eax+7C]; ACF2A: fstp [eax+7C]; resume ACF2D.
// JMP detour preserves the original stack (unlike CALL); all hooks preserve
// GPR/EFLAGS/FXSAVE around the C++ observers before/after the original pair.
__declspec(naked) void S56WriterAHook()
{
  __asm {
    pushfd
    pushad
    mov ebx,esp
    sub esp,210h
    lea edi,[esp+0Fh]
    and edi,0FFFFFFF0h
    fxsave [edi]
    fninit
    cld
    push 0ACF27h
    push ebx
    call S56Enter
    add esp,8
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    fadd dword ptr [eax+7Ch]
    fstp dword ptr [eax+7Ch]
    pushfd
    pushad
    mov ebx,esp
    sub esp,210h
    lea edi,[esp+0Fh]
    and edi,0FFFFFFF0h
    fxsave [edi]
    fninit
    cld
    push 0ACF27h
    push ebx
    call S56Leave
    add esp,8
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    jmp dword ptr [g_s56ResumeA]
  }
}
// ACFA0: fadd [eax+7C]; ACFA3: fstp [eax+7C]; resume ACFA6.
__declspec(naked) void S56WriterBHook()
{
  __asm {
    pushfd
    pushad
    mov ebx,esp
    sub esp,210h
    lea edi,[esp+0Fh]
    and edi,0FFFFFFF0h
    fxsave [edi]
    fninit
    cld
    push 0ACFA0h
    push ebx
    call S56Enter
    add esp,8
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    fadd dword ptr [eax+7Ch]
    fstp dword ptr [eax+7Ch]
    pushfd
    pushad
    mov ebx,esp
    sub esp,210h
    lea edi,[esp+0Fh]
    and edi,0FFFFFFF0h
    fxsave [edi]
    fninit
    cld
    push 0ACFA0h
    push ebx
    call S56Leave
    add esp,8
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    jmp dword ptr [g_s56ResumeB]
  }
}
bool InstallStage56Hooks()
{
  S56Log("STAGE56_ACDF0_WRITER_BUILD=2026-11-10-r1");
  if(InterlockedCompareExchange(&g_s55Installed,0,0)!=3){
    S56Log("ERROR: Stage56 requires installed Stage55 3/3; no patches.");return false;
  }
  if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
    S56Log("ERROR: Stage56 requires FXSAVE; no patches.");return false;
  }
  // Exact SHA-pinned original instruction-aligned signature around each 6-byte pair.
  // Neither signature overlaps a PE HIGHLOW relocation in the original.
  const BYTE sigA[]={0xD9,0x44,0x8D,0xA4,0xD8,0x40,0x7C,0xD9,
                     0x58,0x7C,0x8B,0x97,0x70,0x3B,0x00,0x00};
  const BYTE sigB[]={0xD9,0x84,0x8D,0x54,0xFF,0xFF,0xFF,0xD8,
                     0x40,0x7C,0xD9,0x58,0x7C,0x8B,0x5D,0xF4};
  const DWORD siteRvas[2]={0xACF27u,0xACFA0u};
  BYTE* sites[2]={
    reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+siteRvas[0])),
    reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+siteRvas[1]))
  };
  auto same=[&](DWORD rva,const BYTE* p,size_t count)->bool{
    for(size_t i=0;i<count;++i){
      DWORD value=0;
      if(!SafeReadDword(g_gameBase+rva+static_cast<DWORD>(i),&value)||
         static_cast<BYTE>(value)!=p[i])return false;
    }
    return true;
  };
  auto valid=[&]()->bool{
    const bool a=same(0xACF23u,sigA,sizeof(sigA));
    const bool b=same(0xACF99u,sigB,sizeof(sigB));
    if(!a||!b){
      char status[160]{};
      sprintf_s(status,"STAGE56_SIGNATURE_FAILURE siteACF27=%u siteACFA0=%u; no patches.",
        a?1u:0u,b?1u:0u);
      S56Log(status);
    }
    return a&&b;
  };
  if(!valid()){S56Log("ERROR: Stage56 original signatures mismatch; no patches.");return false;}
  BYTE patches[2][6]{};
  const void* hooks[2]={
    reinterpret_cast<const void*>(&S56WriterAHook),
    reinterpret_cast<const void*>(&S56WriterBHook)
  };
  for(DWORD i=0;i<2u;++i){
    if(!MakeRelativeCall(sites[i],hooks[i],patches[i])){
      S56Log("ERROR: Stage56 detour exceeds rel32 range; no patches.");return false;
    }
    patches[i][0]=0xE9u; // rel32 CALL->JMP, same displacement formula.
    patches[i][5]=0x90u; // the 6th original byte, instruction-aligned NOP.
  }
  HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
  if(!mutex){S56Log("ERROR: Stage56 mutex failed; no patches.");return false;}
  const DWORD wait=WaitForSingleObject(mutex,INFINITE);
  if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
    CloseHandle(mutex);S56Log("ERROR: Stage56 mutex wait failed; no patches.");return false;
  }
  HANDLE suspended[512]{};
  const DWORD stopped=SuspendOtherProcessThreads(suspended,512);
  DWORD oldProtection[2]{};
  bool writable[2]{};
  bool protectionsOk=true;
  for(DWORD i=0;i<2u;++i){
    writable[i]=VirtualProtect(sites[i],6u,PAGE_EXECUTE_READWRITE,&oldProtection[i])!=0;
    if(!writable[i]){protectionsOk=false;break;}
  }
  bool installed=false;
  if(protectionsOk&&valid()){
    g_s56ResumeA=g_gameBase+0xACF2Du;
    g_s56ResumeB=g_gameBase+0xACFA6u;
    for(DWORD i=0;i<2u;++i){
      std::memcpy(sites[i],patches[i],6u);
      FlushInstructionCache(GetCurrentProcess(),sites[i],6u);
    }
    InterlockedExchange(&g_s56Installed,2);
    installed=true;
  }
  for(DWORD i=0;i<2u;++i){
    if(writable[i]){
      DWORD ignored=0;VirtualProtect(sites[i],6u,oldProtection[i],&ignored);
    }
  }
  ResumeThreads(suspended,stopped);ReleaseMutex(mutex);CloseHandle(mutex);
  S56Log(installed?
    "STAGE56_INSTALLED 2/2 ACF27+ACF2A ACFA0+ACFA3; original FADD/FSTP once per hit; no game-state writes.":
    "ERROR: Stage56 install failed; Stage55 and prior hooks unaffected.");
  return installed;
}
void FlushStage56()
{
  for(DWORD i=0;i<kS56Capacity;++i){
    S56Row r{};
    AcquireSRWLockExclusive(&g_s56Lock);
    const bool has=g_s56Count!=0u;
    if(has){
      r=g_s56Ring[g_s56Read];g_s56Read=(g_s56Read+1u)%kS56Capacity;
      --g_s56Count;
    }
    ReleaseSRWLockExclusive(&g_s56Lock);
    if(!has)break;
    char line[550]{};
    sprintf_s(line,
      "STAGE56_WRITER seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu "
      "patchRVA=%08lX storeRVA=%08lX qpcBefore=%I64u qpcAfter=%I64u "
      "preRead=%lu postRead=%lu rawX=%08lX>%08lX changed=%lu",
      r.Seq,r.Side,r.Player,r.Object,r.Tid,r.Site,r.Store,r.Q0,r.Q1,
      r.BeforeRead,r.AfterRead,r.Before,r.After,
      r.BeforeRead&&r.AfterRead&&r.Before!=r.After?1u:0u);
    S56Log(line);
  }
  const LONG calls=InterlockedExchange(&g_s56Calls,0);
  const LONG rows=InterlockedExchange(&g_s56Rows,0);
  const LONG other=InterlockedExchange(&g_s56Other,0);
  const LONG dropped=InterlockedExchange(&g_s56Dropped,0);
  const LONG overflow=InterlockedExchange(&g_s56Overflow,0);
  const LONG mismatch=InterlockedExchange(&g_s56Mismatch,0);
  const LONG readFail=InterlockedExchange(&g_s56ReadFail,0);
  if(calls||rows||other||dropped||overflow||mismatch||readFail){
    char line[310]{};
    sprintf_s(line,
      "STAGE56_HEALTH calls=%ld rows=%ld otherObjects=%ld dropped=%ld "
      "overflow=%ld mismatch=%ld readFail=%ld ring=8192 depth=8.",
      calls,rows,other,dropped,overflow,mismatch,readFail);
    S56Log(line);
  }
}
