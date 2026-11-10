// Stage67 SOURCE ONLY, Windows x86. Included after Stage65RequestOrigin.inl.
// Two passive probes. No DR use, no replacement action values.
// SHA-pinned original opcode checks are reproduced by verify_stage67.py.
constexpr DWORD kS67Capacity=8192u,kS67DepthLimit=8u;
struct S67Rec {
    DWORD Seq=0,Kind=0,P=0,O=0,Side=0,Tid=0,Return=0,Value=0,Eax=0;
    DWORD Pre[5]{},Post[5]{},PreMask=0,PostMask=0;
    ULONGLONG Q0=0,Q1=0;
};
S67Rec g_s67Ring[kS67Capacity]{};
SRWLOCK g_s67Lock=SRWLOCK_INIT;
DWORD g_s67Read=0,g_s67Write=0,g_s67Count=0,g_s67Seq=0;
// Separate TLS lanes: nested zero-writer calls cannot consume a promotion slot.
__declspec(thread) DWORD g_s67Depth[2]{};
__declspec(thread) S67Rec g_s67Pending[2][kS67DepthLimit]{};
volatile LONG g_s67Installed=0,g_s67Calls[2]{},g_s67Filtered[2]{};
volatile LONG g_s67Dropped=0,g_s67Overflow=0,g_s67Mismatch=0,g_s67ReadFail=0;
DWORD g_s67Native=0,g_s67Resume=0;

void S67Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage67NativePathsTrace.log"))return;
    SYSTEMTIME now{};GetLocalTime(&now);
    char line[1800]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,message);
    if(n<=0)return;
    HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return;
    DWORD written=0;WriteFile(f,line,static_cast<DWORD>(n),&written,nullptr);CloseHandle(f);
}
void S67Snapshot(S67Rec& r,bool after)
{
    DWORD* v=after?r.Post:r.Pre;
    DWORD& mask=after?r.PostMask:r.PreMask;
    if(r.Kind==0u){
        // o0=O+08 flags, o1=O+30 gate BYTE (read DWORD then mask),
        // o2=O+60 active, o3=O+64 pending, o4=O+68 deferred.
        const DWORD offsets[5]={8u,0x30u,0x60u,0x64u,0x68u};
        for(DWORD i=0;i<5u;++i){
            if(SafeReadDword(r.O+offsets[i],&v[i]))mask|=1u<<i;
            else InterlockedIncrement(&g_s67ReadFail);
        }
        v[1]&=0xFFu;
    }else{
        // z0=request P+3F4C, z1=flags P+3B7C, z2=runtime source global.
        const DWORD addresses[3]={r.P+0x3F4Cu,r.P+0x3B7Cu,g_gameBase+0x4254E8u};
        for(DWORD i=0;i<3u;++i){
            if(SafeReadDword(addresses[i],&v[i]))mask|=1u<<i;
            else InterlockedIncrement(&g_s67ReadFail);
        }
    }
}
void S67Enter(const DWORD* saved,DWORD kind)
{
    InterlockedIncrement(&g_s67Calls[kind]);
    const DWORD d=g_s67Depth[kind]++;
    if(d>=kS67DepthLimit){InterlockedIncrement(&g_s67Overflow);return;}
    S67Rec& r=g_s67Pending[kind][d];r=S67Rec{};r.Kind=kind;
    if(kind==0u){
        r.O=saved[6];r.Return=saved[9]; // ECX=O, CALL return AD761.
        DWORD o1=0,o2=0;
        const bool ok1=g_playerP1 && SafeReadDword(g_playerP1+4u,&o1);
        const bool ok2=g_playerP2 && SafeReadDword(g_playerP2+4u,&o2);
        if(r.O && ok1 && o1==r.O){r.P=g_playerP1;r.Side=1;}
        else if(r.O && ok2 && o2==r.O){r.P=g_playerP2;r.Side=2;}
        if(r.Return!=g_gameBase+0xAD761u){
            InterlockedIncrement(&g_s67Mismatch);r.Side=0;return;
        }
    }else{
        r.P=saved[1];r.Value=saved[5]; // ESI=P, EDX=actual store operand.
        if(r.P && r.P==g_playerP1)r.Side=1;
        else if(r.P && r.P==g_playerP2)r.Side=2;
        if(r.Side && !SafeReadDword(r.P+4u,&r.O))InterlockedIncrement(&g_s67ReadFail);
    }
    if(!r.Side){InterlockedIncrement(&g_s67Filtered[kind]);return;}
    r.Tid=GetCurrentThreadId();
    S67Snapshot(r,false);r.Q0=Stage31Qpc();
}
void S67Leave(const DWORD* saved,DWORD kind)
{
    if(!g_s67Depth[kind]){InterlockedIncrement(&g_s67Mismatch);return;}
    const DWORD d=--g_s67Depth[kind];
    if(d>=kS67DepthLimit)return;
    S67Rec& r=g_s67Pending[kind][d];
    if(!r.Side)return;
    r.Q1=Stage31Qpc();r.Eax=saved[7];
    if(r.Tid!=GetCurrentThreadId() || (kind==1u &&
        (saved[1]!=r.P || saved[5]!=r.Value))){
        InterlockedIncrement(&g_s67Mismatch);r.Side=0;return;
    }
    // Do not require volatile ECX after the original function returns.
    S67Snapshot(r,true);
    AcquireSRWLockExclusive(&g_s67Lock);
    r.Seq=++g_s67Seq;
    if(g_s67Count==kS67Capacity)InterlockedIncrement(&g_s67Dropped);
    else{
        g_s67Ring[g_s67Write]=r;g_s67Write=(g_s67Write+1u)%kS67Capacity;
        ++g_s67Count;
    }
    ReleaseSRWLockExclusive(&g_s67Lock);r.Side=0;
}
void __cdecl S67PromotionEnter(const DWORD* s){S67Enter(s,0u);}
void __cdecl S67PromotionLeave(const DWORD* s){S67Leave(s,0u);}
void __cdecl S67WriterEnter(const DWORD* s){S67Enter(s,1u);}
void __cdecl S67WriterLeave(const DWORD* s){S67Leave(s,1u);}

