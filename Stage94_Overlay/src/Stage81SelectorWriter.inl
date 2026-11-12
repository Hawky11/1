// ArcanaAI Stage81 - original in-function selector writers (read-only observation).
// Included once at GLOBAL scope from src/Proxy.cpp (not inside another namespace).
// Windows/MSVC x86 ONLY. No action/movement changes. Each original MOV executes once.
// Source-only overlay intended for an existing, working Stage80 local source tree.
#ifndef _M_IX86
#error Stage81 requires the original x86 AALib.exe and MSVC x86 inline assembly
#endif
#ifndef _WIN32
#error Stage81 is for Windows only
#endif

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <climits>

namespace arcana_stage82 { bool Install(); void Flush(); void Health(); }
namespace arcana_stage84 { bool Install(); void Flush(); void Health(); }
namespace arcana_stage85 { bool Install(); void Flush(); void Health(); }
namespace arcana_stage86 { bool Install(); void Flush(); void Health(); }
namespace arcana_stage87 { bool Install(); void Flush(); void Health(); }
namespace arcana_stage88 { void Start(); void Flush(); void Health(); }
namespace arcana_stage89 { bool Install(); void Flush(); void Health(); }
namespace arcana_stage90 { void Pre(const DWORD*); void Post(const DWORD*); void Start(); void Flush(); void Health(); }
namespace arcana_stage91 { bool Install(); void Flush(); void Health(); }
namespace arcana_stage92 { bool Install(); void Flush(); void Health(); }
namespace arcana_stage93 { bool Install(); void Flush(); void Health(); }
namespace arcana_stage94 { bool Install(); void Flush(); void Health(); }

namespace arcana_stage81
{
    constexpr DWORD kImageSize = 0x0E0EB000u;
    constexpr DWORD kTimeStamp = 0x5FE5AA91u;
    constexpr DWORD kP1ControlRva = 0x0DCA5EA4u;
    constexpr DWORD kP2ControlRva = 0x0DCD0BCCu;
    constexpr DWORD kControlOffset = 0x3F2Cu;
    constexpr DWORD kSelectorOffset = 0x3F50u;
    constexpr DWORD kCapacity = 8192u;
    constexpr DWORD kPendingDepth = 16u;
    constexpr DWORD kRvas[3] = { 0x0F7D22u, 0x0F7D38u, 0x0F7D43u };
    constexpr DWORD kLengths[3] = { 6u, 6u, 10u };
    const BYTE kSignatures[3][10] = {
        {0x89,0x97,0x50,0x3F,0x00,0x00,0,0,0,0},
        {0x89,0x8F,0x50,0x3F,0x00,0x00,0,0,0,0},
        {0xC7,0x87,0x50,0x3F,0x00,0x00,0x00,0x00,0x00,0x00}
    };

    struct Pending {
        DWORD writer, player, proposed, previous, ebx, esi, eax;
        DWORD side, branch1A, branch1C, thread;
        LARGE_INTEGER qpc;
    };
    struct Event {
        Pending pre;
        DWORD post, seq;
        LARGE_INTEGER qpcAfter;
    };

    HMODULE g_proxy = nullptr;
    DWORD g_gameBase = 0, g_p1 = 0, g_p2 = 0;
    INIT_ONCE g_once = INIT_ONCE_STATIC_INIT;
    SRWLOCK g_lock = SRWLOCK_INIT;
    Event g_events[kCapacity]{};
    DWORD g_head = 0, g_tail = 0, g_count = 0;
    volatile LONG g_sequence = 0;
    volatile LONG g_calls[3]{};
    volatile LONG g_filtered = 0, g_dropped = 0, g_mismatch = 0, g_pendingOverflow = 0;
    volatile LONG g_readFail = 0, g_installed = 0;
    __declspec(thread) Pending g_pending[kPendingDepth]{};
    __declspec(thread) DWORD g_depth = 0;

