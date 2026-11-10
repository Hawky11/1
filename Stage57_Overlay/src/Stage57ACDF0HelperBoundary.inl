// Stage57: three nested ACDF0 helper native boundaries: early A2C40; final 15CEC0,15B8A0.
// Passive two-player before/after snapshots; native original called once per site.
// Exact original x86 CALL/prologue signatures verified against SHA-pinned AALib.exe.
// Preserve all previous Stage47-56 hooks and current AI decisions.
constexpr DWORD kS57Capacity=8192u,kS57Depth=8u,kS57Fields=13u;
struct S57Row {
  DWORD Seq=0,Side=0,Player=0,Object=0,Tid=0,CallerRva=0,CallerRead=0;
  ULONGLONG Q0=0,Q1=0;
  DWORD PreMask=0,PostMask=0,Pre[kS57Fields]{},Post[kS57Fields]{};
};
struct S57Pending { S57Row rows[2]{}; };
volatile LONG g_s57Installed=0,g_s57Calls=0,g_s57Rows=0,g_s57Missing=0;
volatile LONG g_s57Dropped=0,g_s57Overflow=0,g_s57Mismatch=0,g_s57BadCaller=0;
DWORD g_s57NativeA=0,g_s57NativeB=0,g_s57NativeC=0;
__declspec(thread) DWORD g_s57Depth=0;
__declspec(thread) S57Pending g_s57Pending[kS57Depth]{};
SRWLOCK g_s57Lock=SRWLOCK_INIT;
S57Row g_s57Ring[kS57Capacity]{};
DWORD g_s57Read=0,g_s57Write=0,g_s57Count=0,g_s57Seq=0;