// No stack arguments, native RET. Both native outcomes return through POST.
// Stage43 continues to log RAW stack provenance: its outer return for this
// one wrapped site is now in this DLL. Use Stage67 QPC containment + Return,
// NOT the old 14-callsite knownCaller test, for AD75C attribution in this build.
__declspec(naked) void S67PromotionHook()
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
        call S67PromotionEnter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        call dword ptr [g_s67Native]
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
        call S67PromotionLeave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        ret
    }
}
// JMP detour over ONE six-byte MOV. No added CALL/return stack frame.
// Native MOV runs exactly once with restored ESI/EDX; MOV leaves FLAGS intact.
__declspec(naked) void S67WriterHook()
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
        call S67WriterEnter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        mov dword ptr [esi+3F4Ch],edx
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
        call S67WriterLeave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        jmp dword ptr [g_s67Resume]
    }
}

bool S67OriginalOK()
{
    // Stage42 has already patched 1E9F62: deliberately exclude those 5 bytes.
    const BYTE call[]={0xE8,0xDF,0xC7,0x13,0x00};
    const BYTE promotion[]={0x80,0x79,0x30,0x00,0x74,0x16,0x8B,0x41,0x68,
        0x83,0xF8,0xFF,0x74,0x0E,0x89,0x41,0x64,0xC7,0x41,0x68,0xFF,
        0xFF,0xFF,0xFF,0x83,0x49,0x08,0x01,0xF6,0x41,0x08,0x01,0x74,0x08};
    const BYTE exits[]={0xB0,0x01,0xC3,0x32,0xC0,0xC3};
    const BYTE write[]={0x89,0x96,0x4C,0x3F,0x00,0x00};
    const BYTE gate[]={0x84,0xC0,0x74,0x0C,0x8B,0x15};
    const BYTE next[]={0x81,0xA6,0x84,0x3B,0x00,0x00,0xFF,0xFF,0xFE,0xFF};
    const BYTE* base=reinterpret_cast<const BYTE*>(static_cast<std::uintptr_t>(g_gameBase));
    DWORD globalOperand=0;
    return SafeReadDword(g_gameBase+0xE9E5Cu,&globalOperand) &&
        globalOperand==g_gameBase+0x4254E8u &&
        std::memcmp(base+0xAD75Cu,call,sizeof(call))==0 &&
        std::memcmp(base+0x1E9F40u,promotion,sizeof(promotion))==0 &&
        std::memcmp(base+0x1E9F67u,exits,sizeof(exits))==0 &&
        std::memcmp(base+0xE9E60u,write,sizeof(write))==0 &&
        std::memcmp(base+0xE9E56u,gate,sizeof(gate))==0 &&
        std::memcmp(base+0xE9E66u,next,sizeof(next))==0;
}
// Stronger than the legacy suspension helper: abort if any enumerated peer
// cannot be stopped/queried, capacity is exhausted, or EIP is in either patch.
// Reads CONTEXT_CONTROL only: never requests/writes debug registers.
bool S67SuspendPeers(HANDLE* peers,DWORD& count)
{
    count=0;
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    if(snap==INVALID_HANDLE_VALUE)return false;
    THREADENTRY32 e{};e.dwSize=sizeof(e);
    bool ok=Thread32First(snap,&e)!=0;
    if(ok)do{
        if(e.th32OwnerProcessID==GetCurrentProcessId() && e.th32ThreadID!=GetCurrentThreadId()){
            if(count==512u){ok=false;break;}
            HANDLE t=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT,FALSE,e.th32ThreadID);
            if(!t){ok=false;break;}
            if(SuspendThread(t)==static_cast<DWORD>(-1)){CloseHandle(t);ok=false;break;}
            peers[count++]=t;
            CONTEXT c{};c.ContextFlags=CONTEXT_CONTROL;
            if(!GetThreadContext(t,&c)){ok=false;break;}
            if((c.Eip>=g_gameBase+0xAD75Cu && c.Eip<g_gameBase+0xAD761u) ||
               (c.Eip>=g_gameBase+0xE9E60u && c.Eip<g_gameBase+0xE9E66u)){
                ok=false;break;
            }
        }
        e.dwSize=sizeof(e);
    }while(Thread32Next(snap,&e));
    if(ok && GetLastError()!=ERROR_NO_MORE_FILES)ok=false;
    CloseHandle(snap);return ok;
}
bool InstallStage67Hooks()
{
    S67Log("STAGE67_NATIVE_PATHS_BUILD=2026-11-11-r1 SOURCE_ONLY; DRwrites=0.");
    if(sizeof(void*)!=4u || !IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) ||
        InterlockedCompareExchange(&g_s42Installed,0,0)!=1 ||
        InterlockedCompareExchange(&g_s58Installed,0,0)!=3 ||
        InterlockedCompareExchange(&g_s65Installed,0,0)!=1 ||
        InterlockedCompareExchange(&g_s59Started,0,0)!=0 || !S67OriginalOK()){
        S67Log("ERROR: Stage67 prerequisites/signatures failed; no patches.");return false;
    }
    BYTE* a=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xAD75Cu));
    BYTE* b=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xE9E60u));
    BYTE pa[5]={0xE8},pb[6]={0xE9,0,0,0,0,0x90};
    const DWORD da=static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(&S67PromotionHook)-reinterpret_cast<std::uintptr_t>(a+5));
    const DWORD db=static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(&S67WriterHook)-reinterpret_cast<std::uintptr_t>(b+5));
    std::memcpy(pa+1,&da,4);std::memcpy(pb+1,&db,4);
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S67Log("ERROR: Stage67 mutex failed.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0 && wait!=WAIT_ABANDONED){CloseHandle(mutex);return false;}
    HANDLE peers[512]{};DWORD count=0;
    const bool stopped=S67SuspendPeers(peers,count);
    DWORD oa=0,ob=0;
    // Acquire BOTH writable ranges before making either change.
    const bool wa=stopped && VirtualProtect(a,5,PAGE_EXECUTE_READWRITE,&oa)!=0;
    const bool wb=wa && VirtualProtect(b,6,PAGE_EXECUTE_READWRITE,&ob)!=0;
    bool installed=false,protectOK=true,cacheOK=true;
    if(wb && S67OriginalOK()){
        g_s67Native=g_gameBase+0x1E9F40u;g_s67Resume=g_gameBase+0xE9E66u;
        std::memcpy(a,pa,5);std::memcpy(b,pb,6);
        cacheOK=FlushInstructionCache(GetCurrentProcess(),a,5)!=0;
        cacheOK=(FlushInstructionCache(GetCurrentProcess(),b,6)!=0)&&cacheOK;
        InterlockedExchange(&g_s67Installed,2);installed=true;
    }
    DWORD unused=0;
    if(wb && !VirtualProtect(b,6,ob,&unused))protectOK=false;
    if(wa && !VirtualProtect(a,5,oa,&unused))protectOK=false;
    ResumeThreads(peers,count);ReleaseMutex(mutex);CloseHandle(mutex);
    if(!installed){S67Log("ERROR: Stage67 transaction refused; neither site patched.");return false;}
    if(!protectOK || !cacheOK){
        S67Log("ERROR: Stage67 patched but protect/cache operation failed; capture INVALID; restart with rollback DLL.");
        return false;
    }
    S67Log("STAGE67_INSTALLED 2/2 call=AD75C native=1E9F40 writer=E9E60 resume=E9E66; original operations once; Stage43 raw outer return now DLL, correlate via Stage67.");
    return true;
}
void FlushStage67()
{
    for(DWORD i=0;i<kS67Capacity;++i){
        S67Rec r{};AcquireSRWLockExclusive(&g_s67Lock);
        const bool have=g_s67Count!=0;
        if(have){r=g_s67Ring[g_s67Read];g_s67Read=(g_s67Read+1u)%kS67Capacity;--g_s67Count;}
        ReleaseSRWLockExclusive(&g_s67Lock);if(!have)break;
        char msg[1200]{};
        sprintf_s(msg,
            "STAGE67_NATIVE_WINDOW seq=%lu kind=%lu side=P%lu player=%08lX object=%08lX tid=%lu "
            "qpcBefore=%I64u qpcAfter=%I64u callerReturn=%08lX writeValue=%08lX nativeEAX=%08lX "
            "preMask=%02lX postMask=%02lX v0=%08lX>%08lX v1=%08lX>%08lX "
            "v2=%08lX>%08lX v3=%08lX>%08lX v4=%08lX>%08lX",
            r.Seq,r.Kind,r.Side,r.P,r.O,r.Tid,r.Q0,r.Q1,r.Return,r.Value,r.Eax,
            r.PreMask,r.PostMask,r.Pre[0],r.Post[0],r.Pre[1],r.Post[1],
            r.Pre[2],r.Post[2],r.Pre[3],r.Post[3],r.Pre[4],r.Post[4]);
        S67Log(msg);
    }
    const LONG a=InterlockedExchange(&g_s67Calls[0],0),b=InterlockedExchange(&g_s67Calls[1],0);
    const LONG fa=InterlockedExchange(&g_s67Filtered[0],0),fb=InterlockedExchange(&g_s67Filtered[1],0);
    const LONG drop=InterlockedExchange(&g_s67Dropped,0),over=InterlockedExchange(&g_s67Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s67Mismatch,0),bad=InterlockedExchange(&g_s67ReadFail,0);
    if(a||b||drop||over||mismatch||bad){
        char msg[400]{};
        sprintf_s(msg,"STAGE67_HEALTH promotionCalls=%ld writerCalls=%ld otherObjects=%ld otherPlayers=%ld "
            "dropped=%ld overflow=%ld mismatch=%ld readFail=%ld ring=8192 depthPerKind=8",
            a,b,fa,fb,drop,over,mismatch,bad);S67Log(msg);
    }
}
