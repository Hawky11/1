// Stage40: passive AD72F->E8480, E84E3->125920, E8505->1357F0.
// Included by Stage38CallbackWindow.inl inside AIQueueTrace.cpp anonymous namespace.
// Native callees execute once, all game-state accesses below are read-only.
constexpr DWORD kS40Capacity=2048u, kS40Depth=16u;
struct S40Rec {
    DWORD Seq=0,P=0,Side=0,Tid=0, preMask=0,postMask=0,pre[8]{},post[8]{};
    DWORD selSeen=0,sel=0xFFFFFFFFu,selReq=0,selReqValid=0;
    DWORD actorSeen=0,actor=0,actorSel=0,actorReq=0,actorReqValid=0;
    DWORD actorPreMask=0,actorPostMask=0,actorPre[3]{},actorPost[3]{};
    ULONGLONG q0=0,qs=0,qa=0,q1=0;
};
volatile LONG g_s40Installed=0,g_s40Dropped=0,g_s40Overflow=0,g_s40Mismatch=0;
DWORD g_s40NativeEntry=0,g_s40NativeSelect=0,g_s40NativeActor=0;
__declspec(thread) DWORD g_s40Depth=0;
__declspec(thread) S40Rec g_s40Pending[kS40Depth]{};
SRWLOCK g_s40Lock=SRWLOCK_INIT;
S40Rec g_s40Ring[kS40Capacity]{};
DWORD g_s40Read=0,g_s40Write=0,g_s40Count=0,g_s40Seq=0;