void S57Log(const char* message)
{
  wchar_t path[MAX_PATH+1]{};
  if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage57ACDF0HelperBoundaryTrace.log"))return;
  SYSTEMTIME t{};GetLocalTime(&t);
  char line[2100]{};
  const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
    t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,message);
  if(n<=0)return;
  HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(f==INVALID_HANDLE_VALUE)return;
  DWORD written=0;WriteFile(f,line,static_cast<DWORD>(n),&written,nullptr);CloseHandle(f);
}
void S57Snap(DWORD object,DWORD* data,DWORD& mask)
{
  // Same thirteen field indices and offsets as Stage53/52/51.
  // f0=O+60, f1=O+7C X, f2=O+80 Y, f3=O+AC anchor X,
  // f4=O+B0 anchor Y, f5=O+3C, f6=O+1A4, f7=O+18,
  // f8=O+24, f9=O+2F0, f10=O+88, f11=O+8C, f12=O+10.
  const DWORD offsets[kS57Fields]={0x60u,0x7Cu,0x80u,0xACu,0xB0u,
    0x3Cu,0x1A4u,0x18u,0x24u,0x2F0u,0x88u,0x8Cu,0x10u};
  mask=0;if(!object)return;
  for(DWORD i=0;i<kS57Fields;++i)
    if(SafeReadDword(object+offsets[i],&data[i]))mask|=1u<<i;
}
void __cdecl S57Enter(const DWORD* saved)
{
  InterlockedIncrement(&g_s57Calls);
  DWORD d=g_s57Depth++;
  if(d>=kS57Depth){InterlockedIncrement(&g_s57Overflow);return;}
  S57Pending& p=g_s57Pending[d];p=S57Pending{};
  DWORD ret=0;
  const DWORD entryEsp=saved[3]+4u; // PUSHAD saved stack + PUSHFD.
  const bool callerRead=SafeReadDword(entryEsp,&ret);
  const DWORD caller=(callerRead&&ret>=g_gameBase+5u)?ret-g_gameBase-5u:0u;
  if(!callerRead||(caller!=0xACE11u&&caller!=0xACFD6u&&caller!=0xACFDBu)){InterlockedIncrement(&g_s57BadCaller);return;}
  const DWORD players[2]={g_playerP1,g_playerP2};
  DWORD objects[2]{};
  const bool readable[2]={SafeReadDword(g_playerP1+4u,&objects[0]),
                          SafeReadDword(g_playerP2+4u,&objects[1])};
  for(DWORD i=0;i<2u;++i){
    S57Row& r=p.rows[i];
    if(!readable[i]||!objects[i]){InterlockedIncrement(&g_s57Missing);continue;}
    r.Side=i+1u;r.Player=players[i];r.Object=objects[i];
    r.Tid=GetCurrentThreadId();r.CallerRva=caller;r.CallerRead=1u;
    r.Q0=Stage31Qpc();S57Snap(r.Object,r.Pre,r.PreMask);
  }
}
void __cdecl S57Leave(const DWORD* /*saved*/)
{
  if(!g_s57Depth){InterlockedIncrement(&g_s57Mismatch);return;}
  DWORD d=--g_s57Depth;if(d>=kS57Depth)return;
  S57Pending& p=g_s57Pending[d];
  for(DWORD i=0;i<2u;++i){
    S57Row& r=p.rows[i];if(!r.Side)continue;
    r.Q1=Stage31Qpc();S57Snap(r.Object,r.Post,r.PostMask);
    AcquireSRWLockExclusive(&g_s57Lock);
    r.Seq=++g_s57Seq;
    if(g_s57Count==kS57Capacity)InterlockedIncrement(&g_s57Dropped);
    else {
      g_s57Ring[g_s57Write]=r;g_s57Write=(g_s57Write+1u)%kS57Capacity;
      ++g_s57Count;
    }
    ReleaseSRWLockExclusive(&g_s57Lock);
    InterlockedIncrement(&g_s57Rows);
    r.Side=0;
  }
}
// A2C40 is a no-stack-argument native called once from ACE11.
__declspec(naked) void S57A2C40Hook()
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
    call S57Enter
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    call dword ptr [g_s57NativeA]
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
    call S57Leave
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    ret
  }
}
// 15CEC0 is a no-stack-argument native called once from ACFD6.
__declspec(naked) void S57CEC0Hook()
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
    call S57Enter
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    call dword ptr [g_s57NativeB]
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
    call S57Leave
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    ret
  }
}
// 15B8A0 is a no-stack-argument native called once from ACFDB.
__declspec(naked) void S57B8A0Hook()
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
    call S57Enter
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    call dword ptr [g_s57NativeC]
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
    call S57Leave
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    ret
  }
}
bool InstallStage57Hooks()
{
  S57Log("STAGE57_ACDF0_HELPER_BUILD=2026-11-10-r1");
  if(InterlockedCompareExchange(&g_s56Installed,0,0)!=2||
     InterlockedCompareExchange(&g_s55Installed,0,0)!=3){
    S57Log("ERROR: Stage57 requires installed Stage56 2/2 and Stage55 3/3; no patches.");return false;
  }
  if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
    S57Log("ERROR: Stage57 requires FXSAVE; no patches.");return false;
  }
  const DWORD rvas[3]={0xACE11u,0xACFD6u,0xACFDBu};
  const DWORD targets[3]={0xA2C40u,0x15CEC0u,0x15B8A0u};
  const BYTE originals[3][5]={
    {0xE8,0x2A,0x5E,0xFF,0xFF},
    {0xE8,0xE5,0xFE,0x0A,0x00},
    {0xE8,0xC0,0xE8,0x0A,0x00}
  };
  // Entire instruction sequences, verified against the SHA-pinned original PE.
  // These selected signatures contain no absolute relocated operands.
  const BYTE aPrologue[]={0x55,0x8B,0xEC,0xB8,0x60,0x14,0x00,0x00,0xE8,0x83,0x88,0x16,0x00};
  const BYTE bPrologue[]={0x55,0x8B,0xEC,0x51,0x56,0x57,0xE8,0xA5,0x7D,0x08,0x00};
  const BYTE cPrologue[]={0x55,0x8B,0xEC,0x81,0xEC,0xC8,0x03,0x00,0x00,0x53,0x56,0x57};
  auto same=[&](DWORD rva,const BYTE* bytes,size_t count)->bool{
    for(size_t i=0;i<count;++i){
      DWORD actual=0;
      if(!SafeReadDword(g_gameBase+rva+static_cast<DWORD>(i),&actual)||
         static_cast<BYTE>(actual)!=bytes[i])return false;
    }
    return true;
  };
  auto valid=[&]()->bool{
    const bool aValid=same(0xA2C40u,aPrologue,sizeof(aPrologue));
    const bool bValid=same(0x15CEC0u,bPrologue,sizeof(bPrologue));
    const bool cValid=same(0x15B8A0u,cPrologue,sizeof(cPrologue));
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
      sprintf_s(status,"STAGE57_SIGNATURE_FAILURE A2C40=%u 15CEC0=%u 15B8A0=%u CALL_ACE11=%u CALL_ACFD6=%u CALL_ACFDB=%u; no patches.",
        aValid?1u:0u,bValid?1u:0u,cValid?1u:0u,callsValid[0]?1u:0u,callsValid[1]?1u:0u,callsValid[2]?1u:0u);
      S57Log(status);
    }
    return allValid;
  };
  if(!valid()){S57Log("ERROR: Stage57 helper-call validation mismatch; no patches.");return false;}
  BYTE* sites[3]{};
  BYTE patches[3][5]{};
  const void* wrappers[3]={
    reinterpret_cast<const void*>(&S57A2C40Hook),
    reinterpret_cast<const void*>(&S57CEC0Hook),
    reinterpret_cast<const void*>(&S57B8A0Hook)
  };
  for(DWORD i=0;i<3u;++i){
    sites[i]=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+rvas[i]));
    if(!MakeRelativeCall(sites[i],wrappers[i],patches[i])){
      S57Log("ERROR: Stage57 wrapper call outside rel32 range; no patches.");return false;
    }
  }
  HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
  if(!mutex){S57Log("ERROR: Stage57 mutex failed; no patches.");return false;}
  const DWORD wait=WaitForSingleObject(mutex,INFINITE);
  if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
    CloseHandle(mutex);S57Log("ERROR: Stage57 mutex wait failed; no patches.");return false;
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
    g_s57NativeA=g_gameBase+targets[0];
    g_s57NativeB=g_gameBase+targets[1];
    g_s57NativeC=g_gameBase+targets[2];
    for(DWORD i=0;i<3u;++i){
      std::memcpy(sites[i],patches[i],5u);
      FlushInstructionCache(GetCurrentProcess(),sites[i],5u);
    }
    InterlockedExchange(&g_s57Installed,3);
    installed=true;
  }
  for(DWORD i=0;i<3u;++i){
    if(writable[i]){
      DWORD ignored=0;VirtualProtect(sites[i],5u,oldProtection[i],&ignored);
    }
  }
  ResumeThreads(suspended,stopped);ReleaseMutex(mutex);CloseHandle(mutex);
  S57Log(installed?
    "STAGE57_INSTALLED 3/3 ACE11->A2C40 ACFD6->15CEC0 ACFDB->15B8A0; native called once per site; no gameplay writes.":
    "ERROR: Stage57 install failed; previous hooks unaffected.");
  return installed;
}
void FlushStage57()
{
  for(DWORD i=0;i<kS57Capacity;++i){
    S57Row r{};
    AcquireSRWLockExclusive(&g_s57Lock);
    const bool has=g_s57Count!=0;
    if(has){
      r=g_s57Ring[g_s57Read];g_s57Read=(g_s57Read+1u)%kS57Capacity;
      --g_s57Count;
    }
    ReleaseSRWLockExclusive(&g_s57Lock);
    if(!has)break;
    DWORD changed=0;
    for(DWORD j=0;j<kS57Fields;++j)
      if((r.PreMask&r.PostMask&(1u<<j))&&r.Pre[j]!=r.Post[j])changed|=1u<<j;
    char line[1900]{};
    int n=sprintf_s(line,
      "STAGE57_HELPER_WINDOW seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu callRVA=%08lX "
      "callerRead=%lu qpcBefore=%I64u qpcAfter=%I64u preMask=%04lX postMask=%04lX changedMask=%04lX",
      r.Seq,r.Side,r.Player,r.Object,r.Tid,r.CallerRva,r.CallerRead,
      r.Q0,r.Q1,r.PreMask,r.PostMask,changed);
    if(n<=0)continue;
    for(DWORD j=0;j<kS57Fields;++j){
      const int m=sprintf_s(line+n,sizeof(line)-static_cast<size_t>(n),
        " f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
      if(m<=0){n=0;break;}n+=m;
    }
    if(n>0)S57Log(line);
  }
  LONG calls=InterlockedExchange(&g_s57Calls,0),rows=InterlockedExchange(&g_s57Rows,0);
  LONG missing=InterlockedExchange(&g_s57Missing,0);
  LONG dropped=InterlockedExchange(&g_s57Dropped,0),overflow=InterlockedExchange(&g_s57Overflow,0);
  LONG mismatch=InterlockedExchange(&g_s57Mismatch,0),badCaller=InterlockedExchange(&g_s57BadCaller,0);
  if(calls||rows||missing||dropped||overflow||mismatch||badCaller){
    char line[280]{};
    sprintf_s(line,
      "STAGE57_HEALTH calls=%ld rows=%ld missingObjects=%ld dropped=%ld overflow=%ld "
      "mismatch=%ld badCaller=%ld ring=8192 depth=8.",
      calls,rows,missing,dropped,overflow,mismatch,badCaller);
    S57Log(line);
  }
}
