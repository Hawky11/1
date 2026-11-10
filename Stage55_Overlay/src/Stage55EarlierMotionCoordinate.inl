// Stage55: global AE6E0 (contains Stage47), then ACDF0 and AC8A0.
// No input/state/coordinate writes; every no-arg native is called once.
// Preserve all existing Stage47-54 hooks; compare nested Stage47 POST with AE6E0 POST.
constexpr DWORD kS55Capacity=8192u,kS55Depth=8u,kS55Fields=13u;
struct S55Row {
  DWORD Seq=0,Side=0,Player=0,Object=0,Tid=0,CallerRva=0,CallerRead=0;
  ULONGLONG Q0=0,Q1=0;
  DWORD PreMask=0,PostMask=0,Pre[kS55Fields]{},Post[kS55Fields]{};
};
struct S55Pending { S55Row rows[2]{}; };
volatile LONG g_s55Installed=0,g_s55Calls=0,g_s55Rows=0,g_s55Missing=0;
volatile LONG g_s55Dropped=0,g_s55Overflow=0,g_s55Mismatch=0,g_s55BadCaller=0;
DWORD g_s55NativeA=0,g_s55NativeB=0,g_s55NativeC=0;
__declspec(thread) DWORD g_s55Depth=0;
__declspec(thread) S55Pending g_s55Pending[kS55Depth]{};
SRWLOCK g_s55Lock=SRWLOCK_INIT;
S55Row g_s55Ring[kS55Capacity]{};
DWORD g_s55Read=0,g_s55Write=0,g_s55Count=0,g_s55Seq=0;