    bool GetPath(wchar_t* output, size_t capacity, const wchar_t* filename) {
        if (!output || capacity == 0 || !g_proxy) return false;
        const DWORD n = GetModuleFileNameW(g_proxy, output, static_cast<DWORD>(capacity));
        if (n == 0 || n >= capacity) return false;
        wchar_t* p = wcsrchr(output, L'\\');
        if (!p) return false;
        *(p+1) = L'\0';
        return wcscat_s(output, capacity, filename) == 0;
    }
    void Log(const char* msg) {
        wchar_t filename[MAX_PATH + 1]{};
        if (!GetPath(filename, _countof(filename), L"ArcanaAI_Stage81SelectorWriterTrace.log")) return;
        SYSTEMTIME t{};
        GetLocalTime(&t);
        char line[1400]{};
        const int len = sprintf_s(line, "[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
            t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,msg);
        if (len <= 0) return;
        HANDLE f = CreateFileW(filename, FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (f == INVALID_HANDLE_VALUE) return;
        DWORD written = 0;
        WriteFile(f, line, static_cast<DWORD>(len), &written, nullptr);
        CloseHandle(f);
    }
    DWORD Side(DWORD player) {
        return player == g_p1 ? 1u : (player == g_p2 ? 2u : 0u);
    }
    // The branch EBX is inspected ONLY for logging, using SEH to avoid crashes.
    DWORD ReadBranchByte(DWORD address) {
        if (!address) return 0xFFFFFFFFu;
        __try { return *reinterpret_cast<volatile BYTE*>(static_cast<uintptr_t>(address)); }
        __except (EXCEPTION_EXECUTE_HANDLER) { InterlockedIncrement(&g_readFail); return 0xFFFFFFFFu; }
    }
    DWORD ReadSelector(DWORD player) {
        __try { return *reinterpret_cast<volatile DWORD*>(static_cast<uintptr_t>(player+kSelectorOffset)); }
        __except (EXCEPTION_EXECUTE_HANDLER) { InterlockedIncrement(&g_readFail); return 0xFFFFFFFFu; }
    }
    void Before(DWORD writer, DWORD player, DWORD proposed, DWORD ebx, DWORD esi, DWORD eax) {
        if (writer > 2) return;
        InterlockedIncrement(&g_calls[writer]);
        if (g_depth >= kPendingDepth) { InterlockedIncrement(&g_pendingOverflow); return; }
        Pending& p = g_pending[g_depth++];
        p.writer = writer; p.player = player; p.proposed = proposed;
        p.ebx = ebx; p.esi = esi; p.eax = eax;
        p.side = Side(player); p.thread = GetCurrentThreadId();
        p.previous = ReadSelector(player);
        p.branch1A = ReadBranchByte(ebx+0x1Au);
        p.branch1C = ReadBranchByte(ebx+0x1Cu);
        QueryPerformanceCounter(&p.qpc);
    }
    void After(DWORD writer, DWORD player) {
        if (writer > 2) return;
        if (!g_depth) { InterlockedIncrement(&g_mismatch); return; }
        const Pending p = g_pending[--g_depth];
        if (p.writer != writer || p.player != player) {
            InterlockedIncrement(&g_mismatch);
            return;
        }
        if (!p.side) { InterlockedIncrement(&g_filtered); return; }
        Event e{};
        e.pre = p; e.post = ReadSelector(player);
        QueryPerformanceCounter(&e.qpcAfter);
        e.seq = static_cast<DWORD>(InterlockedIncrement(&g_sequence));
        AcquireSRWLockExclusive(&g_lock);
        if (g_count < kCapacity) {
            g_events[g_tail] = e;
            g_tail = (g_tail + 1u) % kCapacity;
            ++g_count;
        } else InterlockedIncrement(&g_dropped);
        ReleaseSRWLockExclusive(&g_lock);
    }
    void Flush() {
        // Drain without holding a lock across file I/O.
        Event batch[128]{};
        for (;;) {
            DWORD n = 0;
            AcquireSRWLockExclusive(&g_lock);
            while (n < _countof(batch) && g_count) {
                batch[n++] = g_events[g_head];
                g_head = (g_head + 1u) % kCapacity;
                --g_count;
            }
            ReleaseSRWLockExclusive(&g_lock);
            if (!n) break;
            for (DWORD i=0; i<n; ++i) {
                const Event& e = batch[i];
                char line[950]{};
                sprintf_s(line,
                    "STAGE81_NATIVE_WRITE seq=%lu writerRVA=%06lX side=P%lu player=%08lX tid=%lu "
                    "qpcBefore=%I64u qpcAfter=%I64u previous=%08lX proposed=%08lX post=%08lX changed=%lu "
                    "branchEBX=%08lX b1A=%02lX b1C=%02lX esi=%08lX eax=%08lX",
                    static_cast<unsigned long>(e.seq),static_cast<unsigned long>(kRvas[e.pre.writer]),
                    static_cast<unsigned long>(e.pre.side),static_cast<unsigned long>(e.pre.player),
                    static_cast<unsigned long>(e.pre.thread),
                    static_cast<unsigned long long>(e.pre.qpc.QuadPart),
                    static_cast<unsigned long long>(e.qpcAfter.QuadPart),
                    static_cast<unsigned long>(e.pre.previous),static_cast<unsigned long>(e.pre.proposed),
                    static_cast<unsigned long>(e.post),static_cast<unsigned long>(e.pre.previous != e.post),
                    static_cast<unsigned long>(e.pre.ebx),static_cast<unsigned long>(e.pre.branch1A),
                    static_cast<unsigned long>(e.pre.branch1C),static_cast<unsigned long>(e.pre.esi),
                    static_cast<unsigned long>(e.pre.eax));
                Log(line);
            }
        }
    }
    bool Fingerprint(HMODULE m) {
        if (!m) return false;
        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(m);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew < 0x40 || dos->e_lfanew > 0x1000)
            return false;
        const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS32*>(
            reinterpret_cast<const BYTE*>(m) + dos->e_lfanew);
        return nt->Signature == IMAGE_NT_SIGNATURE &&
            nt->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 &&
            nt->FileHeader.TimeDateStamp == kTimeStamp &&
            nt->OptionalHeader.SizeOfImage == kImageSize;
    }
    // Called with the other process threads suspended. Only executable code
    // bytes are patched; data writes remain the exact three original MOVs.
    bool PatchBytes(BYTE* site, const BYTE* bytes, DWORD size) {
        DWORD oldProtection = 0;
        if (!VirtualProtect(site, size, PAGE_EXECUTE_READWRITE, &oldProtection)) return false;
        std::memcpy(site, bytes, size);
        FlushInstructionCache(GetCurrentProcess(), site, size);
        DWORD ignored = 0;
        return VirtualProtect(site, size, oldProtection, &ignored) != FALSE;
    }
    struct Suspended { HANDLE handles[512]; DWORD count; };
    void Resume(Suspended& s) {
        while (s.count) {
            HANDLE h = s.handles[--s.count];
            ResumeThread(h);
            CloseHandle(h);
        }
    }
    bool SuspendAndCheck(Suspended& s) {
        s.count = 0;
        const DWORD pid = GetCurrentProcessId(), self = GetCurrentThreadId();
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snap == INVALID_HANDLE_VALUE) return false;
        THREADENTRY32 t{}; t.dwSize = sizeof(t);
        bool ok = true;
        if (!Thread32First(snap, &t)) ok = false;
        while (ok) {
            if (t.th32OwnerProcessID == pid && t.th32ThreadID != self) {
                if (s.count >= _countof(s.handles)) { ok = false; break; }
                HANDLE h = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, FALSE, t.th32ThreadID);
                if (!h) { ok = false; break; }
                if (SuspendThread(h) == static_cast<DWORD>(-1)) { CloseHandle(h); ok = false; break; }
                s.handles[s.count++] = h;
                CONTEXT c{}; c.ContextFlags = CONTEXT_CONTROL;
                if (!GetThreadContext(h, &c)) { ok = false; break; }
                for (DWORD i=0;i<3;++i) {
                    const DWORD start = g_gameBase + kRvas[i];
                    if (c.Eip >= start && c.Eip < start + kLengths[i]) {
                        ok = false; break;
                    }
                }
                if (!ok) break;
            }
            t.dwSize = sizeof(t);
            if (!Thread32Next(snap, &t)) break;
        }
        CloseHandle(snap);
        if (!ok) Resume(s);
        return ok;
    }
}

