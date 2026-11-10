// Stage50: PASSIVE native AE100 motion-coordinate updates just before Stage49 AF13D.
// Original CALL at AF136 -> AE100; the caller keeps the active object in ESI.
// AE100 writes O+7C/+80/+AC/+B0 on its conditional path.
// Native called once. No gameplay writes, controls, timers or new worker threads.
constexpr DWORD kS50Capacity=4096u,kS50DepthMax=8u,kS50Fields=11u;
struct S50Rec {
    DWORD Seq=0,Side=0,Player=0,Object=0,Tid=0,CallerRva=0,CallerRead=0;
    ULONGLONG Q0=0,Q1=0;
    DWORD PreMask=0,PostMask=0,Pre[kS50Fields]{},Post[kS50Fields]{};
};
volatile LONG g_s50Installed=0,g_s50AllCalls=0,g_s50OtherObjects=0;
volatile LONG g_s50Dropped=0,g_s50Overflow=0,g_s50Mismatch=0,g_s50BadCaller=0;
DWORD g_s50Native=0;
__declspec(thread) DWORD g_s50Depth=0;
__declspec(thread) S50Rec g_s50Pending[kS50DepthMax]{};
SRWLOCK g_s50Lock=SRWLOCK_INIT;
S50Rec g_s50Ring[kS50Capacity]{};
DWORD g_s50Read=0,g_s50Write=0,g_s50Count=0,g_s50Seq=0;

