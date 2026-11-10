// Stage65 SOURCE ONLY: passive per-player native request-origin observer.
// Original AALib.exe SHA256 28cd69a7b6219ff0fa101ecebfcbe98962dc3f391b522d1fd6e32c006cf855ba.
// Hook ONE original RVA E5BF0 entry, never 296 caller instructions, never DR0..DR3.
// The first 7 original bytes (push ebp; mov ebp,esp; cmp byte [ebp+0C],0)
// are replayed by an executable trampoline, then original E5BF7 is resumed.
// Native thiscall ECX=P, stack (request ID, flag), RET 8. Original executes ONCE.
// NOT YET Windows x86 built, installed or tested. Fail closed if any byte differs.
constexpr DWORD kS65Capacity=8192u,kS65DepthLimit=8u;
struct S65Rec {
    DWORD Seq=0,P=0,Side=0,Tid=0,Object=0;
    DWORD CallerReturn=0,CallerReturnRva=0xFFFFFFFFu,CallerCallRva=0xFFFFFFFFu;
    DWORD DirectE8=0,Request=0,Mode=0,EaxAfter=0;
    DWORD BeforeRequest=0,AfterRequest=0,BeforeFlags=0,AfterFlags=0;
    DWORD ReadMask=0;
    ULONGLONG QpcBefore=0,QpcAfter=0;
};
S65Rec g_s65Ring[kS65Capacity]{};
SRWLOCK g_s65Lock=SRWLOCK_INIT;
DWORD g_s65Read=0,g_s65Write=0,g_s65Count=0,g_s65Seq=0;
__declspec(thread) DWORD g_s65Depth=0;
__declspec(thread) S65Rec g_s65Pending[kS65DepthLimit]{};
volatile LONG g_s65Installed=0,g_s65Calls=0,g_s65Filtered=0,g_s65Dropped=0;
volatile LONG g_s65Overflow=0,g_s65Mismatch=0,g_s65BadRead=0;
DWORD g_s65NativeTrampoline=0;