extern "C" {
    volatile DWORD g_Stage81Resume0 = 0, g_Stage81Resume1 = 0, g_Stage81Resume2 = 0;
    void __cdecl Stage81BeforeWriter(DWORD writer, DWORD player, DWORD proposed, DWORD ebx, DWORD esi, DWORD eax) {
        arcana_stage81::Before(writer, player, proposed, ebx, esi, eax);
    }
    void __cdecl Stage81AfterWriter(DWORD writer, DWORD player) {
        arcana_stage81::After(writer, player);
    }
    // FXSAVE/FXRSTOR saves all x87/MMX/XMM0-7 plus MXCSR around C++ logging.
    // EBX temporarily holds the saved PUSHAD frame; it is cdecl callee-saved.
    __declspec(naked) void Stage81WriteEdx() {
        __asm {
            pushfd
            pushad
            mov ebx, esp
            sub esp, 528
            and esp, 0FFFFFFF0h
            fxsave [esp]
            push dword ptr [ebx+28]  // original EAX
            push dword ptr [ebx+4]   // original ESI
            push dword ptr [ebx+16]  // original EBX
            push dword ptr [ebx+20]  // proposed new value
            push dword ptr [ebx]     // original EDI = player
            push 0
            call Stage81BeforeWriter
            add esp, 24
            fxrstor [esp]
            mov esp, ebx
            popad
            popfd
            mov dword ptr [edi+3F50h], edx
            pushfd
            pushad
            mov ebx, esp
            sub esp, 528
            and esp, 0FFFFFFF0h
            fxsave [esp]
            push dword ptr [ebx]
            push 0
            call Stage81AfterWriter
            add esp, 8
            fxrstor [esp]
            mov esp, ebx
            popad
            popfd
            jmp dword ptr [g_Stage81Resume0]
        }
    }
    // FXSAVE/FXRSTOR saves all x87/MMX/XMM0-7 plus MXCSR around C++ logging.
    // EBX temporarily holds the saved PUSHAD frame; it is cdecl callee-saved.
    __declspec(naked) void Stage81WriteEcx() {
        __asm {
            pushfd
            pushad
            mov ebx, esp
            sub esp, 528
            and esp, 0FFFFFFF0h
            fxsave [esp]
            push dword ptr [ebx+28]  // original EAX
            push dword ptr [ebx+4]   // original ESI
            push dword ptr [ebx+16]  // original EBX
            push dword ptr [ebx+24]  // proposed new value
            push dword ptr [ebx]     // original EDI = player
            push 1
            call Stage81BeforeWriter
            add esp, 24
            fxrstor [esp]
            mov esp, ebx
            popad
            popfd
            mov dword ptr [edi+3F50h], ecx
            pushfd
            pushad
            mov ebx, esp
            sub esp, 528
            and esp, 0FFFFFFF0h
            fxsave [esp]
            push dword ptr [ebx]
            push 1
            call Stage81AfterWriter
            add esp, 8
            fxrstor [esp]
            mov esp, ebx
            popad
            popfd
            jmp dword ptr [g_Stage81Resume1]
        }
    }
    // FXSAVE/FXRSTOR saves all x87/MMX/XMM0-7 plus MXCSR around C++ logging.
    // EBX temporarily holds the saved PUSHAD frame; it is cdecl callee-saved.
    __declspec(naked) void Stage81WriteZero() {
        __asm {
            pushfd
            pushad
            mov ebx, esp
            sub esp, 528
            and esp, 0FFFFFFF0h
            fxsave [esp]
            push dword ptr [ebx+28]  // original EAX
            push dword ptr [ebx+4]   // original ESI
            push dword ptr [ebx+16]  // original EBX
            push 0  // proposed new value
            push dword ptr [ebx]     // original EDI = player
            push 2
            call Stage81BeforeWriter
            add esp, 24
            fxrstor [esp]
            mov esp, ebx
            popad
            popfd
            mov dword ptr [edi+3F50h], 0
            pushfd
            pushad
            mov ebx, esp
            sub esp, 528
            and esp, 0FFFFFFF0h
            fxsave [esp]
            push dword ptr [ebx]
            push 2
            call Stage81AfterWriter
            add esp, 8
            fxrstor [esp]
            mov esp, ebx
            popad
            popfd
            jmp dword ptr [g_Stage81Resume2]
        }
    }
}