void S50Log(const char* msg)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage50PreLinkedCoordinateTrace.log"))return;
    SYSTEMTIME t{};GetLocalTime(&t);
    char line[2100]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,msg);
    if(n<=0)return;
    HANDLE file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return;
    DWORD written=0;WriteFile(file,line,static_cast<DWORD>(n),&written,nullptr);
    CloseHandle(file);
}
void S50Snap(DWORD object,DWORD* data,DWORD& mask)
{
    // f0..f3: O+7C/+80/+AC/+B0 (native stores at AE1A6/AE1B8/AE1A9/AE1BE)
    // f4..f7: O+3C / +1A4 / +44 / +5C (native conditional and source lookup)
    // f8..f10: O+10 / +08 / +24 (other native/control flags)
    const DWORD offs[kS50Fields]={
        0x7Cu,0x80u,0xACu,0xB0u,0x3Cu,0x1A4u,
        0x44u,0x5Cu,0x10u,0x08u,0x24u
    };
    mask=0;
    if(!object)return;
    for(DWORD i=0;i<kS50Fields;++i)
        if(SafeReadDword(object+offs[i],&data[i]))mask|=1u<<i;
}
void __cdecl S50Enter(const DWORD* saved)
{
    InterlockedIncrement(&g_s50AllCalls);
    const DWORD d=g_s50Depth++;
    if(d>=kS50DepthMax){InterlockedIncrement(&g_s50Overflow);return;}
    S50Rec& r=g_s50Pending[d];r=S50Rec{};
    const DWORD object=saved[1]; // PUSHAD ESI, *not* ECX: original AE100 uses ESI.
    DWORD a=0,b=0;
    const bool p1=SafeReadDword(g_playerP1+4u,&a);
    const bool p2=SafeReadDword(g_playerP2+4u,&b);
    if(p1&&object==a){r.Side=1;r.Player=g_playerP1;}
    else if(p2&&object==b){r.Side=2;r.Player=g_playerP2;}
    else {InterlockedIncrement(&g_s50OtherObjects);return;}
    r.Object=object;r.Tid=GetCurrentThreadId();
    DWORD ret=0;
    if(SafeReadDword(saved[3]+4u,&ret)&&ret>=g_gameBase+5u){
        r.CallerRva=ret-g_gameBase-5u;r.CallerRead=1u;
    }
    if(!r.CallerRead||r.CallerRva!=0xAF136u){
        InterlockedIncrement(&g_s50BadCaller);r.Side=0;return;
    }
    r.Q0=Stage31Qpc();
    S50Snap(r.Object,r.Pre,r.PreMask);
}
void __cdecl S50Leave(const DWORD* /* saved */)
{
    if(!g_s50Depth){InterlockedIncrement(&g_s50Mismatch);return;}
    const DWORD d=--g_s50Depth;
    if(d>=kS50DepthMax)return;
    S50Rec& r=g_s50Pending[d];
    if(!r.Side)return;
    r.Q1=Stage31Qpc();
    S50Snap(r.Object,r.Post,r.PostMask);
    AcquireSRWLockExclusive(&g_s50Lock);
    r.Seq=++g_s50Seq;
    if(g_s50Count==kS50Capacity)InterlockedIncrement(&g_s50Dropped);
    else {
        g_s50Ring[g_s50Write]=r;
        g_s50Write=(g_s50Write+1u)%kS50Capacity;
        ++g_s50Count;
    }
    ReleaseSRWLockExclusive(&g_s50Lock);
    r.Side=0;
}
// Identical preservation pattern to validated Stage49:
// GPR/EFLAGS and 16-byte-aligned FXSAVE/FXRSTOR around bookkeeping;
// original ESI and all other registers/flags restored before native AE100;
// native called exactly once with original stack, bare RET after post-snapshot.
__declspec(naked) void S50PreLinkedHook()
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
        call S50Enter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        call dword ptr [g_s50Native]
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
        call S50Leave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret
    }
}
bool InstallStage50Hooks()
{
    S50Log("STAGE50_PRELINKED_COORD_BUILD=2026-11-10-r1");
    if(InterlockedCompareExchange(&g_s49Installed,0,0)!=1||
       InterlockedCompareExchange(&g_s48Installed,0,0)!=1||
       InterlockedCompareExchange(&g_s47Installed,0,0)!=1){
        S50Log("ERROR: Stage50 requires installed Stage49/48/47; no patches.");
        return false;
    }
    if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
        S50Log("ERROR: Stage50 requires FXSAVE; no patches.");return false;
    }
    BYTE* native=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAE100u));
    BYTE* site=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAF136u));
    const BYTE original[]={0xE8,0xC5,0xEF,0xFF,0xFF};
    const BYTE callerGuard[]={0x83,0x7E,0x38,0xFF,0x74,0x1A};
    const BYTE afterCall[]={0x8B,0xCE}; // AF13B MOV ECX,ESI
    const BYTE nativePrologue[]={0x55,0x8B,0xEC,0x83,0xEC,0x08,0x83,0x7E,0x3C,0x04};
    const BYTE nativeGate[]={0x39,0xBE,0xA4,0x01,0x00,0x00}; // AE114
    const BYTE writeX[]={0xD9,0x56,0x7C};                       // AE1A6
    const BYTE writeXCopy[]={0xD9,0x9E,0xAC,0x00,0x00,0x00};    // AE1A9
    const BYTE writeY[]={0xD9,0x96,0x80,0x00,0x00,0x00};        // AE1B8
    const BYTE writeYCopy[]={0xD9,0x9E,0xB0,0x00,0x00,0x00};    // AE1BE
    const BYTE nativeRet[]={0xC3};                              // AE216
    auto same=[&](DWORD rva,const BYTE* signature,size_t len)->bool{
        const BYTE* p=reinterpret_cast<const BYTE*>(
            static_cast<std::uintptr_t>(g_gameBase+rva));
        return std::memcmp(p,signature,len)==0;
    };
    auto valid=[&]()->bool{
        if(!same(0xAF130u,callerGuard,sizeof(callerGuard))||
           !same(0xAF136u,original,sizeof(original))||
           !same(0xAF13Bu,afterCall,sizeof(afterCall))||
           !same(0xAE100u,nativePrologue,sizeof(nativePrologue))||
           !same(0xAE114u,nativeGate,sizeof(nativeGate))||
           !same(0xAE1A6u,writeX,sizeof(writeX))||
           !same(0xAE1A9u,writeXCopy,sizeof(writeXCopy))||
           !same(0xAE1B8u,writeY,sizeof(writeY))||
           !same(0xAE1BEu,writeYCopy,sizeof(writeYCopy))||
           !same(0xAE216u,nativeRet,sizeof(nativeRet)))return false;
        std::int32_t rel=0;std::memcpy(&rel,site+1u,sizeof(rel));
        return site+5u+rel==native;
    };
    if(!valid()){S50Log("ERROR: Stage50 native CALL/ABI/coordinate-store signature mismatch; no patches.");return false;}
    BYTE patch[5]{};
    if(!MakeRelativeCall(site,reinterpret_cast<const void*>(&S50PreLinkedHook),patch)){
        S50Log("ERROR: Stage50 rel32 out of range; no patches.");return false;
    }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S50Log("ERROR: Stage50 patch mutex failed; no patches.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
        CloseHandle(mutex);S50Log("ERROR: Stage50 patch mutex wait failed; no patches.");return false;
    }
    HANDLE suspended[512]{};
    const DWORD stopped=SuspendOtherProcessThreads(suspended,512);
    DWORD oldProtection=0;
    const bool writable=VirtualProtect(site,5u,PAGE_EXECUTE_READWRITE,&oldProtection)!=0;
    bool installed=false;
    if(writable&&valid()){
        g_s50Native=g_gameBase+0xAE100u;
        std::memcpy(site,patch,5u);
        FlushInstructionCache(GetCurrentProcess(),site,5u);
        InterlockedExchange(&g_s50Installed,1);
        installed=true;
    }
    if(writable){DWORD ignored=0;VirtualProtect(site,5u,oldProtection,&ignored);}
    ResumeThreads(suspended,stopped);
    ReleaseMutex(mutex);CloseHandle(mutex);
    S50Log(installed?
        "STAGE50_INSTALLED 1/1 callsite=AF136 native=AE100; ESI object; native once; prior Stage49 AF13D unchanged.":
        "ERROR: Stage50 install failed; Stage49/48/47 hooks remain installed.");
    return installed;
}
void FlushStage50()
{
    for(DWORD i=0;i<kS50Capacity;++i){
        S50Rec r{};
        AcquireSRWLockExclusive(&g_s50Lock);
        const bool available=g_s50Count!=0;
        if(available){
            r=g_s50Ring[g_s50Read];
            g_s50Read=(g_s50Read+1u)%kS50Capacity;--g_s50Count;
        }
        ReleaseSRWLockExclusive(&g_s50Lock);
        if(!available)break;
        DWORD changed=0;
        for(DWORD j=0;j<kS50Fields;++j)
            if((r.PreMask&r.PostMask&(1u<<j))&&r.Pre[j]!=r.Post[j])changed|=1u<<j;
        const DWORD gate=static_cast<DWORD>(
            (r.PreMask&(1u<<4u))&&(r.PreMask&(1u<<5u))&&
            (r.Pre[4]==4u||r.Pre[5]==0x32u));
        char line[1800]{};
        int n=sprintf_s(line,
            "STAGE50_PRELINKED_WINDOW seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu "
            "callRVA=%08lX callerRead=%lu qpcBefore=%I64u qpcAfter=%I64u "
            "preMask=%04lX postMask=%04lX changedMask=%04lX gateCandidate=%lu",
            r.Seq,r.Side,r.Player,r.Object,r.Tid,r.CallerRva,r.CallerRead,r.Q0,r.Q1,
            r.PreMask,r.PostMask,changed,gate);
        if(n<=0)continue;
        for(DWORD j=0;j<kS50Fields;++j){
            const int added=sprintf_s(line+n,sizeof(line)-static_cast<size_t>(n),
                " f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
            if(added<=0){n=0;break;}n+=added;
        }
        if(n>0)S50Log(line);
    }
    const LONG calls=InterlockedExchange(&g_s50AllCalls,0);
    const LONG other=InterlockedExchange(&g_s50OtherObjects,0);
    const LONG dropped=InterlockedExchange(&g_s50Dropped,0);
    const LONG overflow=InterlockedExchange(&g_s50Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s50Mismatch,0);
    const LONG badCaller=InterlockedExchange(&g_s50BadCaller,0);
    if(calls||other||dropped||overflow||mismatch||badCaller){
        char line[235]{};
        sprintf_s(line,"STAGE50_HEALTH calls=%ld otherObjects=%ld dropped=%ld overflow=%ld mismatch=%ld badCaller=%ld ring=4096 depth=8.",
            calls,other,dropped,overflow,mismatch,badCaller);
        S50Log(line);
    }
}
