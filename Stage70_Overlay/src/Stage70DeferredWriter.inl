// Stage70 SOURCE ONLY: original-native deferred action store observer.
// Installed AFTER Stage67, BEFORE the Stage59 DR0/DR1 worker. No DR usage.
// Original AALib.exe RVA 156784: mov eax,[ebp+08] ; 156787: mov [eax+68],edx.
// Both original instructions are replayed ONCE, with native GPR/FLAGS/x87/SSE.
// Do not treat SOURCE ONLY as a verified game/DLL build.
constexpr DWORD kS70Capacity=4096u,kS70DepthLimit=8u;
struct S70Rec {
    DWORD Seq=0,P=0,O=0,Side=0,Tid=0,SourceEdx=0,PreEax=0,PostEax=0,PostEdx=0;
    DWORD OldDeferred=0,NewDeferred=0,SelectorBefore=0,SelectorAfter=0;
    DWORD ObjectBefore=0,ObjectAfter=0,PreMask=0,PostMask=0;
    ULONGLONG Q0=0,Q1=0;
};
S70Rec g_s70Ring[kS70Capacity]{};
SRWLOCK g_s70Lock=SRWLOCK_INIT;
DWORD g_s70Read=0,g_s70Write=0,g_s70Count=0,g_s70Seq=0;
__declspec(thread) DWORD g_s70Depth=0;
__declspec(thread) S70Rec g_s70Pending[kS70DepthLimit]{};
volatile LONG g_s70Installed=0,g_s70Calls=0,g_s70Filtered=0;
volatile LONG g_s70Dropped=0,g_s70Overflow=0,g_s70Mismatch=0,g_s70ReadFail=0;
DWORD g_s70Resume=0;

