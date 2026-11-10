// Stage49: PASSIVE linked-motion state propagation around native 1E9170.
// Stage47/48 established action-variant binding and native float updates.
// Original CALLs: AF13D (main E2E02C8 list) and AF1AA (conditional E2E02C0).
// One-argument-free ECX=thiscall; original native called exactly once.
// Inspect guarded native copy linked+7C/+80 -> object+7C/+80/+AC/+B0.
// No gameplay writes, timer changes, artificial inputs or new worker threads.
constexpr DWORD kS49Capacity=4096u,kS49DepthMax=8u,kS49ObjectFields=11u,kS49LinkFields=3u;
struct S49Rec {
    DWORD Seq=0,Side=0,Player=0,Object=0,Tid=0,CallerRva=0,CallerRead=0;
    ULONGLONG Q0=0,Q1=0;
    DWORD PreMask=0,PostMask=0,Pre[kS49ObjectFields]{},Post[kS49ObjectFields]{};
    DWORD PreLink=0,PostLink=0;
    DWORD LinkPreMask=0,LinkPostMask=0;
    DWORD LinkPre[kS49LinkFields]{},LinkPost[kS49LinkFields]{};
};
volatile LONG g_s49Installed=0,g_s49AllCalls=0,g_s49OtherObjects=0;
volatile LONG g_s49Dropped=0,g_s49Overflow=0,g_s49Mismatch=0,g_s49BadCaller=0;
DWORD g_s49Native=0;
__declspec(thread) DWORD g_s49Depth=0;
__declspec(thread) S49Rec g_s49Pending[kS49DepthMax]{};
SRWLOCK g_s49Lock=SRWLOCK_INIT;
S49Rec g_s49Ring[kS49Capacity]{};
DWORD g_s49Read=0,g_s49Write=0,g_s49Count=0,g_s49Seq=0;

