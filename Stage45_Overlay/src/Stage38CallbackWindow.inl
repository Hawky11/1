// Passive window around the ORIGINAL guarded indirect call at EE7BE.
// Included inside AIQueueTrace.cpp anonymous namespace, after Stage35/36.
constexpr DWORD kStage38Capacity = 4096u;
constexpr DWORD kStage38MaxDepth = 16u;
constexpr DWORD kStage38Fields = 30u;
volatile LONG g_stage38Installed = 0, g_stage38Dropped = 0;
volatile LONG g_stage38Overflow = 0, g_stage38Mismatch = 0;
DWORD g_stage38PrepareTarget = 0, g_stage38FinishTarget = 0;
// Stage39 data is captured only for the three real Stage38 callback RVA targets.
#include "Stage39ActionOutput.inl"
#include "Stage40RequestConsumer.inl"
#include "Stage42ActivationHook.inl"
#include "Stage44ActionDefinition.inl"
#include "Stage45DefinitionResolver.inl"
struct Stage38Record {
    DWORD Sequence, Player, Side, Thread, Frame, Context, Callback, CallbackAfter;
    DWORD PreMask, PostMask, Pre[kStage38Fields], Post[kStage38Fields];
    Stage39State Stage39;
    ULONGLONG QpcBefore, QpcAfter;
};
__declspec(thread) DWORD g_stage38PendingDepth = 0;
__declspec(thread) Stage38Record g_stage38Pending[kStage38MaxDepth] = {};
SRWLOCK g_stage38Lock = SRWLOCK_INIT;
Stage38Record g_stage38Ring[kStage38Capacity]{};
DWORD g_stage38Read = 0, g_stage38Write = 0, g_stage38Count = 0, g_stage38Sequence = 0;

