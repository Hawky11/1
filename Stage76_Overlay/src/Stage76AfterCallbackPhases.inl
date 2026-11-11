// Stage76 SOURCE ONLY: passive PRE/POST original-call observer in EE7xx after-callback path.
// Three original 5-byte E8 CALL sites; never uses DR0..DR3 or writes gameplay fields.
// Include from Stage38CallbackWindow.inl AFTER Stage70DeferredWriter.inl.
// IMPORTANT: not built/tested on Windows x86. Preserve Stage74 DLL as rollback.
constexpr DWORD kS76Phases=3u, kS76Capacity=8192u, kS76DepthMax=8u, kS76Fields=10u;
struct S76Record {
    DWORD Seq=0, Phase=0, P=0, O=0, Side=0, Tid=0, ReturnVA=0;
    DWORD Pre[kS76Fields]{}, Post[kS76Fields]{}, PreMask=0, PostMask=0;
    ULONGLONG Q0=0, Q1=0;
};
S76Record g_s76Ring[kS76Capacity]{};
SRWLOCK g_s76Lock=SRWLOCK_INIT;
DWORD g_s76Read=0,g_s76Write=0,g_s76Count=0,g_s76Seq=0;
__declspec(thread) DWORD g_s76Depth[kS76Phases]{};
__declspec(thread) S76Record g_s76Pending[kS76Phases][kS76DepthMax]{};
volatile LONG g_s76Installed=0,g_s76Calls[kS76Phases]{},g_s76Filtered=0;
volatile LONG g_s76Dropped=0,g_s76Overflow=0,g_s76Mismatch=0,g_s76ReadFail=0;
DWORD g_s76NativeEE480=0,g_s76NativeE8DA0=0,g_s76NativeEE5E0=0;
constexpr DWORD kS76Sites[kS76Phases]={0xEE7D1u,0xEE7D6u,0xEE7EAu};
constexpr DWORD kS76Targets[kS76Phases]={0xEE480u,0xE8DA0u,0xEE5E0u};
constexpr DWORD kS76Returns[kS76Phases]={0xEE7D6u,0xEE7DBu,0xEE7EFu};

void S76Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage76AfterCallbackTrace.log"))return;
    SYSTEMTIME now{};GetLocalTime(&now);
    char line[2000]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,message);
    if(n<=0)return;
    HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return;
    DWORD wrote=0;WriteFile(f,line,static_cast<DWORD>(n),&wrote,nullptr);CloseHandle(f);
}
// f0 selector P+3F50; f1 state P+3F54; f2 event header P+3A2C;
// f3 current event code P+3A30; f4 previous mode/code P+3A34;
// f5 registered callback P+3ED0; f6 callback arg P+3ED4;
// f7 flags P+3B7C; f8 immediate request P+3F4C; f9 current object P+4.
// Packed event DWORDs hold two adjacent WORDs; decode offline.
void S76Snapshot(S76Record& r,bool after)
{
    DWORD* dst=after?r.Post:r.Pre;
    DWORD& mask=after?r.PostMask:r.PreMask;
    constexpr DWORD offsets[kS76Fields]={
        0x3F50u,0x3F54u,0x3A2Cu,0x3A30u,0x3A34u,
        0x3ED0u,0x3ED4u,0x3B7Cu,0x3F4Cu,0x04u
    };
    for(DWORD i=0;i<kS76Fields;++i){
        if(SafeReadDword(r.P+offsets[i],&dst[i]))mask|=1u<<i;
        else InterlockedIncrement(&g_s76ReadFail);
    }
    if(!after && (mask&(1u<<9)))r.O=dst[9];
}
void __cdecl S76Enter(const DWORD* saved,DWORD phase)
{
    if(phase>=kS76Phases)return;
    InterlockedIncrement(&g_s76Calls[phase]);
    const DWORD d=g_s76Depth[phase]++;
    if(d>=kS76DepthMax){InterlockedIncrement(&g_s76Overflow);return;}
    S76Record& r=g_s76Pending[phase][d];r=S76Record{};r.Phase=phase;
    // PUSHFD/PUSHAD: EDI[0] ESI[1] EBP[2] ESP[3] EBX[4] EDX[5]
    // ECX[6] EAX[7] EFLAGS[8] native return address[9].
    r.P=saved[1];r.ReturnVA=saved[9];r.Tid=GetCurrentThreadId();
    if(r.P==g_playerP1)r.Side=1u;
    else if(r.P==g_playerP2)r.Side=2u;
    if(!r.Side){InterlockedIncrement(&g_s76Filtered);return;}
    if(r.ReturnVA!=g_gameBase+kS76Returns[phase]){
        InterlockedIncrement(&g_s76Mismatch);r.Side=0;return;
    }
    S76Snapshot(r,false);r.Q0=Stage31Qpc();
}
void __cdecl S76Leave(const DWORD* saved,DWORD phase)
{
    if(phase>=kS76Phases)return;
    if(!g_s76Depth[phase]){InterlockedIncrement(&g_s76Mismatch);return;}
    const DWORD d=--g_s76Depth[phase];
    if(d>=kS76DepthMax)return;
    S76Record& r=g_s76Pending[phase][d];
    if(!r.Side)return;
    r.Q1=Stage31Qpc();
    if(r.P!=saved[1] || r.Tid!=GetCurrentThreadId() ||
       r.ReturnVA!=saved[9]){
        InterlockedIncrement(&g_s76Mismatch);r.Side=0;return;
    }
    S76Snapshot(r,true);
    if((r.PreMask & r.PostMask & (1u<<9)) &&
       (r.O!=r.Pre[9] || r.O!=r.Post[9]))
        InterlockedIncrement(&g_s76Mismatch);
    AcquireSRWLockExclusive(&g_s76Lock);
    r.Seq=++g_s76Seq;
    if(g_s76Count>=kS76Capacity)InterlockedIncrement(&g_s76Dropped);
    else {
        g_s76Ring[g_s76Write]=r;
        g_s76Write=(g_s76Write+1u)%kS76Capacity;++g_s76Count;
    }
    ReleaseSRWLockExclusive(&g_s76Lock);r.Side=0;
}

