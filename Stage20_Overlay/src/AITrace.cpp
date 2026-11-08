#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstddef>
#include <wchar.h>

#include "AITrace.h"
#include "ComVsCom.h"

extern "C"
{
    volatile DWORD g_Stage17Bit113SetterTarget = 0;
    void __cdecl ArcanaLogStage17Bit113SetterCall(const DWORD* savedContext);

    volatile DWORD g_Stage18Bit113GateContinuation = 0;
    void __cdecl ArcanaLogStage18Bit113GateResult(const DWORD* savedContext);

    __declspec(naked) void ArcanaStage18Bit113GateResultHook()
    {
        __asm
        {
            // Replaces add esp,0x0C; test al,al at RVA 0xC64A5.
            // The observer CALL adds one stack DWORD, so clean 0x10 total.
            // Recreate TEST flags and resume at the original JNE at 0xC64AA.
            pushfd
            pushad
            mov edx, esp
            push edx
            call ArcanaLogStage18Bit113GateResult
            add esp, 4
            popad
            popfd
            add esp, 10h
            test al, al
            jmp dword ptr [g_Stage18Bit113GateContinuation]
        }
    }

    volatile DWORD g_Stage19StateMapContinuation = 0;
    void __cdecl ArcanaLogStage19StateMap(const DWORD* savedContext);

    __declspec(naked) void ArcanaStage19StateMapHook()
    {
        __asm
        {
            // Replaces ADD ESP,8; CMP EAX,5 at RVA 0x4F6C08.
            // The hook CALL adds one return DWORD; two original cdecl args remain.
            // Remove all three DWORDs, replay CMP, and resume at the original JNE.
            pushfd
            pushad
            mov edx, esp
            push edx
            call ArcanaLogStage19StateMap
            add esp, 4
            popad
            popfd
            add esp, 0Ch
            cmp eax, 5
            jmp dword ptr [g_Stage19StateMapContinuation]
        }
    }

    volatile DWORD g_Stage20Bit113ObjectGuardTarget = 0;
    void __cdecl ArcanaLogStage20Bit113ObjectGuard(const DWORD* savedContext);

    __declspec(naked) void ArcanaStage20Bit113ObjectGuardHook()
    {
        __asm
        {
            // Replaces the original CALL 0x00495B00 at RVA 0x000C64F1.
            // At entry: [ESP]=original continuation 0x000C64F6, [ESP+4]=bit index 2.
            // Save original ECX for logging, then duplicate the original argument.
            // The helper's RET 4 removes only the duplicate. The observer context
            // therefore has saved ECX, this hook's return, and the original argument.
            // After logging, discard saved ECX and RET 4 to reproduce original cleanup.
            // AL/EAX and EFLAGS from the real helper are preserved; original TEST/JNE
            // at 0x000C64F6/0x000C64F8 remain untouched.
            push ecx
            mov edx, dword ptr [esp+8]
            push edx
            call dword ptr [g_Stage20Bit113ObjectGuardTarget]
            pushfd
            pushad
            mov edx, esp
            push edx
            call ArcanaLogStage20Bit113ObjectGuard
            add esp, 4
            popad
            popfd
            lea esp, [esp+4] // Discard saved ECX without changing helper EFLAGS.
            ret 4
        }
    }

    __declspec(naked) void ArcanaStage17Bit113SetterCallHook()
    {
        __asm
        {
            // Preserve the original caller context and the original CALL stack.
            // Tail-jump to the validated SetBit helper; its RET 4 returns to the
            // original AALib.exe+0xC6507 continuation with the same arguments.
            pushfd
            pushad
            mov edx, esp
            push edx
            call ArcanaLogStage17Bit113SetterCall
            add esp, 4
            popad
            popfd
            jmp dword ptr [g_Stage17Bit113SetterTarget]
        }
    }
}

namespace
{
    // RVAs confirmed from the user's correct COMvsCOM entry in the CE table.
    constexpr DWORD kP1ControlRva = 0x0DCA5EA4;
    constexpr DWORD kP2ControlRva = 0x0DCD0BCC;
    constexpr LONG kSiteCount = 128;
    constexpr DWORD kDefaultScanIntervalMs = 250;
    constexpr DWORD kMinimumScanIntervalMs = 50;
    constexpr DWORD kMaximumScanIntervalMs = 2000;
    constexpr DWORD kPlayerControlOffset = 0x3F2Cu;
    constexpr DWORD kPlayerBit17Offset = 0x3B84u;
    constexpr DWORD kPlayerBit17Mask = 0x00020000u;
    constexpr DWORD kPlayerBit17RecordCapacity = 4096u;
    constexpr DWORD kControlTraceDr7 = 0x00FF0005u; // DR0/DR1, RW=11 (read/write), LEN=11 (4 bytes).
    constexpr DWORD kPlayerBit17WriteTraceDr7 = 0x00DD0005u; // DR0/DR1, RW=01 (write only), LEN=11 (4 bytes).
    constexpr DWORD kStage17Bit113SetterCallRva = 0x000C6502u;
    constexpr DWORD kStage17Bit113SetterReturnRva = kStage17Bit113SetterCallRva + 5u;
    constexpr DWORD kStage17Bit113HelperRva = 0x0000FAA0u;
    constexpr DWORD kStage17Bit113Index = 0x71u;
    constexpr DWORD kStage17Bit113RecordCapacity = 512u;
    constexpr BYTE kStage17Bit113ExpectedCall[5] = { 0xE8, 0x99, 0x95, 0xF4, 0xFF };
    constexpr char kStage17BuildMarker[] = "STAGE17_CALLSITE_DIAG_BUILD=2026-11-08-r2";
    constexpr DWORD kStage18Bit113GateObserverRva = 0x000C64A5u;
    constexpr DWORD kStage18Bit113GateBranchRva = 0x000C64AAu;
    constexpr DWORD kStage18Bit113GateCallRva = 0x000C64A0u;
    constexpr DWORD kStage18Bit113GateHelperRva = 0x000F2FD0u;
    constexpr DWORD kStage18Bit113GateContinuationRva = 0x000C64AAu;
    constexpr DWORD kStage18PrimaryTableRva = 0x04882518u;
    constexpr DWORD kStage18SecondaryTableRva = 0x04882C20u;
    constexpr DWORD kStage18Bit113GateRecordCapacity = 512u;
    constexpr BYTE kStage18ExpectedHelperArguments[4] = { 0x6A, 0x19, 0x50, 0x56 };
    constexpr BYTE kStage18ExpectedHelperCall[5] = { 0xE8, 0x2B, 0xCB, 0x02, 0x00 };
    constexpr BYTE kStage18ExpectedObserverBytes[5] = { 0x83, 0xC4, 0x0C, 0x84, 0xC0 };
    constexpr BYTE kStage18ExpectedBranchBytes[2] = { 0x75, 0x41 };
    constexpr char kStage18BuildMarker[] = "STAGE18_BIT113_GATE_BUILD=2026-11-08-r1";
    constexpr DWORD kStage19StateMapCallRva = 0x000F6C03u;
    constexpr DWORD kStage19StateMapObserverRva = 0x000F6C08u;
    constexpr DWORD kStage19StateMapBranchRva = 0x000F6C0Eu;
    constexpr DWORD kStage19StateMapHelperRva = 0x000F02C0u;
    constexpr DWORD kStage19StateMapContinuationRva = 0x000F6C0Eu;
    constexpr DWORD kStage19StateMapRecordCapacity = 512u;
    constexpr BYTE kStage19ExpectedArguments[2] = { 0x56, 0x57 }; // push ESI(raw code), push EDI(player)
    constexpr BYTE kStage19ExpectedHelperCall[5] = { 0xE8, 0xB8, 0x96, 0xFF, 0xFF };
    constexpr BYTE kStage19ExpectedObserverBytes[6] = { 0x83, 0xC4, 0x08, 0x83, 0xF8, 0x05 };
    constexpr BYTE kStage19ExpectedBranchBytes[2] = { 0x75, 0x0A };
    constexpr char kStage19BuildMarker[] = "STAGE19_BIT113_STATE_MAP_BUILD=2026-11-08-r1";
    constexpr DWORD kStage20ObjectGuardCallRva = 0x000C64F1u;
    constexpr DWORD kStage20ObjectGuardHelperRva = 0x00095B00u;
    constexpr DWORD kStage20ObjectGuardContinuationRva = 0x000C64F6u;
    constexpr DWORD kStage20ObjectGuardTestRva = 0x000C64F6u;
    constexpr DWORD kStage20ObjectGuardBranchRva = 0x000C64F8u;
    constexpr DWORD kStage20ObjectGuardRecordCapacity = 1024u;
    constexpr BYTE kStage20ExpectedArgumentSetup[4] = { 0x8B, 0x0B, 0x6A, 0x02 };
    constexpr BYTE kStage20ExpectedHelperCall[5] = { 0xE8, 0x0A, 0xF6, 0xFC, 0xFF };
    constexpr BYTE kStage20ExpectedTestBytes[2] = { 0x84, 0xC0 };
    constexpr BYTE kStage20ExpectedBranchBytes[6] = { 0x0F, 0x85, 0x0D, 0xFF, 0xFF, 0xFF };
    constexpr char kStage20BuildMarker[] = "STAGE20_BIT113_OBJECT_BIT2_GUARD_BUILD=2026-11-08-r1";


    struct TraceSite
    {
        // State: 0 = empty, -1 = being initialized, 1 = ready.
        volatile LONG State;
        volatile LONG Count;
        volatile LONG Reported;
        DWORD EipAfter;
        DWORD RvaAfter;
        DWORD ThreadId;
        DWORD Eax;
        DWORD Ebx;
        DWORD Ecx;
        DWORD Edx;
        DWORD Esi;
        DWORD Edi;
        DWORD Ebp;
        DWORD Esp;
        DWORD EFlags;
        BYTE CodeWindow[24]; // 16 bytes before EIP, then 8 bytes from EIP.
    };

    struct Stage17Bit113SetterRecord
    {
        volatile LONG State; // 0 = unused, -1 = being written, 1 = ready.
        DWORD Sequence;
        DWORD Side;
        DWORD ThreadId;
        DWORD TickCount;
        SYSTEMTIME LocalTime;
        ULONGLONG QpcTicks;
        DWORD CallRva;
        DWORD ReturnRva;
        DWORD ReturnAddress;
        DWORD SetterIndex;
        DWORD PlayerObject;
        DWORD ThisPointer;
        DWORD Eax;
        DWORD Ebx;
        DWORD Ecx;
        DWORD Edx;
        DWORD Esi;
        DWORD Edi;
        DWORD Ebp;
        DWORD EspAtCall;
        DWORD EFlags;
        DWORD OtherObject;
        DWORD OtherObject3B70;
        DWORD Player3A34;
        DWORD Player3A36;
        DWORD Player3B70;
        DWORD Player3B78;
        DWORD Player3B80;
        DWORD Player3B84;
        DWORD Player3B88;
        DWORD Player3F20;
        DWORD Player3F30;
        DWORD Player3F50;
        DWORD Player3F58;
        DWORD Stack[12];
        BYTE CodeWindow[24];
    };

    struct Stage18Bit113GateRecord
    {
        volatile LONG State;
        DWORD Sequence;
        DWORD Side;
        DWORD ThreadId;
        DWORD TickCount;
        SYSTEMTIME LocalTime;
        ULONGLONG QpcTicks;
        DWORD Player;
        DWORD StateIndex;
        DWORD Key;
        DWORD ResultEax;
        DWORD ResultAl;
        DWORD HookReturnAddress;
        DWORD CallerEbp;
        DWORD ParentEbp;
        DWORD ParentReturnAddress;
        DWORD ParentArgument;
        DWORD Eax;
        DWORD Ebx;
        DWORD Ecx;
        DWORD Edx;
        DWORD Esi;
        DWORD Edi;
        DWORD EspAtHook;
        DWORD EFlags;
        DWORD Player3A34;
        DWORD Player3A36;
        DWORD Player3978;
        DWORD Player3B80;
        DWORD Player3B84;
        DWORD Player3B88;
        DWORD Player3F20;
        DWORD SpecialSelector;
        DWORD SelectedIndex;
        DWORD PrimaryTablePointer;
        DWORD SecondaryTablePointer;
        DWORD SelectedTablePointer;
        DWORD TableByteAddress;
        DWORD TableByteValue;
        DWORD ReadMask;
        DWORD UsedSecondary;
    };

    struct Stage19StateMapRecord
    {
        volatile LONG State; // 0 = unused, -1 = being written, 1 = ready.
        DWORD Sequence;
        DWORD Side;
        DWORD ThreadId;
        DWORD TickCount;
        SYSTEMTIME LocalTime;
        ULONGLONG QpcTicks;
        DWORD Player;
        DWORD RawCode;
        DWORD RawCodeIndex;
        DWORD MappingResult;
        DWORD ExpectedStateIndex;
        DWORD GateEligible;
        DWORD Player3A34;
        DWORD Player3A36;
        DWORD PlayerEntryCount;
        DWORD EntryFlagsAddress;
        DWORD EntryFlags;
        DWORD HookReturnAddress;
        DWORD Ebp;
        DWORD ParentEbp;
        DWORD ParentReturnAddress;
        DWORD Eax;
        DWORD Ebx;
        DWORD Ecx;
        DWORD Edx;
        DWORD Esi;
        DWORD Edi;
        DWORD EspAtHook;
        DWORD EFlags;
        DWORD ReadMask;
    };

    struct Stage20Bit113ObjectGuardRecord
    {
        volatile LONG State; // 0 = unused, -1 = being written, 1 = ready.
        DWORD Sequence;
        DWORD Side;
        DWORD ThreadId;
        DWORD TickCount;
        SYSTEMTIME LocalTime;
        ULONGLONG QpcTicks;
        DWORD Player;
        DWORD ObjectSlotPointer;
        DWORD ObjectPassed;
        DWORD ObjectFromSlot;
        DWORD Object3B70;
        DWORD HelperArgument;
        DWORD ResultEax;
        DWORD ResultAl;
        DWORD Bit2Set;
        DWORD HookReturnAddress;
        DWORD Ebp;
        DWORD ParentReturnAddress;
        DWORD Eax;
        DWORD Ebx;
        DWORD Ecx;
        DWORD Edx;
        DWORD Esi;
        DWORD Edi;
        DWORD EspAtHook;
        DWORD EFlags;
        WORD Player3A34;
        WORD Player3A36;
        DWORD Player3B84;
        DWORD Player3F20;
        DWORD ReadMask;
    };

    struct PlayerBit17WriteRecord
    {
        // State: 0 = empty, -1 = owned by producer/consumer, 1 = ready.
        volatile LONG State;
        DWORD Sequence;
        DWORD Side;
        DWORD ThreadId;
        DWORD TickCount;
        SYSTEMTIME LocalTime;
        ULONGLONG QpcTicks;
        DWORD EipAfter;
        DWORD RvaAfter;
        DWORD EipInAalib;
        DWORD PlayerObject;
        DWORD FieldAddress;
        DWORD OldValue;
        DWORD NewValue;
        DWORD ShadowInitialized;
        DWORD Dr6;
        DWORD Eax;
        DWORD Ebx;
        DWORD Ecx;
        DWORD Edx;
        DWORD Esi;
        DWORD Edi;
        DWORD Ebp;
        DWORD Esp;
        DWORD EFlags;
        BYTE CodeWindow[24];
    };

