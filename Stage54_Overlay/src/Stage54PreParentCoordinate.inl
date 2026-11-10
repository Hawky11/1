// Stage54 passive siblings: A2B81/A2B86/A2B8B, directly before existing Stage53 A2B90.
// No input/state/coordinate writes; every no-arg native is called once.
// Keep Stage53 AF0A0 and Stage52/51/50/49 observational hooks untouched.
constexpr DWORD kS54Capacity=8192u,kS54Depth=8u,kS54Fields=13u;
struct S54Row {
  DWORD Seq=0,Side=0,Player=0,Object=0,Tid=0,CallerRva=0,CallerRead=0;
  ULONGLONG Q0=0,Q1=0;
  DWORD PreMask=0,PostMask=0,Pre[kS54Fields]{},Post[kS54Fields]{};
};
struct S54Pending { S54Row rows[2]{}; };
volatile LONG g_s54Installed=0,g_s54Calls=0,g_s54Rows=0,g_s54Missing=0;
volatile LONG g_s54Dropped=0,g_s54Overflow=0,g_s54Mismatch=0,g_s54BadCaller=0;
DWORD g_s54NativeA=0,g_s54NativeB=0,g_s54NativeC=0;
__declspec(thread) DWORD g_s54Depth=0;
__declspec(thread) S54Pending g_s54Pending[kS54Depth]{};
SRWLOCK g_s54Lock=SRWLOCK_INIT;
S54Row g_s54Ring[kS54Capacity]{};
DWORD g_s54Read=0,g_s54Write=0,g_s54Count=0,g_s54Seq=0;