void S49Log(const char* msg)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage49LinkedMotionTrace.log"))return;
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
void S49Snap(DWORD object,DWORD* data,DWORD& mask,
    DWORD& linked,DWORD* linkData,DWORD& linkMask)
{
    // f0/+7C f1/+80 f2/+AC f3/+B0 f4/+60 f5/+280
    // f6/+2D0 linked pointer f7/+10 flags f8/+14 f9/+08 f10/+24.
    const DWORD offs[kS49ObjectFields]={
        0x7Cu,0x80u,0xACu,0xB0u,0x60u,0x280u,
        0x2D0u,0x10u,0x14u,0x08u,0x24u
    };
    mask=0;linkMask=0;linked=0;
    if(!object)return;
    for(DWORD i=0;i<kS49ObjectFields;++i)
        if(SafeReadDword(object+offs[i],&data[i]))mask|=1u<<i;
    if((mask&(1u<<6u))==0u)return;
    linked=data[6];
    if(!linked)return;
    // l0 linked+7C, l1 linked+80, l2 linked+60 active ID.
    const DWORD loffs[kS49LinkFields]={0x7Cu,0x80u,0x60u};
    for(DWORD i=0;i<kS49LinkFields;++i)
        if(SafeReadDword(linked+loffs[i],&linkData[i]))linkMask|=1u<<i;
}
void __cdecl S49Enter(const DWORD* saved)
{
    InterlockedIncrement(&g_s49AllCalls);
    const DWORD d=g_s49Depth++;
    if(d>=kS49DepthMax){InterlockedIncrement(&g_s49Overflow);return;}
    S49Rec& r=g_s49Pending[d];r=S49Rec{};
    const DWORD object=saved[6]; // PUSHAD ECX, original native thiscall.
    DWORD a=0,b=0;
    const bool p1=SafeReadDword(g_playerP1+4u,&a);
    const bool p2=SafeReadDword(g_playerP2+4u,&b);
    if(p1 && object==a){r.Side=1;r.Player=g_playerP1;}
    else if(p2 && object==b){r.Side=2;r.Player=g_playerP2;}
    else {InterlockedIncrement(&g_s49OtherObjects);return;}
    r.Object=object;r.Tid=GetCurrentThreadId();
    DWORD ret=0;
    if(SafeReadDword(saved[3]+4u,&ret)&&ret>=g_gameBase+5u){
        r.CallerRva=ret-g_gameBase-5u;r.CallerRead=1u;
    }
    if(!r.CallerRead||(r.CallerRva!=0xAF13Du&&r.CallerRva!=0xAF1AAu)){
        InterlockedIncrement(&g_s49BadCaller);r.Side=0;return;
    }
    r.Q0=Stage31Qpc();
    S49Snap(r.Object,r.Pre,r.PreMask,r.PreLink,r.LinkPre,r.LinkPreMask);
}
void __cdecl S49Leave(const DWORD* /* saved */)
{
    if(!g_s49Depth){InterlockedIncrement(&g_s49Mismatch);return;}
    const DWORD d=--g_s49Depth;
    if(d>=kS49DepthMax)return;
    S49Rec& r=g_s49Pending[d];
    if(!r.Side)return;
    r.Q1=Stage31Qpc();
    S49Snap(r.Object,r.Post,r.PostMask,r.PostLink,r.LinkPost,r.LinkPostMask);
    AcquireSRWLockExclusive(&g_s49Lock);
    r.Seq=++g_s49Seq;
    if(g_s49Count==kS49Capacity)InterlockedIncrement(&g_s49Dropped);
    else {
        g_s49Ring[g_s49Write]=r;
        g_s49Write=(g_s49Write+1u)%kS49Capacity;
        ++g_s49Count;
    }
    ReleaseSRWLockExclusive(&g_s49Lock);
    r.Side=0;
}
// Validated Stage44-pattern ABI preservation: pushfd/pushad + aligned
// FXSAVE/FXRSTOR on both C++ bookkeeping calls, original thiscall once,
// original ECX and other GPR/FLAGS/FPU/SSE state preserved, bare RET.
__declspec(naked) void S49LinkedMotionHook()
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
        call S49Enter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        call dword ptr [g_s49Native]
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
        call S49Leave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret
    }
}
bool InstallStage49Hooks()
{
    S49Log("STAGE49_LINKED_MOTION_BUILD=2026-11-10-r1");
    if(InterlockedCompareExchange(&g_s48Installed,0,0)!=1||
       InterlockedCompareExchange(&g_s47Installed,0,0)!=1||
       InterlockedCompareExchange(&g_s45Installed,0,0)!=1){
        S49Log("ERROR: Stage49 requires working Stage48, Stage47 and Stage45; no patches.");
        return false;
    }
    if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
        S49Log("ERROR: Stage49 requires FXSAVE; no patches.");return false;
    }
    BYTE* native=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E9170u));
    BYTE* sites[2]={
        reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAF13Du)),
        reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAF1AAu))
    };
    const BYTE orig[2][5]={
        {0xE8,0x2E,0xA0,0x13,0x00},
        {0xE8,0xC1,0x9F,0x13,0x00}
    };
    const BYTE root0[]={0x8B,0x35,0xC8,0x02,0x2E,0x0E};
    const BYTE root1[]={0x8B,0x35,0xC0,0x02,0x2E,0x0E};
    const BYTE ecxMove[]={0x8B,0xCE};
    const BYTE nativePrologue[]={0x55,0x8B,0xEC,0x83,0xEC,0x08,0x56,0x8B,0xF1};
    const BYTE nativeRet[]={0xC3};
    const BYTE linkRead[]={0x8B,0xBE,0xD0,0x02,0x00,0x00};
    const BYTE idGuard[]={0x3B,0x86,0x80,0x02,0x00,0x00};
    const BYTE sourceX[]={0xD9,0x47,0x7C};
    const BYTE destX[]={0xD9,0x56,0x7C};
    const BYTE destCopyX[]={0xD9,0x9E,0xAC,0x00,0x00,0x00};
    const BYTE sourceY[]={0xD9,0x87,0x80,0x00,0x00,0x00};
    const BYTE destY[]={0xD9,0x96,0x80,0x00,0x00,0x00};
    const BYTE destCopyY[]={0xD9,0x9E,0xB0,0x00,0x00,0x00};
    auto same=[&](DWORD rva,const BYTE* signature,size_t len)->bool{
        return std::memcmp(reinterpret_cast<BYTE*>(
            static_cast<std::uintptr_t>(g_gameBase+rva)),signature,len)==0;
    };
    auto valid=[&]()->bool{
        if(!same(0xAF11Eu,root0,sizeof(root0))||
           !same(0xAF19Eu,root1,sizeof(root1))||
           !same(0x1E9170u,nativePrologue,sizeof(nativePrologue))||
           !same(0x1E9381u,nativeRet,sizeof(nativeRet))||
           !same(0x1E92C8u,linkRead,sizeof(linkRead))||
           !same(0x1E92D1u,idGuard,sizeof(idGuard))||
           !same(0x1E92DDu,sourceX,sizeof(sourceX))||
           !same(0x1E92E6u,destX,sizeof(destX))||
           !same(0x1E92E9u,destCopyX,sizeof(destCopyX))||
           !same(0x1E92EFu,sourceY,sizeof(sourceY))||
           !same(0x1E92FBu,destY,sizeof(destY))||
           !same(0x1E9301u,destCopyY,sizeof(destCopyY)))return false;
        for(DWORD i=0;i<2u;++i){
            if(std::memcmp(sites[i]-2u,ecxMove,2u)||
               std::memcmp(sites[i],orig[i],5u))return false;
            std::int32_t rel=0;std::memcpy(&rel,sites[i]+1u,sizeof(rel));
            if(sites[i]+5u+rel!=native)return false;
        }
        return true;
    };
    if(!valid()){
        S49Log("ERROR: Stage49 root/CALL/linked-field/ABI signature mismatch; no patches.");
        return false;
    }
    BYTE patches[2][5]{};
    for(DWORD i=0;i<2u;++i)
        if(!MakeRelativeCall(sites[i],reinterpret_cast<const void*>(&S49LinkedMotionHook),patches[i])){
            S49Log("ERROR: Stage49 rel32 out of range; no patches.");return false;
        }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S49Log("ERROR: Stage49 patch mutex failed; no patches.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){
        CloseHandle(mutex);S49Log("ERROR: Stage49 patch mutex wait failed; no patches.");return false;
    }
    HANDLE suspended[512]{};
    const DWORD stopped=SuspendOtherProcessThreads(suspended,512);
    // AF13D and AF1AA share the same 4K page; one protect/restore region.
    DWORD oldProtection=0;
    const SIZE_T length=static_cast<SIZE_T>((sites[1]+5u)-sites[0]);
    const bool writable=VirtualProtect(sites[0],length,PAGE_EXECUTE_READWRITE,&oldProtection)!=0;
    bool installed=false;
    if(writable&&valid()){
        g_s49Native=g_gameBase+0x1E9170u;
        for(DWORD i=0;i<2u;++i){
            std::memcpy(sites[i],patches[i],5u);
            FlushInstructionCache(GetCurrentProcess(),sites[i],5u);
        }
        InterlockedExchange(&g_s49Installed,1);
        installed=true;
    }
    if(writable){DWORD ignored=0;VirtualProtect(sites[0],length,oldProtection,&ignored);}
    ResumeThreads(suspended,stopped);
    ReleaseMutex(mutex);CloseHandle(mutex);
    S49Log(installed?
        "STAGE49_INSTALLED 2/2 callsites=AF13D(listC8),AF1AA(listC0) native=1E9170; original no-arg thiscall once; P1/P2-only snapshots.":
        "ERROR: Stage49 install failed; existing Stage48/47/46 hooks remain installed.");
    return installed;
}
void FlushStage49()
{
    for(DWORD i=0;i<kS49Capacity;++i){
        S49Rec r{};
        AcquireSRWLockExclusive(&g_s49Lock);
        const bool have=g_s49Count!=0;
        if(have){
            r=g_s49Ring[g_s49Read];
            g_s49Read=(g_s49Read+1u)%kS49Capacity;--g_s49Count;
        }
        ReleaseSRWLockExclusive(&g_s49Lock);
        if(!have)break;
        DWORD changed=0,linkedChanged=0;
        for(DWORD j=0;j<kS49ObjectFields;++j)
            if((r.PreMask&r.PostMask&(1u<<j))&&r.Pre[j]!=r.Post[j])changed|=1u<<j;
        for(DWORD j=0;j<kS49LinkFields;++j)
            if((r.LinkPreMask&r.LinkPostMask&(1u<<j))&&r.LinkPre[j]!=r.LinkPost[j])
                linkedChanged|=1u<<j;
        // The branch at 1E92C8 checks O+10 bit15 and linked O+60 == O+280.
        const DWORD gate=static_cast<DWORD>(
            (r.PreMask&(1u<<7u))&&(r.Pre[7]&(1u<<15u))&&
            (r.PreMask&(1u<<5u))&&(r.LinkPreMask&(1u<<2u))&&
            r.LinkPre[2]==r.Pre[5]);
        char line[1850]{};
        int n=sprintf_s(line,
            "STAGE49_LINKED_WINDOW seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu "
            "callRVA=%08lX callerRead=%lu qpcBefore=%I64u qpcAfter=%I64u "
            "preMask=%04lX postMask=%04lX changedMask=%04lX "
            "link=%08lX>%08lX linkPreMask=%01lX linkPostMask=%01lX "
            "linkChangedMask=%01lX staticGateCandidate=%lu",
            r.Seq,r.Side,r.Player,r.Object,r.Tid,r.CallerRva,r.CallerRead,r.Q0,r.Q1,
            r.PreMask,r.PostMask,changed,r.PreLink,r.PostLink,r.LinkPreMask,
            r.LinkPostMask,linkedChanged,gate);
        if(n<=0)continue;
        for(DWORD j=0;j<kS49ObjectFields;++j){
            const int added=sprintf_s(line+n,sizeof(line)-static_cast<size_t>(n),
                " f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
            if(added<=0){n=0;break;}n+=added;
        }
        if(!n)continue;
        for(DWORD j=0;j<kS49LinkFields;++j){
            const int added=sprintf_s(line+n,sizeof(line)-static_cast<size_t>(n),
                " l%lu=%08lX>%08lX",j,r.LinkPre[j],r.LinkPost[j]);
            if(added<=0){n=0;break;}n+=added;
        }
        if(n>0)S49Log(line);
    }
    const LONG calls=InterlockedExchange(&g_s49AllCalls,0);
    const LONG other=InterlockedExchange(&g_s49OtherObjects,0);
    const LONG dropped=InterlockedExchange(&g_s49Dropped,0);
    const LONG overflow=InterlockedExchange(&g_s49Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s49Mismatch,0);
    const LONG badCaller=InterlockedExchange(&g_s49BadCaller,0);
    if(calls||other||dropped||overflow||mismatch||badCaller){
        char line[235]{};
        sprintf_s(line,"STAGE49_HEALTH calls=%ld otherObjects=%ld dropped=%ld overflow=%ld mismatch=%ld badCaller=%ld ring=4096 depth=8.",
            calls,other,dropped,overflow,mismatch,badCaller);
        S49Log(line);
    }
}