void AppendStage38Log(const char* message)
{
    wchar_t path[MAX_PATH + 1]{};
    if (!GetSiblingPath(path, MAX_PATH, L"ArcanaAI_Stage38CallbackWindowTrace.log")) return;
    SYSTEMTIME now{}; GetLocalTime(&now);
    char line[3500]{};
    const int length = sprintf_s(line, "[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
        now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, message);
    if (length <= 0) return;
    HANDLE file = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return;
    DWORD written = 0; WriteFile(file, line, static_cast<DWORD>(length), &written, nullptr);
    CloseHandle(file);
}
void Stage38Snapshot(DWORD player, DWORD* values, DWORD& mask)
{
    const DWORD offsets[kStage38Fields] = {
        0x4u,0x3A34u,0x3F64u,0x3ED0u,0x3ED4u,0x3ED8u,0x3EDCu,0x3EE0u,
        0x3B70u,0x3B74u,0x3B78u,0x3B7Cu,0x3B80u,0x3B84u,0x3B88u,
        0x08u,0x0Cu,0x10u,0x60u,0x64u,0x68u,0x6Cu,0x70u,0x78u,
        0x80u,0x88u,0x8Cu,0x184u,0x278u,0x27Cu
    };
    DWORD object = 0;
    for (DWORD i=0; i<kStage38Fields; ++i) {
        if (i>=15u && object==0) continue;
        if (SafeReadDword((i<15u ? player : object)+offsets[i], &values[i])) {
            mask |= 1u<<i;
            if (i==0) object=values[i];
        }
    }
}
// PUSHFD/PUSHAD: EDI[0], ESI[1], EBP[2], return[9], args[10..].
// Called AFTER native 156C00; no snapshot includes its writes.
void __cdecl Stage38Prepared(const DWORD* saved)
{
    const DWORD player=saved[1];
    if (player!=g_playerP1 && player!=g_playerP2) return;
    const DWORD depth=g_stage38PendingDepth++;
    if (depth>=kStage38MaxDepth) { InterlockedIncrement(&g_stage38Overflow); return; }
    Stage38Record& r=g_stage38Pending[depth]; r=Stage38Record{};
    r.Player=player; r.Side=player==g_playerP1?1u:2u;
    r.Thread=GetCurrentThreadId(); r.Frame=saved[2]; r.Context=saved[10]; r.Callback=saved[0];
    DWORD contextPlayer=0;
    if (saved[11]!=player || r.Context!=r.Frame-0x38u ||
        !SafeReadDword(r.Context,&contextPlayer) || contextPlayer!=player) {
        r.Player=0; InterlockedIncrement(&g_stage38Mismatch); return;
    }
    Stage38Snapshot(player,r.Pre,r.PreMask);
    if (Stage39TargetCallback(r.Callback)) Stage39Snapshot(player,r.Stage39.Pre,r.Stage39.PreMask);
    r.QpcBefore=Stage31Qpc();
}
// Called BEFORE native FC60; post snapshot excludes its possible queue writes.
void __cdecl Stage38Finished(const DWORD* saved)
{
    const DWORD player=saved[1];
    if (player!=g_playerP1 && player!=g_playerP2) return;
    const ULONGLONG after=Stage31Qpc();
    if (g_stage38PendingDepth==0) { InterlockedIncrement(&g_stage38Mismatch); return; }
    const DWORD depth=--g_stage38PendingDepth;
    if (depth>=kStage38MaxDepth) return;
    Stage38Record& r=g_stage38Pending[depth];
    if (r.Player==0) return;
    DWORD contextPlayer=0;
    if (r.Player!=player || r.Frame!=saved[2] || r.Context!=saved[10] ||
        !SafeReadDword(r.Context,&contextPlayer) || contextPlayer!=player) {
        InterlockedIncrement(&g_stage38Mismatch); r.Player=0; return;
    }
    r.QpcAfter=after; r.CallbackAfter=saved[0];
    Stage38Snapshot(player,r.Post,r.PostMask);
    if (Stage39TargetCallback(r.Callback)) Stage39Snapshot(player,r.Stage39.Post,r.Stage39.PostMask);
    if (r.CallbackAfter!=r.Callback) InterlockedIncrement(&g_stage38Mismatch);
    AcquireSRWLockExclusive(&g_stage38Lock);
    r.Sequence=++g_stage38Sequence;
    if (g_stage38Count==kStage38Capacity) InterlockedIncrement(&g_stage38Dropped);
    else {
        g_stage38Ring[g_stage38Write]=r;
        g_stage38Write=(g_stage38Write+1u)%kStage38Capacity; ++g_stage38Count;
    }
    ReleaseSRWLockExclusive(&g_stage38Lock);
    r.Player=0;
}
__declspec(naked) void Stage38PrepareHook()
{
    __asm {
        // Native cdecl(context,player), exactly once. LEA preserves its flags.
        push dword ptr [esp+8]
        push dword ptr [esp+8]
        call dword ptr [g_stage38PrepareTarget]
        lea esp, [esp+8]
        pushfd
        pushad
        mov ebx, esp
        sub esp, 210h
        lea edi, [esp+0Fh]
        and edi, 0FFFFFFF0h
        fxsave [edi]
        fninit
        cld
        push ebx
        call Stage38Prepared
        add esp, 4
        fxrstor [edi]
        mov esp, ebx
        popad
        popfd
        ret
    }
}
__declspec(naked) void Stage38FinishHook()
{
    __asm {
        pushfd
        pushad
        mov ebx, esp
        sub esp, 210h
        lea edi, [esp+0Fh]
        and edi, 0FFFFFFF0h
        fxsave [edi]
        fninit
        cld
        push ebx
        call Stage38Finished
        add esp, 4
        fxrstor [edi]
        mov esp, ebx
        popad
        popfd
        // Tail jump: FC60 receives the exact original argument and return stack.
        jmp dword ptr [g_stage38FinishTarget]
    }
}