void S65Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage65RequestOriginTrace.log"))return;
    SYSTEMTIME now{};GetLocalTime(&now);
    char line[1800]{};
    const int n=sprintf_s(line,
        "[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,message);
    if(n<=0)return;
    HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
                         nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return;
    DWORD written=0;WriteFile(f,line,static_cast<DWORD>(n),&written,nullptr);CloseHandle(f);
}
void S65Snapshot(S65Rec& r,bool afterNative)
{
    DWORD request=0,flags=0;
    const bool okRequest=SafeReadDword(r.P+0x3F4Cu,&request);
    const bool okFlags=SafeReadDword(r.P+0x3B7Cu,&flags);
    if(afterNative){
        r.AfterRequest=request;r.AfterFlags=flags;
        if(okRequest)r.ReadMask|=4u;
        if(okFlags)r.ReadMask|=8u;
    }else{
        r.BeforeRequest=request;r.BeforeFlags=flags;
        if(okRequest)r.ReadMask|=1u;
        if(okFlags)r.ReadMask|=2u;
    }
    if(!okRequest||!okFlags)InterlockedIncrement(&g_s65BadRead);
}
// Validates that [original return address - 5] was a real direct E8 to E5BF0.
// Return-site evidence remains available even for an INDIRECT caller.
void S65IdentifyCall(S65Rec& r)
{
    const DWORD begin=g_gameBase+0x1000u, end=g_gameBase+0x2BF000u;
    if(r.CallerReturn<begin+5u || r.CallerReturn>=end)return;
    r.CallerReturnRva=r.CallerReturn-g_gameBase;
    const DWORD site=r.CallerReturn-5u;
    DWORD opcode=0,delta=0;
    if(!SafeReadDword(site,&opcode)||!SafeReadDword(site+1u,&delta))return;
    if((opcode&0xFFu)!=0xE8u)return;
    const DWORD destination=site+5u+delta; // exact modulo-2^32 x86 rel32
    if(destination!=g_gameBase+0xE5BF0u)return;
    r.DirectE8=1u;
    r.CallerCallRva=site-g_gameBase;
    r.ReadMask|=0x20u;
}
// PUSHFD/PUSHAD: saved[0..7]=EDI ESI EBP original-ESP EBX EDX ECX EAX,
// saved[8]=EFLAGS, saved[9]=native return address,
// saved[10]=first argument (action ID), saved[11]=second argument (mode).
void __cdecl S65Enter(const DWORD* saved)
{
    InterlockedIncrement(&g_s65Calls);
    const DWORD d=g_s65Depth++;
    if(d>=kS65DepthLimit){InterlockedIncrement(&g_s65Overflow);return;}
    S65Rec& r=g_s65Pending[d];r=S65Rec{};
    const DWORD p=saved[6];
    if(!p || (p!=g_playerP1 && p!=g_playerP2)){
        InterlockedIncrement(&g_s65Filtered);return;
    }
    r.P=p;r.Side=(p==g_playerP1)?1u:2u;r.Tid=GetCurrentThreadId();
    r.CallerReturn=saved[9];r.Request=saved[10];r.Mode=saved[11];
    if(SafeReadDword(p+4u,&r.Object))r.ReadMask|=0x10u;
    S65IdentifyCall(r);
    S65Snapshot(r,false);
    r.QpcBefore=Stage31Qpc();
}
void __cdecl S65Leave(const DWORD* saved)
{
    if(!g_s65Depth){InterlockedIncrement(&g_s65Mismatch);return;}
    const DWORD d=--g_s65Depth;
    if(d>=kS65DepthLimit)return; // balanced overflowing depth
    S65Rec& r=g_s65Pending[d];
    if(!r.P)return; // deliberately filtered unknown player
    if(saved[6]!=r.P || r.Tid!=GetCurrentThreadId()){
        InterlockedIncrement(&g_s65Mismatch);r.P=0;return;
    }
    r.EaxAfter=saved[7];
    r.QpcAfter=Stage31Qpc();
    S65Snapshot(r,true);
    AcquireSRWLockExclusive(&g_s65Lock);
    if(g_s65Count==kS65Capacity)InterlockedIncrement(&g_s65Dropped);
    else{
        r.Seq=++g_s65Seq;
        g_s65Ring[g_s65Write]=r;
        g_s65Write=(g_s65Write+1u)%kS65Capacity;
        ++g_s65Count;
    }
    ReleaseSRWLockExclusive(&g_s65Lock);
    r.P=0;
}
// IMPORTANT: This is a JMP entry detour, not a CALL-site wrapper.
// Original callers placed [esp]=original return, [esp+4]=ID, [esp+8]=flag.
// The two equal [esp+8] PUSHes copy (flag, ID) for ONE trampoline CALL.
// Native RET 8 consumes COPIED args; our RET 8 consumes ORIGINAL args.
// GPR, EFLAGS, x87, XMM and MXCSR are restored around observer C++ calls.
__declspec(naked) void S65RequestOriginHook()
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
        call S65Enter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        call dword ptr [g_s65NativeTrampoline]
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
        call S65Leave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret 8
    }
}
// Install only after Stage58 hooks, but BEFORE the Stage59 watch thread
// starts setting DR0/DR1 on game threads. Stage65 itself uses no debug regs.
bool InstallStage65Hooks()
{
    S65Log("STAGE65_REQUEST_ORIGIN_BUILD=2026-11-11-r1 SOURCE_ONLY; no gameplay writes.");
    if(InterlockedCompareExchange(&g_s58Installed,0,0)!=3){
        S65Log("ERROR: Stage65 needs verified Stage58 3/3; no patches.");return false;
    }
    if(InterlockedCompareExchange(&g_s59Started,0,0)!=0){
        S65Log("ERROR: Stage59 worker already running; refusing Stage65 install.");return false;
    }
    if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
        S65Log("ERROR: Stage65 needs FXSAVE; no patches.");return false;
    }
    if(sizeof(void*)!=4u){
        S65Log("ERROR: Stage65 only supports Windows x86; no patches.");return false;
    }
    BYTE* site=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xE5BF0u));
    // Exact 7-byte instruction boundary: 55;8B EC;80 7D 0C 00.
    const BYTE expectedEntry[]={0x55,0x8B,0xEC,0x80,0x7D,0x0C,0x00};
    // JE after original compare, plus both native POP EBP / RET 8 exits.
    const BYTE expectedJe[]={0x74,0x17};
    const BYTE expectedExit[]={0x5D,0xC2,0x08,0x00};
    const BYTE expectedSecondExit[]={0x5D,0xC2,0x08,0x00};
    auto originalOK=[&]()->bool{
        return std::memcmp(site,expectedEntry,sizeof(expectedEntry))==0 &&
               std::memcmp(site+7,expectedJe,sizeof(expectedJe))==0 &&
               std::memcmp(site+0x1Cu,expectedExit,sizeof(expectedExit))==0 &&
               std::memcmp(site+0x29u,expectedSecondExit,sizeof(expectedSecondExit))==0;
    };
    if(!originalOK()){
        S65Log("ERROR: original E5BF0 prologue/branches/RET8 mismatch; no patches.");
        return false;
    }
    BYTE* tramp=reinterpret_cast<BYTE*>(VirtualAlloc(nullptr,4096u,
        MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE));
    if(!tramp){S65Log("ERROR: Stage65 trampoline allocation failed.");return false;}
    std::memcpy(tramp,expectedEntry,sizeof(expectedEntry));
    tramp[7]=0xE9;
    const DWORD resumeDelta=static_cast<DWORD>(
        reinterpret_cast<std::uintptr_t>(site+7)-
        reinterpret_cast<std::uintptr_t>(tramp+12)); // x86 rel32 wrap
    std::memcpy(tramp+8,&resumeDelta,4u);
    FlushInstructionCache(GetCurrentProcess(),tramp,12u);
    DWORD trampOld=0;
    if(!VirtualProtect(tramp,12u,PAGE_EXECUTE_READ,&trampOld)){
        VirtualFree(tramp,0,MEM_RELEASE);
        S65Log("ERROR: Stage65 trampoline RX protect failed.");return false;
    }
    BYTE patch[7]={0xE9,0,0,0,0,0x90,0x90};
    const DWORD detourDelta=static_cast<DWORD>(
        reinterpret_cast<std::uintptr_t>(&S65RequestOriginHook)-
        reinterpret_cast<std::uintptr_t>(site+5));
    std::memcpy(patch+1,&detourDelta,4u);
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){VirtualFree(tramp,0,MEM_RELEASE);return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0 && wait!=WAIT_ABANDONED){
        CloseHandle(mutex);VirtualFree(tramp,0,MEM_RELEASE);return false;
    }
    HANDLE suspended[512]{};
    const DWORD count=SuspendOtherProcessThreads(suspended,512);
    DWORD oldProtect=0;
    const bool writable=VirtualProtect(site,7u,PAGE_EXECUTE_READWRITE,&oldProtect)!=0;
    bool installed=false;
    if(writable && originalOK()){
        g_s65NativeTrampoline=static_cast<DWORD>(
            reinterpret_cast<std::uintptr_t>(tramp));
        std::memcpy(site,patch,7u);
        FlushInstructionCache(GetCurrentProcess(),site,7u);
        InterlockedExchange(&g_s65Installed,1);
        installed=true;
    }
    if(writable){DWORD ignored=0;VirtualProtect(site,7u,oldProtect,&ignored);}
    ResumeThreads(suspended,count);
    ReleaseMutex(mutex);CloseHandle(mutex);
    if(!installed){
        VirtualFree(tramp,0,MEM_RELEASE);
        g_s65NativeTrampoline=0;
        S65Log("ERROR: Stage65 install failed closed; Stage59 remains independent.");
        return false;
    }
    S65Log("STAGE65_INSTALLED 1/1 RVA=E5BF0 trampolineOriginal7=1 nativeThiscallRet8=1 DRwrites=0; original called once.");
    return true;
}
void FlushStage65()
{
    for(DWORD i=0;i<kS65Capacity;++i){
        S65Rec r{};
        AcquireSRWLockExclusive(&g_s65Lock);
        const bool have=g_s65Count!=0;
        if(have){
            r=g_s65Ring[g_s65Read];
            g_s65Read=(g_s65Read+1u)%kS65Capacity;
            --g_s65Count;
        }
        ReleaseSRWLockExclusive(&g_s65Lock);
        if(!have)break;
        char msg[600]{};
        sprintf_s(msg,
            "STAGE65_WRITER_WINDOW seq=%lu side=P%lu player=%08lX object=%08lX "
            "tid=%lu qpcBefore=%I64u qpcAfter=%I64u "
            "callerReturn=%08lX callerReturnRVA=%08lX callerCallRVA=%08lX directE8=%lu "
            "request=%08lX mode=%08lX nativeEAX=%08lX "
            "preRequest=%08lX postRequest=%08lX preFlags=%08lX postFlags=%08lX readMask=%02lX",
            static_cast<unsigned long>(r.Seq),static_cast<unsigned long>(r.Side),
            static_cast<unsigned long>(r.P),static_cast<unsigned long>(r.Object),
            static_cast<unsigned long>(r.Tid),
            static_cast<unsigned long long>(r.QpcBefore),
            static_cast<unsigned long long>(r.QpcAfter),
            static_cast<unsigned long>(r.CallerReturn),
            static_cast<unsigned long>(r.CallerReturnRva),
            static_cast<unsigned long>(r.CallerCallRva),
            static_cast<unsigned long>(r.DirectE8),
            static_cast<unsigned long>(r.Request),
            static_cast<unsigned long>(r.Mode),
            static_cast<unsigned long>(r.EaxAfter),
            static_cast<unsigned long>(r.BeforeRequest),
            static_cast<unsigned long>(r.AfterRequest),
            static_cast<unsigned long>(r.BeforeFlags),
            static_cast<unsigned long>(r.AfterFlags),
            static_cast<unsigned long>(r.ReadMask));
        S65Log(msg);
    }
    const LONG calls=InterlockedExchange(&g_s65Calls,0);
    const LONG ignored=InterlockedExchange(&g_s65Filtered,0);
    const LONG lost=InterlockedExchange(&g_s65Dropped,0);
    const LONG overflow=InterlockedExchange(&g_s65Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s65Mismatch,0);
    const LONG badRead=InterlockedExchange(&g_s65BadRead,0);
    if(calls||lost||overflow||mismatch||badRead){
        char msg[220]{};
        sprintf_s(msg,
            "STAGE65_HEALTH calls=%ld ignoredOtherPlayers=%ld dropped=%ld "
            "overflow=%ld mismatch=%ld readFail=%ld ring=8192 depth=8",
            calls,ignored,lost,overflow,mismatch,badRead);
        S65Log(msg);
    }
}