// Every wrapper is reached by an original E8 CALL, so [ESP] is the native
// continuation address. On exit: restore native post-call GPR/EFLAGS/x87/SSE
// and RET to the original continuation. Original call executes EXACTLY ONCE.
// The wrapper changes the return address seen inside the callee: runtime
// equivalence is NOT guaranteed until empirically validated on Windows.
__declspec(naked) void S76HookEE480()
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
        push 0
        push ebx
        call S76Enter
        add esp,8
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd

        call dword ptr [g_s76NativeEE480] // original ECX=this=P; no stack args

        pushfd
        pushad
        mov ebx,esp
        sub esp,210h
        lea edi,[esp+0Fh]
        and edi,0FFFFFFF0h
        fxsave [edi]
        fninit
        cld
        push 0
        push ebx
        call S76Leave
        add esp,8
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret                              // native EE7D6
    }
}
__declspec(naked) void S76HookE8DA0()
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
        push 1
        push ebx
        call S76Enter
        add esp,8
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd

        call dword ptr [g_s76NativeE8DA0] // original uses ESI=P; no stack args

        pushfd
        pushad
        mov ebx,esp
        sub esp,210h
        lea edi,[esp+0Fh]
        and edi,0FFFFFFF0h
        fxsave [edi]
        fninit
        cld
        push 1
        push ebx
        call S76Leave
        add esp,8
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret                               // native EE7DB
    }
}
__declspec(naked) void S76HookEE5E0()
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
        push 2
        push ebx
        call S76Enter
        add esp,8
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd

        // ORIGINAL at EE7E9 did PUSH ESI before CALL EE5E0.
        // Hook entered via CALL, [ESP]=EE7EF and [ESP+4]=the original P arg.
        // Give a copy to EE5E0. Keep the original arg for EE7EF ADD ESP,4.
        push dword ptr [esp+4]
        call dword ptr [g_s76NativeEE5E0]
        add esp,4

        pushfd
        pushad
        mov ebx,esp
        sub esp,210h
        lea edi,[esp+0Fh]
        and edi,0FFFFFFF0h
        fxsave [edi]
        fninit
        cld
        push 2
        push ebx
        call S76Leave
        add esp,8
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret                                // native EE7EF, then ADD ESP,4
    }
}

