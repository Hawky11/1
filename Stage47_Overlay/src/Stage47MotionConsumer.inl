// Stage47 — passive native motion-consumer update for P1/P2 objects.
// SHA-pinned original AALib.exe: 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba
// Two original CALLs from linked-list root E2E02C8:
//   AE70C -> 1E66E0 (ordinary path), AE93E -> 1E66E0 (alternate path).
// Native thiscall ECX=object, one stack DWORD (zero at both sites), RET 4.
// No modified gameplay inputs, fields, original branch conditions or native logic.
// Stage46 S46Snap is reused; f0..f12 have the same field mapping.
constexpr DWORD kS47Capacity=4096u,kS47DepthMax=8u,kS47Fields=13u;
struct S47Rec {
    DWORD Seq=0,Side=0,Player=0,Object=0,Tid=0,CallerRva=0,Argument=0;
    ULONGLONG Q0=0,Q1=0;
    DWORD PreMask=0,PostMask=0,Pre[kS47Fields]{},Post[kS47Fields]{};
    DWORD DefinitionPre=0,DefinitionPost=0,MetaPre=0,MetaPost=0;
    DWORD ExtraMask=0; // 1 definition pre, 2 definition post, 4 mode pre, 8 mode post
};
volatile LONG g_s47Installed=0,g_s47AllCalls=0,g_s47OtherObjects=0;
volatile LONG g_s47Dropped=0,g_s47Overflow=0,g_s47Mismatch=0,g_s47BadCaller=0;
DWORD g_s47NativeUpdate=0;
__declspec(thread) DWORD g_s47Depth=0;
__declspec(thread) S47Rec g_s47Pending[kS47DepthMax]{};
SRWLOCK g_s47Lock=SRWLOCK_INIT;
S47Rec g_s47Ring[kS47Capacity]{};
DWORD g_s47Read=0,g_s47Write=0,g_s47Count=0,g_s47Seq=0;