    TraceSite g_sites[2][kSiteCount]{};
    INIT_ONCE g_startOnce = INIT_ONCE_STATIC_INIT;
    HMODULE g_proxyModule = nullptr;
    HMODULE g_gameModule = nullptr;
    PVOID g_vectoredHandler = nullptr;
    DWORD g_gameImageSize = 0;
    DWORD g_gameBase = 0;
    DWORD g_watchP1 = 0;
    DWORD g_watchP2 = 0;
    DWORD g_scanIntervalMs = kDefaultScanIntervalMs;
    DWORD g_watchDr7Configuration = kControlTraceDr7;
    bool g_playerBit17WriteTrace = false;
    volatile LONG g_playerBit17ShadowValue[2]{};
    volatile LONG g_playerBit17ShadowValid[2]{};
    DWORD g_playerBit17InitialValue[2]{};
    volatile LONG g_playerBit17WriteSequence = 0;
    volatile LONG g_playerBit17WriteDropped = 0;
    volatile LONG g_playerBit17WriteReportedDropped = 0;
    PlayerBit17WriteRecord g_playerBit17WriteRecords[kPlayerBit17RecordCapacity]{};
    bool g_stage17Bit113SetterTrace = false;
    volatile LONG g_stage17Bit113SetterInstalled = 0;
    volatile LONG g_stage17Bit113WriteSequence = 0;
    volatile LONG g_stage17Bit113WriteDropped = 0;
    volatile LONG g_stage17Bit113WriteReportedDropped = 0;
    Stage17Bit113SetterRecord g_stage17Bit113Records[kStage17Bit113RecordCapacity]{};
    bool g_stage18Bit113GateTrace = false;
    volatile LONG g_stage18Bit113GateInstalled = 0;
    volatile LONG g_stage18Bit113GateSequence = 0;
    volatile LONG g_stage18Bit113GateDropped = 0;
    volatile LONG g_stage18Bit113GateReportedDropped = 0;
    Stage18Bit113GateRecord g_stage18Bit113GateRecords[kStage18Bit113GateRecordCapacity]{};
    bool g_stage19StateMapTrace = false;
    volatile LONG g_stage19StateMapInstalled = 0;
    volatile LONG g_stage19StateMapSequence = 0;
    volatile LONG g_stage19StateMapDropped = 0;
    volatile LONG g_stage19StateMapReportedDropped = 0;
    Stage19StateMapRecord g_stage19StateMapRecords[kStage19StateMapRecordCapacity]{};
    bool g_stage20ObjectGuardTrace = false;
    volatile LONG g_stage20ObjectGuardInstalled = 0;
    volatile LONG g_stage20ObjectGuardSequence = 0;
    volatile LONG g_stage20ObjectGuardDropped = 0;
    volatile LONG g_stage20ObjectGuardReportedDropped = 0;
    Stage20Bit113ObjectGuardRecord g_stage20ObjectGuardRecords[kStage20ObjectGuardRecordCapacity]{};
    volatile LONG g_firstInstallLog = 0;
    volatile LONG g_noThreadInstallLog = 0;

    // Keep handles for inspected threads so the scan loop does not suspend
    // every game thread every 250 ms. A signalled handle is a terminated thread;
    // its slot is then reclaimed (also making thread-ID reuse safe).
    struct TrackedThread
    {
        DWORD Id;
        HANDLE Handle;
        bool WatchpointsInstalled;
    };
    constexpr DWORD kTrackedThreadCapacity = 256;
    TrackedThread g_trackedThreads[kTrackedThreadCapacity]{};
    volatile LONG g_capacityWarningLogged = 0;

    bool GetSiblingPath(wchar_t* output, DWORD capacity, const wchar_t* filename)
    {
        if (output == nullptr || capacity == 0 || filename == nullptr || g_proxyModule == nullptr)
        {
            return false;
        }

        const DWORD length = GetModuleFileNameW(g_proxyModule, output, capacity);
        if (length == 0 || length >= capacity)
        {
            return false;
        }

        wchar_t* slash = wcsrchr(output, L'\\');
        wchar_t* forwardSlash = wcsrchr(output, L'/');
        if (forwardSlash != nullptr && (slash == nullptr || forwardSlash > slash))
        {
            slash = forwardSlash;
        }
        if (slash == nullptr)
        {
            return false;
        }

        *(slash + 1) = L'\0';
        const std::size_t used = wcslen(output);
        const std::size_t nameLength = wcslen(filename);
        if (used + nameLength + 1 > capacity)
        {
            return false;
        }
        wcscat_s(output, capacity, filename);
        return true;
    }