void S40Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage40ActionConsumerTrace.log"))return;
    SYSTEMTIME now{};GetLocalTime(&now);
    char line[2500]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,message);
    if(n<=0)return;
    HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return;
    DWORD nWritten=0;WriteFile(f,line,static_cast<DWORD>(n),&nWritten,nullptr);CloseHandle(f);
}
void S40SnapP(DWORD p,DWORD* fields,DWORD& mask)
{
    const DWORD offs[8]={0x3F4Cu,0x4u,0x3B88u,0x3B7Cu,0x43B4u,0x3978u,0x3F20u,0x3B84u};
    mask=0;
    for(DWORD i=0;i<8u;++i)if(SafeReadDword(p+offs[i],&fields[i]))mask|=1u<<i;
}
void S40SnapActor(DWORD a,DWORD* fields,DWORD& mask)
{
    const DWORD offs[3]={0x10u,0x280u,0x2D0u};
    mask=0;
    if(!a)return;
    for(DWORD i=0;i<3u;++i)if(SafeReadDword(a+offs[i],&fields[i]))mask|=1u<<i;
}
void __cdecl S40Enter(const DWORD* saved)
{
    const DWORD d=g_s40Depth++;
    if(d>=kS40Depth){InterlockedIncrement(&g_s40Overflow);return;}
    S40Rec& r=g_s40Pending[d];r=S40Rec{};
    const DWORD p=saved[6]; // PUSHAD ECX: native thiscall P
    if(p!=g_playerP1 && p!=g_playerP2)return;
    r.P=p;r.Side=p==g_playerP1?1u:2u;r.Tid=GetCurrentThreadId();
    S40SnapP(p,r.pre,r.preMask);r.q0=Stage31Qpc();
}
void __cdecl S40Select(const DWORD* saved)
{
    const DWORD d=g_s40Depth;
    if(!d || d>kS40Depth)return;
    S40Rec& r=g_s40Pending[d-1u];
    if(!r.P)return;
    if(saved[1]!=r.P){InterlockedIncrement(&g_s40Mismatch);return;} // ESI=P
    r.selSeen=1;r.sel=saved[7]; // original 125920 EAX
    r.selReqValid=SafeReadDword(r.P+0x3F4Cu,&r.selReq)?1u:0u;
    r.qs=Stage31Qpc();
}
void __cdecl S40Actor(const DWORD* saved)
{
    const DWORD d=g_s40Depth;
    if(!d || d>kS40Depth)return;
    S40Rec& r=g_s40Pending[d-1u];
    if(!r.P)return;
    if(saved[1]!=r.P){InterlockedIncrement(&g_s40Mismatch);return;}
    r.actorSeen=1;r.actor=saved[7]; // 1357F0 returns exact actor in EAX
    r.actorSel=saved[0]; // EDI is validated selector
    r.actorReqValid=SafeReadDword(r.P+0x3F4Cu,&r.actorReq)?1u:0u;
    S40SnapActor(r.actor,r.actorPre,r.actorPreMask);r.qa=Stage31Qpc();
}
void __cdecl S40Leave(const DWORD* /* saved */)
{
    if(!g_s40Depth){InterlockedIncrement(&g_s40Mismatch);return;}
    const DWORD d=--g_s40Depth;
    if(d>=kS40Depth)return;
    S40Rec& r=g_s40Pending[d];
    if(!r.P)return;
    r.q1=Stage31Qpc();S40SnapP(r.P,r.post,r.postMask);
    if(r.actorSeen)S40SnapActor(r.actor,r.actorPost,r.actorPostMask);
    AcquireSRWLockExclusive(&g_s40Lock);
    r.Seq=++g_s40Seq;
    if(g_s40Count==kS40Capacity)InterlockedIncrement(&g_s40Dropped);
    else {
        g_s40Ring[g_s40Write]=r;
        g_s40Write=(g_s40Write+1u)%kS40Capacity;
        ++g_s40Count;
    }
    ReleaseSRWLockExclusive(&g_s40Lock);
    r.P=0;
}
// saved[] PUSHFD/PUSHAD layout EDI,ESI,EBP,ESP,EBX,EDX,ECX,EAX.
// FXSAVE/FXRSTOR preserve x87/SSE; original GPRs/EFLAGS are restored.
// The frequent no-request path tail-jumps without logging or C++ calls.
__declspec(naked) void S40EntryHook()
{
    __asm {
        pushfd
        cmp dword ptr [ecx+03F4Ch],0FFFFFFFFh
        jne s40_capture
        popfd
        jmp dword ptr [g_s40NativeEntry]
    s40_capture:
        popfd
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
        call S40Enter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        call dword ptr [g_s40NativeEntry]
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
        call S40Leave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret
    }
}
__declspec(naked) void S40SelectHook()
{
    __asm {
        // Native 125920 is cdecl(arg0,arg1), AD-E8480 caller cleans 8 bytes.
        // Each push moves ESP by 4: the same [esp+8] fetches arg1 then arg0.
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        call dword ptr [g_s40NativeSelect]
        add esp,8
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
        call S40Select
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret
    }
}
__declspec(naked) void S40ActorHook()
{
    __asm {
        // Native 1357F0 is cdecl(arg0,arg1,arg2,arg3), caller cleans 16.
        // Four equal [esp+10h] offsets replay arg3,arg2,arg1,arg0 in order.
        push dword ptr [esp+10h]
        push dword ptr [esp+10h]
        push dword ptr [esp+10h]
        push dword ptr [esp+10h]
        call dword ptr [g_s40NativeActor]
        add esp,10h
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
        call S40Actor
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret
    }
}
bool InstallStage40Hooks()
{
    if(!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)){
        S40Log("ERROR: Stage40 FXSAVE unsupported, no patches.");return false;
    }
    BYTE* e=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAD72Fu));
    BYTE* s=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xE84E3u));
    BYTE* a=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xE8505u));
    BYTE* ne=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xE8480u));
    BYTE* ns=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x125920u));
    BYTE* na=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x1357F0u));
    const BYTE be[5]={0xE8,0x4C,0xAD,0x03,0x00};
    const BYTE bs[5]={0xE8,0x38,0xD4,0x03,0x00};
    const BYTE ba[5]={0xE8,0xE6,0xD2,0x04,0x00};
    const BYTE pe[]={0x55,0x8B,0xEC,0x83,0xEC,0x10,0x56,0x8B,0xF1,0x8B,0x86,0x4C,0x3F,0x00,0x00,0x83,0xF8,0xFF};
    // 125920+5 is a PE HIGHLOW relocation (RVA 125925). At runtime Windows
    // adds (loadedImageBase - 00400000) to original VA 05CF7260.
    // Compare opcode bytes and the RELOCATED address instead of raw disk bytes.
    const BYTE psHead[]={0x55,0x8B,0xEC,0x8A,0x15};
    const DWORD expectedSelectGlobal=0x05CF7260u+(g_gameBase-0x00400000u);
    const BYTE pa[]={0x55,0x8B,0xEC,0x8B,0x45,0x08,0x83,0xEC,0x08};
    auto valid=[&]()->bool{
        if(std::memcmp(e,be,5)||std::memcmp(s,bs,5)||std::memcmp(a,ba,5)||
           std::memcmp(ne,pe,sizeof(pe))||std::memcmp(ns,psHead,sizeof(psHead))||std::memcmp(na,pa,sizeof(pa)))
            return false;
        DWORD actualSelectGlobal=0;
        std::memcpy(&actualSelectGlobal,ns+5u,sizeof(actualSelectGlobal));
        if(actualSelectGlobal!=expectedSelectGlobal)return false;
        const BYTE* site[3]={e,s,a};const BYTE* dest[3]={ne,ns,na};
        for(int i=0;i<3;++i){
            std::int32_t rel=0;std::memcpy(&rel,site[i]+1,4);
            if(site[i]+5+rel!=dest[i])return false;
        }
        return true;
    };
    if(!valid()){S40Log("ERROR: Stage40 signatures/call targets mismatch; no patches.");return false;}
    BYTE patchE[5]{},patchS[5]{},patchA[5]{};
    if(!MakeRelativeCall(e,reinterpret_cast<const void*>(&S40EntryHook),patchE)||
       !MakeRelativeCall(s,reinterpret_cast<const void*>(&S40SelectHook),patchS)||
       !MakeRelativeCall(a,reinterpret_cast<const void*>(&S40ActorHook),patchA)){
        S40Log("ERROR: Stage40 rel32 out of range; no patches.");return false;
    }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex)return false;
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0 && wait!=WAIT_ABANDONED){CloseHandle(mutex);return false;}
    HANDLE suspended[512]{};
    const DWORD count=SuspendOtherProcessThreads(suspended,512);
    DWORD oe=0,os=0,oa=0;
    const bool ve=VirtualProtect(e,5,PAGE_EXECUTE_READWRITE,&oe)!=0;
    const bool vs=ve && VirtualProtect(s,5,PAGE_EXECUTE_READWRITE,&os)!=0;
    const bool va=vs && VirtualProtect(a,5,PAGE_EXECUTE_READWRITE,&oa)!=0;
    bool installed=false;
    if(va && valid()){
        g_s40NativeEntry=g_gameBase+0xE8480u;
        g_s40NativeSelect=g_gameBase+0x125920u;
        g_s40NativeActor=g_gameBase+0x1357F0u;
        std::memcpy(e,patchE,5);std::memcpy(s,patchS,5);std::memcpy(a,patchA,5);
        FlushInstructionCache(GetCurrentProcess(),e,5);
        FlushInstructionCache(GetCurrentProcess(),s,5);
        FlushInstructionCache(GetCurrentProcess(),a,5);
        InterlockedExchange(&g_s40Installed,1);installed=true;
    }
    if(va){DWORD x=0;VirtualProtect(a,5,oa,&x);}
    if(vs){DWORD x=0;VirtualProtect(s,5,os,&x);}
    if(ve){DWORD x=0;VirtualProtect(e,5,oe,&x);}
    ResumeThreads(suspended,count);ReleaseMutex(mutex);CloseHandle(mutex);
    S40Log("STAGE40_ACTION_CONSUMER_BUILD=2026-11-10-r1");
    S40Log(installed?
        "STAGE40_INSTALLED 3/3 AD72F->E8480 E84E3->125920 E8505->1357F0 original calls once; passive.":
        "ERROR: Stage40 install failed; no patches.");
    return installed;
}
void FlushStage40()
{
    for(DWORD i=0;i<kS40Capacity;++i){
        S40Rec r{};
        AcquireSRWLockExclusive(&g_s40Lock);
        const bool have=g_s40Count!=0;
        if(have){r=g_s40Ring[g_s40Read];g_s40Read=(g_s40Read+1u)%kS40Capacity;--g_s40Count;}
        ReleaseSRWLockExclusive(&g_s40Lock);
        if(!have)break;
        const char* path=!r.selSeen?"NO_SELECTOR":r.sel==0xFFFFFFFFu?"SELECTOR_SENTINEL":r.actorSeen?"ACTOR_RETURNED":"NO_ACTOR";
        char line[1600]{};
        const int n=sprintf_s(line,
            "STAGE40_CONSUMER_WINDOW seq=%lu side=P%lu player=%08lX tid=%lu "
            "qpcBefore=%I64u qpcSelector=%I64u qpcActor=%I64u qpcAfter=%I64u "
            "path=%s selectorSeen=%lu selector=%08lX selectorRequestRead=%lu requestAtSelector=%08lX "
            "actorSeen=%lu actor=%08lX selectorAtActor=%08lX actorRequestRead=%lu requestAtActor=%08lX "
            "preMask=%02lX postMask=%02lX actorPreMask=%01lX actorPostMask=%01lX "
            "p0=%08lX>%08lX p1=%08lX>%08lX p2=%08lX>%08lX p3=%08lX>%08lX "
            "p4=%08lX>%08lX p5=%08lX>%08lX p6=%08lX>%08lX p7=%08lX>%08lX "
            "a0=%08lX>%08lX a1=%08lX>%08lX a2=%08lX>%08lX",
            static_cast<unsigned long>(r.Seq),static_cast<unsigned long>(r.Side),
            static_cast<unsigned long>(r.P),static_cast<unsigned long>(r.Tid),
            static_cast<unsigned long long>(r.q0),static_cast<unsigned long long>(r.qs),
            static_cast<unsigned long long>(r.qa),static_cast<unsigned long long>(r.q1),
            path,static_cast<unsigned long>(r.selSeen),static_cast<unsigned long>(r.sel),
            static_cast<unsigned long>(r.selReqValid),static_cast<unsigned long>(r.selReq),
            static_cast<unsigned long>(r.actorSeen),static_cast<unsigned long>(r.actor),
            static_cast<unsigned long>(r.actorSel),static_cast<unsigned long>(r.actorReqValid),
            static_cast<unsigned long>(r.actorReq),
            static_cast<unsigned long>(r.preMask),static_cast<unsigned long>(r.postMask),
            static_cast<unsigned long>(r.actorPreMask),static_cast<unsigned long>(r.actorPostMask),
            static_cast<unsigned long>(r.pre[0]),static_cast<unsigned long>(r.post[0]),
            static_cast<unsigned long>(r.pre[1]),static_cast<unsigned long>(r.post[1]),
            static_cast<unsigned long>(r.pre[2]),static_cast<unsigned long>(r.post[2]),
            static_cast<unsigned long>(r.pre[3]),static_cast<unsigned long>(r.post[3]),
            static_cast<unsigned long>(r.pre[4]),static_cast<unsigned long>(r.post[4]),
            static_cast<unsigned long>(r.pre[5]),static_cast<unsigned long>(r.post[5]),
            static_cast<unsigned long>(r.pre[6]),static_cast<unsigned long>(r.post[6]),
            static_cast<unsigned long>(r.pre[7]),static_cast<unsigned long>(r.post[7]),
            static_cast<unsigned long>(r.actorPre[0]),static_cast<unsigned long>(r.actorPost[0]),
            static_cast<unsigned long>(r.actorPre[1]),static_cast<unsigned long>(r.actorPost[1]),
            static_cast<unsigned long>(r.actorPre[2]),static_cast<unsigned long>(r.actorPost[2]));
        if(n>0)S40Log(line);
    }
    const LONG dropped=InterlockedExchange(&g_s40Dropped,0);
    const LONG overflow=InterlockedExchange(&g_s40Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s40Mismatch,0);
    if(dropped||overflow||mismatch){
        char line[200]{};
        sprintf_s(line,"WARNING: Stage40 dropped=%ld pendingOverflow=%ld mismatch=%ld ring=2048 depth=16.",
            dropped,overflow,mismatch);
        S40Log(line);
    }
}