void S70Log(const char* message)
{
    wchar_t path[MAX_PATH+1]{};
    if(!GetSiblingPath(path,MAX_PATH,L"ArcanaAI_Stage70DeferredWriterTrace.log"))return;
    SYSTEMTIME now{};GetLocalTime(&now);
    char line[1400]{};
    const int n=sprintf_s(line,"[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,message);
    if(n<=0)return;
    HANDLE f=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,
        nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return;
    DWORD wrote=0;WriteFile(f,line,static_cast<DWORD>(n),&wrote,nullptr);CloseHandle(f);
}
// Saved image after PUSHFD/PUSHAD: EDI[0],ESI[1],EBP[2],ESP[3],EBX[4],
// EDX[5],ECX[6],EAX[7],FLAGS[8]. Called at ORIGINAL RVA 156784.
void __cdecl S70Enter(const DWORD* saved)
{
    InterlockedIncrement(&g_s70Calls);
    const DWORD d=g_s70Depth++;
    if(d>=kS70DepthLimit){InterlockedIncrement(&g_s70Overflow);return;}
    S70Rec& r=g_s70Pending[d];r=S70Rec{};
    r.SourceEdx=saved[5];r.PreEax=saved[7];
    if(!saved[2] || !SafeReadDword(saved[2]+8u,&r.O) || !r.O){
        InterlockedIncrement(&g_s70ReadFail);return;
    }
    DWORD o1=0,o2=0;
    const bool ok1=g_playerP1 && SafeReadDword(g_playerP1+4u,&o1);
    const bool ok2=g_playerP2 && SafeReadDword(g_playerP2+4u,&o2);
    if(r.O==o1 && ok1){r.P=g_playerP1;r.Side=1u;}
    else if(r.O==o2 && ok2){r.P=g_playerP2;r.Side=2u;}
    if(!r.Side){InterlockedIncrement(&g_s70Filtered);return;}
    r.Tid=GetCurrentThreadId();
    if(SafeReadDword(r.O+0x68u,&r.OldDeferred))r.PreMask|=1u;
    else InterlockedIncrement(&g_s70ReadFail);
    if(SafeReadDword(r.P+0x3F50u,&r.SelectorBefore))r.PreMask|=2u;
    else InterlockedIncrement(&g_s70ReadFail);
    if(SafeReadDword(r.P+4u,&r.ObjectBefore))r.PreMask|=4u;
    else InterlockedIncrement(&g_s70ReadFail);
    r.Q0=Stage31Qpc();
}
void __cdecl S70Leave(const DWORD* saved)
{
    if(!g_s70Depth){InterlockedIncrement(&g_s70Mismatch);return;}
    const DWORD d=--g_s70Depth;
    if(d>=kS70DepthLimit)return;
    S70Rec& r=g_s70Pending[d];
    if(!r.Side)return;
    r.PostEax=saved[7];r.PostEdx=saved[5];r.Q1=Stage31Qpc();
    if(r.Tid!=GetCurrentThreadId() || r.PostEax!=r.O ||
       r.PostEdx!=r.SourceEdx){
        InterlockedIncrement(&g_s70Mismatch);r.Side=0;return;
    }
    if(SafeReadDword(r.O+0x68u,&r.NewDeferred))r.PostMask|=1u;
    else InterlockedIncrement(&g_s70ReadFail);
    if(SafeReadDword(r.P+0x3F50u,&r.SelectorAfter))r.PostMask|=2u;
    else InterlockedIncrement(&g_s70ReadFail);
    if(SafeReadDword(r.P+4u,&r.ObjectAfter))r.PostMask|=4u;
    else InterlockedIncrement(&g_s70ReadFail);
    if(r.PreMask==7u && r.PostMask==7u &&
       (r.ObjectBefore!=r.O || r.ObjectAfter!=r.O ||
        r.NewDeferred!=r.SourceEdx))
        InterlockedIncrement(&g_s70Mismatch);
    AcquireSRWLockExclusive(&g_s70Lock);
    r.Seq=++g_s70Seq;
    if(g_s70Count==kS70Capacity)InterlockedIncrement(&g_s70Dropped);
    else{
        g_s70Ring[g_s70Write]=r;
        g_s70Write=(g_s70Write+1u)%kS70Capacity;
        ++g_s70Count;
    }
    ReleaseSRWLockExclusive(&g_s70Lock);r.Side=0;
}

// No extra caller-visible RET frame; restores all registers/FLAGS/FPU/SSE.
// Fxsave scratch is 16-byte aligned and never leaks to original stack.
// Native MOV EAX,[EBP+8] changes EAX; MOV [EAX+68],EDX leaves FLAGS intact.
__declspec(naked) void S70DeferredWriterHook()
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
        call S70Enter
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd

        // Exactly the TWO displaced ORIGINAL instructions, in order.
        mov eax,dword ptr [ebp+8]
        mov dword ptr [eax+68h],edx

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
        call S70Leave
        add esp,4
        fxrstor [edi]
        mov esp,ebx
        popad
        popfd
        jmp dword ptr [g_s70Resume]
    }
}
bool S70OriginalOK()
{
    // Adjacent exact native opcodes: table load, target six bytes, continuation.
    const BYTE prev[]={0x8B,0x14,0x8D,0xE8,0x54,0x82,0x00};
    const BYTE site[]={0x8B,0x45,0x08,0x89,0x50,0x68};
    const BYTE next[]={0x8B,0x0F,0x6A,0xFF,0x81,0xC1,0xD0,0x3E,0x00,0x00};
    const BYTE* base=reinterpret_cast<const BYTE*>(static_cast<std::uintptr_t>(g_gameBase));
    return std::memcmp(base+0x15677Du,prev,sizeof(prev))==0 &&
           std::memcmp(base+0x156784u,site,sizeof(site))==0 &&
           std::memcmp(base+0x15678Au,next,sizeof(next))==0;
}
// Stop/inspect peers BEFORE patch. No DR context access, no debug state writes.
// Unlike Stage67's helper, range is this Stage70 site, not AD75C/E9E60.
bool S70SuspendPeers(HANDLE* peers,DWORD& count)
{
    count=0;
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    if(snap==INVALID_HANDLE_VALUE)return false;
    THREADENTRY32 e{};e.dwSize=sizeof(e);
    bool ok=Thread32First(snap,&e)!=0;
    if(ok)do{
        if(e.th32OwnerProcessID==GetCurrentProcessId() &&
           e.th32ThreadID!=GetCurrentThreadId()){
            if(count==512u){ok=false;break;}
            HANDLE t=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT,FALSE,e.th32ThreadID);
            if(!t){ok=false;break;}
            if(SuspendThread(t)==static_cast<DWORD>(-1)){
                CloseHandle(t);ok=false;break;
            }
            peers[count++]=t;
            CONTEXT c{};c.ContextFlags=CONTEXT_CONTROL;
            if(!GetThreadContext(t,&c)){ok=false;break;}
            if(c.Eip>=g_gameBase+0x156784u &&
               c.Eip<g_gameBase+0x15678Au){ok=false;break;}
        }
        e.dwSize=sizeof(e);
    }while(Thread32Next(snap,&e));
    if(ok && GetLastError()!=ERROR_NO_MORE_FILES)ok=false;
    CloseHandle(snap);return ok;
}
bool InstallStage70Hooks()
{
    S70Log("STAGE70_DEFERRED_BUILD=2026-11-11-r1 SOURCE_ONLY; no DR usage.");
    if(sizeof(void*)!=4u ||
       !IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) ||
       InterlockedCompareExchange(&g_s67Installed,0,0)!=2 ||
       InterlockedCompareExchange(&g_s65Installed,0,0)!=1 ||
       InterlockedCompareExchange(&g_s59Started,0,0)!=0 ||
       InterlockedCompareExchange(&g_s70Installed,0,0)!=0 ||
       !S70OriginalOK()){
        S70Log("ERROR: Stage70 prerequisites or signature mismatch; no patch.");
        return false;
    }
    BYTE* a=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x156784u));
    BYTE patch[6]={0xE9,0,0,0,0,0x90};
    // In a 32-bit address space rel32 is computed modulo 2^32.
    // Subtract unsigned pointer values, avoiding signed overflow.
    const DWORD rel=static_cast<DWORD>(
        reinterpret_cast<std::uintptr_t>(&S70DeferredWriterHook) -
        reinterpret_cast<std::uintptr_t>(a+5));
    std::memcpy(patch+1,&rel,4);
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if(!mutex){S70Log("ERROR: Stage70 mutex unavailable.");return false;}
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if(wait!=WAIT_OBJECT_0 && wait!=WAIT_ABANDONED){CloseHandle(mutex);return false;}
    HANDLE peers[512]{};DWORD count=0;
    const bool stopped=S70SuspendPeers(peers,count);
    DWORD oldProtect=0,unused=0;
    const bool writable=stopped && VirtualProtect(a,6,PAGE_EXECUTE_READWRITE,&oldProtect)!=0;
    bool installed=false,restore=true,flushed=true;
    if(writable && S70OriginalOK()){
        g_s70Resume=g_gameBase+0x15678Au;
        std::memcpy(a,patch,6);
        flushed=FlushInstructionCache(GetCurrentProcess(),a,6)!=0;
        InterlockedExchange(&g_s70Installed,1);installed=true;
    }
    if(writable && !VirtualProtect(a,6,oldProtect,&unused))restore=false;
    ResumeThreads(peers,count);ReleaseMutex(mutex);CloseHandle(mutex);
    if(!installed){
        S70Log("ERROR: Stage70 transaction refused; no patch.");
        return false;
    }
    if(!restore || !flushed){
        S70Log("ERROR: Stage70 patched but protection/cache failed; capture INVALID; exit and restore known-good DLL.");
        return false;
    }
    S70Log("STAGE70_INSTALLED 1/1 site=156784 store=156787 resume=15678A; source MOV executes once; DRwrites=0");
    return true;
}
void FlushStage70()
{
    for(DWORD i=0;i<kS70Capacity;++i){
        S70Rec r{};
        AcquireSRWLockExclusive(&g_s70Lock);
        const bool have=g_s70Count!=0;
        if(have){
            r=g_s70Ring[g_s70Read];
            g_s70Read=(g_s70Read+1u)%kS70Capacity;--g_s70Count;
        }
        ReleaseSRWLockExclusive(&g_s70Lock);
        if(!have)break;
        char line[900]{};
        sprintf_s(line,
            "STAGE70_DEFERRED_WRITE seq=%lu side=P%lu player=%08lX object=%08lX tid=%lu "
            "qpcBefore=%I64u qpcAfter=%I64u writerRVA=156787 "
            "valueEDX=%08lX eaxBefore=%08lX eaxAfter=%08lX edxAfter=%08lX "
            "preMask=%02lX postMask=%02lX "
            "deferred=%08lX>%08lX selector=%08lX>%08lX objectPtr=%08lX>%08lX",
            r.Seq,r.Side,r.P,r.O,r.Tid,r.Q0,r.Q1,
            r.SourceEdx,r.PreEax,r.PostEax,r.PostEdx,
            r.PreMask,r.PostMask,r.OldDeferred,r.NewDeferred,
            r.SelectorBefore,r.SelectorAfter,r.ObjectBefore,r.ObjectAfter);
        S70Log(line);
    }
    const LONG calls=InterlockedExchange(&g_s70Calls,0);
    const LONG filtered=InterlockedExchange(&g_s70Filtered,0);
    const LONG dropped=InterlockedExchange(&g_s70Dropped,0);
    const LONG over=InterlockedExchange(&g_s70Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_s70Mismatch,0);
    const LONG readFail=InterlockedExchange(&g_s70ReadFail,0);
    if(calls||filtered||dropped||over||mismatch||readFail){
        char line[450]{};
        sprintf_s(line,"STAGE70_HEALTH calls=%ld filtered=%ld dropped=%ld overflow=%ld mismatch=%ld readFail=%ld ring=4096 depth=8",
            calls,filtered,dropped,over,mismatch,readFail);
        S70Log(line);
    }
}