    void AppendTraceLog(const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_Trace.log"))
        {
            return;
        }

        SYSTEMTIME now{};
        GetLocalTime(&now);

        char line[2048]{};
        const int count = sprintf_s(
            line,
            "[%04u-%02u-%02u %02u:%02u:%02u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond,
            message != nullptr ? message : "(empty)");
        if (count <= 0)
        {
            return;
        }

        HANDLE file = CreateFileW(
            logPath,
            FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (file == INVALID_HANDLE_VALUE)
        {
            return;
        }

        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    void AppendPlayerBit17TraceLog(const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_PlayerBit17WriteTrace.log"))
        {
            return;
        }

        SYSTEMTIME now{};
        GetLocalTime(&now);
        char line[2048]{};
        const int count = sprintf_s(
            line,
            "[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
            message != nullptr ? message : "(empty)");
        if (count <= 0) return;

        HANDLE file = CreateFileW(
            logPath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;
        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    void AppendStage18Bit113GateLog(const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_Bit113GateTrace.log"))
            return;

        SYSTEMTIME now{};
        GetLocalTime(&now);
        char line[3072]{};
        const int count = sprintf_s(
            line, "[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
            message != nullptr ? message : "(empty)");
        if (count <= 0) return;

        HANDLE file = CreateFileW(
            logPath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;
        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    void AppendStage20ObjectGuardLog(const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_Bit113ObjectGuardTrace.log"))
            return;

        SYSTEMTIME now{};
        GetLocalTime(&now);
        char line[3072]{};
        const int count = sprintf_s(
            line, "[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
            message != nullptr ? message : "(empty)");
        if (count <= 0) return;

        HANDLE file = CreateFileW(
            logPath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;
        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    void AppendStage19StateMapLog(const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_StateMapTrace.log"))
            return;

        SYSTEMTIME now{};
        GetLocalTime(&now);
        char line[3072]{};
        const int count = sprintf_s(
            line, "[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
            message != nullptr ? message : "(empty)");
        if (count <= 0) return;

        HANDLE file = CreateFileW(
            logPath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;
        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    void AppendStage17Bit113Log(const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_Bit113SetterTrace.log"))
            return;

        SYSTEMTIME now{};
        GetLocalTime(&now);
        char line[2048]{};
        const int count = sprintf_s(
            line, "[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
            message != nullptr ? message : "(empty)");
        if (count <= 0) return;

        HANDLE file = CreateFileW(
            logPath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;
        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    int ReadIniInt(const wchar_t* section, const wchar_t* key, int defaultValue)
    {
        wchar_t iniPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(iniPath, MAX_PATH, L"ArcanaAI.ini"))
        {
            return defaultValue;
        }
        return GetPrivateProfileIntW(section, key, defaultValue, iniPath);
    }

    DWORD GetImageSize32(HMODULE module)
    {
        if (module == nullptr)
        {
            return 0;
        }

        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0)
        {
            return 0;
        }

        const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS32*>(
            reinterpret_cast<const BYTE*>(module) + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE ||
            nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        {
            return 0;
        }
        return nt->OptionalHeader.SizeOfImage;
    }

    bool IsGameImageAddress(DWORD address)
    {
        return g_gameBase != 0 && g_gameImageSize != 0 &&
            address >= g_gameBase + 0x1000 && address - g_gameBase < g_gameImageSize;
    }

    void CaptureCodeWindow(DWORD eipAfter, BYTE output[24])
    {
        std::memset(output, 0, 24);
        if (g_gameBase == 0 || eipAfter < g_gameBase + 0x1000 ||
            eipAfter >= g_gameBase + g_gameImageSize)
        {
            return;
        }

        DWORD start = eipAfter >= g_gameBase + 0x1010 ? eipAfter - 16 : g_gameBase + 0x1000;
        MEMORY_BASIC_INFORMATION info{};
        if (VirtualQuery(reinterpret_cast<const void*>(start), &info, sizeof(info)) != sizeof(info))
        {
            return;
        }
        if (info.State != MEM_COMMIT || (info.Protect & PAGE_GUARD) != 0 ||
            (info.Protect & 0xFF) == PAGE_NOACCESS)
        {
            return;
        }

        const auto regionStart = reinterpret_cast<std::uintptr_t>(info.BaseAddress);
        const auto regionEnd = regionStart + info.RegionSize;
        const auto copyStart = static_cast<std::uintptr_t>(start);
        const std::size_t available = copyStart < regionEnd
            ? static_cast<std::size_t>(regionEnd - copyStart)
            : 0;
        const std::size_t length = available < 24 ? available : 24;
        if (length != 0)
        {
            std::memcpy(output, reinterpret_cast<const void*>(start), length);
        }
    }

    void CaptureAnyCodeWindow(DWORD eipAfter, BYTE output[24])
    {
        std::memset(output, 0, 24);
        if (eipAfter < 0x10010u) return;

        const DWORD start = eipAfter - 16;
        MEMORY_BASIC_INFORMATION info{};
        if (VirtualQuery(reinterpret_cast<const void*>(start), &info, sizeof(info)) != sizeof(info))
            return;
        if (info.State != MEM_COMMIT || (info.Protect & PAGE_GUARD) != 0 ||
            (info.Protect & 0xFF) == PAGE_NOACCESS)
            return;

        const auto regionStart = reinterpret_cast<std::uintptr_t>(info.BaseAddress);
        const auto regionEnd = regionStart + info.RegionSize;
        const auto copyStart = static_cast<std::uintptr_t>(start);
        const std::size_t available = copyStart < regionEnd
            ? static_cast<std::size_t>(regionEnd - copyStart) : 0;
        const std::size_t length = available < 24 ? available : 24;
        if (length != 0)
            std::memcpy(output, reinterpret_cast<const void*>(start), length);
    }

    // The Stage17 recorder uses this helper before its implementation below.
    // Keep the declaration in this anonymous namespace so MSVC sees the signature.
    bool ReadDwordAtAddress(DWORD address, DWORD* value);
    bool ReadByteAtAddress(DWORD address, BYTE* value);
    bool ReadWordAtAddress(DWORD address, WORD* value);

    void QueueStage17Bit113SetterRecord(const DWORD* context)
    {
        if (!g_stage17Bit113SetterTrace || context == nullptr)
            return;

        const DWORD returnAddress = context[9];
        const DWORD setterIndex = context[10];
        const DWORD player = context[1]; // Original ESI; ECX is also captured independently.
        const DWORD p1Player = g_gameBase + kP1ControlRva - kPlayerControlOffset;
        const DWORD p2Player = g_gameBase + kP2ControlRva - kPlayerControlOffset;
        const DWORD side = player == p1Player ? 1u : (player == p2Player ? 2u : 0u);

        const LONG sequence = InterlockedIncrement(&g_stage17Bit113WriteSequence);
        Stage17Bit113SetterRecord& record =
            g_stage17Bit113Records[(static_cast<DWORD>(sequence - 1)) % kStage17Bit113RecordCapacity];
        if (InterlockedCompareExchange(&record.State, -1, 0) != 0)
        {
            InterlockedIncrement(&g_stage17Bit113WriteDropped);
            return;
        }

        record.Sequence = static_cast<DWORD>(sequence);
        record.Side = side;
        record.ThreadId = GetCurrentThreadId();
        record.TickCount = GetTickCount();
        GetLocalTime(&record.LocalTime);
        LARGE_INTEGER qpc{};
        QueryPerformanceCounter(&qpc);
        record.QpcTicks = static_cast<ULONGLONG>(qpc.QuadPart);
        record.ReturnAddress = returnAddress;
        record.ReturnRva = returnAddress >= g_gameBase ? returnAddress - g_gameBase : 0;
        record.CallRva = record.ReturnRva >= 5u ? record.ReturnRva - 5u : 0;
        record.SetterIndex = setterIndex;
        record.PlayerObject = player;
        record.ThisPointer = context[6];
        record.Edi = context[0];
        record.Esi = context[1];
        record.Ebp = context[2];
        record.EspAtCall = context[3] + 4u; // pushfd consumed four bytes before pushad saved ESP.
        record.Ebx = context[4];
        record.Edx = context[5];
        record.Ecx = context[6];
        record.Eax = context[7];
        record.EFlags = context[8];

        DWORD value = 0;
        if (ReadDwordAtAddress(record.Ebx, &value)) record.OtherObject = value;
        if (record.OtherObject != 0)
            ReadDwordAtAddress(record.OtherObject + 0x3B70u, &record.OtherObject3B70);
        if (player != 0)
        {
            ReadDwordAtAddress(player + 0x3A34u, &record.Player3A34);
            ReadDwordAtAddress(player + 0x3A36u, &record.Player3A36);
            ReadDwordAtAddress(player + 0x3B70u, &record.Player3B70);
            ReadDwordAtAddress(player + 0x3B78u, &record.Player3B78);
            ReadDwordAtAddress(player + 0x3B80u, &record.Player3B80);
            ReadDwordAtAddress(player + 0x3B84u, &record.Player3B84);
            ReadDwordAtAddress(player + 0x3B88u, &record.Player3B88);
            ReadDwordAtAddress(player + 0x3F20u, &record.Player3F20);
            ReadDwordAtAddress(player + 0x3F30u, &record.Player3F30);
            ReadDwordAtAddress(player + 0x3F50u, &record.Player3F50);
            ReadDwordAtAddress(player + 0x3F58u, &record.Player3F58);
        }

        for (DWORD i = 0; i < 12; ++i)
            ReadDwordAtAddress(record.EspAtCall + i * sizeof(DWORD), &record.Stack[i]);
        CaptureAnyCodeWindow(g_gameBase + kStage17Bit113SetterReturnRva, record.CodeWindow);
        InterlockedExchange(&record.State, 1);
    }

    void FlushStage17Bit113SetterRecords()
    {
        for (DWORD i = 0; i < kStage17Bit113RecordCapacity; ++i)
        {
            Stage17Bit113SetterRecord& record = g_stage17Bit113Records[i];
            if (InterlockedCompareExchange(&record.State, -1, 1) != 1)
                continue;

            char stackHex[12 * 24 + 1]{};
            for (DWORD j = 0; j < 12; ++j)
            {
                sprintf_s(stackHex + j * 24, sizeof(stackHex) - j * 24,
                    "S+%02lX=%08lX ",
                    static_cast<unsigned long>(j * 4),
                    static_cast<unsigned long>(record.Stack[j]));
            }
            char codeHex[24 * 3 + 1]{};
            for (int j = 0; j < 24; ++j)
                sprintf_s(codeHex + j * 3, sizeof(codeHex) - j * 3, "%02X ", record.CodeWindow[j]);

            const char* sideName = record.Side == 1 ? "P1" : (record.Side == 2 ? "P2" : "UNKNOWN");
            char message[3072]{};
            sprintf_s(
                message, sizeof(message),
                "BIT113_SET_CALL seq=%lu side=%s tid=%lu tick=%lu time=%02u:%02u:%02u.%03u qpc=0x%08lX%08lX "
                "callRVA=0x%08lX returnRVA=0x%08lX return=0x%08lX index=0x%08lX player=0x%08lX this=0x%08lX "
                "regs=(EAX:%08lX EBX:%08lX ECX:%08lX EDX:%08lX ESI:%08lX EDI:%08lX EBP:%08lX ESP:%08lX EFLAGS:%08lX) "
                "otherObject=0x%08lX other3B70=0x%08lX "
                "player=(3A34:%08lX 3A36:%08lX 3B70:%08lX 3B78:%08lX 3B80:%08lX 3B84:%08lX 3B88:%08lX 3F20:%08lX 3F30:%08lX 3F50:%08lX 3F58:%08lX) "
                "stack=%s codeBytes=%s",
                static_cast<unsigned long>(record.Sequence), sideName,
                static_cast<unsigned long>(record.ThreadId), static_cast<unsigned long>(record.TickCount),
                record.LocalTime.wHour, record.LocalTime.wMinute, record.LocalTime.wSecond, record.LocalTime.wMilliseconds,
                static_cast<unsigned long>(record.QpcTicks >> 32),
                static_cast<unsigned long>(record.QpcTicks & 0xFFFFFFFFu),
                static_cast<unsigned long>(record.CallRva), static_cast<unsigned long>(record.ReturnRva),
                static_cast<unsigned long>(record.ReturnAddress), static_cast<unsigned long>(record.SetterIndex),
                static_cast<unsigned long>(record.PlayerObject), static_cast<unsigned long>(record.ThisPointer),
                static_cast<unsigned long>(record.Eax), static_cast<unsigned long>(record.Ebx),
                static_cast<unsigned long>(record.Ecx), static_cast<unsigned long>(record.Edx),
                static_cast<unsigned long>(record.Esi), static_cast<unsigned long>(record.Edi),
                static_cast<unsigned long>(record.Ebp), static_cast<unsigned long>(record.EspAtCall),
                static_cast<unsigned long>(record.EFlags),
                static_cast<unsigned long>(record.OtherObject), static_cast<unsigned long>(record.OtherObject3B70),
                static_cast<unsigned long>(record.Player3A34), static_cast<unsigned long>(record.Player3A36),
                static_cast<unsigned long>(record.Player3B70), static_cast<unsigned long>(record.Player3B78),
                static_cast<unsigned long>(record.Player3B80), static_cast<unsigned long>(record.Player3B84),
                static_cast<unsigned long>(record.Player3B88), static_cast<unsigned long>(record.Player3F20),
                static_cast<unsigned long>(record.Player3F30), static_cast<unsigned long>(record.Player3F50),
                static_cast<unsigned long>(record.Player3F58), stackHex, codeHex);
            AppendStage17Bit113Log(message);
            InterlockedExchange(&record.State, 0);
        }

        const LONG dropped = InterlockedCompareExchange(&g_stage17Bit113WriteDropped, 0, 0);
        const LONG reported = InterlockedCompareExchange(&g_stage17Bit113WriteReportedDropped, 0, 0);
        if (dropped > reported)
        {
            char warning[192]{};
            sprintf_s(warning, "WARNING: Stage17 setter ring dropped %ld record(s); capacity=%lu.",
                static_cast<long>(dropped), static_cast<unsigned long>(kStage17Bit113RecordCapacity));
            AppendStage17Bit113Log(warning);
            InterlockedExchange(&g_stage17Bit113WriteReportedDropped, dropped);
        }
    }

    void AddTraceSite(int fieldIndex, PCONTEXT context)
    {
        if (fieldIndex < 0 || fieldIndex > 1 || context == nullptr)
        {
            return;
        }

        const DWORD eip = context->Eip;
        if (eip == 0)
        {
            return;
        }

        const DWORD startIndex = ((eip >> 4) ^ (static_cast<DWORD>(fieldIndex + 1) * 0x9E3779B9u)) % kSiteCount;
        for (DWORD probe = 0; probe < static_cast<DWORD>(kSiteCount); ++probe)
        {
            TraceSite& site = g_sites[fieldIndex][(startIndex + probe) % kSiteCount];
            const LONG state = InterlockedCompareExchange(&site.State, 0, 0);

            if (state == 1 && site.EipAfter == eip)
            {
                InterlockedIncrement(&site.Count);
                return;
            }

            if (state == 0 && InterlockedCompareExchange(&site.State, -1, 0) == 0)
            {
                site.EipAfter = eip;
                site.RvaAfter = eip - g_gameBase;
                site.ThreadId = GetCurrentThreadId();
                site.Eax = context->Eax;
                site.Ebx = context->Ebx;
                site.Ecx = context->Ecx;
                site.Edx = context->Edx;
                site.Esi = context->Esi;
                site.Edi = context->Edi;
                site.Ebp = context->Ebp;
                site.Esp = context->Esp;
                site.EFlags = context->EFlags;
                CaptureCodeWindow(eip, site.CodeWindow);
                InterlockedExchange(&site.Count, 1);
                InterlockedExchange(&site.State, 1);
                return;
            }
        }
    }

    bool ReadDwordAtAddress(DWORD address, DWORD* value)
    {
        if (address == 0 || value == nullptr) return false;
        SIZE_T bytesRead = 0;
        return ReadProcessMemory(
            GetCurrentProcess(), reinterpret_cast<LPCVOID>(address),
            value, sizeof(*value), &bytesRead) != FALSE && bytesRead == sizeof(*value);
    }

    bool ReadByteAtAddress(DWORD address, BYTE* value)
    {
        if (address == 0 || value == nullptr) return false;
        SIZE_T bytesRead = 0;
        return ReadProcessMemory(
            GetCurrentProcess(), reinterpret_cast<LPCVOID>(address),
            value, sizeof(*value), &bytesRead) != FALSE && bytesRead == sizeof(*value);
    }

    bool ReadWordAtAddress(DWORD address, WORD* value)
    {
        if (address == 0 || value == nullptr) return false;
        SIZE_T bytesRead = 0;
        return ReadProcessMemory(
            GetCurrentProcess(), reinterpret_cast<LPCVOID>(address),
            value, sizeof(*value), &bytesRead) != FALSE && bytesRead == sizeof(*value);
    }

    void QueueStage18Bit113GateRecord(const DWORD* context)
    {
        if (!g_stage18Bit113GateTrace || context == nullptr)
            return;

        // At hook entry: [CALL continuation][player][stateIndex][0x19].
        // context[3]+4 restores the hook-entry ESP after pushfd/pushad.
        const DWORD hookEntryEsp = context[3] + 4u;
        const DWORD player = context[10];
        const DWORD stateIndex = context[11];
        const DWORD key = context[12];
        const DWORD p1Player = g_gameBase + kP1ControlRva - kPlayerControlOffset;
        const DWORD p2Player = g_gameBase + kP2ControlRva - kPlayerControlOffset;
        const DWORD side = player == p1Player ? 1u : (player == p2Player ? 2u : 0u);
        if (side == 0)
            return;

        const LONG sequence = InterlockedIncrement(&g_stage18Bit113GateSequence);
        Stage18Bit113GateRecord& record =
            g_stage18Bit113GateRecords[(static_cast<DWORD>(sequence - 1)) % kStage18Bit113GateRecordCapacity];
        if (InterlockedCompareExchange(&record.State, -1, 0) != 0)
        {
            InterlockedIncrement(&g_stage18Bit113GateDropped);
            return;
        }

        record.Sequence = static_cast<DWORD>(sequence);
        record.Side = side;
        record.ThreadId = GetCurrentThreadId();
        record.TickCount = GetTickCount();
        GetLocalTime(&record.LocalTime);
        LARGE_INTEGER qpc{};
        QueryPerformanceCounter(&qpc);
        record.QpcTicks = static_cast<ULONGLONG>(qpc.QuadPart);
        record.Player = player;
        record.StateIndex = stateIndex;
        record.Key = key;
        record.ResultEax = context[7];
        record.ResultAl = context[7] & 0xFFu;
        record.HookReturnAddress = context[9];
        record.CallerEbp = context[2];
        record.Eax = context[7];
        record.Ebx = context[4];
        record.Ecx = context[6];
        record.Edx = context[5];
        record.Esi = context[1];
        record.Edi = context[0];
        record.EspAtHook = hookEntryEsp;
        record.EFlags = context[8];

        record.ParentEbp = 0;
        record.ParentReturnAddress = 0;
        record.ParentArgument = 0;
        if (record.CallerEbp != 0)
        {
            ReadDwordAtAddress(record.CallerEbp, &record.ParentEbp);
            ReadDwordAtAddress(record.CallerEbp + 4u, &record.ParentReturnAddress);
            ReadDwordAtAddress(record.CallerEbp + 8u, &record.ParentArgument);
        }

        record.Player3A34 = 0;
        record.Player3A36 = 0;
        record.Player3978 = 0;
        record.Player3B80 = 0;
        record.Player3B84 = 0;
        record.Player3B88 = 0;
        record.Player3F20 = 0;
        DWORD readMask = 0;
        if (ReadDwordAtAddress(player + 0x3A34u, &record.Player3A34)) readMask |= 0x0001u;
        if (ReadDwordAtAddress(player + 0x3A36u, &record.Player3A36)) readMask |= 0x0002u;
        if (ReadDwordAtAddress(player + 0x3978u, &record.Player3978)) readMask |= 0x0004u;
        if (ReadDwordAtAddress(player + 0x3B80u, &record.Player3B80)) readMask |= 0x0008u;
        if (ReadDwordAtAddress(player + 0x3B84u, &record.Player3B84)) readMask |= 0x0010u;
        if (ReadDwordAtAddress(player + 0x3B88u, &record.Player3B88)) readMask |= 0x0020u;
        if (ReadDwordAtAddress(player + 0x3F20u, &record.Player3F20)) readMask |= 0x0040u;

        record.SpecialSelector =
            record.Player3978 == 0x1Du &&
            (record.Player3B84 & 0x00010000u) != 0 &&
            (record.Player3B88 & 0x10000000u) != 0 ? 1u : 0u;
        record.SelectedIndex = record.SpecialSelector ? 0x16u : record.Player3F20;
        record.PrimaryTablePointer = 0;
        record.SecondaryTablePointer = 0;
        record.SelectedTablePointer = 0;
        record.TableByteAddress = 0;
        record.TableByteValue = 0xFFFFFFFFu;
        record.UsedSecondary = 0;

        const DWORD selector = record.SelectedIndex;
        const DWORD primarySlotRva = kStage18PrimaryTableRva + selector * sizeof(DWORD);
        const DWORD secondarySlotRva = kStage18SecondaryTableRva + selector * sizeof(DWORD);
        if ((readMask & 0x007Cu) == 0x007Cu && selector <= 0xFFu &&
            g_gameImageSize >= sizeof(DWORD) &&
            primarySlotRva <= g_gameImageSize - sizeof(DWORD) &&
            secondarySlotRva <= g_gameImageSize - sizeof(DWORD))
        {
            if (ReadDwordAtAddress(g_gameBase + primarySlotRva, &record.PrimaryTablePointer))
                readMask |= 0x0080u;
            record.SelectedTablePointer = record.PrimaryTablePointer;

            if ((record.Player3B80 & 0x02000000u) != 0)
            {
                if (ReadDwordAtAddress(g_gameBase + secondarySlotRva, &record.SecondaryTablePointer))
                {
                    readMask |= 0x0100u;
                    if (record.SecondaryTablePointer != 0)
                    {
                        record.SelectedTablePointer = record.SecondaryTablePointer;
                        record.UsedSecondary = 1;
                    }
                }
            }

            if (record.SelectedTablePointer != 0 && stateIndex <= 0x1Bu && key <= 0x1Au)
            {
                const ULONGLONG byteAddress =
                    static_cast<ULONGLONG>(record.SelectedTablePointer) +
                    static_cast<ULONGLONG>(stateIndex) * 0x1Au + key;
                if (byteAddress <= 0xFFFFFFFFull)
                {
                    record.TableByteAddress = static_cast<DWORD>(byteAddress);
                    BYTE byteValue = 0;
                    if (ReadByteAtAddress(record.TableByteAddress, &byteValue))
                    {
                        record.TableByteValue = byteValue;
                        readMask |= 0x0200u;
                    }
                }
            }
        }
        record.ReadMask = readMask;
        InterlockedExchange(&record.State, 1);
    }

    void FlushStage18Bit113GateRecords()
    {
        for (DWORD i = 0; i < kStage18Bit113GateRecordCapacity; ++i)
        {
            Stage18Bit113GateRecord& record = g_stage18Bit113GateRecords[i];
            if (InterlockedCompareExchange(&record.State, -1, 1) != 1)
                continue;

            char message[2048]{};
            const char* sideName = record.Side == 1 ? "P1" : "P2";
            sprintf_s(
                message, sizeof(message),
                "BIT113_GATE_RESULT seq=%lu side=%s tid=%lu tick=%lu time=%02u:%02u:%02u.%03u "
                "qpc=0x%08lX%08lX player=0x%08lX stateIndex=0x%08lX key=0x%08lX "
                "resultAL=%lu resultEAX=0x%08lX hookReturn=0x%08lX "
                "frame=(EBP:0x%08lX parentEBP:0x%08lX parentReturn:0x%08lX parentArg:0x%08lX) "
                "regs=(EAX:%08lX EBX:%08lX ECX:%08lX EDX:%08lX ESI:%08lX EDI:%08lX ESP:%08lX EFLAGS:%08lX) "
                "player=(3A34:%08lX 3A36:%08lX 3978:%08lX 3B80:%08lX 3B84:%08lX 3B88:%08lX 3F20:%08lX) "
                "table=(special:%lu selector:0x%08lX primary:0x%08lX secondary:0x%08lX selected:0x%08lX usedSecondary:%lu "
                "byteAddress:0x%08lX byte:0x%08lX readMask:0x%04lX)",
                static_cast<unsigned long>(record.Sequence), sideName,
                static_cast<unsigned long>(record.ThreadId), static_cast<unsigned long>(record.TickCount),
                record.LocalTime.wHour, record.LocalTime.wMinute, record.LocalTime.wSecond, record.LocalTime.wMilliseconds,
                static_cast<unsigned long>(record.QpcTicks >> 32),
                static_cast<unsigned long>(record.QpcTicks & 0xFFFFFFFFu),
                static_cast<unsigned long>(record.Player),
                static_cast<unsigned long>(record.StateIndex),
                static_cast<unsigned long>(record.Key),
                static_cast<unsigned long>(record.ResultAl),
                static_cast<unsigned long>(record.ResultEax),
                static_cast<unsigned long>(record.HookReturnAddress),
                static_cast<unsigned long>(record.CallerEbp),
                static_cast<unsigned long>(record.ParentEbp),
                static_cast<unsigned long>(record.ParentReturnAddress),
                static_cast<unsigned long>(record.ParentArgument),
                static_cast<unsigned long>(record.Eax),
                static_cast<unsigned long>(record.Ebx),
                static_cast<unsigned long>(record.Ecx),
                static_cast<unsigned long>(record.Edx),
                static_cast<unsigned long>(record.Esi),
                static_cast<unsigned long>(record.Edi),
                static_cast<unsigned long>(record.EspAtHook),
                static_cast<unsigned long>(record.EFlags),
                static_cast<unsigned long>(record.Player3A34),
                static_cast<unsigned long>(record.Player3A36),
                static_cast<unsigned long>(record.Player3978),
                static_cast<unsigned long>(record.Player3B80),
                static_cast<unsigned long>(record.Player3B84),
                static_cast<unsigned long>(record.Player3B88),
                static_cast<unsigned long>(record.Player3F20),
                static_cast<unsigned long>(record.SpecialSelector),
                static_cast<unsigned long>(record.SelectedIndex),
                static_cast<unsigned long>(record.PrimaryTablePointer),
                static_cast<unsigned long>(record.SecondaryTablePointer),
                static_cast<unsigned long>(record.SelectedTablePointer),
                static_cast<unsigned long>(record.UsedSecondary),
                static_cast<unsigned long>(record.TableByteAddress),
                static_cast<unsigned long>(record.TableByteValue),
                static_cast<unsigned long>(record.ReadMask));
            AppendStage18Bit113GateLog(message);
            InterlockedExchange(&record.State, 0);
        }

        const LONG dropped = InterlockedCompareExchange(&g_stage18Bit113GateDropped, 0, 0);
        const LONG reported = InterlockedCompareExchange(&g_stage18Bit113GateReportedDropped, 0, 0);
        if (dropped > reported)
        {
            char warning[192]{};
            sprintf_s(warning, "WARNING: Stage18 gate ring dropped %ld record(s); capacity=%lu.",
                static_cast<long>(dropped), static_cast<unsigned long>(kStage18Bit113GateRecordCapacity));
            AppendStage18Bit113GateLog(warning);
            InterlockedExchange(&g_stage18Bit113GateReportedDropped, dropped);
        }
    }

    void QueueStage19StateMapRecord(const DWORD* context)
    {
        if (!g_stage19StateMapTrace || context == nullptr)
            return;

        // The patched CALL has added a synthetic return address. At hook entry:
        // [ESP+0]=hook return, [ESP+4]=player, [ESP+8]=raw state code.
        const DWORD player = context[10];
        const DWORD rawCode = context[11];
        const DWORD p1Player = g_gameBase + kP1ControlRva - kPlayerControlOffset;
        const DWORD p2Player = g_gameBase + kP2ControlRva - kPlayerControlOffset;
        const DWORD side = player == p1Player ? 1u : (player == p2Player ? 2u : 0u);
        if (side == 0)
            return;

        const LONG sequence = InterlockedIncrement(&g_stage19StateMapSequence);
        Stage19StateMapRecord& record =
            g_stage19StateMapRecords[(static_cast<DWORD>(sequence - 1)) % kStage19StateMapRecordCapacity];
        if (InterlockedCompareExchange(&record.State, -1, 0) != 0)
        {
            InterlockedIncrement(&g_stage19StateMapDropped);
            return;
        }

        record.Sequence = static_cast<DWORD>(sequence);
        record.Side = side;
        record.ThreadId = GetCurrentThreadId();
        record.TickCount = GetTickCount();
        GetLocalTime(&record.LocalTime);
        LARGE_INTEGER qpc{};
        QueryPerformanceCounter(&qpc);
        record.QpcTicks = static_cast<ULONGLONG>(qpc.QuadPart);
        record.Player = player;
        record.RawCode = rawCode;
        record.RawCodeIndex = rawCode >= 0x20u ? rawCode - 0x20u : 0xFFFFFFFFu;
        record.MappingResult = context[7]; // EAX returned by 0x4F02C0.
        record.HookReturnAddress = context[9];
        record.Ebp = context[2];
        record.Eax = context[7];
        record.Ebx = context[4];
        record.Ecx = context[6];
        record.Edx = context[5];
        record.Esi = context[1];
        record.Edi = context[0];
        record.EspAtHook = context[3] + 4u;
        record.EFlags = context[8];

        // Exact post-0x4F02C0 mapping in 0x4F6BA0:
        // 0..4 -> 0x18..0x1C; 5 -> 0x1A; 6 -> 0x1B; 7 -> 0x1C; -1 -> -1.
        const DWORD result = record.MappingResult;
        if (result == 0xFFFFFFFFu)
            record.ExpectedStateIndex = 0xFFFFFFFFu;
        else if (result == 5u)
            record.ExpectedStateIndex = 0x1Au;
        else if (result == 6u)
            record.ExpectedStateIndex = 0x1Bu;
        else if (result == 7u)
            record.ExpectedStateIndex = 0x1Cu;
        else if (result <= 4u)
            record.ExpectedStateIndex = 0x18u + result;
        else
            record.ExpectedStateIndex = 0xFFFFFFFEu;
        record.GateEligible =
            record.ExpectedStateIndex < 0x1Cu ? 1u : 0u;

        record.Player3A34 = 0;
        record.Player3A36 = 0;
        record.PlayerEntryCount = 0;
        record.EntryFlagsAddress = 0;
        record.EntryFlags = 0xFFFFFFFFu;
        record.ParentEbp = 0;
        record.ParentReturnAddress = 0;
        DWORD readMask = 0;

        WORD stateType = 0;
        WORD rawState = 0;
        if (ReadWordAtAddress(player + 0x3A34u, &stateType)) readMask |= 0x0001u;
        if (ReadWordAtAddress(player + 0x3A36u, &rawState)) readMask |= 0x0002u;
        record.Player3A34 = stateType;
        record.Player3A36 = rawState;

        if (record.Ebp != 0)
        {
            ReadDwordAtAddress(record.Ebp, &record.ParentEbp);
            ReadDwordAtAddress(record.Ebp + 4u, &record.ParentReturnAddress);
        }

        DWORD entryCount = 0;
        if (ReadDwordAtAddress(player + 0x1220u, &entryCount))
        {
            record.PlayerEntryCount = entryCount;
            readMask |= 0x0004u;
            if (rawCode >= 0x20u)
            {
                const DWORD rawIndex = rawCode - 0x20u;
                if (rawIndex < entryCount && rawIndex <= 0x10000u)
                {
                    const ULONGLONG address =
                        static_cast<ULONGLONG>(player) + 0x2B9Eu +
                        static_cast<ULONGLONG>(rawIndex) * 0x14u;
                    if (address <= 0xFFFFFFFFull - sizeof(WORD))
                    {
                        record.EntryFlagsAddress = static_cast<DWORD>(address);
                        WORD flags = 0;
                        if (ReadWordAtAddress(record.EntryFlagsAddress, &flags))
                        {
                            record.EntryFlags = flags;
                            readMask |= 0x0008u;
                        }
                    }
                }
            }
        }
        record.ReadMask = readMask;
        InterlockedExchange(&record.State, 1);
    }

    void FlushStage19StateMapRecords()
    {
        for (DWORD i = 0; i < kStage19StateMapRecordCapacity; ++i)
        {
            Stage19StateMapRecord& record = g_stage19StateMapRecords[i];
            if (InterlockedCompareExchange(&record.State, -1, 1) != 1)
                continue;

            char message[2048]{};
            const char* sideName = record.Side == 1 ? "P1" : "P2";
            sprintf_s(
                message, sizeof(message),
                "STATE_MAP_CALL seq=%lu side=%s tid=%lu tick=%lu time=%02u:%02u:%02u.%03u "
                "qpc=0x%08lX%08lX player=0x%08lX rawCode=0x%08lX rawIndex=0x%08lX "
                "mapResult=0x%08lX expectedStateIndex=0x%08lX gateEligible=%lu "
                "player=(3A34:0x%04lX 3A36:0x%04lX entryCount:0x%08lX) "
                "entry=(address:0x%08lX flags:0x%04lX) "
                "frame=(hookReturn:0x%08lX EBP:0x%08lX parentEBP:0x%08lX parentReturn:0x%08lX) "
                "regs=(EAX:%08lX EBX:%08lX ECX:%08lX EDX:%08lX ESI:%08lX EDI:%08lX ESP:%08lX EFLAGS:%08lX) "
                "readMask=0x%02lX",
                static_cast<unsigned long>(record.Sequence), sideName,
                static_cast<unsigned long>(record.ThreadId), static_cast<unsigned long>(record.TickCount),
                record.LocalTime.wHour, record.LocalTime.wMinute, record.LocalTime.wSecond, record.LocalTime.wMilliseconds,
                static_cast<unsigned long>(record.QpcTicks >> 32),
                static_cast<unsigned long>(record.QpcTicks & 0xFFFFFFFFu),
                static_cast<unsigned long>(record.Player),
                static_cast<unsigned long>(record.RawCode),
                static_cast<unsigned long>(record.RawCodeIndex),
                static_cast<unsigned long>(record.MappingResult),
                static_cast<unsigned long>(record.ExpectedStateIndex),
                static_cast<unsigned long>(record.GateEligible),
                static_cast<unsigned long>(record.Player3A34),
                static_cast<unsigned long>(record.Player3A36),
                static_cast<unsigned long>(record.PlayerEntryCount),
                static_cast<unsigned long>(record.EntryFlagsAddress),
                static_cast<unsigned long>(record.EntryFlags),
                static_cast<unsigned long>(record.HookReturnAddress),
                static_cast<unsigned long>(record.Ebp),
                static_cast<unsigned long>(record.ParentEbp),
                static_cast<unsigned long>(record.ParentReturnAddress),
                static_cast<unsigned long>(record.Eax),
                static_cast<unsigned long>(record.Ebx),
                static_cast<unsigned long>(record.Ecx),
                static_cast<unsigned long>(record.Edx),
                static_cast<unsigned long>(record.Esi),
                static_cast<unsigned long>(record.Edi),
                static_cast<unsigned long>(record.EspAtHook),
                static_cast<unsigned long>(record.EFlags),
                static_cast<unsigned long>(record.ReadMask));
            AppendStage19StateMapLog(message);
            InterlockedExchange(&record.State, 0);
        }

        const LONG dropped = InterlockedCompareExchange(&g_stage19StateMapDropped, 0, 0);
        const LONG reported = InterlockedCompareExchange(&g_stage19StateMapReportedDropped, 0, 0);
        if (dropped > reported)
        {
            char warning[192]{};
            sprintf_s(warning, "WARNING: Stage19 state-map ring dropped %ld record(s); capacity=%lu.",
                static_cast<long>(dropped), static_cast<unsigned long>(kStage19StateMapRecordCapacity));
            AppendStage19StateMapLog(warning);
            InterlockedExchange(&g_stage19StateMapReportedDropped, dropped);
        }
    }

    void QueueStage20Bit113ObjectGuardRecord(const DWORD* context)
    {
        if (!g_stage20ObjectGuardTrace || context == nullptr)
            return;

        // The wrapper saves the original ECX immediately before calling the helper.
        // Saved-context stack: context[9]=saved ECX, [10]=wrapper return, [11]=original arg 2.
        const DWORD player = context[1]; // Caller ESI; same player used by Stage17 setter.
        const DWORD objectSlotPointer = context[4]; // EBX is preserved by the helper; [EBX] is the object slot.
        const DWORD objectPassed = context[9]; // Original ECX saved by the wrapper before helper call.
        const DWORD p1Player = g_gameBase + kP1ControlRva - kPlayerControlOffset;
        const DWORD p2Player = g_gameBase + kP2ControlRva - kPlayerControlOffset;
        const DWORD side = player == p1Player ? 1u : (player == p2Player ? 2u : 0u);
        if (side == 0)
            return;

        const LONG sequence = InterlockedIncrement(&g_stage20ObjectGuardSequence);
        Stage20Bit113ObjectGuardRecord& record =
            g_stage20ObjectGuardRecords[(static_cast<DWORD>(sequence - 1)) % kStage20ObjectGuardRecordCapacity];
        if (InterlockedCompareExchange(&record.State, -1, 0) != 0)
        {
            InterlockedIncrement(&g_stage20ObjectGuardDropped);
            return;
        }

        record.Sequence = static_cast<DWORD>(sequence);
        record.Side = side;
        record.ThreadId = GetCurrentThreadId();
        record.TickCount = GetTickCount();
        GetLocalTime(&record.LocalTime);
        LARGE_INTEGER qpc{};
        QueryPerformanceCounter(&qpc);
        record.QpcTicks = static_cast<ULONGLONG>(qpc.QuadPart);

        record.Player = player;
        record.ObjectSlotPointer = objectSlotPointer;
        record.ObjectPassed = objectPassed;
        record.ObjectFromSlot = 0;
        record.Object3B70 = 0xFFFFFFFFu;
        record.HelperArgument = context[11];
        record.ResultEax = context[7];
        record.ResultAl = context[7] & 0xFFu;
        record.Bit2Set = 0xFFFFFFFFu;
        record.HookReturnAddress = context[10];
        record.Ebp = context[2];
        record.ParentReturnAddress = 0;
        if (record.Ebp != 0)
            ReadDwordAtAddress(record.Ebp + 4u, &record.ParentReturnAddress);

        record.Eax = context[7];
        record.Ebx = context[4];
        record.Ecx = context[6];
        record.Edx = context[5];
        record.Esi = context[1];
        record.Edi = context[0];
        record.EspAtHook = context[3] + 8u; // Account for saved ECX plus PUSHFD.
        record.EFlags = context[8];

        record.Player3A34 = 0;
        record.Player3A36 = 0;
        record.Player3B84 = 0;
        record.Player3F20 = 0;
        DWORD readMask = 0;
        if (objectSlotPointer != 0 &&
            ReadDwordAtAddress(objectSlotPointer, &record.ObjectFromSlot))
            readMask |= 0x0001u;
        if (objectPassed != 0 &&
            ReadDwordAtAddress(objectPassed + 0x3B70u, &record.Object3B70))
        {
            readMask |= 0x0002u;
            record.Bit2Set = (record.Object3B70 & 0x00000004u) != 0 ? 1u : 0u;
        }
        if (ReadWordAtAddress(player + 0x3A34u, &record.Player3A34))
            readMask |= 0x0004u;
        if (ReadWordAtAddress(player + 0x3A36u, &record.Player3A36))
            readMask |= 0x0008u;
        if (ReadDwordAtAddress(player + 0x3B84u, &record.Player3B84))
            readMask |= 0x0010u;
        if (ReadDwordAtAddress(player + 0x3F20u, &record.Player3F20))
            readMask |= 0x0020u;
        record.ReadMask = readMask;
        InterlockedExchange(&record.State, 1);
    }

    void FlushStage20Bit113ObjectGuardRecords()
    {
        for (DWORD i = 0; i < kStage20ObjectGuardRecordCapacity; ++i)
        {
            Stage20Bit113ObjectGuardRecord& record = g_stage20ObjectGuardRecords[i];
            if (InterlockedCompareExchange(&record.State, -1, 1) != 1)
                continue;

            char message[2048]{};
            const char* sideName = record.Side == 1 ? "P1" : "P2";
            sprintf_s(
                message, sizeof(message),
                "BIT113_OBJECT_GUARD seq=%lu side=%s tid=%lu tick=%lu time=%02u:%02u:%02u.%03u "
                "qpc=0x%08lX%08lX player=0x%08lX helperArg=0x%08lX resultAL=%lu resultEAX=0x%08lX "
                "object=(slotPtr:0x%08lX slotValue:0x%08lX passedECX:0x%08lX dword3B70:0x%08lX bit2Set:%lu) "
                "frame=(hookReturn:0x%08lX EBP:0x%08lX parentReturn:0x%08lX) "
                "regs=(EAX:%08lX EBX:%08lX ECX:%08lX EDX:%08lX ESI:%08lX EDI:%08lX ESP:%08lX EFLAGS:%08lX) "
                "playerFields=(3A34:0x%04lX 3A36:0x%04lX 3B84:0x%08lX 3F20:0x%08lX) readMask=0x%02lX",
                static_cast<unsigned long>(record.Sequence), sideName,
                static_cast<unsigned long>(record.ThreadId), static_cast<unsigned long>(record.TickCount),
                record.LocalTime.wHour, record.LocalTime.wMinute, record.LocalTime.wSecond, record.LocalTime.wMilliseconds,
                static_cast<unsigned long>(record.QpcTicks >> 32),
                static_cast<unsigned long>(record.QpcTicks & 0xFFFFFFFFu),
                static_cast<unsigned long>(record.Player),
                static_cast<unsigned long>(record.HelperArgument),
                static_cast<unsigned long>(record.ResultAl),
                static_cast<unsigned long>(record.ResultEax),
                static_cast<unsigned long>(record.ObjectSlotPointer),
                static_cast<unsigned long>(record.ObjectFromSlot),
                static_cast<unsigned long>(record.ObjectPassed),
                static_cast<unsigned long>(record.Object3B70),
                static_cast<unsigned long>(record.Bit2Set),
                static_cast<unsigned long>(record.HookReturnAddress),
                static_cast<unsigned long>(record.Ebp),
                static_cast<unsigned long>(record.ParentReturnAddress),
                static_cast<unsigned long>(record.Eax),
                static_cast<unsigned long>(record.Ebx),
                static_cast<unsigned long>(record.Ecx),
                static_cast<unsigned long>(record.Edx),
                static_cast<unsigned long>(record.Esi),
                static_cast<unsigned long>(record.Edi),
                static_cast<unsigned long>(record.EspAtHook),
                static_cast<unsigned long>(record.EFlags),
                static_cast<unsigned long>(record.Player3A34),
                static_cast<unsigned long>(record.Player3A36),
                static_cast<unsigned long>(record.Player3B84),
                static_cast<unsigned long>(record.Player3F20),
                static_cast<unsigned long>(record.ReadMask));
            AppendStage20ObjectGuardLog(message);
            InterlockedExchange(&record.State, 0);
        }

        const LONG dropped = InterlockedCompareExchange(&g_stage20ObjectGuardDropped, 0, 0);
        const LONG reported = InterlockedCompareExchange(&g_stage20ObjectGuardReportedDropped, 0, 0);
        if (dropped > reported)
        {
            char warning[192]{};
            sprintf_s(warning, "WARNING: Stage20 object-guard ring dropped %ld record(s); capacity=%lu.",
                static_cast<long>(dropped), static_cast<unsigned long>(kStage20ObjectGuardRecordCapacity));
            AppendStage20ObjectGuardLog(warning);
            InterlockedExchange(&g_stage20ObjectGuardReportedDropped, dropped);
        }
    }

    void QueuePlayerBit17WriteRecord(int sideIndex, PCONTEXT context)
    {
        if (!g_playerBit17WriteTrace || sideIndex < 0 || sideIndex > 1 || context == nullptr)
            return;

        const DWORD fieldAddress = sideIndex == 0 ? g_watchP1 : g_watchP2;
        DWORD newValue = 0;
        if (!ReadDwordAtAddress(fieldAddress, &newValue))
        {
            InterlockedIncrement(&g_playerBit17WriteDropped);
            return;
        }

        const LONG sequence = InterlockedIncrement(&g_playerBit17WriteSequence);
        PlayerBit17WriteRecord& record =
            g_playerBit17WriteRecords[(static_cast<DWORD>(sequence - 1)) % kPlayerBit17RecordCapacity];
        if (InterlockedCompareExchange(&record.State, -1, 0) != 0)
        {
            InterlockedIncrement(&g_playerBit17WriteDropped);
            return;
        }

        const LONG oldKnown = InterlockedCompareExchange(&g_playerBit17ShadowValid[sideIndex], 0, 0);
        const LONG oldValue = InterlockedExchange(
            &g_playerBit17ShadowValue[sideIndex], static_cast<LONG>(newValue));

        record.Sequence = static_cast<DWORD>(sequence);
        record.Side = static_cast<DWORD>(sideIndex + 1);
        record.ThreadId = GetCurrentThreadId();
        record.TickCount = GetTickCount();
        GetLocalTime(&record.LocalTime);
        LARGE_INTEGER qpc{};
        QueryPerformanceCounter(&qpc);
        record.QpcTicks = static_cast<ULONGLONG>(qpc.QuadPart);
        record.EipAfter = context->Eip;
        record.EipInAalib = IsGameImageAddress(context->Eip) ? 1u : 0u;
        record.RvaAfter = record.EipInAalib != 0 ? context->Eip - g_gameBase : 0;
        record.PlayerObject = fieldAddress - kPlayerBit17Offset;
        record.FieldAddress = fieldAddress;
        record.OldValue = oldKnown != 0 ? static_cast<DWORD>(oldValue) : newValue;
        record.NewValue = newValue;
        record.ShadowInitialized = oldKnown != 0 ? 1u : 0u;
        record.Dr6 = context->Dr6;
        record.Eax = context->Eax;
        record.Ebx = context->Ebx;
        record.Ecx = context->Ecx;
        record.Edx = context->Edx;
        record.Esi = context->Esi;
        record.Edi = context->Edi;
        record.Ebp = context->Ebp;
        record.Esp = context->Esp;
        record.EFlags = context->EFlags;
        CaptureAnyCodeWindow(context->Eip, record.CodeWindow);
        InterlockedExchange(&record.State, 1);
    }

    void FlushPlayerBit17WriteRecords()
    {
        for (DWORD i = 0; i < kPlayerBit17RecordCapacity; ++i)
        {
            PlayerBit17WriteRecord& record = g_playerBit17WriteRecords[i];
            if (InterlockedCompareExchange(&record.State, -1, 1) != 1)
                continue;

            char codeHex[24 * 3 + 1]{};
            for (int j = 0; j < 24; ++j)
            {
                sprintf_s(codeHex + j * 3, sizeof(codeHex) - j * 3, "%02X ", record.CodeWindow[j]);
            }

            const DWORD oldBit17 = (record.OldValue & kPlayerBit17Mask) != 0 ? 1u : 0u;
            const DWORD newBit17 = (record.NewValue & kPlayerBit17Mask) != 0 ? 1u : 0u;
            char message[1536]{};
            sprintf_s(
                message, sizeof(message),
                "PLAYER_BIT17_WRITE seq=%lu side=P%lu tid=%lu tick=%lu time=%02u:%02u:%02u.%03u qpc=0x%08lX%08lX "
                "eipAfter=0x%08lX inAALib=%lu rvaAfter=0x%08lX player=0x%08lX field=0x%08lX old=0x%08lX new=0x%08lX "
                "shadowInitialized=%lu oldBit17=%s newBit17=%lu shadowBit17Changed=%lu dr6=0x%08lX "
                "EAX=%08lX EBX=%08lX ECX=%08lX EDX=%08lX ESI=%08lX EDI=%08lX EBP=%08lX ESP=%08lX EFLAGS=%08lX "
                "bytes_at_EIP_minus_16=%s",
                static_cast<unsigned long>(record.Sequence),
                static_cast<unsigned long>(record.Side),
                static_cast<unsigned long>(record.ThreadId),
                static_cast<unsigned long>(record.TickCount),
                record.LocalTime.wHour, record.LocalTime.wMinute,
                record.LocalTime.wSecond, record.LocalTime.wMilliseconds,
                static_cast<unsigned long>(record.QpcTicks >> 32),
                static_cast<unsigned long>(record.QpcTicks & 0xFFFFFFFFu),
                static_cast<unsigned long>(record.EipAfter),
                static_cast<unsigned long>(record.EipInAalib),
                static_cast<unsigned long>(record.RvaAfter),
                static_cast<unsigned long>(record.PlayerObject),
                static_cast<unsigned long>(record.FieldAddress),
                static_cast<unsigned long>(record.OldValue),
                static_cast<unsigned long>(record.NewValue),
                static_cast<unsigned long>(record.ShadowInitialized),
                record.ShadowInitialized != 0 ? (oldBit17 != 0 ? "1" : "0") : "NA",
                static_cast<unsigned long>(newBit17),
                static_cast<unsigned long>(record.ShadowInitialized != 0 && oldBit17 != newBit17),
                static_cast<unsigned long>(record.Dr6),
                static_cast<unsigned long>(record.Eax),
                static_cast<unsigned long>(record.Ebx),
                static_cast<unsigned long>(record.Ecx),
                static_cast<unsigned long>(record.Edx),
                static_cast<unsigned long>(record.Esi),
                static_cast<unsigned long>(record.Edi),
                static_cast<unsigned long>(record.Ebp),
                static_cast<unsigned long>(record.Esp),
                static_cast<unsigned long>(record.EFlags),
                codeHex);
            AppendPlayerBit17TraceLog(message);
            InterlockedExchange(&record.State, 0);
        }

        const LONG dropped = InterlockedCompareExchange(&g_playerBit17WriteDropped, 0, 0);
        const LONG reported = InterlockedCompareExchange(&g_playerBit17WriteReportedDropped, 0, 0);
        if (dropped > reported)
        {
            char warning[192]{};
            sprintf_s(warning,
                "WARNING: Stage16 write-record buffer dropped %ld record(s); capacity=%lu.",
                static_cast<long>(dropped), static_cast<unsigned long>(kPlayerBit17RecordCapacity));
            AppendPlayerBit17TraceLog(warning);
            InterlockedExchange(&g_playerBit17WriteReportedDropped, dropped);
        }
    }

    LONG CALLBACK TraceExceptionHandler(EXCEPTION_POINTERS* exceptionInfo)
    {
        if (exceptionInfo == nullptr || exceptionInfo->ExceptionRecord == nullptr ||
            exceptionInfo->ContextRecord == nullptr ||
            exceptionInfo->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP)
        {
            return EXCEPTION_CONTINUE_SEARCH;
        }

        PCONTEXT context = exceptionInfo->ContextRecord;
        const DWORD dr6 = context->Dr6;
        const DWORD ownedHits = dr6 & 0x3;
        const DWORD commandTraceHits = dr6 & 0xCu; // DR2/DR3 belong to AICommandTrace.
        const DWORD otherDebugHits = dr6 & 0x0000E000u; // BD, BS, BT only.
        if (ownedHits == 0 || otherDebugHits != 0)
        {
            return EXCEPTION_CONTINUE_SEARCH;
        }

        // Clear only the status bits belonging to our two data watchpoints.
        context->Dr6 &= ~0x3u;

        // The legacy control-field mode retains its original AALib-only filter.
        // Stage16 records writes from any user-mode module in the target process;
        // the COMvsCOM worker is excluded before watchpoint installation.
        if (!g_playerBit17WriteTrace && !IsGameImageAddress(context->Eip))
        {
            return EXCEPTION_CONTINUE_EXECUTION;
        }

        if ((ownedHits & 0x1) != 0)
        {
            if (g_playerBit17WriteTrace) QueuePlayerBit17WriteRecord(0, context);
            else AddTraceSite(0, context);
        }
        if ((ownedHits & 0x2) != 0)
        {
            if (g_playerBit17WriteTrace) QueuePlayerBit17WriteRecord(1, context);
            else AddTraceSite(1, context);
        }

        if (commandTraceHits != 0)
        {
            // Stage16 has already recorded any simultaneous DR0/DR1 data hit.
            // Pass the still-pending DR2/DR3 execution hit to AICommandTrace.
            return EXCEPTION_CONTINUE_SEARCH;
        }

        return EXCEPTION_CONTINUE_EXECUTION;
    }

    bool IsOurWatchpointConfiguration(const CONTEXT& context)
    {
        return context.Dr0 == g_watchP1 &&
            context.Dr1 == g_watchP2 &&
            (context.Dr7 & 0x00FF000Fu) == (g_watchDr7Configuration & 0x00FF000Fu);
    }

    void PruneTrackedThreads()
    {
        for (DWORD i = 0; i < kTrackedThreadCapacity; ++i)
        {
            TrackedThread& slot = g_trackedThreads[i];
            if (slot.Handle != nullptr && WaitForSingleObject(slot.Handle, 0) == WAIT_OBJECT_0)
            {
                CloseHandle(slot.Handle);
                slot = {};
            }
        }
    }

    bool IsThreadTracked(DWORD threadId)
    {
        for (DWORD i = 0; i < kTrackedThreadCapacity; ++i)
        {
            if (g_trackedThreads[i].Handle != nullptr && g_trackedThreads[i].Id == threadId)
            {
                return true;
            }
        }
        return false;
    }

    bool InspectThreadAndKeepHandle(DWORD threadId, HANDLE* retainedHandle, bool* watchpointsInstalled)
    {
        if (retainedHandle == nullptr || watchpointsInstalled == nullptr ||
            threadId == GetCurrentThreadId())
        {
            return false;
        }

        *retainedHandle = nullptr;
        *watchpointsInstalled = false;

        HANDLE thread = OpenThread(
            THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT |
                THREAD_QUERY_INFORMATION | SYNCHRONIZE,
            FALSE,
            threadId);
        if (thread == nullptr)
        {
            return false;
        }

        bool inspected = false;
        bool installed = false;
        const DWORD suspendResult = SuspendThread(thread);
        if (suspendResult != static_cast<DWORD>(-1))
        {
            CONTEXT context{};
            context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
            if (GetThreadContext(thread, &context))
            {
                inspected = true;
                if (IsOurWatchpointConfiguration(context))
                {
                    installed = true;
                }
                else if (context.Dr0 == 0 && context.Dr1 == 0 &&
                    (context.Dr7 & 0x00FF000Fu) == 0)
                {
                    context.Dr0 = g_watchP1;
                    context.Dr1 = g_watchP2;
                    context.Dr6 &= ~0x3u;
                    // DR0/DR1 are local, 4-byte data breakpoints. The configured
                    // mode selects read/write (Stage 2) or write-only (Stage 16).
                    // DR2/DR3 and unrelated DR7 fields are preserved.
                    context.Dr7 = (context.Dr7 & ~0x00FF000Fu) | g_watchDr7Configuration;
                    installed = SetThreadContext(thread, &context) != FALSE;
                }
                // If either address register or any DR0/DR1 enable/RW/LEN bit
                // is already populated, keep the thread tracked but leave the
                // existing debug-register state untouched.
            }
            ResumeThread(thread);
        }

        if (!inspected)
        {
            CloseHandle(thread);
            return false;
        }

        *retainedHandle = thread;
        *watchpointsInstalled = installed;
        return true;
    }

    DWORD InstallWatchpointsOnCurrentThreads()
    {
        const DWORD processId = GetCurrentProcessId();
        const DWORD currentThreadId = GetCurrentThreadId();
        const DWORD comVsComThreadId = GetComVsComThreadId();
        DWORD newlyInstalled = 0;
        DWORD newlyInspected = 0;
        DWORD newlySkipped = 0;
        PruneTrackedThreads();

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
        {
            return 0;
        }

        THREADENTRY32 entry{};
        entry.dwSize = sizeof(entry);
        if (Thread32First(snapshot, &entry))
        {
            do
            {
                if (entry.th32OwnerProcessID == processId &&
                    entry.th32ThreadID != currentThreadId &&
                    entry.th32ThreadID != comVsComThreadId &&
                    !IsThreadTracked(entry.th32ThreadID))
                {
                    HANDLE retainedHandle = nullptr;
                    bool watchpointsInstalled = false;
                    if (InspectThreadAndKeepHandle(
                        entry.th32ThreadID, &retainedHandle, &watchpointsInstalled))
                    {
                        bool stored = false;
                        for (DWORD slotIndex = 0; slotIndex < kTrackedThreadCapacity; ++slotIndex)
                        {
                            TrackedThread& slot = g_trackedThreads[slotIndex];
                            if (slot.Handle == nullptr)
                            {
                                slot.Id = entry.th32ThreadID;
                                slot.Handle = retainedHandle;
                                slot.WatchpointsInstalled = watchpointsInstalled;
                                stored = true;
                                ++newlyInspected;
                                if (watchpointsInstalled)
                                {
                                    ++newlyInstalled;
                                }
                                else
                                {
                                    ++newlySkipped;
                                }
                                break;
                            }
                        }

                        if (!stored)
                        {
                            CloseHandle(retainedHandle);
                            if (InterlockedCompareExchange(&g_capacityWarningLogged, 1, 0) == 0)
                            {
                                AppendTraceLog("WARNING: tracked-thread capacity reached; additional threads will not be watched.");
                            }
                        }
                    }
                }
                entry.dwSize = sizeof(entry);
            } while (Thread32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
        if (newlySkipped != 0)
        {
            char warning[256]{};
            sprintf_s(warning,
                "WARNING: %lu newly inspected game thread(s) were not given DR0/DR1 because those slots were occupied or unavailable; these threads are tracked but will not be retried.",
                static_cast<unsigned long>(newlySkipped));
            AppendTraceLog(warning);
            if (g_playerBit17WriteTrace) AppendPlayerBit17TraceLog(warning);
        }
        if (newlyInspected != 0 && newlyInstalled == 0 &&
            InterlockedCompareExchange(&g_noThreadInstallLog, 1, 0) == 0)
        {
            AppendTraceLog("Threads were inspected, but DR0/DR1 were unavailable or could not be set. Existing debug-register owners were left untouched.");
            if (g_playerBit17WriteTrace)
                AppendPlayerBit17TraceLog("ERROR: no newly inspected game thread accepted the P1/P2 write watchpoints; external DR0/DR1 owners were left untouched.");
        }
        return newlyInstalled;
    }

    void FlushNewSites()
    {
        for (int field = 0; field < 2; ++field)
        {
            for (LONG i = 0; i < kSiteCount; ++i)
            {
                TraceSite& site = g_sites[field][i];
                if (InterlockedCompareExchange(&site.State, 0, 0) != 1 ||
                    InterlockedCompareExchange(&site.Reported, 1, 0) != 0)
                {
                    continue;
                }

                char codeHex[24 * 3 + 1]{};
                for (int j = 0; j < 24; ++j)
                {
                    sprintf_s(codeHex + j * 3, sizeof(codeHex) - j * 3, "%02X ", site.CodeWindow[j]);
                }

                char message[1536]{};
                sprintf_s(
                    message,
                    "WATCH_SITE field=%s tid=%lu EIP_after=0x%08lX RVA_after=0x%08lX hits=%ld "
                    "EAX=%08lX EBX=%08lX ECX=%08lX EDX=%08lX ESI=%08lX EDI=%08lX EBP=%08lX ESP=%08lX "
                    "EFLAGS=%08lX bytes_at_EIP_minus_16=%s",
                    field == 0 ? "P1_COM" : "P2_COM",
                    static_cast<unsigned long>(site.ThreadId),
                    static_cast<unsigned long>(site.EipAfter),
                    static_cast<unsigned long>(site.RvaAfter),
                    static_cast<long>(InterlockedCompareExchange(&site.Count, 0, 0)),
                    static_cast<unsigned long>(site.Eax),
                    static_cast<unsigned long>(site.Ebx),
                    static_cast<unsigned long>(site.Ecx),
                    static_cast<unsigned long>(site.Edx),
                    static_cast<unsigned long>(site.Esi),
                    static_cast<unsigned long>(site.Edi),
                    static_cast<unsigned long>(site.Ebp),
                    static_cast<unsigned long>(site.Esp),
                    static_cast<unsigned long>(site.EFlags),
                    codeHex);
                AppendTraceLog(message);
            }
        }
    }

    DWORD SuspendOtherThreadsForPatch(HANDLE* handles, DWORD capacity, bool* safeToPatch, DWORD targetAddress, DWORD patchLength)
    {
        if (handles == nullptr || safeToPatch == nullptr || capacity == 0) return 0;
        *safeToPatch = false;
        const DWORD processId = GetCurrentProcessId();
        const DWORD currentThreadId = GetCurrentThreadId();
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return 0;

        DWORD count = 0;
        bool safe = true;
        THREADENTRY32 entry{};
        entry.dwSize = sizeof(entry);
        if (!Thread32First(snapshot, &entry))
        {
            CloseHandle(snapshot);
            *safeToPatch = false;
            return 0;
        }
        {
            do
            {
                if (entry.th32OwnerProcessID == processId && entry.th32ThreadID != currentThreadId)
                {
                    if (count >= capacity)
                    {
                        safe = false;
                        break;
                    }
                    HANDLE thread = OpenThread(
                        THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
                        FALSE, entry.th32ThreadID);
                    if (thread == nullptr)
                    {
                        // A thread can disappear between the snapshot and OpenThread.
                        if (GetLastError() != ERROR_INVALID_PARAMETER) safe = false;
                    }
                    else
                    {
                        if (SuspendThread(thread) == static_cast<DWORD>(-1))
                        {
                            CloseHandle(thread);
                            safe = false;
                        }
                        else
                        {
                            handles[count++] = thread;
                            CONTEXT context{};
                            context.ContextFlags = CONTEXT_CONTROL;
                            if (!GetThreadContext(thread, &context))
                                safe = false;
                            else
                            {
                                if (context.Eip >= targetAddress && context.Eip < targetAddress + patchLength)
                                    safe = false; // Do not rewrite a CALL while a thread is inside its five bytes.
                            }
                        }
                    }
                }
                entry.dwSize = sizeof(entry);
            } while (safe && Thread32Next(snapshot, &entry));
        }
        CloseHandle(snapshot);
        *safeToPatch = safe;
        return count;
    }

    void ResumeStage17Threads(HANDLE* handles, DWORD count)
    {
        if (handles == nullptr) return;
        for (DWORD i = 0; i < count; ++i)
        {
            if (handles[i] != nullptr)
            {
                ResumeThread(handles[i]);
                CloseHandle(handles[i]);
                handles[i] = nullptr;
            }
        }
    }

    bool InstallStage17Bit113SetterCallsiteHook()
    {
        if (InterlockedCompareExchange(&g_stage17Bit113SetterInstalled, 0, 0) != 0)
            return true;

        if (g_gameBase == 0 ||
            g_gameImageSize < kStage17Bit113SetterCallRva + 5u ||
            kStage17Bit113HelperRva >= g_gameImageSize)
        {
            AppendStage17Bit113Log("ERROR: AALib image does not contain callsite RVA 0x000C6502.");
            return false;
        }

        BYTE* target = reinterpret_cast<BYTE*>(g_gameBase + kStage17Bit113SetterCallRva);
        if (std::memcmp(target, kStage17Bit113ExpectedCall, sizeof(kStage17Bit113ExpectedCall)) != 0)
        {
            AppendStage17Bit113Log("ERROR: bytes at RVA 0x000C6502 do not match the verified CALL-to-0x40FAA0 signature; refusing to patch.");
            return false;
        }

        std::int32_t originalRelative = 0;
        std::memcpy(&originalRelative, target + 1, sizeof(originalRelative));
        const std::intptr_t originalDestination =
            reinterpret_cast<std::intptr_t>(target + 5) + originalRelative;
        if (originalDestination != static_cast<std::intptr_t>(g_gameBase + kStage17Bit113HelperRva))
        {
            AppendStage17Bit113Log("ERROR: RVA 0x000C6502 no longer calls the generic bitset setter RVA 0x0000FAA0; refusing to patch.");
            return false;
        }

        BYTE patch[5]{};
        const std::int64_t relative =
            static_cast<std::int64_t>(reinterpret_cast<std::uintptr_t>(&ArcanaStage17Bit113SetterCallHook)) -
            (static_cast<std::int64_t>(reinterpret_cast<std::uintptr_t>(target)) + 5);
        if (relative < INT32_MIN || relative > INT32_MAX)
        {
            AppendStage17Bit113Log("ERROR: Stage17 hook is outside the rel32 CALL range.");
            return false;
        }
        patch[0] = 0xE8;
        const std::int32_t rel32 = static_cast<std::int32_t>(relative);
        std::memcpy(patch + 1, &rel32, sizeof(rel32));

        HANDLE patchMutex = CreateMutexW(nullptr, FALSE, L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
        if (patchMutex == nullptr)
        {
            AppendStage17Bit113Log("ERROR: could not create the shared patch-install mutex; callsite was not patched.");
            return false;
        }
        const DWORD mutexWait = WaitForSingleObject(patchMutex, INFINITE);
        if (mutexWait != WAIT_OBJECT_0 && mutexWait != WAIT_ABANDONED)
        {
            AppendStage17Bit113Log("ERROR: could not acquire the shared patch-install mutex; callsite was not patched.");
            CloseHandle(patchMutex);
            return false;
        }

        HANDLE suspended[512]{};
        bool safeToPatch = false;
        const DWORD suspendedCount = SuspendOtherThreadsForPatch(
            suspended, static_cast<DWORD>(sizeof(suspended) / sizeof(suspended[0])), &safeToPatch,
            g_gameBase + kStage17Bit113SetterCallRva, 5u);
        if (!safeToPatch)
        {
            ResumeStage17Threads(suspended, suspendedCount);
            ReleaseMutex(patchMutex);
            CloseHandle(patchMutex);
            AppendStage17Bit113Log("WARNING: could not safely suspend all process threads, or a thread was inside the five-byte callsite; hook was not installed.");
            return false;
        }

        DWORD oldProtect = 0;
        if (!VirtualProtect(target, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            ResumeStage17Threads(suspended, suspendedCount);
            ReleaseMutex(patchMutex);
            CloseHandle(patchMutex);
            AppendStage17Bit113Log("ERROR: VirtualProtect failed for RVA 0x000C6502.");
            return false;
        }

        g_Stage17Bit113SetterTarget = static_cast<DWORD>(originalDestination);
        std::memcpy(target, patch, sizeof(patch));
        FlushInstructionCache(GetCurrentProcess(), target, sizeof(patch));
        DWORD ignored = 0;
        VirtualProtect(target, sizeof(patch), oldProtect, &ignored);
        InterlockedExchange(&g_stage17Bit113SetterInstalled, 1);

        ResumeStage17Threads(suspended, suspendedCount);
        ReleaseMutex(patchMutex);
        CloseHandle(patchMutex);

        char message[512]{};
        sprintf_s(message,
            "Installed Stage17 passive callsite observer at RVA=0x%08lX; return RVA=0x%08lX; original helper RVA=0x%08lX; suspended=%lu thread(s). The hook tail-calls the original helper with the unchanged stack/return address.",
            static_cast<unsigned long>(kStage17Bit113SetterCallRva),
            static_cast<unsigned long>(kStage17Bit113SetterReturnRva),
            static_cast<unsigned long>(kStage17Bit113HelperRva),
            static_cast<unsigned long>(suspendedCount));
        AppendStage17Bit113Log(message);
        return true;
    }

    bool InstallStage18Bit113GateResultHook()
    {
        if (InterlockedCompareExchange(&g_stage18Bit113GateInstalled, 0, 0) != 0)
            return true;

        if (g_gameBase == 0 ||
            g_gameImageSize < kStage18Bit113GateBranchRva + sizeof(kStage18ExpectedBranchBytes) ||
            g_gameImageSize < kStage18SecondaryTableRva + 0x100u * sizeof(DWORD))
        {
            AppendStage18Bit113GateLog("ERROR: AALib image does not contain the Stage18 observer or state-table selector range.");
            return false;
        }

        BYTE* helperCall = reinterpret_cast<BYTE*>(g_gameBase + kStage18Bit113GateCallRva);
        BYTE* helperArguments = helperCall - sizeof(kStage18ExpectedHelperArguments);
        if (std::memcmp(helperArguments, kStage18ExpectedHelperArguments, sizeof(kStage18ExpectedHelperArguments)) != 0)
        {
            AppendStage18Bit113GateLog("ERROR: argument setup before RVA 0x000C64A0 does not match push 0x19 / push EAX / push ESI; refusing to patch.");
            return false;
        }
        if (std::memcmp(helperCall, kStage18ExpectedHelperCall, sizeof(kStage18ExpectedHelperCall)) != 0)
        {
            AppendStage18Bit113GateLog("ERROR: bytes at RVA 0x000C64A0 do not match the verified CALL-to-0x4F2FD0 signature; refusing to patch.");
            return false;
        }
        std::int32_t helperRelative = 0;
        std::memcpy(&helperRelative, helperCall + 1, sizeof(helperRelative));
        const std::intptr_t helperDestination =
            reinterpret_cast<std::intptr_t>(helperCall + 5) + helperRelative;
        if (helperDestination != static_cast<std::intptr_t>(g_gameBase + kStage18Bit113GateHelperRva))
        {
            AppendStage18Bit113GateLog("ERROR: RVA 0x000C64A0 no longer calls helper RVA 0x000F2FD0; refusing to patch.");
            return false;
        }

        BYTE* observerSite = reinterpret_cast<BYTE*>(g_gameBase + kStage18Bit113GateObserverRva);
        if (std::memcmp(observerSite, kStage18ExpectedObserverBytes, sizeof(kStage18ExpectedObserverBytes)) != 0)
        {
            AppendStage18Bit113GateLog("ERROR: bytes at RVA 0x000C64A5 do not match 'add esp,0x0C; test al,al'; refusing to patch.");
            return false;
        }
        BYTE* branchSite = reinterpret_cast<BYTE*>(g_gameBase + kStage18Bit113GateBranchRva);
        if (std::memcmp(branchSite, kStage18ExpectedBranchBytes, sizeof(kStage18ExpectedBranchBytes)) != 0)
        {
            AppendStage18Bit113GateLog("ERROR: branch at RVA 0x000C64AA is not the verified JNE +0x41; refusing to patch.");
            return false;
        }

        BYTE patch[5]{};
        const std::int64_t relative =
            static_cast<std::int64_t>(reinterpret_cast<std::uintptr_t>(&ArcanaStage18Bit113GateResultHook)) -
            (static_cast<std::int64_t>(reinterpret_cast<std::uintptr_t>(observerSite)) + 5);
        if (relative < INT32_MIN || relative > INT32_MAX)
        {
            AppendStage18Bit113GateLog("ERROR: Stage18 hook is outside the rel32 CALL range.");
            return false;
        }
        patch[0] = 0xE8;
        const std::int32_t rel32 = static_cast<std::int32_t>(relative);
        std::memcpy(patch + 1, &rel32, sizeof(rel32));

        HANDLE patchMutex = CreateMutexW(nullptr, FALSE, L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
        if (patchMutex == nullptr)
        {
            AppendStage18Bit113GateLog("ERROR: could not create the shared patch-install mutex; gate observer was not patched.");
            return false;
        }
        const DWORD mutexWait = WaitForSingleObject(patchMutex, INFINITE);
        if (mutexWait != WAIT_OBJECT_0 && mutexWait != WAIT_ABANDONED)
        {
            AppendStage18Bit113GateLog("ERROR: could not acquire the shared patch-install mutex; gate observer was not patched.");
            CloseHandle(patchMutex);
            return false;
        }

        HANDLE suspended[512]{};
        bool safeToPatch = false;
        const DWORD suspendedCount = SuspendOtherThreadsForPatch(
            suspended, static_cast<DWORD>(sizeof(suspended) / sizeof(suspended[0])), &safeToPatch,
            g_gameBase + kStage18Bit113GateObserverRva, 5u);
        if (!safeToPatch)
        {
            ResumeStage17Threads(suspended, suspendedCount);
            ReleaseMutex(patchMutex);
            CloseHandle(patchMutex);
            AppendStage18Bit113GateLog("WARNING: could not safely suspend all process threads, or a thread was inside the five-byte observer site; Stage18 hook was not installed.");
            return false;
        }

        DWORD oldProtect = 0;
        if (!VirtualProtect(observerSite, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            ResumeStage17Threads(suspended, suspendedCount);
            ReleaseMutex(patchMutex);
            CloseHandle(patchMutex);
            AppendStage18Bit113GateLog("ERROR: VirtualProtect failed for RVA 0x000C64A5.");
            return false;
        }

        g_Stage18Bit113GateContinuation = g_gameBase + kStage18Bit113GateContinuationRva;
        std::memcpy(observerSite, patch, sizeof(patch));
        FlushInstructionCache(GetCurrentProcess(), observerSite, sizeof(patch));
        DWORD ignored = 0;
        VirtualProtect(observerSite, sizeof(patch), oldProtect, &ignored);
        InterlockedExchange(&g_stage18Bit113GateInstalled, 1);

        ResumeStage17Threads(suspended, suspendedCount);
        ReleaseMutex(patchMutex);
        CloseHandle(patchMutex);

        char message[512]{};
        sprintf_s(message,
            "Installed Stage18 passive gate-result observer at RVA=0x%08lX after helper CALL RVA=0x%08lX (target helper RVA=0x%08lX, resume RVA=0x%08lX); suspended=%lu thread(s).",
            static_cast<unsigned long>(kStage18Bit113GateObserverRva),
            static_cast<unsigned long>(kStage18Bit113GateCallRva),
            static_cast<unsigned long>(kStage18Bit113GateHelperRva),
            static_cast<unsigned long>(kStage18Bit113GateContinuationRva),
            static_cast<unsigned long>(suspendedCount));
        AppendStage18Bit113GateLog(message);
        return true;
    }

    bool InstallStage19StateMapHook()
    {
        if (InterlockedCompareExchange(&g_stage19StateMapInstalled, 0, 0) != 0)
            return true;

        if (g_gameBase == 0 ||
            g_gameImageSize < kStage19StateMapBranchRva + sizeof(kStage19ExpectedBranchBytes) ||
            g_gameImageSize < kStage19StateMapObserverRva + sizeof(kStage19ExpectedObserverBytes))
        {
            AppendStage19StateMapLog("ERROR: AALib image does not contain the Stage19 state-map observer site.");
            return false;
        }

        BYTE* helperCall = reinterpret_cast<BYTE*>(g_gameBase + kStage19StateMapCallRva);
        BYTE* helperArguments = helperCall - sizeof(kStage19ExpectedArguments);
        if (std::memcmp(helperArguments, kStage19ExpectedArguments, sizeof(kStage19ExpectedArguments)) != 0)
        {
            AppendStage19StateMapLog("ERROR: argument setup before RVA 0x000F6C03 is not push ESI / push EDI; refusing to patch.");
            return false;
        }
        if (std::memcmp(helperCall, kStage19ExpectedHelperCall, sizeof(kStage19ExpectedHelperCall)) != 0)
        {
            AppendStage19StateMapLog("ERROR: CALL at RVA 0x000F6C03 does not match the verified 0x000F02C0 target; refusing to patch.");
            return false;
        }
        std::int32_t helperRelative = 0;
        std::memcpy(&helperRelative, helperCall + 1, sizeof(helperRelative));
        const std::intptr_t helperDestination =
            reinterpret_cast<std::intptr_t>(helperCall + 5) + helperRelative;
        if (helperDestination != static_cast<std::intptr_t>(g_gameBase + kStage19StateMapHelperRva))
        {
            AppendStage19StateMapLog("ERROR: RVA 0x000F6C03 no longer calls helper RVA 0x000F02C0; refusing to patch.");
            return false;
        }

        BYTE* observerSite = reinterpret_cast<BYTE*>(g_gameBase + kStage19StateMapObserverRva);
        if (std::memcmp(observerSite, kStage19ExpectedObserverBytes, sizeof(kStage19ExpectedObserverBytes)) != 0)
        {
            AppendStage19StateMapLog("ERROR: bytes at RVA 0x000F6C08 do not match ADD ESP,8; CMP EAX,5; refusing to patch.");
            return false;
        }
        BYTE* branchSite = reinterpret_cast<BYTE*>(g_gameBase + kStage19StateMapBranchRva);
        if (std::memcmp(branchSite, kStage19ExpectedBranchBytes, sizeof(kStage19ExpectedBranchBytes)) != 0)
        {
            AppendStage19StateMapLog("ERROR: branch at RVA 0x000F6C0E is not the verified JNE +0x0A; refusing to patch.");
            return false;
        }

        BYTE patch[sizeof(kStage19ExpectedObserverBytes)]{};
        const std::int64_t relative =
            static_cast<std::int64_t>(reinterpret_cast<std::uintptr_t>(&ArcanaStage19StateMapHook)) -
            (static_cast<std::int64_t>(reinterpret_cast<std::uintptr_t>(observerSite)) + 5);
        if (relative < INT32_MIN || relative > INT32_MAX)
        {
            AppendStage19StateMapLog("ERROR: Stage19 hook is outside the rel32 CALL range.");
            return false;
        }
        patch[0] = 0xE8;
        const std::int32_t rel32 = static_cast<std::int32_t>(relative);
        std::memcpy(patch + 1, &rel32, sizeof(rel32));
        patch[5] = 0x90; // The sixth overwritten byte is a NOP; the hook resumes at 0xF6C0E.

        HANDLE patchMutex = CreateMutexW(nullptr, FALSE, L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
        if (patchMutex == nullptr)
        {
            AppendStage19StateMapLog("ERROR: could not create the shared patch-install mutex; state-map observer was not patched.");
            return false;
        }
        const DWORD mutexWait = WaitForSingleObject(patchMutex, INFINITE);
        if (mutexWait != WAIT_OBJECT_0 && mutexWait != WAIT_ABANDONED)
        {
            AppendStage19StateMapLog("ERROR: could not acquire the shared patch-install mutex; state-map observer was not patched.");
            CloseHandle(patchMutex);
            return false;
        }

        HANDLE suspended[512]{};
        bool safeToPatch = false;
        const DWORD suspendedCount = SuspendOtherThreadsForPatch(
            suspended, static_cast<DWORD>(sizeof(suspended) / sizeof(suspended[0])),
            &safeToPatch, g_gameBase + kStage19StateMapObserverRva,
            static_cast<DWORD>(sizeof(patch)));
        if (!safeToPatch)
        {
            ResumeStage17Threads(suspended, suspendedCount);
            ReleaseMutex(patchMutex);
            CloseHandle(patchMutex);
            AppendStage19StateMapLog("WARNING: could not safely suspend all process threads, or a thread was inside the six-byte observer site; Stage19 hook was not installed.");
            return false;
        }

        DWORD oldProtect = 0;
        if (!VirtualProtect(observerSite, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            ResumeStage17Threads(suspended, suspendedCount);
            ReleaseMutex(patchMutex);
            CloseHandle(patchMutex);
            AppendStage19StateMapLog("ERROR: VirtualProtect failed for RVA 0x000F6C08.");
            return false;
        }

        g_Stage19StateMapContinuation = g_gameBase + kStage19StateMapContinuationRva;
        std::memcpy(observerSite, patch, sizeof(patch));
        FlushInstructionCache(GetCurrentProcess(), observerSite, sizeof(patch));
        DWORD ignored = 0;
        VirtualProtect(observerSite, sizeof(patch), oldProtect, &ignored);
        InterlockedExchange(&g_stage19StateMapInstalled, 1);

        ResumeStage17Threads(suspended, suspendedCount);
        ReleaseMutex(patchMutex);
        CloseHandle(patchMutex);

        char message[512]{};
        sprintf_s(message,
            "Installed Stage19 passive state-map observer at RVA=0x%08lX after helper CALL RVA=0x%08lX (target helper RVA=0x%08lX, resume RVA=0x%08lX); patchLength=%lu; suspended=%lu thread(s).",
            static_cast<unsigned long>(kStage19StateMapObserverRva),
            static_cast<unsigned long>(kStage19StateMapCallRva),
            static_cast<unsigned long>(kStage19StateMapHelperRva),
            static_cast<unsigned long>(kStage19StateMapContinuationRva),
            static_cast<unsigned long>(sizeof(patch)),
            static_cast<unsigned long>(suspendedCount));
        AppendStage19StateMapLog(message);
        return true;
    }

    bool InstallStage20Bit113ObjectGuardHook()
    {
        if (InterlockedCompareExchange(&g_stage20ObjectGuardInstalled, 0, 0) != 0)
            return true;

        if (g_gameBase == 0 ||
            g_gameImageSize < kStage20ObjectGuardBranchRva + sizeof(kStage20ExpectedBranchBytes))
        {
            AppendStage20ObjectGuardLog("ERROR: AALib image does not contain the Stage20 object-guard callsite or following branch.");
            return false;
        }

        BYTE* callSite = reinterpret_cast<BYTE*>(g_gameBase + kStage20ObjectGuardCallRva);
        if (std::memcmp(callSite - sizeof(kStage20ExpectedArgumentSetup), kStage20ExpectedArgumentSetup, sizeof(kStage20ExpectedArgumentSetup)) != 0)
        {
            AppendStage20ObjectGuardLog("ERROR: setup before RVA 0x000C64F1 does not match MOV ECX,[EBX]; PUSH 2; refusing to patch.");
            return false;
        }

        if (std::memcmp(callSite, kStage20ExpectedHelperCall, sizeof(kStage20ExpectedHelperCall)) != 0)
        {
            AppendStage20ObjectGuardLog("ERROR: CALL at RVA 0x000C64F1 does not match the verified 0x00495B00 helper; refusing to patch.");
            return false;
        }
        std::int32_t helperRelative = 0;
        std::memcpy(&helperRelative, callSite + 1, sizeof(helperRelative));
        const std::intptr_t helperDestination =
            reinterpret_cast<std::intptr_t>(callSite + 5) + helperRelative;
        if (helperDestination != static_cast<std::intptr_t>(g_gameBase + kStage20ObjectGuardHelperRva))
        {
            AppendStage20ObjectGuardLog("ERROR: RVA 0x000C64F1 no longer calls helper RVA 0x000095B00; refusing to patch.");
            return false;
        }

        BYTE* testSite = reinterpret_cast<BYTE*>(g_gameBase + kStage20ObjectGuardTestRva);
        if (std::memcmp(testSite, kStage20ExpectedTestBytes, sizeof(kStage20ExpectedTestBytes)) != 0)
        {
            AppendStage20ObjectGuardLog("ERROR: original TEST AL,AL at RVA 0x000C64F6 does not match; refusing to patch.");
            return false;
        }
        BYTE* branchSite = reinterpret_cast<BYTE*>(g_gameBase + kStage20ObjectGuardBranchRva);
        if (std::memcmp(branchSite, kStage20ExpectedBranchBytes, sizeof(kStage20ExpectedBranchBytes)) != 0)
        {
            AppendStage20ObjectGuardLog("ERROR: original JNE at RVA 0x000C64F8 does not match; refusing to patch.");
            return false;
        }

        BYTE patch[5]{};
        const std::int64_t relative =
            static_cast<std::int64_t>(reinterpret_cast<std::uintptr_t>(&ArcanaStage20Bit113ObjectGuardHook)) -
            (static_cast<std::int64_t>(reinterpret_cast<std::uintptr_t>(callSite)) + 5);
        if (relative < INT32_MIN || relative > INT32_MAX)
        {
            AppendStage20ObjectGuardLog("ERROR: Stage20 wrapper is outside the rel32 CALL range.");
            return false;
        }
        patch[0] = 0xE8;
        const std::int32_t rel32 = static_cast<std::int32_t>(relative);
        std::memcpy(patch + 1, &rel32, sizeof(rel32));

        HANDLE patchMutex = CreateMutexW(nullptr, FALSE, L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
        if (patchMutex == nullptr)
        {
            AppendStage20ObjectGuardLog("ERROR: could not create the shared patch-install mutex; Stage20 wrapper was not installed.");
            return false;
        }
        const DWORD mutexWait = WaitForSingleObject(patchMutex, INFINITE);
        if (mutexWait != WAIT_OBJECT_0 && mutexWait != WAIT_ABANDONED)
        {
            AppendStage20ObjectGuardLog("ERROR: could not acquire the shared patch-install mutex; Stage20 wrapper was not installed.");
            CloseHandle(patchMutex);
            return false;
        }

        HANDLE suspended[512]{};
        bool safeToPatch = false;
        const DWORD suspendedCount = SuspendOtherThreadsForPatch(
            suspended, static_cast<DWORD>(sizeof(suspended) / sizeof(suspended[0])),
            &safeToPatch, g_gameBase + kStage20ObjectGuardCallRva,
            static_cast<DWORD>(sizeof(patch)));
        if (!safeToPatch)
        {
            ResumeStage17Threads(suspended, suspendedCount);
            ReleaseMutex(patchMutex);
            CloseHandle(patchMutex);
            AppendStage20ObjectGuardLog("WARNING: could not safely suspend all process threads, or a thread was inside the five-byte CALL site; Stage20 wrapper was not installed.");
            return false;
        }

        DWORD oldProtect = 0;
        if (!VirtualProtect(callSite, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            ResumeStage17Threads(suspended, suspendedCount);
            ReleaseMutex(patchMutex);
            CloseHandle(patchMutex);
            AppendStage20ObjectGuardLog("ERROR: VirtualProtect failed for RVA 0x000C64F1.");
            return false;
        }

        g_Stage20Bit113ObjectGuardTarget = g_gameBase + kStage20ObjectGuardHelperRva;
        std::memcpy(callSite, patch, sizeof(patch));
        FlushInstructionCache(GetCurrentProcess(), callSite, sizeof(patch));
        DWORD ignored = 0;
        VirtualProtect(callSite, sizeof(patch), oldProtect, &ignored);
        InterlockedExchange(&g_stage20ObjectGuardInstalled, 1);

        ResumeStage17Threads(suspended, suspendedCount);
        ReleaseMutex(patchMutex);
        CloseHandle(patchMutex);

        char message[512]{};
        sprintf_s(message,
            "Installed Stage20 observational wrapper at CALL RVA=0x%08lX; original helper RVA=0x%08lX; resume RVA=0x%08lX; following TEST/JNE left untouched; suspended=%lu thread(s).",
            static_cast<unsigned long>(kStage20ObjectGuardCallRva),
            static_cast<unsigned long>(kStage20ObjectGuardHelperRva),
            static_cast<unsigned long>(kStage20ObjectGuardContinuationRva),
            static_cast<unsigned long>(suspendedCount));
        AppendStage20ObjectGuardLog(message);
        return true;
    }

    DWORD WINAPI FieldTraceThread(void*)
    {
        // Emit a Stage17-specific startup marker before any validation or early return.
        // Its presence proves that this exact Stage17 AITrace.cpp reached runtime.
        AppendStage17Bit113Log(kStage17BuildMarker);
        AppendStage17Bit113Log("STAGE17_INIT: FieldTraceThread entered.");
        AppendStage18Bit113GateLog(kStage18BuildMarker);
        AppendStage18Bit113GateLog("STAGE18_INIT: FieldTraceThread entered.");
        AppendStage19StateMapLog(kStage19BuildMarker);
        AppendStage19StateMapLog("STAGE19_INIT: FieldTraceThread entered.");
        AppendStage20ObjectGuardLog(kStage20BuildMarker);
        AppendStage20ObjectGuardLog("STAGE20_INIT: FieldTraceThread entered.");

        const int controlTraceEnabled = ReadIniInt(L"AIResearch", L"EnableControlFieldTrace", 0);
        const int bit17TraceEnabled = ReadIniInt(L"AIResearch", L"EnablePlayerBit17WriteTrace", 0);
        const int stage17SetterEnabled = ReadIniInt(L"AIResearch", L"EnableStage17Bit113SetterCallTrace", 0);
        const int stage18GateEnabled = ReadIniInt(L"AIResearch", L"EnableStage18Bit113GateTrace", 0);
        const int stage19StateMapEnabled = ReadIniInt(L"AIResearch", L"EnableStage19StateMapTrace", 0);
        const int stage20ObjectGuardEnabled = ReadIniInt(L"AIResearch", L"EnableStage20Bit113ObjectGuardTrace", 0);
        g_stage17Bit113SetterTrace = stage17SetterEnabled != 0;
        g_stage18Bit113GateTrace = stage18GateEnabled != 0;
        g_stage19StateMapTrace = stage19StateMapEnabled != 0;
        g_stage20ObjectGuardTrace = stage20ObjectGuardEnabled != 0;

        char configMessage[256]{};
        sprintf_s(configMessage,
            "STAGE17_CONFIG: EnableControlFieldTrace=%d EnablePlayerBit17WriteTrace=%d EnableStage17Bit113SetterCallTrace=%d.",
            controlTraceEnabled, bit17TraceEnabled, stage17SetterEnabled);
        AppendStage17Bit113Log(configMessage);
        char stage18ConfigMessage[192]{};
        sprintf_s(stage18ConfigMessage,
            "STAGE18_CONFIG: EnableStage18Bit113GateTrace=%d EnableStage17Bit113SetterCallTrace=%d EnablePlayerBit17WriteTrace=%d.",
            stage18GateEnabled, stage17SetterEnabled, bit17TraceEnabled);
        AppendStage18Bit113GateLog(stage18ConfigMessage);
        char stage19ConfigMessage[160]{};
        sprintf_s(stage19ConfigMessage,
            "STAGE19_CONFIG: EnableStage19StateMapTrace=%d EnableStage18Bit113GateTrace=%d EnableStage17Bit113SetterCallTrace=%d.",
            stage19StateMapEnabled, stage18GateEnabled, stage17SetterEnabled);
        AppendStage19StateMapLog(stage19ConfigMessage);
        char stage20ConfigMessage[192]{};
        sprintf_s(stage20ConfigMessage,
            "STAGE20_CONFIG: EnableStage20Bit113ObjectGuardTrace=%d EnableStage19StateMapTrace=%d EnableStage18Bit113GateTrace=%d.",
            stage20ObjectGuardEnabled, stage19StateMapEnabled, stage18GateEnabled);
        AppendStage20ObjectGuardLog(stage20ConfigMessage);

        if (controlTraceEnabled != 0 && bit17TraceEnabled != 0)
        {
            AppendStage17Bit113Log("ERROR: conflicting DR0/DR1 modes; FieldTraceThread is stopping before Stage17 hook installation.");
            AppendStage18Bit113GateLog("ERROR: conflicting DR0/DR1 modes; FieldTraceThread stopped before Stage18 hook installation.");
            AppendTraceLog("ERROR: EnableControlFieldTrace and EnablePlayerBit17WriteTrace both use DR0/DR1; disable one mode. No DR0/DR1 watchpoints were installed.");
            AppendPlayerBit17TraceLog("ERROR: conflicting DR0/DR1 modes. Set EnableControlFieldTrace=0 and keep EnablePlayerBit17WriteTrace=1.");
            return 0;
        }
        g_playerBit17WriteTrace = bit17TraceEnabled != 0;
        g_watchDr7Configuration = g_playerBit17WriteTrace
            ? kPlayerBit17WriteTraceDr7 : kControlTraceDr7;
        bool watchpointsEnabled = controlTraceEnabled != 0 || bit17TraceEnabled != 0;
        const int enabled = watchpointsEnabled || stage17SetterEnabled != 0 ||
            stage18GateEnabled != 0 || stage19StateMapEnabled != 0 ||
            stage20ObjectGuardEnabled != 0;
        int interval = ReadIniInt(L"AIResearch", L"ThreadScanIntervalMs", static_cast<int>(kDefaultScanIntervalMs));
        if (interval < static_cast<int>(kMinimumScanIntervalMs)) interval = static_cast<int>(kMinimumScanIntervalMs);
        if (interval > static_cast<int>(kMaximumScanIntervalMs)) interval = static_cast<int>(kMaximumScanIntervalMs);
        g_scanIntervalMs = static_cast<DWORD>(interval);

        if (enabled == 0)
        {
            AppendTraceLog("AI field trace is disabled in ArcanaAI.ini. No hardware watchpoints were installed.");
            AppendPlayerBit17TraceLog("Stage 16 player bit-17 write trace is disabled in ArcanaAI.ini.");
            AppendStage17Bit113Log("Stage 17 bit-113 setter callsite trace is disabled in ArcanaAI.ini.");
            AppendStage18Bit113GateLog("Stage 18 bit-113 gate trace is disabled in ArcanaAI.ini.");
            AppendStage19StateMapLog("Stage 19 state-map trace is disabled in ArcanaAI.ini.");
            AppendStage20ObjectGuardLog("Stage 20 object-guard trace is disabled in ArcanaAI.ini.");
            return 0;
        }

        for (int attempt = 0; attempt < 300 && g_gameModule == nullptr; ++attempt)
        {
            g_gameModule = GetModuleHandleW(L"AALib.exe");
            if (g_gameModule == nullptr)
            {
                Sleep(100);
            }
        }

        if (g_gameModule == nullptr)
        {
            AppendTraceLog("ERROR: AALib.exe was not found; AI field trace stopped.");
            if (g_playerBit17WriteTrace) AppendPlayerBit17TraceLog("ERROR: AALib.exe was not found; Stage16 trace stopped.");
            AppendStage17Bit113Log("ERROR: AALib.exe was not found within the module wait window; Stage17 hook installation was skipped.");
            AppendStage18Bit113GateLog("ERROR: AALib.exe was not found within the module wait window; Stage18 hook installation was skipped.");
            AppendStage19StateMapLog("ERROR: AALib.exe was not found within the module wait window; Stage19 hook installation was skipped.");
            AppendStage20ObjectGuardLog("ERROR: AALib.exe was not found within the module wait window; Stage20 hook installation was skipped.");
            return 0;
        }

        g_gameBase = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(g_gameModule));
        g_gameImageSize = GetImageSize32(g_gameModule);
        const DWORD p1PlayerRva = kP1ControlRva - kPlayerControlOffset;
        const DWORD p2PlayerRva = kP2ControlRva - kPlayerControlOffset;
        const DWORD p1WatchRva = g_playerBit17WriteTrace
            ? p1PlayerRva + kPlayerBit17Offset : kP1ControlRva;
        const DWORD p2WatchRva = g_playerBit17WriteTrace
            ? p2PlayerRva + kPlayerBit17Offset : kP2ControlRva;
        if (g_gameImageSize < sizeof(DWORD) ||
            p1WatchRva > g_gameImageSize - sizeof(DWORD) ||
            p2WatchRva > g_gameImageSize - sizeof(DWORD))
        {
            AppendTraceLog("ERROR: the loaded AALib.exe image does not contain the selected P1/P2 watchpoint RVAs.");
            AppendPlayerBit17TraceLog("ERROR: the loaded AALib.exe image does not contain both player+0x3B84 watchpoint RVAs.");
            AppendStage17Bit113Log("ERROR: loaded AALib.exe image is too small for the configured P1/P2 trace RVAs; Stage17 initialization stopped.");
            AppendStage18Bit113GateLog("ERROR: loaded AALib.exe image is too small for the configured P1/P2 trace RVAs; Stage18 initialization stopped.");
            AppendStage19StateMapLog("ERROR: loaded AALib.exe image is too small for the configured P1/P2 trace RVAs; Stage19 initialization stopped.");
            AppendStage20ObjectGuardLog("ERROR: loaded AALib.exe image is too small for the configured P1/P2 trace RVAs; Stage20 initialization stopped.");
            return 0;
        }

        g_watchP1 = g_gameBase + p1WatchRva;
        g_watchP2 = g_gameBase + p2WatchRva;
        if (g_playerBit17WriteTrace)
        {
            DWORD initialP1 = 0, initialP2 = 0;
            const bool p1Read = ReadDwordAtAddress(g_watchP1, &initialP1);
            const bool p2Read = ReadDwordAtAddress(g_watchP2, &initialP2);
            if (!p1Read || !p2Read)
            {
                AppendPlayerBit17TraceLog("ERROR: could not read initial P1/P2 player+0x3B84 values; refusing to start because the old-value shadow would be invalid.");
                AppendStage17Bit113Log("ERROR: initial P1/P2 field read failed; Stage17 initialization stopped before callsite validation.");
                AppendStage18Bit113GateLog("ERROR: initial P1/P2 field read failed; Stage18 initialization stopped before observer validation.");
                AppendStage19StateMapLog("ERROR: initial P1/P2 field read failed; FieldTraceThread stopped before Stage19 observer validation.");
                AppendStage20ObjectGuardLog("ERROR: initial P1/P2 field read failed; FieldTraceThread stopped before Stage20 observer validation.");
                return 0;
            }
            g_playerBit17InitialValue[0] = initialP1;
            g_playerBit17InitialValue[1] = initialP2;
            InterlockedExchange(&g_playerBit17ShadowValue[0], static_cast<LONG>(initialP1));
            InterlockedExchange(&g_playerBit17ShadowValue[1], static_cast<LONG>(initialP2));
            InterlockedExchange(&g_playerBit17ShadowValid[0], 1);
            InterlockedExchange(&g_playerBit17ShadowValid[1], 1);
        }

        // Wait for the COMvsCOM writer only when DR0/DR1 data watchpoints are requested.
        // Stage17 callsite observation does not use debug registers and can run alone.
        DWORD comThreadId = 0;
        if (watchpointsEnabled)
        {
            for (int attempt = 0; attempt < 100 && comThreadId == 0; ++attempt)
            {
                comThreadId = GetComVsComThreadId();
                if (comThreadId == 0) Sleep(20);
            }
            if (comThreadId == 0)
            {
                AppendTraceLog("ERROR: COMvsCOM worker thread ID was not published; DR0/DR1 field trace stopped.");
                if (g_playerBit17WriteTrace) AppendPlayerBit17TraceLog("ERROR: COMvsCOM worker thread ID was not published; Stage16 watchpoints stopped.");
                watchpointsEnabled = false;
            }
        }

        if (watchpointsEnabled && g_vectoredHandler == nullptr)
        {
            g_vectoredHandler = AddVectoredExceptionHandler(1, TraceExceptionHandler);
        }
        if (watchpointsEnabled && g_vectoredHandler == nullptr)
        {
            AppendTraceLog("ERROR: AddVectoredExceptionHandler failed; DR0/DR1 field trace stopped.");
            if (g_playerBit17WriteTrace) AppendPlayerBit17TraceLog("ERROR: AddVectoredExceptionHandler failed; Stage16 watchpoints stopped.");
            watchpointsEnabled = false;
        }

        if (g_stage17Bit113SetterTrace)
        {
            if (InstallStage17Bit113SetterCallsiteHook())
                AppendStage17Bit113Log("Stage17 trace active. Captures the original bit index, player/context registers, related object and nearby player fields, caller stack, QPC and code bytes before tail-calling the original setter.");
            else
                AppendStage17Bit113Log("ERROR: Stage17 callsite observer was not installed; inspect the preceding validation/patch message.");
        }
        else
            AppendStage17Bit113Log("Stage17 bit-113 setter callsite trace is disabled in ArcanaAI.ini.");

        if (g_stage18Bit113GateTrace)
        {
            if (InstallStage18Bit113GateResultHook())
                AppendStage18Bit113GateLog("Stage18 trace active. Captures the 0x4F2FD0 AL result, state index/key, player fields and derived state-table byte before resuming at the original JNE.");
            else
                AppendStage18Bit113GateLog("ERROR: Stage18 gate-result observer was not installed; inspect the preceding validation/patch message.");
        }
        else
            AppendStage18Bit113GateLog("Stage18 bit-113 gate trace is disabled in ArcanaAI.ini.");

        if (g_stage19StateMapTrace)
        {
            if (InstallStage19StateMapHook())
                AppendStage19StateMapLog("Stage19 trace active. Captures the 0x4F02C0 mapping result, raw player state code, associated WORD flags and the state index derived by the caller.");
            else
                AppendStage19StateMapLog("ERROR: Stage19 state-map observer was not installed; inspect the preceding validation/patch message.");
        }
        else
            AppendStage19StateMapLog("Stage19 state-map trace is disabled in ArcanaAI.ini.");

        if (g_stage20ObjectGuardTrace)
        {
            if (InstallStage20Bit113ObjectGuardHook())
                AppendStage20ObjectGuardLog("Stage20 trace active. Wraps the original 0x495B00 object bit-2 helper, records its real AL and checked object DWORD, then returns through the original caller TEST/JNE.");
            else
                AppendStage20ObjectGuardLog("ERROR: Stage20 object-guard wrapper was not installed; inspect the preceding validation/patch message.");
        }
        else
            AppendStage20ObjectGuardLog("Stage20 object-guard trace is disabled in ArcanaAI.ini.");

        char startMessage[768]{};
        if (g_playerBit17WriteTrace && watchpointsEnabled)
        {
            sprintf_s(
                startMessage,
                "STAGE16 active. Write-only DR0/DR1 watchpoints cover exact P1/P2 DWORD player+0x3B84: "
                "P1=0x%08lX (RVA 0x%08lX, initial=0x%08lX), P2=0x%08lX (RVA 0x%08lX, initial=0x%08lX). "
                "DR7 config=0x%08lX; scan=%lu ms; COMvsCOM writer TID=%lu excluded; external DR0/DR1 owners are not overwritten.",
                static_cast<unsigned long>(g_watchP1), static_cast<unsigned long>(p1WatchRva),
                static_cast<unsigned long>(g_playerBit17InitialValue[0]),
                static_cast<unsigned long>(g_watchP2), static_cast<unsigned long>(p2WatchRva),
                static_cast<unsigned long>(g_playerBit17InitialValue[1]),
                static_cast<unsigned long>(g_watchDr7Configuration),
                static_cast<unsigned long>(g_scanIntervalMs), static_cast<unsigned long>(comThreadId));
            AppendPlayerBit17TraceLog(startMessage);
            AppendPlayerBit17TraceLog("Each record is a post-write hardware-breakpoint snapshot. EIP/RVA is after the writing instruction; bytes_at_EIP_minus_16 allows offline instruction decoding. oldValue is a best-effort shadow of the last observed field value; shadowInitialized=1 means only that the initial baseline read succeeded, not that no write was missed on an uninstrumented thread.");
        }
        else if (watchpointsEnabled)
        {
            sprintf_s(
                startMessage,
                "AI control-field trace active. Watching reads/writes at P1=0x%08lX (RVA 0x%08lX), "
                "P2=0x%08lX (RVA 0x%08lX). Thread scan interval=%lu ms. COMvsCOM writer TID=%lu is excluded; existing DR0/DR1 owners are not overwritten.",
                static_cast<unsigned long>(g_watchP1),
                static_cast<unsigned long>(kP1ControlRva),
                static_cast<unsigned long>(g_watchP2),
                static_cast<unsigned long>(kP2ControlRva),
                static_cast<unsigned long>(g_scanIntervalMs),
                static_cast<unsigned long>(comThreadId));
            AppendTraceLog(startMessage);
        }
        else
        {
            AppendTraceLog("DR0/DR1 field watchpoints are disabled or unavailable; Stage17 callsite observer may still run independently.");
        }

        if (g_playerBit17WriteTrace && !watchpointsEnabled)
            AppendPlayerBit17TraceLog("WARNING: Stage16 write watchpoints were requested but could not be enabled; Stage17 callsite trace may still run.");

        for (;;)
        {
            const DWORD newlyInstalled = watchpointsEnabled
                ? InstallWatchpointsOnCurrentThreads() : 0;
            if (newlyInstalled != 0)
            {
                char message[256]{};
                sprintf_s(message, "Hardware watchpoint install batch: %lu new game thread(s).",
                    static_cast<unsigned long>(newlyInstalled));
                AppendTraceLog(message);
                if (g_playerBit17WriteTrace) AppendPlayerBit17TraceLog(message);
                InterlockedExchange(&g_firstInstallLog, 1);
            }
            else if (watchpointsEnabled &&
                InterlockedCompareExchange(&g_firstInstallLog, 0, 0) == 0 &&
                InterlockedCompareExchange(&g_noThreadInstallLog, 1, 0) == 0)
            {
                AppendTraceLog("No eligible game thread accepted DR0/DR1 yet. Threads using those debug-register slots are left untouched; scanning will continue.");
                if (g_playerBit17WriteTrace)
                    AppendPlayerBit17TraceLog("WARNING: no eligible game thread has accepted the P1/P2 write watchpoints yet.");
            }

            if (watchpointsEnabled)
            {
                if (g_playerBit17WriteTrace)
                    FlushPlayerBit17WriteRecords();
                else
                    FlushNewSites();
            }
            if (g_stage17Bit113SetterTrace)
                FlushStage17Bit113SetterRecords();
            if (g_stage18Bit113GateTrace)
                FlushStage18Bit113GateRecords();
            if (g_stage19StateMapTrace)
                FlushStage19StateMapRecords();
            if (g_stage20ObjectGuardTrace)
                FlushStage20Bit113ObjectGuardRecords();
            Sleep(g_scanIntervalMs);
        }
    }

    BOOL CALLBACK StartTraceWorker(PINIT_ONCE, PVOID, PVOID*)
    {
        HANDLE thread = CreateThread(nullptr, 0, FieldTraceThread, nullptr, 0, nullptr);
        if (thread == nullptr)
        {
            AppendTraceLog("ERROR: failed to create AI field-trace worker thread.");
            AppendStage17Bit113Log("ERROR: failed to create the FieldTraceThread worker; Stage17 did not initialize.");
            AppendStage18Bit113GateLog("ERROR: failed to create the FieldTraceThread worker; Stage18 did not initialize.");
            AppendStage19StateMapLog("ERROR: failed to create the FieldTraceThread worker; Stage19 did not initialize.");
            AppendStage20ObjectGuardLog("ERROR: failed to create the FieldTraceThread worker; Stage20 did not initialize.");
            return TRUE;
        }
        CloseHandle(thread);
        return TRUE;
    }
}

extern "C" void __cdecl ArcanaLogStage17Bit113SetterCall(const DWORD* savedContext)
{
    QueueStage17Bit113SetterRecord(savedContext);
}

extern "C" void __cdecl ArcanaLogStage18Bit113GateResult(const DWORD* savedContext)
{
    QueueStage18Bit113GateRecord(savedContext);
}

extern "C" void __cdecl ArcanaLogStage19StateMap(const DWORD* savedContext)
{
    QueueStage19StateMapRecord(savedContext);
}

extern "C" void __cdecl ArcanaLogStage20Bit113ObjectGuard(const DWORD* savedContext)
{
    QueueStage20Bit113ObjectGuardRecord(savedContext);
}

void StartAIFieldTrace(HMODULE proxyModule)
{
    if (proxyModule != nullptr)
    {
        g_proxyModule = proxyModule;
    }

    InitOnceExecuteOnce(&g_startOnce, StartTraceWorker, nullptr, nullptr);
}