bool S76OriginalOK()
{
    // NO reference to the Stage38 patched EE7C7 site: it is already detoured.
    const BYTE atD1[5]={0xE8,0xAA,0xFC,0xFF,0xFF};
    const BYTE atD6[5]={0xE8,0xC5,0xA5,0xFF,0xFF};
    const BYTE atEA[5]={0xE8,0xF1,0xFD,0xFF,0xFF};
    const BYTE preD1[2]={0x8B,0xCE};       // EE7CF: MOV ECX,ESI
    const BYTE postD6[6]={0x8B,0x8E,0x7C,0x3B,0x00,0x00};
    const BYTE preEA[6]={0xF6,0xC1,0x01,0x74,0x09,0x56}; // EE7E4
    const BYTE postEA[3]={0x83,0xC4,0x04}; // EE7EF
    const BYTE* base=reinterpret_cast<const BYTE*>(static_cast<std::uintptr_t>(g_gameBase));
    return std::memcmp(base+0xEE7CFu,preD1,2u)==0 &&
           std::memcmp(base+0xEE7D1u,atD1,5u)==0 &&
           std::memcmp(base+0xEE7D6u,atD6,5u)==0 &&
           std::memcmp(base+0xEE7DBu,postD6,6u)==0 &&
           std::memcmp(base+0xEE7E4u,preEA,6u)==0 &&
           std::memcmp(base+0xEE7EAu,atEA,5u)==0 &&
           std::memcmp(base+0xEE7EFu,postEA,3u)==0;
}
bool S76SuspendPeers(HANDLE* peers,DWORD& count)
{
    count=0;
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    if(snap==INVALID_HANDLE_VALUE)return false;
    THREADENTRY32 e{};e.dwSize=sizeof(e);
    bool ok=Thread32First(snap,&e)!=0;
    if(ok)do{
        if(e.th32OwnerProcessID==GetCurrentProcessId() &&
           e.th32ThreadID!=GetCurrentThreadId()){
            if(count>=512u){ok=false;break;}
            HANDLE h=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT,FALSE,e.th32ThreadID);
            if(!h){ok=false;break;}
            if(SuspendThread(h)==static_cast<DWORD>(-1)){
                CloseHandle(h);ok=false;break;
            }
            peers[count++]=h;
            CONTEXT c{};c.ContextFlags=CONTEXT_CONTROL;
            if(!GetThreadContext(h,&c)){ok=false;break;}
            for(DWORD j=0;j<kS76Phases;++j){
                if(c.Eip>=g_gameBase+kS76Sites[j] &&
                   c.Eip<g_gameBase+kS76Sites[j]+5u){ok=false;break;}
            }
            if(!ok)break;
        }
        e.dwSize=sizeof(e);
    }while(Thread32Next(snap,&e));
    if(ok && GetLastError()!=ERROR_NO_MORE_FILES)ok=false;
    CloseHandle(snap);return ok;
}
bool InstallStage76Hooks()
{
    S76Log("STAGE76_BUILD=2026-11-11-r1 SOURCE_ONLY after-callback E8 phases; DRwrites=0; gameWrites=0");
    const bool x86=sizeof(void*)==4u;
    const bool xmm=IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)!=0;
    const LONG s38=InterlockedCompareExchange(&g_stage38Installed,0,0);
    const LONG s70=InterlockedCompareExchange(&g_s70Installed,0,0);
    const LONG s59=InterlockedCompareExchange(&g_s59Started,0,0);
    const LONG s76=InterlockedCompareExchange(&g_s76Installed,0,0);
    const bool signature=S76OriginalOK();
    char guard[450]{};
    sprintf_s(guard,
        "STAGE76_GUARD x86=%u xmm=%u stage38=%ld stage70=%ld stage59Started=%ld stage76=%ld nativeOK=%u gameBase=%08lX",
        x86?1u:0u,xmm?1u:0u,s38,s70,s59,s76,signature?1u:0u,g_gameBase);
    S76Log(guard);
    if(!x86 || !xmm || s38!=1 || s70!=1 || s59!=0 || s76!=0 || !signature){
        S76Log("ERROR: Stage76 prerequisites/signature failed; NO patch.");
        return false;
    }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S76Log("ERROR: Stage76 patch mutex unavailable.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0 && wait!=WAIT_ABANDONED){
        CloseHandle(mutex);S76Log("ERROR: Stage76 patch mutex wait failed.");return false;
    }
    HANDLE peers[512]{};DWORD count=0;
    const bool suspended=S76SuspendPeers(peers,count);
    BYTE* addr[kS76Phases]{};
    for(DWORD i=0;i<kS76Phases;++i)
        addr[i]=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+kS76Sites[i]));
    BYTE* hooks[kS76Phases]={
        reinterpret_cast<BYTE*>(&S76HookEE480),
        reinterpret_cast<BYTE*>(&S76HookE8DA0),
        reinterpret_cast<BYTE*>(&S76HookEE5E0)
    };
    BYTE original[kS76Phases][5]{};
    BYTE patches[kS76Phases][5]{};
    for(DWORD i=0;i<kS76Phases;++i){
        patches[i][0]=0xE8u;
        const DWORD disp=static_cast<DWORD>(
            reinterpret_cast<std::uintptr_t>(hooks[i])-
            reinterpret_cast<std::uintptr_t>(addr[i]+5u));
        std::memcpy(patches[i]+1u,&disp,4u);
    }
    // Entire set fits into one existing executable page; never patch call sites
    // individually without locking and suspending peers. Original CALLs are
    // unchanged in length and have no PE HIGHLOW relocation operand.
    DWORD oldProtect=0,unused=0;
    const bool writable=suspended &&
        VirtualProtect(addr[0],0x1Eu,PAGE_EXECUTE_READWRITE,&oldProtect)!=0;
    bool installed=false,restoreOK=true,cacheOK=false;
    if(writable && S76OriginalOK()){
        for(DWORD i=0;i<kS76Phases;++i)
            std::memcpy(original[i],addr[i],5u);
        g_s76NativeEE480=g_gameBase+kS76Targets[0];
        g_s76NativeE8DA0=g_gameBase+kS76Targets[1];
        g_s76NativeEE5E0=g_gameBase+kS76Targets[2];
        for(DWORD i=0;i<kS76Phases;++i)
            std::memcpy(addr[i],patches[i],5u);
        cacheOK=FlushInstructionCache(GetCurrentProcess(),addr[0],0x1Eu)!=0;
        if(cacheOK)installed=true;
        else{
            for(DWORD i=0;i<kS76Phases;++i)
                std::memcpy(addr[i],original[i],5u);
            FlushInstructionCache(GetCurrentProcess(),addr[0],0x1Eu);
        }
    }
    if(writable && !VirtualProtect(addr[0],0x1Eu,oldProtect,&unused)){
        restoreOK=false;
        if(installed){
            // Best-effort fail closed: return original CALL bytes before peers resume.
            for(DWORD i=0;i<kS76Phases;++i)
                std::memcpy(addr[i],original[i],5u);
            FlushInstructionCache(GetCurrentProcess(),addr[0],0x1Eu);
            installed=false;
        }
    }
    ResumeThreads(peers,count);ReleaseMutex(mutex);CloseHandle(mutex);
    if(!installed || !restoreOK || !cacheOK){
        S76Log("ERROR: Stage76 transaction refused or rollback; leave known-good Stage74 DLL in reserve.");
        return false;
    }
    InterlockedExchange(&g_s76Installed,3);
    S76Log("STAGE76_INSTALLED 3/3 EE7D1->EE480 EE7D6->E8DA0 EE7EA->EE5E0; originals once; DRwrites=0");
    return true;
}
void FlushStage76()
{
    for(DWORD i=0;i<kS76Capacity;++i){
        S76Record r{};
        AcquireSRWLockExclusive(&g_s76Lock);
        const bool available=g_s76Count!=0;
        if(available){
            r=g_s76Ring[g_s76Read];
            g_s76Read=(g_s76Read+1u)%kS76Capacity;--g_s76Count;
        }
        ReleaseSRWLockExclusive(&g_s76Lock);
        if(!available)break;
        char msg[1800]{};
        int n=sprintf_s(msg,
            "STAGE76_PHASE seq=%lu phase=%lu callRVA=%06lX targetRVA=%06lX side=P%lu player=%08lX object=%08lX tid=%lu "
            "qpcBefore=%I64u qpcAfter=%I64u preMask=%03lX postMask=%03lX",
            r.Seq,r.Phase,kS76Sites[r.Phase],kS76Targets[r.Phase],
            r.Side,r.P,r.O,r.Tid,r.Q0,r.Q1,r.PreMask,r.PostMask);
        if(n<=0)continue;
        for(DWORD j=0;j<kS76Fields;++j){
            const int next=sprintf_s(msg+n,sizeof(msg)-static_cast<size_t>(n),
                " f%lu=%08lX>%08lX",j,r.Pre[j],r.Post[j]);
            if(next<=0)break;
            n+=next;
        }
        S76Log(msg);
    }
    const LONG c0=InterlockedExchange(&g_s76Calls[0],0);
    const LONG c1=InterlockedExchange(&g_s76Calls[1],0);
    const LONG c2=InterlockedExchange(&g_s76Calls[2],0);
    const LONG dropped=InterlockedExchange(&g_s76Dropped,0);
    const LONG overflow=InterlockedExchange(&g_s76Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s76Mismatch,0);
    const LONG readFail=InterlockedExchange(&g_s76ReadFail,0);
    const LONG filtered=InterlockedExchange(&g_s76Filtered,0);
    if(c0||c1||c2||dropped||overflow||mismatch||readFail||filtered){
        char health[400]{};
        sprintf_s(health,
            "STAGE76_HEALTH callsEE480=%ld callsE8DA0=%ld callsEE5E0=%ld filtered=%ld dropped=%ld overflow=%ld mismatch=%ld readFail=%ld ring=8192 depth=8",
            c0,c1,c2,filtered,dropped,overflow,mismatch,readFail);
        S76Log(health);
    }
}