void S54Log(const char* message)
{
  wchar_t path[MAX_PATH+1]{};
  if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage54PreParentCoordinateTrace.log"))return;
  SYSTEMTIME t{};GetLocalTime(&t);
  char line[2100]{};
  const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
    t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,message);
  if(n<=0)return;
  HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(f==INVALID_HANDLE_VALUE)return;
  DWORD written=0;WriteFile(f,line,static_cast<DWORD>(n),&written,nullptr);CloseHandle(f);
}
void S54Snap(DWORD object,DWORD* data,DWORD& mask)
{
  // Same thirteen field indices and offsets as Stage53/52/51.
  // f0=O+60, f1=O+7C X, f2=O+80 Y, f3=O+AC anchor X,
  // f4=O+B0 anchor Y, f5=O+3C, f6=O+1A4, f7=O+18,
  // f8=O+24, f9=O+2F0, f10=O+88, f11=O+8C, f12=O+10.
  const DWORD offsets[kS54Fields]={0x60u,0x7Cu,0x80u,0xACu,0xB0u,
    0x3Cu,0x1A4u,0x18u,0x24u,0x2F0u,0x88u,0x8Cu,0x10u};
  mask=0;if(!object)return;
  for(DWORD i=0;i<kS54Fields;++i)
    if(SafeReadDword(object+offsets[i],&data[i]))mask|=1u<<i;
}
void __cdecl S54Enter(const DWORD* saved)
{
  InterlockedIncrement(&g_s54Calls);
  DWORD d=g_s54Depth++;
  if(d>=kS54Depth){InterlockedIncrement(&g_s54Overflow);return;}
  S54Pending& p=g_s54Pending[d];p=S54Pending{};
  DWORD ret=0;
  const DWORD entryEsp=saved[3]+4u; // PUSHAD saved stack + PUSHFD.
  const bool callerRead=SafeReadDword(entryEsp,&ret);
  const DWORD caller=(callerRead&&ret>=g_gameBase+5u)?ret-g_gameBase-5u:0u;
  if(!callerRead||(caller!=0xA2B81u&&caller!=0xA2B86u&&caller!=0xA2B8Bu)){InterlockedIncrement(&g_s54BadCaller);return;}
  const DWORD players[2]={g_playerP1,g_playerP2};
  DWORD objects[2]{};
  const bool readable[2]={SafeReadDword(g_playerP1+4u,&objects[0]),
                          SafeReadDword(g_playerP2+4u,&objects[1])};
  for(DWORD i=0;i<2u;++i){
    S54Row& r=p.rows[i];
    if(!readable[i]||!objects[i]){InterlockedIncrement(&g_s54Missing);continue;}
    r.Side=i+1u;r.Player=players[i];r.Object=objects[i];
    r.Tid=GetCurrentThreadId();r.CallerRva=caller;r.CallerRead=1u;
    r.Q0=Stage31Qpc();S54Snap(r.Object,r.Pre,r.PreMask);
  }
}
void __cdecl S54Leave(const DWORD* /*saved*/)
{
  if(!g_s54Depth){InterlockedIncrement(&g_s54Mismatch);return;}
  DWORD d=--g_s54Depth;if(d>=kS54Depth)return;
  S54Pending& p=g_s54Pending[d];
  for(DWORD i=0;i<2u;++i){
    S54Row& r=p.rows[i];if(!r.Side)continue;
    r.Q1=Stage31Qpc();S54Snap(r.Object,r.Post,r.PostMask);
    AcquireSRWLockExclusive(&g_s54Lock);
    r.Seq=++g_s54Seq;
    if(g_s54Count==kS54Capacity)InterlockedIncrement(&g_s54Dropped);
    else {
      g_s54Ring[g_s54Write]=r;g_s54Write=(g_s54Write+1u)%kS54Capacity;
      ++g_s54Count;
    }
    ReleaseSRWLockExclusive(&g_s54Lock);
    InterlockedIncrement(&g_s54Rows);
    r.Side=0;
  }
}
// A1B80 is a no-argument native called from one five-byte direct CALL.
__declspec(naked) void S54A1B80Hook()
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
    call S54Enter
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    call dword ptr [g_s54NativeA]
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
    call S54Leave
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    ret
  }
}
// CFB80 is a no-argument native called from one five-byte direct CALL.
__declspec(naked) void S54CFB80Hook()
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
    call S54Enter
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    call dword ptr [g_s54NativeB]
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
    call S54Leave
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    ret
  }
}
// BAB60 is a no-argument native called from one five-byte direct CALL.
__declspec(naked) void S54BAB60Hook()
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
    call S54Enter
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    call dword ptr [g_s54NativeC]
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
    call S54Leave
    add esp,4
    fxrstor [edi]
    mov esp,ebx
    popad
    popfd
    ret
  }
}
bool InstallStage54Hooks()
{
  S54Log("STAGE54_PRE_PARENT_COORD_BUILD=2026-11-10-r2");
  if(InterlockedCompareExchange(&g_s53Installed,0,0)!=1||
     InterlockedCompareExchange(&g_s52Installed,0,0)!=2||
     InterlockedCompareExchange(&g_s47Installed,0,0)!=1){
    S54Log("ERROR: Stage54 requires installed Stage53/52/47; no patches.");return false;
  }
  if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
    S54Log("ERROR: Stage54 requires FXSAVE; no patches.");return false;
  }
  const DWORD rvas[3]={0xA2B81u,0xA2B86u,0xA2B8Bu};
  const DWORD targets[3]={0xA1B80u,0xCFB80u,0xBAB60u};
  const BYTE originals[3][5]={
    {0xE8,0xFA,0xEF,0xFF,0xFF},
    {0xE8,0xF5,0xCF,0x02,0x00},
    {0xE8,0xD0,0x7F,0x01,0x00}
  };
  // SHA-pinned AALib.exe ImageBase=0x00400000. PE base relocation HIGHLOW
  // at RVA A1B82 changes the absolute operand in CMP BYTE PTR [abs32],0.
  // The original on-disk seven-byte prologue must NOT be compared verbatim
  // against a relocated in-memory image.
  const BYTE aPrefix[]={0x80,0x3D};       // A1B80 CMP BYTE PTR [abs32], imm8
  const BYTE aSuffix[]={0x00,0x74,0x36};  // A1B86 IMM 00, then JZ +0x36
  const DWORD expectedAOperand=0x0E0A1B5Cu+(g_gameBase-0x00400000u);
  const BYTE bPrologue[]={0xE8,0x5B,0x06,0x11,0x00,0x6A,0x01,0xE8,0x54,0x06,0x11,0x00,0x59,0xC3};
  const BYTE cPrologue[]={0x55,0x8B,0xEC,0x81,0xEC,0xC0,0x00,0x00,0x00,0x53,0x56,0x33,0xDB,0x57};
  const BYTE after[]={0xE8,0xF6,0xCE,0x02,0x00}; // A2B95->CFA90
  auto same=[&](DWORD rva,const BYTE* bytes,size_t count)->bool{
    return std::memcmp(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(g_gameBase+rva)),bytes,count)==0;
  };
  DWORD actualAOperand=0;
  const bool aOperandRead=SafeReadDword(g_gameBase+0xA1B82u,&actualAOperand);
  {
    char status[256]{};
    sprintf_s(status,"STAGE54_ASLR_CHECK loadBase=%08lX preferredBase=00400000 relocRVA=000A1B82 expectedOperand=%08lX actualOperand=%08lX readable=%u",
      g_gameBase,expectedAOperand,actualAOperand,aOperandRead?1u:0u);
    S54Log(status);
  }
  auto valid=[&]()->bool{
    const bool aValid=aOperandRead&&actualAOperand==expectedAOperand&&
      same(0xA1B80u,aPrefix,sizeof(aPrefix))&&
      same(0xA1B86u,aSuffix,sizeof(aSuffix));
    const bool bValid=same(0xCFB80u,bPrologue,sizeof(bPrologue));
    const bool cValid=same(0xBAB60u,cPrologue,sizeof(cPrologue));
    const bool afterValid=same(0xA2B95u,after,sizeof(after));
    bool callsValid[3]{};
    for(DWORD i=0;i<3u;++i){
      BYTE* site=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+rvas[i]));
      callsValid[i]=same(rvas[i],originals[i],5u);
      if(callsValid[i]){
        std::int32_t rel=0;std::memcpy(&rel,site+1u,4u);
        callsValid[i]=site+5u+rel==
          reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+targets[i]));
      }
    }
    const bool allValid=aValid&&bValid&&cValid&&afterValid&&
      callsValid[0]&&callsValid[1]&&callsValid[2];
    if(!allValid){
      char status[256]{};
      sprintf_s(status,
        "STAGE54_SIGNATURE_FAILURE A1B80=%u CFB80=%u BAB60=%u A2B95=%u CALL_A2B81=%u CALL_A2B86=%u CALL_A2B8B=%u; refusing all patches.",
        aValid?1u:0u,bValid?1u:0u,cValid?1u:0u,afterValid?1u:0u,
        callsValid[0]?1u:0u,callsValid[1]?1u:0u,callsValid[2]?1u:0u);
      S54Log(status);
    }
    return allValid;
  };
  if(!valid()){S54Log("ERROR: Stage54 sibling validation mismatch; no patches.");return false;}
  BYTE* sites[3]{};
  BYTE patches[3][5]{};
  const void* wrappers[3]={
    reinterpret_cast<const void*>(&S54A1B80Hook),
    reinterpret_cast<const void*>(&S54CFB80Hook),
    reinterpret_cast<const void*>(&S54BAB60Hook)
  };
  for(DWORD i=0;i<3u;++i){
    sites[i]=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+rvas[i]));
    if(!MakeRelativeCall(sites[i],wrappers[i],patches[i])){
      S54Log("ERROR: Stage54 wrapper call outside rel32 range; no patches.");return false;
    }
  }
  HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
  if(!mutex){S54Log("ERROR: Stage54 mutex failed; no patches.");return false;}
  const DWORD wait=WaitForSingleObject(mutex,INFINITE);
  if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
    CloseHandle(mutex);S54Log("ERROR: Stage54 mutex wait failed; no patches.");return false;
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
    g_s54NativeA=g_gameBase+targets[0];
    g_s54NativeB=g_gameBase+targets[1];
    g_s54NativeC=g_gameBase+targets[2];
    for(DWORD i=0;i<3u;++i){
      std::memcpy(sites[i],patches[i],5u);
      FlushInstructionCache(GetCurrentProcess(),sites[i],5u);
    }
    InterlockedExchange(&g_s54Installed,3);
    installed=true;
  }
  for(DWORD i=0;i<3u;++i){
    if(writable[i]){
      DWORD ignored=0;VirtualProtect(sites[i],5u,oldProtection[i],&ignored);
    }
  }
  ResumeThreads(suspended,stopped);ReleaseMutex(mutex);CloseHandle(mutex);
  S54Log(installed?
    "STAGE54_INSTALLED 3/3 A2B81->A1B80 A2B86->CFB80 A2B8B->BAB60; native called once per site; no gameplay writes.":
    "ERROR: Stage54 install failed; previous hooks unaffected.");
  return installed;
}
void FlushStage54()
{
  for(DWORD i=0;i<kS54Capacity;++i){
    S54Row r{};
    AcquireSRWLockExclusive(&g_s54Lock);
    const bool has=g_s54Count!=0;
    if(has){
      r=g_s54Ring[g_s54Read];g_s54Read=(g_s54Read+1u)%kS54Capacity;
      --g_s54Count;
    }
    ReleaseSRWLockExclusive(&g_s54Lock);
    if(!has)break;
    DWORD changed=0;
    for(DWORD j=0;j<kS54Fields;++j)
      if((r.PreMask&r.PostMask&(1u<<j))&&r.Pre[j]!=r.Post[j])changed|=1u<<j;
    char line[1900]{};
    int n=sprintf_s(line,
      "STAGE54_SIBLING_WINDOW seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu callRVA=%08lX "
      "callerRead=%lu qpcBefore=%I64u qpcAfter=%I64u preMask=%04lX postMask=%04lX changedMask=%04lX",
      r.Seq,r.Side,r.Player,r.Object,r.Tid,r.CallerRva,r.CallerRead,
      r.Q0,r.Q1,r.PreMask,r.PostMask,changed);
    if(n<=0)continue;
    for(DWORD j=0;j<kS54Fields;++j){
      const int m=sprintf_s(line+n,sizeof(line)-static_cast<size_t>(n),
        " f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
      if(m<=0){n=0;break;}n+=m;
    }
    if(n>0)S54Log(line);
  }
  LONG calls=InterlockedExchange(&g_s54Calls,0),rows=InterlockedExchange(&g_s54Rows,0);
  LONG missing=InterlockedExchange(&g_s54Missing,0);
  LONG dropped=InterlockedExchange(&g_s54Dropped,0),overflow=InterlockedExchange(&g_s54Overflow,0);
  LONG mismatch=InterlockedExchange(&g_s54Mismatch,0),badCaller=InterlockedExchange(&g_s54BadCaller,0);
  if(calls||rows||missing||dropped||overflow||mismatch||badCaller){
    char line[280]{};
    sprintf_s(line,
      "STAGE54_HEALTH calls=%ld rows=%ld missingObjects=%ld dropped=%ld overflow=%ld "
      "mismatch=%ld badCaller=%ld ring=8192 depth=8.",
      calls,rows,missing,dropped,overflow,mismatch,badCaller);
    S54Log(line);
  }
}