namespace arcana_stage81
{
    bool Install() {
        HMODULE game = GetModuleHandleW(L"AALib.exe");
        if (!Fingerprint(game)) { Log("STAGE81_ERROR fingerprint mismatch; refusing all patches"); return false; }
        g_gameBase = static_cast<DWORD>(reinterpret_cast<uintptr_t>(game));
        g_p1 = g_gameBase + kP1ControlRva - kControlOffset;
        g_p2 = g_gameBase + kP2ControlRva - kControlOffset;
        BYTE* sites[3]{};
        const void* stubs[3] = { reinterpret_cast<const void*>(&Stage81WriteEdx),
            reinterpret_cast<const void*>(&Stage81WriteEcx), reinterpret_cast<const void*>(&Stage81WriteZero) };
        BYTE patches[3][10]{};
        for (DWORD i=0;i<3;++i) {
            sites[i] = reinterpret_cast<BYTE*>(static_cast<uintptr_t>(g_gameBase+kRvas[i]));
            if (std::memcmp(sites[i],kSignatures[i],kLengths[i])!=0) {
                char msg[150]{};
                sprintf_s(msg,"STAGE81_ERROR signature mismatch at RVA=%06lX; refusing all patches",kRvas[i]);
                Log(msg); return false;
            }
            const std::int64_t distance = static_cast<std::int64_t>(reinterpret_cast<uintptr_t>(stubs[i]))
                - static_cast<std::int64_t>(reinterpret_cast<uintptr_t>(sites[i]+5));
            if (distance < INT32_MIN || distance > INT32_MAX) {
                Log("STAGE81_ERROR stub out of rel32 range; refusing all patches"); return false;
            }
            std::memset(patches[i],0x90,sizeof(patches[i]));
            patches[i][0] = 0xE9;
            const std::int32_t rel = static_cast<std::int32_t>(distance);
            std::memcpy(patches[i]+1,&rel,sizeof(rel));
        }
        g_Stage81Resume0 = g_gameBase+kRvas[0]+kLengths[0];
        g_Stage81Resume1 = g_gameBase+kRvas[1]+kLengths[1];
        g_Stage81Resume2 = g_gameBase+kRvas[2]+kLengths[2];
        HANDLE mutex = CreateMutexW(nullptr,FALSE,L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
        if (!mutex) { Log("STAGE81_ERROR patch mutex creation failed"); return false; }
        const DWORD acquired = WaitForSingleObject(mutex,10000);
        if (acquired!=WAIT_OBJECT_0 && acquired!=WAIT_ABANDONED) {
            CloseHandle(mutex); Log("STAGE81_ERROR patch mutex wait failed"); return false;
        }
        Suspended threads{};
        bool good = SuspendAndCheck(threads);
        if (!good) {
            Log("STAGE81_ERROR cannot safely suspend/check all threads; no patches installed");
        } else {
            // Revalidate under suspension; other probes may have changed the code.
            for (DWORD i=0;i<3;++i)
                if (std::memcmp(sites[i],kSignatures[i],kLengths[i])!=0) good=false;
            DWORD installed=0;
            if (good) {
                for (DWORD i=0;i<3;++i) {
                    if (!PatchBytes(sites[i],patches[i],kLengths[i])) { good=false; break; }
                    ++installed;
                }
            }
            if (!good) {
                // Roll back every touched site, including a partial last write.
                // No file logging while any process threads remain suspended.
                for (DWORD i=0;i<3;++i)
                    if (std::memcmp(sites[i],kSignatures[i],kLengths[i])!=0)
                        PatchBytes(sites[i],kSignatures[i],kLengths[i]);
            }
            Resume(threads);
        }
        ReleaseMutex(mutex);
        CloseHandle(mutex);
        if (!good) {
            Log("STAGE81_ERROR installation failed; rollback attempted (verify native code before retry)");
            return false;
        }
        InterlockedExchange(&g_installed,1);
        char start[330]{};
        sprintf_s(start,
          "STAGE81_INSTALLED 3/3 rvas=0F7D22,0F7D38,0F7D43 lengths=6,6,10 originalMOV=once fxsave=1 "
          "gameBase=%08lX P1=%08lX P2=%08lX DRwrites=0 extraGameDataWrites=0",
          g_gameBase,g_p1,g_p2);
        Log(start);
        return true;
    }
    void Health() {
        char msg[500]{};
        sprintf_s(msg,
            "STAGE81_HEALTH callsEdx=%ld callsEcx=%ld callsZero=%ld filtered=%ld "
            "dropped=%ld pendingOverflow=%ld mismatch=%ld readFail=%ld "
            "ring=%lu depth=%lu DRwrites=0",
            InterlockedExchange(&g_calls[0],0),InterlockedExchange(&g_calls[1],0),
            InterlockedExchange(&g_calls[2],0),InterlockedExchange(&g_filtered,0),
            InterlockedExchange(&g_dropped,0),InterlockedExchange(&g_pendingOverflow,0),
            InterlockedExchange(&g_mismatch,0),InterlockedExchange(&g_readFail,0),
            kCapacity,kPendingDepth);
        Log(msg);
    }
    // Stage80 checks the ORIGINAL bytes at all three Stage81 writer RVAs.
    // Stage81 must not replace them until Stage80 has installed its own call hook.
    // The patched E8 at E8A8E must branch into our own dinput8 proxy module.
    // We intentionally do not infer Stage80 readiness from a stale log file.
    bool WaitForStage80(HMODULE game) {
        if (!Fingerprint(game)) {
            Log("STAGE81_ERROR Stage80 wait: original AALib.exe fingerprint mismatch");
            return false;
        }
        const BYTE originalCall[5] = {0xE8,0xBD,0xE6,0x00,0x00};
        const BYTE* site = reinterpret_cast<const BYTE*>(game) + 0x0E8A8Eu;
        Log("STAGE81_WAIT_STAGE80 callRVA=0E8A8E requires patched CALL into current proxy; writers untouched");
        for (DWORD attempt=0; attempt<300u; ++attempt) {
            BYTE current[5]{};
            const volatile BYTE* observed = reinterpret_cast<const volatile BYTE*>(site);
            for (DWORD i=0; i<5u; ++i) current[i] = observed[i];
            if (std::memcmp(current, originalCall, 5u)==0) {
                Sleep(100);
                continue;
            }
            if (current[0]!=0xE8) {
                Log("STAGE81_ERROR Stage80 wait: E8A8E changed to unsupported opcode; no Stage81 patches");
                return false;
            }
            std::int32_t rel=0;
            std::memcpy(&rel,current+1,4u);
            const std::intptr_t target = reinterpret_cast<std::intptr_t>(site+5) + rel;
            HMODULE owner=nullptr;
            const DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
            const BOOL found = GetModuleHandleExW(flags,
                reinterpret_cast<LPCWSTR>(target),&owner);
            if (!found || owner!=g_proxy) {
                Log("STAGE81_ERROR Stage80 wait: E8A8E hook target not in this dinput8 proxy; no Stage81 patches");
                return false;
            }
            Log("STAGE81_PREREQ_STAGE80_OK E8A8E redirected into proxy; starting writer installation");
            return true;
        }
        Log("STAGE81_ERROR Stage80 wait timeout (30s): original E8A8E CALL unchanged; Stage81 not installed");
        return false;
    }

    DWORD WINAPI Worker(void*) {
        Log("STAGE81_BUILD=2026-11-11-r2 SOURCE_ONLY wait Stage80 then 3 exact selector writers; original MOV once");
        wchar_t ini[MAX_PATH+1]{};
        if (GetPath(ini,_countof(ini),L"ArcanaAI.ini") &&
            GetPrivateProfileIntW(L"AIResearch",L"EnableStage81",1,ini)==0) {
            Log("STAGE81_DISABLED ini EnableStage81=0");
            return 0;
        }
        HMODULE game=nullptr;
        for (DWORD i=0;i<300 && !game;++i) { game=GetModuleHandleW(L"AALib.exe"); if(!game) Sleep(100); }
        if (!game) { Log("STAGE81_ERROR AALib.exe not loaded"); return 0; }
        if (!WaitForStage80(game)) return 0;
        if (!Install()) return 0;
        // Stage84 is gated by a verified Stage82 installation: no unattached probe.
        if (arcana_stage82::Install()) {
            if (arcana_stage84::Install()) {
                if (arcana_stage85::Install()) {
                    if (arcana_stage86::Install()) { if (arcana_stage87::Install()) { arcana_stage88::Start(); arcana_stage89::Install(); arcana_stage90::Start(); if (arcana_stage91::Install()) { if (arcana_stage92::Install()) { if (arcana_stage93::Install()) arcana_stage94::Install(); } } } }
                    else Log("STAGE87_SKIPPED Stage86 not installed; Stage81/82/84/85 baseline retained");
                }
                else Log("STAGE86_SKIPPED Stage85 not installed; Stage84/85 baseline retained");
            }
            else Log("STAGE85_SKIPPED Stage84 not installed; Stage81/82 baseline retained");
        } else Log("STAGE84_SKIPPED Stage82 not installed; original Stage81/82 baseline retained");
        for (;;) {
            for (int i=0;i<8;++i) {
                Sleep(250); Flush(); arcana_stage82::Flush(); arcana_stage84::Flush(); arcana_stage85::Flush(); arcana_stage86::Flush(); arcana_stage87::Flush(); arcana_stage88::Flush(); arcana_stage89::Flush(); arcana_stage90::Flush(); arcana_stage91::Flush(); arcana_stage92::Flush(); arcana_stage93::Flush(); arcana_stage94::Flush();
            }
            Health(); arcana_stage82::Health(); arcana_stage84::Health(); arcana_stage85::Health(); arcana_stage86::Health(); arcana_stage87::Health(); arcana_stage88::Health(); arcana_stage89::Health(); arcana_stage90::Health(); arcana_stage91::Health(); arcana_stage92::Health(); arcana_stage93::Health(); arcana_stage94::Health();
        }
    }
    BOOL CALLBACK StartOnce(PINIT_ONCE,PVOID,PVOID*) {
        HANDLE thread = CreateThread(nullptr,0,Worker,nullptr,0,nullptr);
        if (!thread) Log("STAGE81_ERROR worker creation failed");
        else CloseHandle(thread);
        return TRUE;
    }
}
inline void StartStage81SelectorWriter(HMODULE proxyModule) {
    arcana_stage81::g_proxy = proxyModule;
    InitOnceExecuteOnce(&arcana_stage81::g_once,arcana_stage81::StartOnce,nullptr,nullptr);
}

#include "Stage82NestedSelector.inl"

#include "Stage84F3930Writer.inl"

#include "Stage85F7150Join.inl"

#include "Stage86E8560CallBoundary.inl"

#include "Stage87SelectorConsumer.inl"

#include "Stage88Stage24Join.inl"

#include "Stage89F58E0Reader.inl"
#include "Stage90AllF57D0.inl"

#include "Stage91DirectSpecialReader.inl"

#include "Stage92F5990BranchTrace.inl"
#include "Stage93F5A68DirectBit3.inl"
#include "Stage94Type1ProducerGate.inl"