void S47Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage47MotionConsumerTrace.log"))return;
    SYSTEMTIME now{};GetLocalTime(&now);
    char line[2000]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,message);
    if(n<=0)return;
    HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return;
    DWORD written=0;WriteFile(f,line,static_cast<DWORD>(n),&written,nullptr);
    CloseHandle(f);
}
void S47Snapshot(DWORD object,DWORD* fields,DWORD& mask,DWORD& definition,
    DWORD& metadata,DWORD& extraMask,DWORD defBit,DWORD metaBit)
{
    S46Snap(object,fields,mask);
    if(SafeReadDword(object+0x34u,&definition)){
        extraMask|=defBit;
        if(definition && SafeReadDword(definition+0x30u,&metadata))
            extraMask|=metaBit;
    }
}
void __cdecl S47Enter(const DWORD* saved)
{
    InterlockedIncrement(&g_s47AllCalls);
    const DWORD d=g_s47Depth++;
    if(d>=kS47DepthMax){InterlockedIncrement(&g_s47Overflow);return;}
    S47Rec& r=g_s47Pending[d];r=S47Rec{};
    const DWORD object=saved[6]; // PUSHAD ECX = native this pointer
    DWORD p1=0,p2=0;
    const bool v1=SafeReadDword(g_playerP1+4u,&p1);
    const bool v2=SafeReadDword(g_playerP2+4u,&p2);
    if(v1 && object==p1){r.Side=1;r.Player=g_playerP1;}
    else if(v2 && object==p2){r.Side=2;r.Player=g_playerP2;}
    else {InterlockedIncrement(&g_s47OtherObjects);return;}
    r.Object=object;r.Tid=GetCurrentThreadId();
    const DWORD entryEsp=saved[3]+4u; // saved ESP after PUSHFD, +4 = original ESP
    DWORD ret=0;
    if(!SafeReadDword(entryEsp,&ret) || ret<g_gameBase+5u){
        InterlockedIncrement(&g_s47BadCaller);r.Side=0;return;
    }
    r.CallerRva=ret-g_gameBase-5u;
    if(r.CallerRva!=0xAE70Cu && r.CallerRva!=0xAE93Eu){
        InterlockedIncrement(&g_s47BadCaller);r.Side=0;return;
    }
    SafeReadDword(entryEsp+4u,&r.Argument);
    r.Q0=Stage31Qpc();
    S47Snapshot(object,r.Pre,r.PreMask,r.DefinitionPre,r.MetaPre,
        r.ExtraMask,1u,4u);
}
void __cdecl S47Leave(const DWORD* /* saved */)
{
    if(!g_s47Depth){InterlockedIncrement(&g_s47Mismatch);return;}
    const DWORD d=--g_s47Depth;
    if(d>=kS47DepthMax)return;
    S47Rec& r=g_s47Pending[d];
    if(!r.Side)return;
    r.Q1=Stage31Qpc();
    S47Snapshot(r.Object,r.Post,r.PostMask,r.DefinitionPost,r.MetaPost,
        r.ExtraMask,2u,8u);
    AcquireSRWLockExclusive(&g_s47Lock);
    r.Seq=++g_s47Seq;
    if(g_s47Count==kS47Capacity)InterlockedIncrement(&g_s47Dropped);
    else {
        g_s47Ring[g_s47Write]=r;
        g_s47Write=(g_s47Write+1u)%kS47Capacity;
        ++g_s47Count;
    }
    ReleaseSRWLockExclusive(&g_s47Lock);
    r.Side=0;
}
// Original CALL site has one stack DWORD (PUSH 0). The native RET 4 consumes
// only a COPIED argument. Wrapper RET 4 consumes the original argument.
// Preserve native EAX, GPR, EFLAGS, x87 FPCW+stack and SSE/MXCSR around
// the bookkeeping. No extra game-state write and no new worker.
__declspec(naked) void S47MotionConsumerHook()
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
        call S47Enter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        // Original argument is [ESP+4]. Native RET 4 removes only this copy.
        push dword ptr [esp+4]
        call dword ptr [g_s47NativeUpdate]
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
        call S47Leave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret 4
    }
}
bool InstallStage47Hooks()
{
    S47Log("STAGE47_MOTION_CONSUMER_BUILD=2026-11-10-r1");
    if(InterlockedCompareExchange(&g_s45Installed,0,0)!=1){
        S47Log("ERROR: Stage47 requires installed Stage45/46; no patches.");return false;
    }
    if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
        S47Log("ERROR: Stage47 requires FXSAVE; no patches.");return false;
    }
    BYTE* native=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E66E0u));
    BYTE* sites[2]={
        reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAE70Cu)),
        reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAE93Eu))
    };
    const BYTE original[2][5]={
        {0xE8,0xCF,0x7F,0x13,0x00},
        {0xE8,0x9D,0x7D,0x13,0x00}
    };
    const BYTE callerPrefix[]={0x6A,0x00,0x8B,0xCE}; // push 0; mov ecx,esi
    const BYTE nativePrologue[]={0x55,0x8B,0xEC,0x83,0xEC,0x24,0x56,0x8B,0xF1};
    const BYTE ret4[]={0xC2,0x04,0x00};
    const BYTE read88[]={0xD9,0x86,0x88,0x00,0x00,0x00};
    const BYTE write7c[]={0xD9,0x5E,0x7C};
    const BYTE read90[]={0xD9,0x86,0x90,0x00,0x00,0x00};
    const BYTE add88[]={0xD8,0x86,0x88,0x00,0x00,0x00};
    const BYTE add80[]={0xD8,0x86,0x80,0x00,0x00,0x00};
    const BYTE add8c[]={0xD8,0x86,0x8C,0x00,0x00,0x00};
    auto valid=[&]()->bool{
        if(std::memcmp(native,nativePrologue,sizeof(nativePrologue)) ||
           std::memcmp(reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E67F1u)),ret4,sizeof(ret4)) ||
           std::memcmp(reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E78D4u)),ret4,sizeof(ret4)) ||
           std::memcmp(reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E7089u)),read88,sizeof(read88)) ||
           std::memcmp(reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E70DBu)),write7c,sizeof(write7c)) ||
           std::memcmp(reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E70DEu)),read90,sizeof(read90)) ||
           std::memcmp(reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E70E8u)),add88,sizeof(add88)) ||
           std::memcmp(reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E7114u)),add80,sizeof(add80)) ||
           std::memcmp(reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1E7353u)),add8c,sizeof(add8c)))return false;
        for(DWORD i=0;i<2u;++i){
            if(std::memcmp(sites[i],original[i],5u) ||
               std::memcmp(sites[i]-4u,callerPrefix,4u))return false;
            std::int32_t rel=0;std::memcpy(&rel,sites[i]+1u,sizeof(rel));
            if(sites[i]+5u+rel!=native)return false;
        }
        return true;
    };
    if(!valid()){S47Log("ERROR: Stage47 native CALL/ABI/consumer signatures mismatch; no patches.");return false;}
    BYTE patches[2][5]{};
    for(DWORD i=0;i<2u;++i)
        if(!MakeRelativeCall(sites[i],reinterpret_cast<const void*>(&S47MotionConsumerHook),patches[i])){
            S47Log("ERROR: Stage47 patch rel32 out of range; no patches.");return false;
        }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S47Log("ERROR: Stage47 mutex creation failed; no patches.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0 && wait!=WAIT_ABANDONED){
        CloseHandle(mutex);S47Log("ERROR: Stage47 mutex wait failed; no patches.");return false;
    }
    HANDLE suspended[512]{};
    const DWORD stopped=SuspendOtherProcessThreads(suspended,512);
    // Both CALLs are in the SAME original 4K .text page.
    // One VirtualProtect region avoids restoring PAGE_EXECUTE_READWRITE twice.
    DWORD oldProtection=0;
    const SIZE_T length=static_cast<SIZE_T>((sites[1]+5u)-sites[0]);
    const bool writable=VirtualProtect(sites[0],length,PAGE_EXECUTE_READWRITE,&oldProtection)!=0;
    bool installed=false;
    if(writable && valid()){
        g_s47NativeUpdate=g_gameBase+0x1E66E0u;
        for(DWORD i=0;i<2u;++i){
            std::memcpy(sites[i],patches[i],5u);
            FlushInstructionCache(GetCurrentProcess(),sites[i],5u);
        }
        InterlockedExchange(&g_s47Installed,1);
        installed=true;
    }
    if(writable){DWORD ignored=0;VirtualProtect(sites[0],length,oldProtection,&ignored);}
    ResumeThreads(suspended,stopped);
    ReleaseMutex(mutex);CloseHandle(mutex);
    S47Log(installed?
        "STAGE47_INSTALLED 2/2 callsites=AE70C,AE93E native=1E66E0; original thiscall once per hit, copied arg, RET4; P1/P2 only.":
        "ERROR: Stage47 installation failed; Stage46 and all earlier hooks remain as installed.");
    return installed;
}
void FlushStage47()
{
    for(DWORD i=0;i<kS47Capacity;++i){
        S47Rec r{};
        AcquireSRWLockExclusive(&g_s47Lock);
        const bool available=g_s47Count!=0;
        if(available){
            r=g_s47Ring[g_s47Read];
            g_s47Read=(g_s47Read+1u)%kS47Capacity;--g_s47Count;
        }
        ReleaseSRWLockExclusive(&g_s47Lock);
        if(!available)break;
        DWORD changed=0;
        for(DWORD j=0;j<kS47Fields;++j)
            if((r.PreMask&r.PostMask&(1u<<j))&&r.Pre[j]!=r.Post[j])changed|=1u<<j;
        char line[1600]{};
        int n=sprintf_s(line,
            "STAGE47_CONSUMER_WINDOW seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu "
            "callRVA=%08lX argument=%08lX qpcBefore=%I64u qpcAfter=%I64u "
            "preMask=%04lX postMask=%04lX changedMask=%04lX extraMask=%01lX "
            "def=%08lX>%08lX modeDword=%08lX>%08lX",
            r.Seq,r.Side,r.Player,r.Object,r.Tid,r.CallerRva,r.Argument,r.Q0,r.Q1,
            r.PreMask,r.PostMask,changed,r.ExtraMask,
            r.DefinitionPre,r.DefinitionPost,r.MetaPre,r.MetaPost);
        if(n<=0)continue;
        for(DWORD j=0;j<kS47Fields;++j){
            const int added=sprintf_s(line+n,sizeof(line)-static_cast<size_t>(n),
                " f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
            if(added<=0){n=0;break;}n+=added;
        }
        if(n>0)S47Log(line);
    }
    const LONG calls=InterlockedExchange(&g_s47AllCalls,0);
    const LONG other=InterlockedExchange(&g_s47OtherObjects,0);
    const LONG dropped=InterlockedExchange(&g_s47Dropped,0);
    const LONG overflow=InterlockedExchange(&g_s47Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s47Mismatch,0);
    const LONG badCaller=InterlockedExchange(&g_s47BadCaller,0);
    if(calls||other||dropped||overflow||mismatch||badCaller){
        char line[240]{};
        sprintf_s(line,
            "STAGE47_HEALTH calls=%ld otherObjects=%ld dropped=%ld overflow=%ld mismatch=%ld badCaller=%ld ring=4096 depth=8.",
            calls,other,dropped,overflow,mismatch,badCaller);
        S47Log(line);
    }
}