void S55Log(const char* message)
{
  wchar_t path[MAX_PATH+1]{};
  if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage55EarlierMotionCoordinateTrace.log"))return;
  SYSTEMTIME t{};GetLocalTime(&t);
  char line[2100]{};
  const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
    t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,message);
  if(n<=0)return;
  HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(f==INVALID_HANDLE_VALUE)return;
  DWORD written=0;WriteFile(f,line,static_cast<DWORD>(n),&written,nullptr);CloseHandle(f);
}
void S55Snap(DWORD object,DWORD* data,DWORD& mask)
{
  // Same thirteen field indices and offsets as Stage53/52/51.
  // f0=O+60, f1=O+7C X, f2=O+80 Y, f3=O+AC anchor X,
  // f4=O+B0 anchor Y, f5=O+3C, f6=O+1A4, f7=O+18,
  // f8=O+24, f9=O+2F0, f10=O+88, f11=O+8C, f12=O+10.
  const DWORD offsets[kS55Fields]={0x60u,0x7Cu,0x80u,0xACu,0xB0u,
    0x3Cu,0x1A4u,0x18u,0x24u,0x2F0u,0x88u,0x8Cu,0x10u};
  mask=0;if(!object)return;
  for(DWORD i=0;i<kS55Fields;++i)
    if(SafeReadDword(object+offsets[i],&data[i]))mask|=1u<<i;
}
void __cdecl S55Enter(const DWORD* saved)
{
  InterlockedIncrement(&g_s55Calls);
  DWORD d=g_s55Depth++;
  if(d>=kS55Depth){InterlockedIncrement(&g_s55Overflow);return;}
  S55Pending& p=g_s55Pending[d];p=S55Pending{};
  DWORD ret=0;
  const DWORD entryEsp=saved[3]+4u; // PUSHAD saved stack + PUSHFD.
  const bool callerRead=SafeReadDword(entryEsp,&ret);
  const DWORD caller=(callerRead&&ret>=g_gameBase+5u)?ret-g_gameBase-5u:0u;
  if(!callerRead||(caller!=0xA119Bu&&caller!=0xA11A0u&&caller!=0xA11A5u)){InterlockedIncrement(&g_s55BadCaller);return;}
  const DWORD players[2]={g_playerP1,g_playerP2};
  DWORD objects[2]{};
  const bool readable[2]={SafeReadDword(g_playerP1+4u,&objects[0]),
                          SafeReadDword(g_playerP2+4u,&objects[1])};
  for(DWORD i=0;i<2u;++i){
    S55Row& r=p.rows[i];
    if(!readable[i]||!objects[i]){InterlockedIncrement(&g_s55Missing);continue;}
    r.Side=i+1u;r.Player=players[i];r.Object=objects[i];
    r.Tid=GetCurrentThreadId();r.CallerRva=caller;r.CallerRead=1u;
    r.Q0=Stage31Qpc();S55Snap(r.Object,r.Pre,r.PreMask);
  }
}
void __cdecl S55Leave(const DWORD* /*saved*/)
{
  if(!g_s55Depth){InterlockedIncrement(&g_s55Mismatch);return;}
  DWORD d=--g_s55Depth;if(d>=kS55Depth)return;
  S55Pending& p=g_s55Pending[d];
  for(DWORD i=0;i<2u;++i){
    S55Row& r=p.rows[i];if(!r.Side)continue;
    r.Q1=Stage31Qpc();S55Snap(r.Object,r.Post,r.PostMask);
    AcquireSRWLockExclusive(&g_s55Lock);
    r.Seq=++g_s55Seq;
    if(g_s55Count==kS55Capacity)InterlockedIncrement(&g_s55Dropped);
    else {
      g_s55Ring[g_s55Write]=r;g_s55Write=(g_s55Write+1u)%kS55Capacity;
      ++g_s55Count;
    }
    ReleaseSRWLockExclusive(&g_s55Lock);
    InterlockedIncrement(&g_s55Rows);
    r.Side=0;
  }
}
// AE6E0 is a no-argument native called from one five-byte direct CALL.
__declspec(naked) void S55AE6E0Hook()
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
    push ebx
    call S55Enter
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    call dword ptr [g_s55NativeA]
    pushfd
    pushad
    mov ebx,esp
    sub esp,210h
    lea edi,[esp+0Fh]
    and edi,0FFFFFFF0h
    fxsave [edi]
    fninit
    cld
    push ebx
    call S55Leave
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    ret
  }
}
// ACDF0 is a no-argument native called from one five-byte direct CALL.
__declspec(naked) void S55ACDF0Hook()
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
    push ebx
    call S55Enter
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    call dword ptr [g_s55NativeB]
    pushfd
    pushad
    mov ebx,esp
    sub esp,210h
    lea edi,[esp+0Fh]
    and edi,0FFFFFFF0h
    fxsave [edi]
    fninit
    cld
    push ebx
    call S55Leave
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    ret
  }
}
// AC8A0 is a no-argument native called from one five-byte direct CALL.
__declspec(naked) void S55AC8A0Hook()
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
    push ebx
    call S55Enter
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    call dword ptr [g_s55NativeC]
    pushfd
    pushad
    mov ebx,esp
    sub esp,210h
    lea edi,[esp+0Fh]
    and edi,0FFFFFFF0h
    fxsave [edi]
    fninit
    cld
    push ebx
    call S55Leave
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    ret
  }
}
bool InstallStage55Hooks()
{
  S55Log("STAGE55_EARLIER_MOTION_COORD_BUILD=2026-11-10-r1");
  if(InterlockedCompareExchange(&g_s54Installed,0,0)!=3||
     InterlockedCompareExchange(&g_s53Installed,0,0)!=1||
     InterlockedCompareExchange(&g_s52Installed,0,0)!=2||
     InterlockedCompareExchange(&g_s47Installed,0,0)!=1){
    S55Log("ERROR: Stage55 requires installed Stage54/53/52/47; no patches.");return false;
  }
  if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
    S55Log("ERROR: Stage55 requires FXSAVE; no patches.");return false;
  }
  const DWORD rvas[3]={0xA119Bu,0xA11A0u,0xA11A5u};
  const DWORD targets[3]={0xAE6E0u,0xACDF0u,0xAC8A0u};
  const BYTE originals[3][5]={
    {0xE8,0x40,0xD5,0x00,0x00},
    {0xE8,0x4B,0xBC,0x00,0x00},
    {0xE8,0xF6,0xB6,0x00,0x00}
  };
  // Entire instruction sequences, verified against the SHA-pinned original PE.
  // These selected signatures contain no absolute relocated operands.
  const BYTE aPrologue[]={0x55,0x8B,0xEC,0x51,0x53,0x56,0x6A,0x00};
  const BYTE bPrologue[]={0x55,0x8B,0xEC,0x81,0xEC,0xAC,0x00,0x00,0x00,0x53,0x56,0x57};
  const BYTE cPrologue[]={0x55,0x8B,0xEC,0x83,0xEC,0x0C,0x53,0x56,0x57};
  auto same=[&](DWORD rva,const BYTE* bytes,size_t count)->bool{
    for(size_t i=0;i<count;++i){
      DWORD actual=0;
      if(!SafeReadDword(g_gameBase+rva+static_cast<DWORD>(i),&actual)||
         static_cast<BYTE>(actual)!=bytes[i])return false;
    }
    return true;
  };
  auto valid=[&]()->bool{
    const bool aValid=same(0xAE6E0u,aPrologue,sizeof(aPrologue));
    const bool bValid=same(0xACDF0u,bPrologue,sizeof(bPrologue));
    const bool cValid=same(0xAC8A0u,cPrologue,sizeof(cPrologue));
    bool callsValid[3]{};
    for(DWORD i=0;i<3u;++i){
      callsValid[i]=same(rvas[i],originals[i],5u);
      DWORD rel=0;
      callsValid[i]=callsValid[i]&&SafeReadDword(g_gameBase+rvas[i]+1u,&rel)&&
        (rvas[i]+5u+rel==targets[i]);
    }
    const bool allValid=aValid&&bValid&&cValid&&callsValid[0]&&callsValid[1]&&callsValid[2];
    if(!allValid){
      char status[256]{};
      sprintf_s(status,"STAGE55_SIGNATURE_FAILURE AE6E0=%u ACDF0=%u AC8A0=%u CALL_A119B=%u CALL_A11A0=%u CALL_A11A5=%u; no patches.",
        aValid?1u:0u,bValid?1u:0u,cValid?1u:0u,callsValid[0]?1u:0u,callsValid[1]?1u:0u,callsValid[2]?1u:0u);
      S55Log(status);
    }
    return allValid;
  };
  if(!valid()){S55Log("ERROR: Stage55 earlier-motion validation mismatch; no patches.");return false;}
  BYTE* sites[3]{};
  BYTE patches[3][5]{};
  const void* wrappers[3]={
    reinterpret_cast<const void*>(&S55AE6E0Hook),
    reinterpret_cast<const void*>(&S55ACDF0Hook),
    reinterpret_cast<const void*>(&S55AC8A0Hook)
  };
  for(DWORD i=0;i<3u;++i){
    sites[i]=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+rvas[i]));
    if(!MakeRelativeCall(sites[i],wrappers[i],patches[i])){
      S55Log("ERROR: Stage55 wrapper call outside rel32 range; no patches.");return false;
    }
  }
  HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
  if(!mutex){S55Log("ERROR: Stage55 mutex failed; no patches.");return false;}
  const DWORD wait=WaitForSingleObject(mutex,INFINITE);
  if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
    CloseHandle(mutex);S55Log("ERROR: Stage55 mutex wait failed; no patches.");return false;
  }
  HANDLE suspended[512]{};
  const DWORD stopped=SuspendOtherProcessThreads(suspended,512);
  DWORD oldProtection[3]{};
  bool writable[3]{};
  bool protectionsOk=true;
  for(DWORD i=0;i<3u;++i){
    writable[i]=VirtualProtect(sites[i],5u,PAGE_EXECUTE_READWRITE,&oldProtection[i])!=0;
    if(!writable[i]){protectionsOk=false;break;}
  }
  bool installed=false;
  if(protectionsOk&&valid()){
    g_s55NativeA=g_gameBase+targets[0];
    g_s55NativeB=g_gameBase+targets[1];
    g_s55NativeC=g_gameBase+targets[2];
    for(DWORD i=0;i<3u;++i){
      std::memcpy(sites[i],patches[i],5u);
      FlushInstructionCache(GetCurrentProcess(),sites[i],5u);
    }
    InterlockedExchange(&g_s55Installed,3);
    installed=true;
  }
  for(DWORD i=0;i<3u;++i){
    if(writable[i]){
      DWORD ignored=0;VirtualProtect(sites[i],5u,oldProtection[i],&ignored);
    }
  }
  ResumeThreads(suspended,stopped);ReleaseMutex(mutex);CloseHandle(mutex);
  S55Log(installed?
    "STAGE55_INSTALLED 3/3 A119B->AE6E0 A11A0->ACDF0 A11A5->AC8A0; native called once per site; no gameplay writes.":
    "ERROR: Stage55 install failed; previous hooks unaffected.");
  return installed;
}
void FlushStage55()
{
  for(DWORD i=0;i<kS55Capacity;++i){
    S55Row r{};
    AcquireSRWLockExclusive(&g_s55Lock);
    const bool has=g_s55Count!=0;
    if(has){
      r=g_s55Ring[g_s55Read];g_s55Read=(g_s55Read+1u)%kS55Capacity;
      --g_s55Count;
    }
    ReleaseSRWLockExclusive(&g_s55Lock);
    if(!has)break;
    DWORD changed=0;
    for(DWORD j=0;j<kS55Fields;++j)
      if((r.PreMask&r.PostMask&(1u<<j))&&r.Pre[j]!=r.Post[j])changed|=1u<<j;
    char line[1900]{};
    int n=sprintf_s(line,
      "STAGE55_EARLIER_WINDOW seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu callRVA=%08lX "
      "callerRead=%lu qpcBefore=%I64u qpcAfter=%I64u preMask=%04lX postMask=%04lX changedMask=%04lX",
      r.Seq,r.Side,r.Player,r.Object,r.Tid,r.CallerRva,r.CallerRead,
      r.Q0,r.Q1,r.PreMask,r.PostMask,changed);
    if(n<=0)continue;
    for(DWORD j=0;j<kS55Fields;++j){
      const int m=sprintf_s(line+n,sizeof(line)-static_cast<size_t>(n),
        " f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
      if(m<=0){n=0;break;}n+=m;
    }
    if(n>0)S55Log(line);
  }
  LONG calls=InterlockedExchange(&g_s55Calls,0),rows=InterlockedExchange(&g_s55Rows,0);
  LONG missing=InterlockedExchange(&g_s55Missing,0);
  LONG dropped=InterlockedExchange(&g_s55Dropped,0),overflow=InterlockedExchange(&g_s55Overflow,0);
  LONG mismatch=InterlockedExchange(&g_s55Mismatch,0),badCaller=InterlockedExchange(&g_s55BadCaller,0);
  if(calls||rows||missing||dropped||overflow||mismatch||badCaller){
    char line[280]{};
    sprintf_s(line,
      "STAGE55_HEALTH calls=%ld rows=%ld missingObjects=%ld dropped=%ld overflow=%ld "
      "mismatch=%ld badCaller=%ld ring=8192 depth=8.",
      calls,rows,missing,dropped,overflow,mismatch,badCaller);
    S55Log(line);
  }
}