bool InstallStage38Hooks()
{
    if (!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)) {
        AppendStage38Log("ERROR: Stage38 FXSAVE unsupported; no patches."); return false;
    }
    BYTE* corridor=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xEE7A9u));
    BYTE* prepare=corridor+5u; // EE7AE
    BYTE* finish=corridor+0x1Eu; // EE7C7
    BYTE* prepareNative=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0x156C00u));
    BYTE* finishNative=reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase+0xFC60u));
    const BYTE expected[] = {0x8D,0x4D,0xC8,0x56,0x51,0xE8,0x4D,0x84,0x06,0x00,0x83,0xC4,0x08,0x85,0xFF,0x74,0x09,0x8D,0x55,0xC8,0x52,0xFF,0xD7,0x83,0xC4,0x04,0x8D,0x45,0xC8,0x50,0xE8,0x94,0x14,0xF2,0xFF,0x83,0xC4,0x04,0x8B,0xCE,0xE8,0xAA,0xFC,0xFF,0xFF};
    const BYTE preparePrologue[] = {0x55,0x8B,0xEC,0x8B,0x45,0x0C,0x8B,0x4D,0x08,0x89,0x01,0x8B,0x90,0xD4,0x3E,0x00,0x00,0x89,0x51,0x0C,0x8B,0x10,0x89,0x51,0x08,0x8B,0x40,0x04,0x89,0x41,0x04,0x5D,0xC3};
    const BYTE finishPrologue[] = {0x55,0x8B,0xEC,0x8B,0x45,0x08,0x56,0x8B,0x30};
    auto match=[&]() -> bool {
        if (std::memcmp(corridor,expected,sizeof(expected))!=0 ||
            std::memcmp(prepareNative,preparePrologue,sizeof(preparePrologue))!=0 ||
            std::memcmp(finishNative,finishPrologue,sizeof(finishPrologue))!=0) return false;
        std::int32_t dp=0,df=0;
        std::memcpy(&dp,prepare+1,4); std::memcpy(&df,finish+1,4);
        return prepare+5+dp==prepareNative && finish+5+df==finishNative;
    };
    if (!match()) { AppendStage38Log("ERROR: Stage38 corridor/helper signatures mismatch; no patches."); return false; }
    BYTE pp[5]{},pf[5]{};
    if (!MakeRelativeCall(prepare,reinterpret_cast<const void*>(&Stage38PrepareHook),pp) ||
        !MakeRelativeCall(finish,reinterpret_cast<const void*>(&Stage38FinishHook),pf)) {
        AppendStage38Log("ERROR: Stage38 rel32 range; no patches."); return false;
    }
    HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
    if (mutex==nullptr) return false;
    const DWORD wait=WaitForSingleObject(mutex,INFINITE);
    if (wait!=WAIT_OBJECT_0 && wait!=WAIT_ABANDONED) { CloseHandle(mutex); return false; }
    HANDLE suspended[512]{};
    const DWORD stopped=SuspendOtherProcessThreads(suspended,512);
    DWORD oldPrepare=0,oldFinish=0;
    const bool vp=VirtualProtect(prepare,5,PAGE_EXECUTE_READWRITE,&oldPrepare)!=0;
    const bool vf=vp && VirtualProtect(finish,5,PAGE_EXECUTE_READWRITE,&oldFinish)!=0;
    bool installed=false;
    if (vp && vf && match()) {
        g_stage38PrepareTarget=g_gameBase+0x156C00u; g_stage38FinishTarget=g_gameBase+0xFC60u;
        std::memcpy(prepare,pp,5); std::memcpy(finish,pf,5);
        FlushInstructionCache(GetCurrentProcess(),prepare,5);
        FlushInstructionCache(GetCurrentProcess(),finish,5);
        InterlockedExchange(&g_stage38Installed,1); installed=true;
    }
    if (vf) { DWORD ignored=0; VirtualProtect(finish,5,oldFinish,&ignored); }
    if (vp) { DWORD ignored=0; VirtualProtect(prepare,5,oldPrepare,&ignored); }
    ResumeThreads(suspended,stopped); ReleaseMutex(mutex); CloseHandle(mutex);
    AppendStage38Log(installed ? "STAGE38_INSTALLED 2/2 EE7AE->156C00 after, EE7C7->FC60 before; original EE7BE CALL EDI untouched."
                              : "ERROR: Stage38 install failed; no patches.");
    AppendStage39Log("STAGE39_ACTION_OUTPUT_BUILD=2026-11-10-r1");
    AppendStage39Log(installed ? "STAGE39_ENABLED piggyback=Stage38 2/2 targetRVA=(12EB10,123AF0,1DA490) no additional hooks or gameplay writes."
                               : "ERROR: Stage39 depends on Stage38; Stage38 hook install failed.");
    if(installed){
        const bool consumerInstalled=InstallStage40Hooks();
        if(consumerInstalled){
            const bool activationInstalled=InstallStage42Hooks();
            if(activationInstalled){
                if(InstallStage44Hooks())InstallStage45Hooks();
            }
            else S44Log("ERROR: Stage44 requires installed Stage42/43; no Stage44 patches.");
        }
        else {
            S42Log("ERROR: Stage42 requires installed Stage40/41; no Stage42 patches.");
            S44Log("ERROR: Stage44 requires installed Stage40/41/42; no Stage44 patches.");
        }
    }else{
        S40Log("ERROR: Stage40 requires installed Stage38; no Stage40 patches.");
        S42Log("ERROR: Stage42 requires installed Stage38; no Stage42 patches.");
        S44Log("ERROR: Stage44 requires installed Stage38; no Stage44 patches.");
    }
    return installed;
}
void FlushStage38()
{
    for (DWORD n=0;n<kStage38Capacity;++n) {
        Stage38Record r{};
        AcquireSRWLockExclusive(&g_stage38Lock);
        const bool available=g_stage38Count!=0;
        if (available) { r=g_stage38Ring[g_stage38Read]; g_stage38Read=(g_stage38Read+1u)%kStage38Capacity; --g_stage38Count; }
        ReleaseSRWLockExclusive(&g_stage38Lock);
        if (!available) break;
        DWORD changed=0;
        for (DWORD i=0;i<kStage38Fields;++i)
            if ((r.PreMask&r.PostMask&(1u<<i)) && r.Pre[i]!=r.Post[i]) changed|=1u<<i;
        char line[3100]{};
        int count=sprintf_s(line,
            "STAGE38_CALLBACK_WINDOW seq=%lu side=P%lu player=0x%08lX tid=%lu frame=0x%08lX context=0x%08lX callback=0x%08lX callbackAfter=0x%08lX guardedCall=%lu qpcBefore=%I64u qpcAfter=%I64u preMask=0x%08lX postMask=0x%08lX changedMask=0x%08lX",
            static_cast<unsigned long>(r.Sequence),static_cast<unsigned long>(r.Side),
            static_cast<unsigned long>(r.Player),static_cast<unsigned long>(r.Thread),
            static_cast<unsigned long>(r.Frame),static_cast<unsigned long>(r.Context),
            static_cast<unsigned long>(r.Callback),static_cast<unsigned long>(r.CallbackAfter),
            r.Callback!=0?1ul:0ul,static_cast<unsigned long long>(r.QpcBefore),static_cast<unsigned long long>(r.QpcAfter),
            static_cast<unsigned long>(r.PreMask),static_cast<unsigned long>(r.PostMask),static_cast<unsigned long>(changed));
        if (count<=0) continue;
        for (DWORD i=0;i<kStage38Fields;++i) {
            const int added=sprintf_s(line+count,sizeof(line)-static_cast<size_t>(count)," f%lu=%08lX>%08lX",
                static_cast<unsigned long>(i),static_cast<unsigned long>(r.Pre[i]),static_cast<unsigned long>(r.Post[i]));
            if (added<=0) break; count+=added;
        }
        AppendStage38Log(line);
        if (Stage39TargetCallback(r.Callback))
            Stage39LogWindow(r.Sequence,r.Side,r.Player,r.Thread,r.Callback,r.CallbackAfter,
                r.Pre[1],r.Pre[0],r.Pre[28],r.QpcBefore,r.QpcAfter,r.Stage39);
    }
    FlushStage40();
    FlushStage42();
    FlushStage44();
    FlushStage45();
    const LONG dropped=InterlockedExchange(&g_stage38Dropped,0);
    const LONG overflow=InterlockedExchange(&g_stage38Overflow,0);
    const LONG mismatch=InterlockedExchange(&g_stage38Mismatch,0);
    if (dropped!=0 || overflow!=0 || mismatch!=0) {
        char warning[200]{};
        sprintf_s(warning,"WARNING: Stage38 dropped=%ld pendingOverflow=%ld pairMismatch=%ld ring=4096 depth=16.",dropped,overflow,mismatch);
        AppendStage38Log(warning);
    }
}

