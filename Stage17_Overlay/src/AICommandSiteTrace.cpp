#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cwchar>

#include "AICommandSiteTrace.h"

// The hook replaces only the CALL instruction at AALib.exe+0xF7C5D.
// It snapshots the caller's registers/locals and tail-jumps to the original
// queue helper, preserving the original stack and return address exactly.
extern "C"
{
    volatile DWORD g_ArcanaQueueInsertTarget = 0;
    volatile DWORD g_Stage14F2F50Target = 0;
    volatile DWORD g_Stage14F2ED0Target = 0;
    volatile DWORD g_Stage14ParentContinuation = 0;
    volatile DWORD g_Stage14F2F50ReturnContinuation = 0;
    volatile DWORD g_Stage14F2ED0ReturnContinuation = 0;

    // Stage 15 branch continuations/targets inside F69F0.
    volatile DWORD g_Stage15F6A5DTarget = 0;
    volatile DWORD g_Stage15F6A63Continuation = 0;
    volatile DWORD g_Stage15F6A63Target = 0;
    volatile DWORD g_Stage15F6A68Continuation = 0;
    volatile DWORD g_Stage15F6A68Target = 0;
    volatile DWORD g_Stage15F6A75Continuation = 0;
    volatile DWORD g_Stage15F6A9DTarget = 0;
    volatile DWORD g_Stage15F6AA3Continuation = 0;
    volatile DWORD g_Stage15F6AACTarget = 0;
    volatile DWORD g_Stage15F6AB4Continuation = 0;
    volatile DWORD g_Stage17Bit113SetterContinuation = 0;

    void __cdecl ArcanaLogStage14Probe(DWORD probeId, const DWORD* savedContext);

    void __cdecl ArcanaLogFinalCommandSite(
        DWORD parentEbp,
        DWORD playerObject,
        DWORD payloadRegister,
        DWORD queueAddress,
        DWORD eventType,
        DWORD payloadArgument,
        DWORD callReturnAddress);

    void __cdecl ArcanaLogAlternateQueueCallSite(
        DWORD stackBase,
        DWORD eaxRegister,
        DWORD ecxRegister,
        DWORD edxRegister,
        DWORD ebxRegister,
        DWORD ebpRegister,
        DWORD esiRegister,
        DWORD ediRegister,
        DWORD eflagsRegister);

    __declspec(naked) void ArcanaFinalCommandCallHook()
    {
        __asm
        {
            pushfd
            pushad
            mov edx, esp

            // ArcanaLogFinalCommandSite(parentEbp, player, payloadReg,
            //                           queue, eventType, payload, returnAddress)
            push dword ptr [edx + 36]
            push dword ptr [edx + 48]
            push dword ptr [edx + 44]
            push dword ptr [edx + 40]
            push dword ptr [edx + 4]
            push dword ptr [edx + 0]
            push dword ptr [edx + 8]
            call ArcanaLogFinalCommandSite
            add esp, 28

            popad
            popfd
            jmp dword ptr [g_ArcanaQueueInsertTarget]
        }
    }

    // Stage 13: observe the alternate CALL at AALib.exe+0xF6AD7.
    // Restore all registers and EFLAGS before tail-jumping to the game's
    // original queue helper, preserving the original return address/arguments.
    __declspec(naked) void ArcanaAlternateQueueCallHook()
    {
        __asm
        {
            pushfd
            pushad
            mov edx, esp

            // Arguments: stackBase, EAX, ECX, EDX, EBX, EBP, ESI, EDI, EFLAGS.
            push dword ptr [edx + 32]
            push dword ptr [edx + 0]
            push dword ptr [edx + 4]
            push dword ptr [edx + 8]
            push dword ptr [edx + 16]
            push dword ptr [edx + 20]
            push dword ptr [edx + 24]
            push dword ptr [edx + 28]
            lea eax, [edx + 36]
            push eax
            call ArcanaLogAlternateQueueCallSite
            add esp, 36

            popad
            popfd
            jmp dword ptr [g_ArcanaQueueInsertTarget]
        }
    }

    // Stage 14 probe IDs:
    // 1 = F69F0 parent entry; 2 = F2F50 call; 3 = F2F50 return;
    // 4 = F2ED0 call; 5 = F2ED0 return.
    __declspec(naked) void ArcanaStage14ParentEntryHook()
    {
        __asm
        {
            pushfd
            pushad
            mov edx, esp
            push edx
            push 1
            call ArcanaLogStage14Probe
            add esp, 8
            popad
            popfd

            // Reproduce the verified six-byte prologue at F69F0.
            push ebp
            mov ebp, esp
            sub esp, 18h
            jmp dword ptr [g_Stage14ParentContinuation]
        }
    }

    __declspec(naked) void ArcanaStage14F2F50CallHook()
    {
        __asm
        {
            pushfd
            pushad
            mov edx, esp
            push edx
            push 2
            call ArcanaLogStage14Probe
            add esp, 8
            popad
            popfd
            jmp dword ptr [g_Stage14F2F50Target]
        }
    }

    __declspec(naked) void ArcanaStage14F2F50ReturnHook()
    {
        __asm
        {
            pushfd
            pushad
            mov edx, esp
            push edx
            push 3
            call ArcanaLogStage14Probe
            add esp, 8
            popad
            popfd

            // Replay the nine original bytes at F6A19.
            mov ecx, eax
            cdq
            and edx, 0FFh
            jmp dword ptr [g_Stage14F2F50ReturnContinuation]
        }
    }

    __declspec(naked) void ArcanaStage14F2ED0CallHook()
    {
        __asm
        {
            pushfd
            pushad
            mov edx, esp
            push edx
            push 4
            call ArcanaLogStage14Probe
            add esp, 8
            popad
            popfd
            jmp dword ptr [g_Stage14F2ED0Target]
        }
    }

    __declspec(naked) void ArcanaStage14F2ED0ReturnHook()
    {
        __asm
        {
            pushfd
            pushad
            mov edx, esp
            push edx
            push 5
            call ArcanaLogStage14Probe
            add esp, 8
            popad
            popfd

            // Replay the original add esp,0x0C; test al,al at F6A58.
            add esp, 0Ch
            test al, al
            jmp dword ptr [g_Stage14F2ED0ReturnContinuation]
        }
    }

    // Stage 15 observes and reproduces the original conditional flow after F2ED0.
    // Probe 6: F6A5D JNE (flags from test al,al).
    __declspec(naked) void ArcanaStage15F6A5DBranchHook()
    {
        __asm
        {
            pushfd
            pushad
            mov edx, esp
            push edx
            push 6
            call ArcanaLogStage14Probe
            add esp, 8
            popad
            popfd

            jz s15_f6a5d_not_taken
            jmp dword ptr [g_Stage15F6A5DTarget]
        s15_f6a5d_not_taken:
            jmp dword ptr [g_Stage15F6A63Continuation]
        }
    }

    // Probe 7: replay cmp edi,1Eh; jne F6AC8.
    __declspec(naked) void ArcanaStage15F6A63PayloadBranchHook()
    {
        __asm
        {
            pushfd
            pushad
            mov edx, esp
            push edx
            push 7
            call ArcanaLogStage14Probe
            add esp, 8
            popad
            popfd

            cmp edi, 1Eh
            jne s15_f6a63_not_payload_1e
            jmp dword ptr [g_Stage15F6A68Continuation]
        s15_f6a63_not_payload_1e:
            jmp dword ptr [g_Stage15F6A63Target]
        }
    }

    // Probe 8: replay mov eax,[esi+3B84h]; shr eax,11h; test al,1; je F6A9D.
    __declspec(naked) void ArcanaStage15F6A68Bit17BranchHook()
    {
        __asm
        {
            pushfd
            pushad
            mov edx, esp
            push edx
            push 8
            call ArcanaLogStage14Probe
            add esp, 8
            popad
            popfd

            mov eax, dword ptr [esi + 3B84h]
            shr eax, 11h
            test al, 1
            jz s15_f6a68_bit17_clear
            jmp dword ptr [g_Stage15F6A75Continuation]
        s15_f6a68_bit17_clear:
            jmp dword ptr [g_Stage15F6A68Target]
        }
    }

    // Probe 9: replay cmp dword ptr [ebp-4],2; jne F6AC8.
    __declspec(naked) void ArcanaStage15F6A9DLocalBranchHook()
    {
        __asm
        {
            pushfd
            pushad
            mov edx, esp
            push edx
            push 9
            call ArcanaLogStage14Probe
            add esp, 8
            popad
            popfd

            cmp dword ptr [ebp - 4], 2
            jne s15_f6a9d_local_not_2
            jmp dword ptr [g_Stage15F6AA3Continuation]
        s15_f6a9d_local_not_2:
            jmp dword ptr [g_Stage15F6A9DTarget]
        }
    }

    // Probe 10: replay jne F6AC8 after the original test al,al.
    // On the fall-through path, also replay mov eax,[esi+3F30h].
    __declspec(naked) void ArcanaStage15F6AACHelperBranchHook()
    {
        __asm
        {
            pushfd
            pushad
            mov edx, esp
            push edx
            push 10
            call ArcanaLogStage14Probe
            add esp, 8
            popad
            popfd

            jnz s15_f6aac_helper_nonzero
            mov eax, dword ptr [esi + 3F30h]
            jmp dword ptr [g_Stage15F6AB4Continuation]
        s15_f6aac_helper_nonzero:
            jmp dword ptr [g_Stage15F6AACTarget]
        }
    }

    __declspec(naked) void ArcanaStage17Bit113SetterEntryHook()
    {
        __asm
        {
            pushfd
            cmp dword ptr [esp + 8], 71h
            jne s17_skip_log
            pushad
            mov eax, esp
            push eax
            push 11
            call ArcanaLogStage14Probe
            add esp, 8
            popad
        s17_skip_log:
            popfd
            push ebp
            mov ebp, esp
            push esi
            mov esi, dword ptr [ebp + 8]
            jmp dword ptr [g_Stage17Bit113SetterContinuation]
        }
    }
}

namespace
{
    constexpr DWORD kExpectedImageSize = 0x0E0EB000u;
    constexpr DWORD kExpectedTimeDateStamp = 0x5FE5AA91u;

    constexpr DWORD kP1ControlRva = 0x0DCA5EA4u;
    constexpr DWORD kP2ControlRva = 0x0DCD0BCCu;
    constexpr DWORD kPlayerControlOffset = 0x3F2Cu;
    constexpr DWORD kPlayerQueueOffset = 0x3A2Cu;

    constexpr DWORD kQueueInsertRva = 0x000F2DE0u;
    constexpr DWORD kFinalCommandCallRva = 0x000F7C5Du;
    constexpr DWORD kFinalCommandReturnRva = 0x000F7C62u;

    // The QueueTrace caller return address is F6ADC. A near CALL is five
    // bytes long, so the instruction itself begins at F6AD7.
    constexpr DWORD kAlternateQueueCallRva = 0x000F6AD7u;
    constexpr DWORD kAlternateQueueReturnRva = 0x000F6ADCu;

    constexpr DWORD kStage14ParentEntryRva = 0x000F69F0u;
    constexpr DWORD kStage14F2F50CallRva = 0x000F6A14u;
    constexpr DWORD kStage14F2F50Rva = 0x000F2F50u;
    constexpr DWORD kStage14F2F50ReturnRva = 0x000F6A19u;
    constexpr DWORD kStage14F2ED0CallRva = 0x000F6A53u;
    constexpr DWORD kStage14F2ED0Rva = 0x000F2ED0u;
    constexpr DWORD kStage14F2ED0ReturnRva = 0x000F6A58u;
    constexpr DWORD kStage14ProbeRecordCapacity = 256u;
    constexpr DWORD kStage14StackWordCount = 12u;
    constexpr DWORD kStage14FrameBytesBefore = 0x040u;
    constexpr DWORD kStage14FrameBytesAfter = 0x020u;
    constexpr DWORD kStage14FrameValueCount =
        (kStage14FrameBytesBefore + kStage14FrameBytesAfter) / sizeof(DWORD) + 1u;
    constexpr DWORD kStage14FlushDefaultMs = 250u;

    constexpr DWORD kStage15F6A5DBranchRva = 0x000F6A5Du;
    constexpr DWORD kStage15F6A63PayloadBranchRva = 0x000F6A63u;
    constexpr DWORD kStage15F6A68Bit17BranchRva = 0x000F6A68u;
    constexpr DWORD kStage15F6A9DLocalBranchRva = 0x000F6A9Du;
    constexpr DWORD kStage15F6AACHelperBranchRva = 0x000F6AACu;
    constexpr DWORD kStage17Bit113SetterRva = 0x0000FAA0u;
    constexpr DWORD kStage17Bit113SetterContinuationRva = 0x0000FAA7u;
    constexpr BYTE kStage17Bit113SetterBytes[7] = { 0x55, 0x8B, 0xEC, 0x56, 0x8B, 0x75, 0x08 };

    constexpr BYTE kStage15F6A5DBranchBytes[6] =
        { 0x0F, 0x85, 0x85, 0x00, 0x00, 0x00 };
    constexpr BYTE kStage15F6A63PayloadBranchBytes[5] =
        { 0x83, 0xFF, 0x1E, 0x75, 0x60 };
    constexpr BYTE kStage15F6A68Bit17BranchBytes[13] =
        { 0x8B, 0x86, 0x84, 0x3B, 0x00, 0x00,
          0xC1, 0xE8, 0x11, 0xA8, 0x01, 0x74, 0x28 };
    constexpr BYTE kStage15F6A9DLocalBranchBytes[6] =
        { 0x83, 0x7D, 0xFC, 0x02, 0x75, 0x25 };
    constexpr BYTE kStage15F6AACHelperBranchBytes[8] =
        { 0x75, 0x1A, 0x8B, 0x86, 0x30, 0x3F, 0x00, 0x00 };

    constexpr BYTE kStage14ParentEntryBytes[6] =
        { 0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x18 };
    constexpr BYTE kStage14F2F50CallBytes[5] =
        { 0xE8, 0x37, 0xC5, 0xFF, 0xFF };
    constexpr BYTE kStage14F2F50ReturnBytes[9] =
        { 0x8B, 0xC8, 0x99, 0x81, 0xE2, 0xFF, 0x00, 0x00, 0x00 };
    constexpr BYTE kStage14F2ED0CallBytes[5] =
        { 0xE8, 0x78, 0xC4, 0xFF, 0xFF };
    constexpr BYTE kStage14F2ED0ReturnBytes[5] =
        { 0x83, 0xC4, 0x0C, 0x84, 0xC0 };

    constexpr DWORD kAlternateRecordCapacity = 512u;
    constexpr DWORD kAlternateStackWordCount = 16u;
    constexpr DWORD kAlternateFrameBytesBefore = 0x100u;
    constexpr DWORD kAlternateFrameBytesAfter = 0x020u;
    constexpr DWORD kAlternateFrameValueCount =
        (kAlternateFrameBytesBefore + kAlternateFrameBytesAfter) / sizeof(DWORD) + 1u;
    constexpr DWORD kAlternateCodeBytesBefore = 0x300u;
    constexpr DWORD kAlternateCodeBytesAfter = 0x080u;
    constexpr DWORD kAlternateCodeByteCount =
        kAlternateCodeBytesBefore + kAlternateCodeBytesAfter;
    constexpr DWORD kLocalBytes = 0xD4u;
    constexpr DWORD kLocalDwordCount = kLocalBytes / sizeof(DWORD);
    constexpr DWORD kRecordCapacity = 4096u;
    constexpr DWORD kDefaultFlushIntervalMs = 250u;
    constexpr DWORD kMinFlushIntervalMs = 100u;
    constexpr DWORD kMaxFlushIntervalMs = 5000u;

    constexpr BYTE kExpectedCallBytes[5] = { 0xE8, 0x7E, 0xB1, 0xFF, 0xFF };

    struct PlayerFields
    {
        DWORD cpuControl;
        DWORD field3A34;
        DWORD field3A36;
        DWORD field3F20;
        DWORD field3F30;
        DWORD field3B70;
        DWORD field3B78;
        DWORD field3B7C;
        DWORD field3B80;
        DWORD field3B84;
        DWORD field3B88;
        DWORD field3F50;
        DWORD field3F58;
    };

    struct FinalCommandRecord
    {
        volatile LONG State; // 0 = unused, 1 = ready, 2 = written
        DWORD Side;
        DWORD ThreadId;
        DWORD TickCount;
        SYSTEMTIME LocalTime;
        DWORD ParentEbp;
        DWORD PlayerObject;
        DWORD EdiRegister;
        DWORD QueueAddress;
        DWORD PayloadRegister;
        DWORD EventType;
        DWORD PayloadArgument;
        DWORD CallReturnRva;
        DWORD PayloadMatchesRegister;
        DWORD LocalsReadable;
        PlayerFields Player;
        DWORD Locals[kLocalDwordCount]; // [EBP-0xD4] .. [EBP-0x04]
    };

    struct AlternateCallRecord
    {
        volatile LONG State;
        DWORD Side;
        DWORD ThreadId;
        DWORD TickCount;
        SYSTEMTIME LocalTime;
        ULONGLONG QpcTicks;
        DWORD CallRva;
        DWORD ReturnRva;
        DWORD ReturnAddress;
        DWORD ParentEbp;
        DWORD StackBase;
        DWORD PlayerObject;
        DWORD CpuControl;
        DWORD QueueAddress;
        DWORD EventType;
        DWORD Payload;
        DWORD Eax;
        DWORD Ecx;
        DWORD Edx;
        DWORD Ebx;
        DWORD Ebp;
        DWORD Esi;
        DWORD Edi;
        DWORD Eflags;
        DWORD StackReadMask;
        DWORD StackWords[kAlternateStackWordCount];
        DWORD FrameReadCount;
        BYTE FrameValid[kAlternateFrameValueCount];
        DWORD FrameWords[kAlternateFrameValueCount];
        DWORD CodeReadCount;
        BYTE CodeValid[kAlternateCodeByteCount];
        BYTE CodeBytes[kAlternateCodeByteCount];
    };

    struct Stage14ProbeRecord
    {
        SYSTEMTIME LocalTime;
        char Message[4096];
    };

    INIT_ONCE g_startOnce = INIT_ONCE_STATIC_INIT;
    HMODULE g_proxyModule = nullptr;
    HMODULE g_gameModule = nullptr;
    DWORD g_gameBase = 0;
    DWORD g_gameImageSize = 0;
    DWORD g_playerP1 = 0;
    DWORD g_playerP2 = 0;
    DWORD g_flushIntervalMs = kDefaultFlushIntervalMs;

    volatile LONG g_hookInstalled = 0;
    volatile LONG g_writeIndex = 0;
    volatile LONG g_readIndex = 0;
    volatile LONG g_droppedRecords = 0;
    volatile LONG g_reportedDroppedRecords = 0;
    FinalCommandRecord g_records[kRecordCapacity]{};

    volatile LONG g_alternateHookInstalled = 0;
    volatile LONG g_alternateWriteIndex = 0;
    volatile LONG g_alternateReadIndex = 0;
    DWORD g_alternateFlushIntervalMs = kDefaultFlushIntervalMs;
    volatile LONG g_alternateDroppedRecords = 0;
    volatile LONG g_alternateReportedDroppedRecords = 0;
    AlternateCallRecord g_alternateRecords[kAlternateRecordCapacity]{};
    BYTE g_alternateOriginalCallBytes[5]{};

    SRWLOCK g_stage14ProbeLock = SRWLOCK_INIT;
    Stage14ProbeRecord g_stage14ProbeRecords[kStage14ProbeRecordCapacity]{};
    DWORD g_stage14ProbeReadIndex = 0;
    DWORD g_stage14ProbeWriteIndex = 0;
    DWORD g_stage14ProbeCount = 0;
    volatile LONG g_stage14ProbeSequence = 0;
    volatile LONG g_stage14ProbeDropped = 0;
    volatile LONG g_stage14ProbeReportedDropped = 0;
    DWORD g_stage14FlushIntervalMs = kStage14FlushDefaultMs;
    volatile LONG g_stage14ProbeEnabled = 0;
    volatile LONG g_stage15Installed = 0;
    volatile LONG g_stage17Installed = 0;

    bool GetSiblingPath(wchar_t* output, DWORD capacity, const wchar_t* filename)
    {
        if (output == nullptr || capacity == 0 || filename == nullptr || g_proxyModule == nullptr)
            return false;

        const DWORD length = GetModuleFileNameW(g_proxyModule, output, capacity);
        if (length == 0 || length >= capacity)
            return false;

        wchar_t* slash = wcsrchr(output, L'\\');
        wchar_t* forwardSlash = wcsrchr(output, L'/');
        if (forwardSlash != nullptr && (slash == nullptr || forwardSlash > slash))
            slash = forwardSlash;
        if (slash == nullptr)
            return false;

        *(slash + 1) = L'\0';
        const std::size_t used = wcslen(output);
        const std::size_t nameLength = wcslen(filename);
        if (used + nameLength + 1 > capacity)
            return false;
        wcscat_s(output, capacity, filename);
        return true;
    }

    void AppendLogAtTime(const SYSTEMTIME* timestamp, const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_FinalCommandTrace.log"))
            return;

        SYSTEMTIME now{};
        if (timestamp != nullptr) now = *timestamp;
        else GetLocalTime(&now);

        char line[3500]{};
        const int count = sprintf_s(
            line, sizeof(line),
            "[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
            message != nullptr ? message : "(empty)");
        if (count <= 0)
            return;

        HANDLE file = CreateFileW(
            logPath, FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE)
            return;

        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    void AppendLog(const char* message)
    {
        AppendLogAtTime(nullptr, message);
    }

    void AppendAlternateLogAtTime(const SYSTEMTIME* timestamp, const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_AlternateCallTrace.log"))
            return;

        SYSTEMTIME now{};
        if (timestamp != nullptr) now = *timestamp;
        else GetLocalTime(&now);

        char line[8192]{};
        const int count = sprintf_s(
            line, sizeof(line),
            "[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
            message != nullptr ? message : "(empty)");
        if (count <= 0) return;

        HANDLE file = CreateFileW(
            logPath, FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;

        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    void AppendAlternateLog(const char* message)
    {
        AppendAlternateLogAtTime(nullptr, message);
    }

    void AppendStage14LogAtTime(const SYSTEMTIME* timestamp, const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_ProducerCallTrace.log"))
            return;

        SYSTEMTIME now{};
        if (timestamp != nullptr) now = *timestamp;
        else GetLocalTime(&now);

        char line[8192]{};
        const int count = sprintf_s(
            line, sizeof(line),
            "[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s\r\n",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
            message != nullptr ? message : "(empty)");
        if (count <= 0) return;

        HANDLE file = CreateFileW(
            logPath, FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;

        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    void AppendStage14Log(const char* message)
    {
        AppendStage14LogAtTime(nullptr, message);
    }

    void AppendStage17LogAtTime(const SYSTEMTIME* timestamp, const char* message)
    {
        wchar_t logPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(logPath, MAX_PATH, L"ArcanaAI_Bit113SetterTrace.log")) return;
        SYSTEMTIME now{};
        if (timestamp != nullptr) now = *timestamp;
        else GetLocalTime(&now);
        char line[4600]{};
        const char newline[3] = { '\\r', '\\n', '\\0' };
        const int count = sprintf_s(line, sizeof(line),
            "[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s%s",
            now.wYear, now.wMonth, now.wDay,
            now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
            message != nullptr ? message : "(empty)", newline);
        if (count <= 0) return;
        HANDLE file = CreateFileW(logPath, FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;
        DWORD written = 0;
        WriteFile(file, line, static_cast<DWORD>(count), &written, nullptr);
        CloseHandle(file);
    }

    void AppendStage17Log(const char* message)
    {
        AppendStage17LogAtTime(nullptr, message);
    }

    int ReadIniInt(const wchar_t* section, const wchar_t* key, int defaultValue)
    {
        wchar_t iniPath[MAX_PATH + 1]{};
        if (!GetSiblingPath(iniPath, MAX_PATH, L"ArcanaAI.ini"))
            return defaultValue;
        return GetPrivateProfileIntW(section, key, defaultValue, iniPath);
    }

    bool GetImageFingerprint32(HMODULE module, DWORD* imageSize, DWORD* timeDateStamp)
    {
        if (module == nullptr || imageSize == nullptr || timeDateStamp == nullptr)
            return false;

        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0)
            return false;

        const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS32*>(
            reinterpret_cast<const BYTE*>(module) + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE ||
            nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
            return false;

        *imageSize = nt->OptionalHeader.SizeOfImage;
        *timeDateStamp = nt->FileHeader.TimeDateStamp;
        return true;
    }

    bool SafeReadDword(DWORD address, DWORD* value)
    {
        if (address == 0 || value == nullptr)
            return false;
        __try
        {
            *value = *reinterpret_cast<const volatile DWORD*>(address);
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            *value = 0;
            return false;
        }
    }

    bool SafeReadWord(DWORD address, WORD* value)
    {
        if (address == 0 || value == nullptr)
            return false;
        __try
        {
            *value = *reinterpret_cast<const volatile WORD*>(address);
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            *value = 0;
            return false;
        }
    }

    bool SafeReadByte(DWORD address, BYTE* value)
    {
        if (address == 0 || value == nullptr)
            return false;
        __try
        {
            *value = *reinterpret_cast<const volatile BYTE*>(address);
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            *value = 0;
            return false;
        }
    }

    const char* Stage14ProbeName(DWORD probeId)
    {
        switch (probeId)
        {
        case 1: return "F69F0_ENTRY";
        case 2: return "F2F50_CALL";
        case 3: return "F2F50_RETURN";
        case 4: return "F2ED0_CALL";
        case 5: return "F2ED0_RETURN";
        case 6: return "F6A5D_F2ED0_BRANCH";
        case 7: return "F6A63_PAYLOAD_BRANCH";
        case 8: return "F6A68_PLAYER_BIT17_BRANCH";
        case 9: return "F6A9D_LOCAL_GUARD_BRANCH";
        case 10: return "F6AAC_SECONDARY_HELPER_BRANCH";
        case 11: return "BIT113_SETTER_ENTRY";
        default: return "UNKNOWN";
        }
    }

    void QueueStage14ProbeRecord(DWORD probeId, const DWORD* savedContext)
    {
        if (savedContext == nullptr ||
            InterlockedCompareExchange(&g_stage14ProbeEnabled, 0, 0) == 0)
            return;

        const DWORD eaxValue = savedContext[7];
        const DWORD ecxValue = savedContext[6];
        const DWORD edxValue = savedContext[5];
        const DWORD ebxValue = savedContext[4];
        const DWORD ebpValue = savedContext[2];
        const DWORD esiValue = savedContext[1];
        const DWORD ediValue = savedContext[0];
        const DWORD eflagsValue = savedContext[8];
        const DWORD stackBase = static_cast<DWORD>(
            reinterpret_cast<std::uintptr_t>(savedContext) + 36u);

        DWORD stackWords[kStage14StackWordCount]{};
        DWORD stackReadMask = 0;
        for (DWORD i = 0; i < kStage14StackWordCount; ++i)
        {
            const std::uint64_t address =
                static_cast<std::uint64_t>(stackBase) + i * sizeof(DWORD);
            DWORD value = 0;
            if (address <= 0xFFFFFFFFull &&
                SafeReadDword(static_cast<DWORD>(address), &value))
            {
                stackWords[i] = value;
                stackReadMask |= (1u << i);
            }
        }

        const DWORD playerObject = probeId == 1 ? stackWords[1] :
            (probeId == 11 ? ecxValue : esiValue);
        DWORD cpuControl = 0;
        if (playerObject != 0)
            SafeReadDword(playerObject + kPlayerControlOffset, &cpuControl);

        DWORD side = 0;
        if (playerObject == g_playerP1) side = 1;
        else if (playerObject == g_playerP2) side = 2;

        DWORD frameWords[kStage14FrameValueCount]{};
        BYTE frameValid[kStage14FrameValueCount]{};
        DWORD frameReadMask = 0;
        if (probeId != 1 && ebpValue != 0)
        {
            for (DWORD i = 0; i < kStage14FrameValueCount; ++i)
            {
                const LONG displacement =
                    -static_cast<LONG>(kStage14FrameBytesBefore) +
                    static_cast<LONG>(i * sizeof(DWORD));
                const std::int64_t address =
                    static_cast<std::int64_t>(ebpValue) + displacement;
                DWORD value = 0;
                if (address > 0 && address <= 0xFFFFFFFFll &&
                    SafeReadDword(static_cast<DWORD>(address), &value))
                {
                    frameWords[i] = value;
                    frameValid[i] = 1;
                    frameReadMask |= (1u << i);
                }
            }
        }

        LARGE_INTEGER qpc{};
        QueryPerformanceCounter(&qpc);
        SYSTEMTIME localTime{};
        GetLocalTime(&localTime);
        const LONG sequence = InterlockedIncrement(&g_stage14ProbeSequence);

        char message[4096]{};
        const char* recordPrefix = probeId == 11u ? "STAGE17_SETTER" :
            (probeId >= 6u ? "STAGE15_BRANCH" : "STAGE14_PROBE");
        int used = sprintf_s(
            message, sizeof(message),
            "%s seq=%ld probe=%s side=%s tid=%lu tick=%lu qpc=%llu stackBase=0x%08lX ebp=0x%08lX player=0x%08lX cpu=0x%08lX stack0=0x%08lX regs=(EAX:%08lX ECX:%08lX EDX:%08lX EBX:%08lX EBP:%08lX ESI:%08lX EDI:%08lX EFLAGS:%08lX) stackMask=0x%03lX stack=",
            recordPrefix,
            static_cast<long>(sequence),
            Stage14ProbeName(probeId),
            side == 1 ? "P1" : (side == 2 ? "P2" : "UNKNOWN"),
            static_cast<unsigned long>(GetCurrentThreadId()),
            static_cast<unsigned long>(GetTickCount()),
            static_cast<unsigned long long>(qpc.QuadPart),
            static_cast<unsigned long>(stackBase),
            static_cast<unsigned long>(ebpValue),
            static_cast<unsigned long>(playerObject),
            static_cast<unsigned long>(cpuControl),
            static_cast<unsigned long>(stackWords[0]),
            static_cast<unsigned long>(eaxValue),
            static_cast<unsigned long>(ecxValue),
            static_cast<unsigned long>(edxValue),
            static_cast<unsigned long>(ebxValue),
            static_cast<unsigned long>(ebpValue),
            static_cast<unsigned long>(esiValue),
            static_cast<unsigned long>(ediValue),
            static_cast<unsigned long>(eflagsValue),
            static_cast<unsigned long>(stackReadMask));

        if (used > 0)
        {
            for (DWORD i = 0; i < kStage14StackWordCount &&
                static_cast<std::size_t>(used) < sizeof(message) - 32; ++i)
            {
                const int written = sprintf_s(
                    message + used, sizeof(message) - static_cast<std::size_t>(used),
                    "%sS+%02lX=%08lX",
                    i == 0 ? "" : ",",
                    static_cast<unsigned long>(i * sizeof(DWORD)),
                    static_cast<unsigned long>(stackWords[i]));
                if (written <= 0) break;
                used += written;
            }
        }

        if (used > 0 && probeId != 1)
        {
            const int header = sprintf_s(
                message + used, sizeof(message) - static_cast<std::size_t>(used),
                " frameMask=0x%08lX frame=",
                static_cast<unsigned long>(frameReadMask));
            if (header > 0) used += header;
            bool first = true;
            for (DWORD i = 0; i < kStage14FrameValueCount &&
                static_cast<std::size_t>(used) < sizeof(message) - 32; ++i)
            {
                if (frameValid[i] == 0) continue;
                const LONG displacement =
                    -static_cast<LONG>(kStage14FrameBytesBefore) +
                    static_cast<LONG>(i * sizeof(DWORD));
                char label[16]{};
                if (displacement < 0)
                    sprintf_s(label, sizeof(label), "F-%03lX", static_cast<unsigned long>(-displacement));
                else
                    sprintf_s(label, sizeof(label), "F+%03lX", static_cast<unsigned long>(displacement));
                const int written = sprintf_s(
                    message + used, sizeof(message) - static_cast<std::size_t>(used),
                    "%s%s=%08lX",
                    first ? "" : ",",
                    label,
                    static_cast<unsigned long>(frameWords[i]));
                if (written <= 0) break;
                used += written;
                first = false;
            }
        }

        if (used > 0 && probeId >= 6u)
        {
            DWORD player3B70 = 0, player3B7C = 0, player3B80 = 0;
            DWORD player3B84 = 0, player3B88 = 0, player3F20 = 0, player3F30 = 0;
            DWORD player3F50 = 0, player3F58 = 0, queueAddress = 0;
            WORD player3A34 = 0, player3A36 = 0, queueType = 0, queuePayload = 0;
            DWORD localFC = 0, localF8 = 0;
            DWORD fieldsReadMask = 0;
            if (SafeReadWord(playerObject + 0x3A34u, &player3A34)) fieldsReadMask |= 1u << 0;
            if (SafeReadWord(playerObject + 0x3A36u, &player3A36)) fieldsReadMask |= 1u << 1;
            if (SafeReadDword(playerObject + 0x3B70u, &player3B70)) fieldsReadMask |= 1u << 2;
            if (SafeReadDword(playerObject + 0x3B7Cu, &player3B7C)) fieldsReadMask |= 1u << 3;
            if (SafeReadDword(playerObject + 0x3B80u, &player3B80)) fieldsReadMask |= 1u << 4;
            if (SafeReadDword(playerObject + 0x3B84u, &player3B84)) fieldsReadMask |= 1u << 5;
            if (SafeReadDword(playerObject + 0x3B88u, &player3B88)) fieldsReadMask |= 1u << 6;
            if (SafeReadDword(playerObject + 0x3F20u, &player3F20)) fieldsReadMask |= 1u << 7;
            if (SafeReadDword(playerObject + 0x3F30u, &player3F30)) fieldsReadMask |= 1u << 8;
            if (SafeReadDword(playerObject + 0x3F50u, &player3F50)) fieldsReadMask |= 1u << 9;
            if (SafeReadDword(playerObject + 0x3F58u, &player3F58)) fieldsReadMask |= 1u << 10;
            if (SafeReadDword(playerObject + kPlayerQueueOffset, &queueAddress))
                fieldsReadMask |= 1u << 11;
            if (queueAddress != 0)
            {
                if (SafeReadWord(queueAddress + 0x08u, &queueType)) fieldsReadMask |= 1u << 12;
                if (SafeReadWord(queueAddress + 0x0Au, &queuePayload)) fieldsReadMask |= 1u << 13;
            }
            const bool localFCReadable = ebpValue >= 4u && SafeReadDword(ebpValue - 4u, &localFC);
            const bool localF8Readable = ebpValue >= 8u && SafeReadDword(ebpValue - 8u, &localF8);

            bool branchKnown = true;
            DWORD branchTaken = 0;
            switch (probeId)
            {
            case 6: branchTaken = (eflagsValue & 0x40u) == 0 ? 1u : 0u; break;
            case 7: branchTaken = ediValue != 0x1Eu ? 1u : 0u; break;
            case 8:
                branchKnown = (fieldsReadMask & (1u << 5)) != 0;
                branchTaken = (player3B84 & 0x00020000u) == 0 ? 1u : 0u;
                break;
            case 9:
                branchKnown = localFCReadable;
                branchTaken = localFC != 2u ? 1u : 0u;
                break;
            case 10: branchTaken = (eaxValue & 0xFFu) != 0 ? 1u : 0u; break;
            default: branchKnown = false; break;
            }
            const char* branchTakenText =
                !branchKnown ? "NA" : (branchTaken != 0 ? "1" : "0");

            const char* suffixName = probeId == 11u ? " stage17" : " stage15";
            const int written = sprintf_s(
                message + used, sizeof(message) - static_cast<std::size_t>(used),
                "%s=(zf=%lu branchTaken=%s fieldsMask=0x%04lX "
                "3A34=%04X 3A36=%04X 3B70=%08lX 3B7C=%08lX 3B80=%08lX "
                "3B84=%08lX bit17=%lu 3B88=%08lX 3F20=%08lX 3F30=%08lX "
                "3F50=%08lX 3F58=%08lX queue=%08lX queuePair=(%04X,%04X) "
                "localFC=%08lX localF8=%08lX localRead=(%u,%u))",
                suffixName,
                static_cast<unsigned long>((eflagsValue >> 6) & 1u),
                branchTakenText,
                static_cast<unsigned long>(fieldsReadMask),
                player3A34, player3A36,
                static_cast<unsigned long>(player3B70),
                static_cast<unsigned long>(player3B7C),
                static_cast<unsigned long>(player3B80),
                static_cast<unsigned long>(player3B84),
                static_cast<unsigned long>((player3B84 >> 17) & 1u),
                static_cast<unsigned long>(player3B88),
                static_cast<unsigned long>(player3F20),
                static_cast<unsigned long>(player3F30),
                static_cast<unsigned long>(player3F50),
                static_cast<unsigned long>(player3F58),
                static_cast<unsigned long>(queueAddress),
                queueType, queuePayload,
                static_cast<unsigned long>(localFC),
                static_cast<unsigned long>(localF8),
                localFCReadable ? 1u : 0u,
                localF8Readable ? 1u : 0u);
            if (written > 0) used += written;
        }

        if (probeId == 11u && used > 0)
        {
            const DWORD callerReturnAddress = stackWords[0];
            const DWORD bitIndex = stackWords[1];
            const DWORD callerCallAddress = callerReturnAddress >= 5u ? callerReturnAddress - 5u : 0u;
            BYTE callBytes[5]{};
            DWORD callBytesMask = 0;
            for (DWORD i = 0; i < 5u && callerCallAddress != 0; ++i)
                if (SafeReadByte(callerCallAddress + i, &callBytes[i])) callBytesMask |= 1u << i;
            DWORD decodedTarget = 0;
            bool callTargetVerified = false;
            if (callBytesMask == 0x1Fu && callBytes[0] == 0xE8)
            {
                std::int32_t relative = 0;
                std::memcpy(&relative, callBytes + 1, sizeof(relative));
                const std::int64_t destination = static_cast<std::int64_t>(callerCallAddress) + 5 + relative;
                if (destination > 0 && destination <= 0xFFFFFFFFll)
                {
                    decodedTarget = static_cast<DWORD>(destination);
                    callTargetVerified = decodedTarget == g_gameBase + kStage17Bit113SetterRva;
                }
            }
            char callHex[16]{};
            for (DWORD i = 0; i < 5u; ++i)
                sprintf_s(callHex + i * 3u, sizeof(callHex) - i * 3u, "%02X%s", callBytes[i], i == 4u ? "" : " ");
            const DWORD returnRva = callerReturnAddress >= g_gameBase ? callerReturnAddress - g_gameBase : 0xFFFFFFFFu;
            const DWORD callRva = callerCallAddress >= g_gameBase ? callerCallAddress - g_gameBase : 0xFFFFFFFFu;
            const int extra = sprintf_s(message + used,
                sizeof(message) - static_cast<std::size_t>(used),
                " callerReturn=0x%08lX callerReturnRVA=0x%08lX callerCall=0x%08lX callerCallRVA=0x%08lX "
                "bitIndex=0x%08lX callBytes=%s callBytesMask=0x%02lX decodedTarget=0x%08lX callTargetVerified=%lu "
                "expectedCallerCallRVA=0x000C6502 expectedReturnRVA=0x000C6507 entryZF=%lu",
                static_cast<unsigned long>(callerReturnAddress), static_cast<unsigned long>(returnRva),
                static_cast<unsigned long>(callerCallAddress), static_cast<unsigned long>(callRva),
                static_cast<unsigned long>(bitIndex), callHex, static_cast<unsigned long>(callBytesMask),
                static_cast<unsigned long>(decodedTarget), static_cast<unsigned long>(callTargetVerified),
                static_cast<unsigned long>((eflagsValue >> 6) & 1u));
            if (extra > 0) used += extra;
        }

        if (used <= 0)
            return;

        AcquireSRWLockExclusive(&g_stage14ProbeLock);
        if (g_stage14ProbeCount >= kStage14ProbeRecordCapacity)
        {
            ReleaseSRWLockExclusive(&g_stage14ProbeLock);
            InterlockedIncrement(&g_stage14ProbeDropped);
            return;
        }

        Stage14ProbeRecord& slot = g_stage14ProbeRecords[g_stage14ProbeWriteIndex];
        slot.LocalTime = localTime;
        strcpy_s(slot.Message, sizeof(slot.Message), message);
        g_stage14ProbeWriteIndex =
            (g_stage14ProbeWriteIndex + 1u) % kStage14ProbeRecordCapacity;
        ++g_stage14ProbeCount;
        ReleaseSRWLockExclusive(&g_stage14ProbeLock);
    }

    bool SafeCopyLocals(DWORD parentEbp, DWORD* output, DWORD count)
    {
        if (parentEbp == 0 || output == nullptr || count != kLocalDwordCount)
            return false;
        __try
        {
            const volatile DWORD* source = reinterpret_cast<const volatile DWORD*>(parentEbp - kLocalBytes);
            for (DWORD i = 0; i < count; ++i)
                output[i] = source[i];
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            for (DWORD i = 0; i < count; ++i)
                output[i] = 0;
            return false;
        }
    }

    void QueueAlternateCallRecord(
        DWORD stackBase,
        DWORD eaxRegister, DWORD ecxRegister, DWORD edxRegister, DWORD ebxRegister,
        DWORD ebpRegister, DWORD esiRegister, DWORD ediRegister, DWORD eflagsRegister)
    {
        if (InterlockedCompareExchange(&g_alternateHookInstalled, 0, 0) == 0 ||
            g_gameBase == 0 || stackBase == 0)
            return;

        DWORD returnAddress = 0;
        DWORD queueAddress = 0;
        DWORD eventType = 0;
        DWORD payload = 0;
        if (!SafeReadDword(stackBase, &returnAddress) ||
            !SafeReadDword(stackBase + 4u, &queueAddress) ||
            !SafeReadDword(stackBase + 8u, &eventType) ||
            !SafeReadDword(stackBase + 12u, &payload))
            return;

        const DWORD returnRva = returnAddress >= g_gameBase
            ? returnAddress - g_gameBase : 0;
        if (returnRva != kAlternateQueueReturnRva)
            return;

        DWORD side = 0;
        DWORD playerObject = 0;
        if (queueAddress == g_playerP1 + kPlayerQueueOffset)
        {
            side = 1;
            playerObject = g_playerP1;
        }
        else if (queueAddress == g_playerP2 + kPlayerQueueOffset)
        {
            side = 2;
            playerObject = g_playerP2;
        }
        else
            return;

        const LONG slotIndex = InterlockedIncrement(&g_alternateWriteIndex) - 1;
        if (slotIndex < 0 || static_cast<DWORD>(slotIndex) >= kAlternateRecordCapacity)
        {
            InterlockedIncrement(&g_alternateDroppedRecords);
            return;
        }

        AlternateCallRecord& record = g_alternateRecords[slotIndex];
        record.Side = side;
        record.ThreadId = GetCurrentThreadId();
        record.TickCount = GetTickCount();
        GetLocalTime(&record.LocalTime);
        LARGE_INTEGER qpc{};
        QueryPerformanceCounter(&qpc);
        record.QpcTicks = static_cast<ULONGLONG>(qpc.QuadPart);
        record.CallRva = kAlternateQueueCallRva;
        record.ReturnRva = returnRva;
        record.ReturnAddress = returnAddress;
        record.ParentEbp = ebpRegister;
        record.StackBase = stackBase;
        record.PlayerObject = playerObject;
        record.QueueAddress = queueAddress;
        record.EventType = eventType;
        record.Payload = payload;
        record.Eax = eaxRegister;
        record.Ecx = ecxRegister;
        record.Edx = edxRegister;
        record.Ebx = ebxRegister;
        record.Ebp = ebpRegister;
        record.Esi = esiRegister;
        record.Edi = ediRegister;
        record.Eflags = eflagsRegister;
        SafeReadDword(playerObject + kPlayerControlOffset, &record.CpuControl);

        record.StackReadMask = 0;
        for (DWORD i = 0; i < kAlternateStackWordCount; ++i)
        {
            DWORD value = 0;
            if (SafeReadDword(stackBase + i * sizeof(DWORD), &value))
            {
                record.StackWords[i] = value;
                record.StackReadMask |= (1u << i);
            }
            else
                record.StackWords[i] = 0;
        }

        record.FrameReadCount = 0;
        for (DWORD i = 0; i < kAlternateFrameValueCount; ++i)
        {
            const LONG displacement =
                -static_cast<LONG>(kAlternateFrameBytesBefore) +
                static_cast<LONG>(i * sizeof(DWORD));
            const std::int64_t addressValue =
                static_cast<std::int64_t>(ebpRegister) + displacement;
            DWORD value = 0;
            if (addressValue > 0 &&
                addressValue <= 0xFFFFFFFFll &&
                SafeReadDword(static_cast<DWORD>(addressValue), &value))
            {
                record.FrameWords[i] = value;
                record.FrameValid[i] = 1;
                ++record.FrameReadCount;
            }
            else
            {
                record.FrameWords[i] = 0;
                record.FrameValid[i] = 0;
            }
        }

        const DWORD codeStartRva = kAlternateQueueCallRva - kAlternateCodeBytesBefore;
        record.CodeReadCount = 0;
        for (DWORD i = 0; i < kAlternateCodeByteCount; ++i)
        {
            BYTE value = 0;
            if (SafeReadByte(g_gameBase + codeStartRva + i, &value))
            {
                record.CodeBytes[i] = value;
                record.CodeValid[i] = 1;
                ++record.CodeReadCount;
            }
            else
            {
                record.CodeBytes[i] = 0;
                record.CodeValid[i] = 0;
            }
        }

        // Replace the live patched CALL in the dump with its original bytes.
        for (DWORD i = 0; i < 5; ++i)
        {
            const DWORD codeIndex = kAlternateCodeBytesBefore + i;
            if (record.CodeValid[codeIndex] == 0)
                ++record.CodeReadCount;
            record.CodeBytes[codeIndex] = g_alternateOriginalCallBytes[i];
            record.CodeValid[codeIndex] = 1;
        }

        MemoryBarrier();
        InterlockedExchange(&record.State, 1);
    }

    PlayerFields ReadPlayerFields(DWORD player)
    {
        PlayerFields fields{};
        SafeReadDword(player + kPlayerControlOffset, &fields.cpuControl);
        WORD word = 0;
        if (SafeReadWord(player + 0x3A34u, &word)) fields.field3A34 = word;
        if (SafeReadWord(player + 0x3A36u, &word)) fields.field3A36 = word;
        SafeReadDword(player + 0x3F20u, &fields.field3F20);
        SafeReadDword(player + 0x3F30u, &fields.field3F30);
        SafeReadDword(player + 0x3B70u, &fields.field3B70);
        SafeReadDword(player + 0x3B78u, &fields.field3B78);
        SafeReadDword(player + 0x3B7Cu, &fields.field3B7C);
        SafeReadDword(player + 0x3B80u, &fields.field3B80);
        SafeReadDword(player + 0x3B84u, &fields.field3B84);
        SafeReadDword(player + 0x3B88u, &fields.field3B88);
        SafeReadDword(player + 0x3F50u, &fields.field3F50);
        SafeReadDword(player + 0x3F58u, &fields.field3F58);
        return fields;
    }

    void QueueFinalCommandRecord(
        DWORD parentEbp, DWORD ediRegister, DWORD payloadRegister,
        DWORD queueAddress, DWORD eventType, DWORD payloadArgument,
        DWORD callReturnAddress)
    {
        if (InterlockedCompareExchange(&g_hookInstalled, 0, 0) == 0 ||
            g_gameBase == 0)
            return;

        // EDI is retained for diagnosis only. The actual queue argument is
        // authoritative for identifying which CPU player receives the command.
        DWORD side = 0;
        DWORD playerObject = 0;
        if (queueAddress == g_playerP1 + kPlayerQueueOffset)
        {
            side = 1;
            playerObject = g_playerP1;
        }
        else if (queueAddress == g_playerP2 + kPlayerQueueOffset)
        {
            side = 2;
            playerObject = g_playerP2;
        }
        else return;

        if (eventType != 1)
            return;

        DWORD cpuMode = 0;
        if (!SafeReadDword(playerObject + kPlayerControlOffset, &cpuMode) || cpuMode != 1)
            return;

        const DWORD returnRva = callReturnAddress >= g_gameBase
            ? callReturnAddress - g_gameBase : 0;
        if (returnRva != kFinalCommandReturnRva)
            return;

        const LONG slot = InterlockedIncrement(&g_writeIndex) - 1;
        if (slot < 0 || static_cast<DWORD>(slot) >= kRecordCapacity)
        {
            InterlockedIncrement(&g_droppedRecords);
            return;
        }

        FinalCommandRecord& record = g_records[slot];
        record.Side = side;
        record.ThreadId = GetCurrentThreadId();
        record.TickCount = GetTickCount();
        GetLocalTime(&record.LocalTime);
        record.ParentEbp = parentEbp;
        record.PlayerObject = playerObject;
        record.EdiRegister = ediRegister;
        record.QueueAddress = queueAddress;
        record.PayloadRegister = payloadRegister;
        record.EventType = eventType;
        record.PayloadArgument = payloadArgument;
        record.CallReturnRva = returnRva;
        record.PayloadMatchesRegister = payloadRegister == payloadArgument ? 1u : 0u;
        record.Player = ReadPlayerFields(playerObject);
        record.LocalsReadable = SafeCopyLocals(parentEbp, record.Locals, kLocalDwordCount) ? 1u : 0u;
        MemoryBarrier();
        InterlockedExchange(&record.State, 1);
    }

    void FlushReadyRecords()
    {
        for (;;)
        {
            const LONG readIndex = InterlockedCompareExchange(&g_readIndex, 0, 0);
            const LONG writeIndex = InterlockedCompareExchange(&g_writeIndex, 0, 0);
            if (readIndex >= writeIndex || readIndex < 0 ||
                static_cast<DWORD>(readIndex) >= kRecordCapacity)
                break;

            FinalCommandRecord& record = g_records[readIndex];
            if (InterlockedCompareExchange(&record.State, 0, 0) != 1)
                break;

            char message[3200]{};
            int used = sprintf_s(
                message, sizeof(message),
                "FINAL_COMMAND side=P%lu tid=%lu tick=%lu returnRVA=0x%08lX parentEBP=0x%08lX player=0x%08lX edi=0x%08lX ediMatchesPlayer=%lu queue=0x%08lX type=0x%08lX payloadESI=0x%08lX payloadArg=0x%08lX payloadMatch=%lu cpu=0x%08lX fields=(3A34:%04lX 3A36:%04lX 3F20:%08lX 3F30:%08lX 3B70:%08lX 3B78:%08lX 3B7C:%08lX 3B80:%08lX 3B84:%08lX 3B88:%08lX 3F50:%08lX 3F58:%08lX) localsReadable=%lu locals=",
                static_cast<unsigned long>(record.Side),
                static_cast<unsigned long>(record.ThreadId),
                static_cast<unsigned long>(record.TickCount),
                static_cast<unsigned long>(record.CallReturnRva),
                static_cast<unsigned long>(record.ParentEbp),
                static_cast<unsigned long>(record.PlayerObject),
                static_cast<unsigned long>(record.EdiRegister),
                static_cast<unsigned long>(record.EdiRegister == record.PlayerObject ? 1u : 0u),
                static_cast<unsigned long>(record.QueueAddress),
                static_cast<unsigned long>(record.EventType),
                static_cast<unsigned long>(record.PayloadRegister),
                static_cast<unsigned long>(record.PayloadArgument),
                static_cast<unsigned long>(record.PayloadMatchesRegister),
                static_cast<unsigned long>(record.Player.cpuControl),
                static_cast<unsigned long>(record.Player.field3A34),
                static_cast<unsigned long>(record.Player.field3A36),
                static_cast<unsigned long>(record.Player.field3F20),
                static_cast<unsigned long>(record.Player.field3F30),
                static_cast<unsigned long>(record.Player.field3B70),
                static_cast<unsigned long>(record.Player.field3B78),
                static_cast<unsigned long>(record.Player.field3B7C),
                static_cast<unsigned long>(record.Player.field3B80),
                static_cast<unsigned long>(record.Player.field3B84),
                static_cast<unsigned long>(record.Player.field3B88),
                static_cast<unsigned long>(record.Player.field3F50),
                static_cast<unsigned long>(record.Player.field3F58),
                static_cast<unsigned long>(record.LocalsReadable));

            if (used > 0)
            {
                for (DWORD i = 0; i < kLocalDwordCount &&
                    static_cast<std::size_t>(used) < sizeof(message) - 24; ++i)
                {
                    const DWORD displacement = 0xD4u - i * 4u;
                    const int written = sprintf_s(
                        message + used, sizeof(message) - static_cast<std::size_t>(used),
                        "%sL-%03lX=%08lX",
                        i == 0 ? "" : ",",
                        static_cast<unsigned long>(displacement),
                        static_cast<unsigned long>(record.Locals[i]));
                    if (written <= 0) break;
                    used += written;
                }
            }

            // Preserve the event timestamp captured on the game thread.
            AppendLogAtTime(&record.LocalTime, message);

            InterlockedExchange(&record.State, 2);
            InterlockedExchange(&g_readIndex, readIndex + 1);
        }
    }

    void FlushAlternateReadyRecords()
    {
        for (;;)
        {
            const LONG readIndex = InterlockedCompareExchange(&g_alternateReadIndex, 0, 0);
            const LONG writeIndex = InterlockedCompareExchange(&g_alternateWriteIndex, 0, 0);
            if (readIndex >= writeIndex || readIndex < 0 ||
                static_cast<DWORD>(readIndex) >= kAlternateRecordCapacity)
                break;

            AlternateCallRecord& record = g_alternateRecords[readIndex];
            if (InterlockedCompareExchange(&record.State, 0, 0) != 1)
                break;

            char message[8192]{};
            int used = sprintf_s(
                message, sizeof(message),
                "ALT_CALLSITE side=P%lu tid=%lu tick=%lu qpc=%llu callRVA=0x%08lX returnRVA=0x%08lX ret=0x%08lX stackBase=0x%08lX parentEBP=0x%08lX player=0x%08lX cpu=0x%08lX queue=0x%08lX type=0x%08lX payload=0x%08lX regs=(EAX:%08lX ECX:%08lX EDX:%08lX EBX:%08lX EBP:%08lX ESI:%08lX EDI:%08lX EFLAGS:%08lX) stackMask=0x%04lX stack=",
                static_cast<unsigned long>(record.Side),
                static_cast<unsigned long>(record.ThreadId),
                static_cast<unsigned long>(record.TickCount),
                static_cast<unsigned long long>(record.QpcTicks),
                static_cast<unsigned long>(record.CallRva),
                static_cast<unsigned long>(record.ReturnRva),
                static_cast<unsigned long>(record.ReturnAddress),
                static_cast<unsigned long>(record.StackBase),
                static_cast<unsigned long>(record.ParentEbp),
                static_cast<unsigned long>(record.PlayerObject),
                static_cast<unsigned long>(record.CpuControl),
                static_cast<unsigned long>(record.QueueAddress),
                static_cast<unsigned long>(record.EventType),
                static_cast<unsigned long>(record.Payload),
                static_cast<unsigned long>(record.Eax),
                static_cast<unsigned long>(record.Ecx),
                static_cast<unsigned long>(record.Edx),
                static_cast<unsigned long>(record.Ebx),
                static_cast<unsigned long>(record.Ebp),
                static_cast<unsigned long>(record.Esi),
                static_cast<unsigned long>(record.Edi),
                static_cast<unsigned long>(record.Eflags),
                static_cast<unsigned long>(record.StackReadMask));

            if (used > 0)
            {
                for (DWORD i = 0; i < kAlternateStackWordCount &&
                    static_cast<std::size_t>(used) < sizeof(message) - 32; ++i)
                {
                    const int written = sprintf_s(
                        message + used, sizeof(message) - static_cast<std::size_t>(used),
                        "%sS+%02lX=%08lX",
                        i == 0 ? "" : ",",
                        static_cast<unsigned long>(i * sizeof(DWORD)),
                        static_cast<unsigned long>(record.StackWords[i]));
                    if (written <= 0) break;
                    used += written;
                }

                const int frameHeader = sprintf_s(
                    message + used, sizeof(message) - static_cast<std::size_t>(used),
                    " frameRead=%lu frame=",
                    static_cast<unsigned long>(record.FrameReadCount));
                if (frameHeader > 0) used += frameHeader;
            }

            if (used > 0)
            {
                bool firstFrame = true;
                for (DWORD i = 0; i < kAlternateFrameValueCount &&
                    static_cast<std::size_t>(used) < sizeof(message) - 32; ++i)
                {
                    if (record.FrameValid[i] == 0) continue;
                    const LONG displacement =
                        -static_cast<LONG>(kAlternateFrameBytesBefore) +
                        static_cast<LONG>(i * sizeof(DWORD));
                    char label[16]{};
                    if (displacement < 0)
                        sprintf_s(label, sizeof(label), "F-%03lX", static_cast<unsigned long>(-displacement));
                    else
                        sprintf_s(label, sizeof(label), "F+%03lX", static_cast<unsigned long>(displacement));
                    const int written = sprintf_s(
                        message + used, sizeof(message) - static_cast<std::size_t>(used),
                        "%s%s=%08lX",
                        firstFrame ? "" : ",",
                        label,
                        static_cast<unsigned long>(record.FrameWords[i]));
                    if (written <= 0) break;
                    used += written;
                    firstFrame = false;
                }
            }

            if (used > 0 && static_cast<std::size_t>(used) < sizeof(message) - 512)
            {
                const int codeHeader = sprintf_s(
                    message + used, sizeof(message) - static_cast<std::size_t>(used),
                    " codeRVA=0x%08lX codeRead=%lu code=",
                    static_cast<unsigned long>(record.CallRva - kAlternateCodeBytesBefore),
                    static_cast<unsigned long>(record.CodeReadCount));
                if (codeHeader > 0) used += codeHeader;
            }

            if (used > 0)
            {
                for (DWORD i = 0; i < kAlternateCodeByteCount &&
                    static_cast<std::size_t>(used) < sizeof(message) - 8; ++i)
                {
                    const int written = record.CodeValid[i]
                        ? sprintf_s(
                            message + used, sizeof(message) - static_cast<std::size_t>(used),
                            "%02X", static_cast<unsigned int>(record.CodeBytes[i]))
                        : sprintf_s(
                            message + used, sizeof(message) - static_cast<std::size_t>(used),
                            "??");
                    if (written <= 0) break;
                    used += written;
                }
            }

            AppendAlternateLogAtTime(&record.LocalTime, message);
            InterlockedExchange(&record.State, 2);
            InterlockedExchange(&g_alternateReadIndex, readIndex + 1);
        }
    }

    void FlushStage14ProbeRecords()
    {
        for (;;)
        {
            Stage14ProbeRecord record{};
            AcquireSRWLockExclusive(&g_stage14ProbeLock);
            if (g_stage14ProbeCount == 0)
            {
                ReleaseSRWLockExclusive(&g_stage14ProbeLock);
                break;
            }

            record = g_stage14ProbeRecords[g_stage14ProbeReadIndex];
            g_stage14ProbeReadIndex =
                (g_stage14ProbeReadIndex + 1u) % kStage14ProbeRecordCapacity;
            --g_stage14ProbeCount;
            ReleaseSRWLockExclusive(&g_stage14ProbeLock);

            if (std::strncmp(record.Message, "STAGE17_SETTER", 14) == 0)
                AppendStage17LogAtTime(&record.LocalTime, record.Message);
            else
                AppendStage14LogAtTime(&record.LocalTime, record.Message);
        }
    }

    void AppendStage14CodeWindow(const char* label, DWORD rva, DWORD byteCount)
    {
        if (g_gameBase == 0 || rva >= g_gameImageSize ||
            byteCount > g_gameImageSize - rva)
        {
            char warning[256]{};
            sprintf_s(warning, "STAGE14_CODE label=%s RVA=0x%08lX length=0x%lX outside image bounds.",
                label, static_cast<unsigned long>(rva),
                static_cast<unsigned long>(byteCount));
            AppendStage14Log(warning);
            return;
        }

        char header[256]{};
        sprintf_s(header, "STAGE14_CODE_BEGIN label=%s RVA=0x%08lX bytes=0x%lX",
            label, static_cast<unsigned long>(rva),
            static_cast<unsigned long>(byteCount));
        AppendStage14Log(header);

        for (DWORD offset = 0; offset < byteCount; offset += 32u)
        {
            char line[256]{};
            int used = sprintf_s(line, "STAGE14_CODE label=%s RVA=0x%08lX bytes=",
                label, static_cast<unsigned long>(rva + offset));
            if (used <= 0) continue;
            for (DWORD i = 0; i < 32u && offset + i < byteCount; ++i)
            {
                BYTE value = 0;
                const DWORD addressRva = rva + offset + i;
                bool readable = false;
                if (InterlockedCompareExchange(&g_alternateHookInstalled, 0, 0) != 0 &&
                    addressRva >= kAlternateQueueCallRva &&
                    addressRva < kAlternateQueueCallRva + 5u)
                {
                    value = g_alternateOriginalCallBytes[addressRva - kAlternateQueueCallRva];
                    readable = true;
                }
                else
                    readable = SafeReadByte(g_gameBase + addressRva, &value);
                const int written = readable
                    ? sprintf_s(line + used, sizeof(line) - static_cast<std::size_t>(used),
                        "%02X", static_cast<unsigned int>(value))
                    : sprintf_s(line + used, sizeof(line) - static_cast<std::size_t>(used), "??");
                if (written <= 0) break;
                used += written;
            }
            AppendStage14Log(line);
        }
    }

    bool MakeRelativeCall(BYTE* source, const void* destination, BYTE output[5])
    {
        if (source == nullptr || destination == nullptr || output == nullptr)
            return false;

        const std::intptr_t delta =
            reinterpret_cast<std::intptr_t>(destination) -
            (reinterpret_cast<std::intptr_t>(source) + 5);
        if (delta < INT32_MIN || delta > INT32_MAX)
            return false;

        output[0] = 0xE8;
        const std::int32_t relative = static_cast<std::int32_t>(delta);
        std::memcpy(output + 1, &relative, sizeof(relative));
        return true;
    }

    DWORD SuspendOtherProcessThreads(HANDLE* suspended, DWORD capacity)
    {
        if (suspended == nullptr || capacity == 0)
            return 0;

        const DWORD processId = GetCurrentProcessId();
        const DWORD currentThreadId = GetCurrentThreadId();
        DWORD count = 0;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
            return 0;

        THREADENTRY32 entry{};
        entry.dwSize = sizeof(entry);
        if (Thread32First(snapshot, &entry))
        {
            do
            {
                if (entry.th32OwnerProcessID == processId &&
                    entry.th32ThreadID != currentThreadId && count < capacity)
                {
                    HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, entry.th32ThreadID);
                    if (thread != nullptr)
                    {
                        if (SuspendThread(thread) != static_cast<DWORD>(-1))
                            suspended[count++] = thread;
                        else
                            CloseHandle(thread);
                    }
                }
                entry.dwSize = sizeof(entry);
            } while (Thread32Next(snapshot, &entry));
        }
        CloseHandle(snapshot);
        return count;
    }

    void ResumeThreads(HANDLE* suspended, DWORD count)
    {
        if (suspended == nullptr) return;
        while (count > 0)
        {
            HANDLE thread = suspended[--count];
            ResumeThread(thread);
            CloseHandle(thread);
        }
    }

    bool MakeRelativeJump(BYTE* source, const void* destination, BYTE output[5])
    {
        if (source == nullptr || destination == nullptr || output == nullptr)
            return false;

        const std::intptr_t delta =
            reinterpret_cast<std::intptr_t>(destination) -
            (reinterpret_cast<std::intptr_t>(source) + 5);
        if (delta < INT32_MIN || delta > INT32_MAX)
            return false;

        output[0] = 0xE9;
        const std::int32_t relative = static_cast<std::int32_t>(delta);
        std::memcpy(output + 1, &relative, sizeof(relative));
        return true;
    }

    bool ApplyStage14Patch(DWORD rva, const void* hook, DWORD patchLength,
        const BYTE* expectedBytes, const char* label)
    {
        if (hook == nullptr || expectedBytes == nullptr || patchLength < 5u ||
            patchLength > 16u || rva >= g_gameImageSize ||
            patchLength > g_gameImageSize - rva)
        {
            AppendStage14Log("ERROR: invalid Stage 14 patch parameters.");
            return false;
        }

        BYTE* target = reinterpret_cast<BYTE*>(
            static_cast<std::uintptr_t>(g_gameBase + rva));
        if (std::memcmp(target, expectedBytes, patchLength) != 0)
        {
            char message[320]{};
            sprintf_s(message,
                "ERROR: Stage 14 bytes at RVA=0x%08lX do not match the analyzed instruction window for %s; refusing to patch.",
                static_cast<unsigned long>(rva), label);
            AppendStage14Log(message);
            return false;
        }

        BYTE patch[16]{};
        if (!MakeRelativeJump(target, hook, patch))
        {
            AppendStage14Log("ERROR: Stage 14 relative JMP could not be generated.");
            return false;
        }
        for (DWORD i = 5; i < patchLength; ++i)
            patch[i] = 0x90;

        HANDLE suspended[512]{};
        const DWORD suspendedCapacity = static_cast<DWORD>(sizeof(suspended) / sizeof(suspended[0]));
        const DWORD suspendedCount = SuspendOtherProcessThreads(suspended, suspendedCapacity);

        DWORD oldProtect = 0;
        if (!VirtualProtect(target, patchLength, PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            ResumeThreads(suspended, suspendedCount);
            char message[256]{};
            sprintf_s(message, "ERROR: VirtualProtect failed for Stage 14 probe %s.", label);
            AppendStage14Log(message);
            return false;
        }

        std::memcpy(target, patch, patchLength);
        FlushInstructionCache(GetCurrentProcess(), target, patchLength);
        DWORD ignored = 0;
        VirtualProtect(target, patchLength, oldProtect, &ignored);
        ResumeThreads(suspended, suspendedCount);

        char message[384]{};
        sprintf_s(message,
            "Installed Stage 14 JMP probe %s at RVA=0x%08lX patchBytes=%lu suspended=%lu.",
            label, static_cast<unsigned long>(rva),
            static_cast<unsigned long>(patchLength),
            static_cast<unsigned long>(suspendedCount));
        AppendStage14Log(message);
        return true;
    }

    bool InstallStage14CallProbe(DWORD callRva, DWORD expectedTargetRva,
        const BYTE expectedBytes[5], void* hook, volatile DWORD* targetSlot,
        const char* label)
    {
        if (hook == nullptr || targetSlot == nullptr ||
            callRva + 5u >= g_gameImageSize || expectedTargetRva >= g_gameImageSize)
        {
            AppendStage14Log("ERROR: invalid Stage 14 CALL probe bounds or target.");
            return false;
        }

        BYTE* target = reinterpret_cast<BYTE*>(
            static_cast<std::uintptr_t>(g_gameBase + callRva));
        if (std::memcmp(target, expectedBytes, 5u) != 0 || target[0] != 0xE8)
        {
            char message[320]{};
            sprintf_s(message,
                "ERROR: Stage 14 CALL bytes at RVA=0x%08lX do not match for %s; refusing to patch.",
                static_cast<unsigned long>(callRva), label);
            AppendStage14Log(message);
            return false;
        }

        std::int32_t originalRelative = 0;
        std::memcpy(&originalRelative, target + 1, sizeof(originalRelative));
        const std::intptr_t originalDestination =
            reinterpret_cast<std::intptr_t>(target + 5) + originalRelative;
        if (originalDestination != static_cast<std::intptr_t>(g_gameBase + expectedTargetRva))
        {
            char message[320]{};
            sprintf_s(message,
                "ERROR: Stage 14 CALL at RVA=0x%08lX resolves to an unexpected target for %s; refusing to patch.",
                static_cast<unsigned long>(callRva), label);
            AppendStage14Log(message);
            return false;
        }

        BYTE patch[5]{};
        if (!MakeRelativeCall(target, hook, patch))
        {
            AppendStage14Log("ERROR: Stage 14 relative CALL could not be generated.");
            return false;
        }

        *targetSlot = g_gameBase + expectedTargetRva;

        HANDLE suspended[512]{};
        const DWORD suspendedCapacity = static_cast<DWORD>(sizeof(suspended) / sizeof(suspended[0]));
        const DWORD suspendedCount = SuspendOtherProcessThreads(suspended, suspendedCapacity);

        DWORD oldProtect = 0;
        if (!VirtualProtect(target, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            ResumeThreads(suspended, suspendedCount);
            char message[256]{};
            sprintf_s(message, "ERROR: VirtualProtect failed for Stage 14 CALL probe %s.", label);
            AppendStage14Log(message);
            return false;
        }

        std::memcpy(target, patch, sizeof(patch));
        FlushInstructionCache(GetCurrentProcess(), target, sizeof(patch));
        DWORD ignored = 0;
        VirtualProtect(target, sizeof(patch), oldProtect, &ignored);
        ResumeThreads(suspended, suspendedCount);

        char message[384]{};
        sprintf_s(message,
            "Installed Stage 14 CALL probe %s at RVA=0x%08lX -> target RVA=0x%08lX originalBytes=%02X %02X %02X %02X %02X suspended=%lu.",
            label,
            static_cast<unsigned long>(callRva),
            static_cast<unsigned long>(expectedTargetRva),
            expectedBytes[0], expectedBytes[1], expectedBytes[2],
            expectedBytes[3], expectedBytes[4],
            static_cast<unsigned long>(suspendedCount));
        AppendStage14Log(message);
        return true;
    }

    bool InstallStage15BranchHooks();

    bool InstallStage14ProducerHooks()
    {
        // Dump untouched code before installing any Stage 14 patches.
        AppendStage14Log("Stage 14 producer-call trace requested. Probes cover F69F0 entry, F2F50 input/return, and F2ED0 input/return.");
        AppendStage14CodeWindow("F69F0_PARENT", kStage14ParentEntryRva, 0x140u);
        AppendStage14CodeWindow("F2F50_PRODUCER", kStage14F2F50Rva, 0xC0u);
        AppendStage14CodeWindow("F2ED0_GUARD", kStage14F2ED0Rva, 0xC0u);
        if (ReadIniInt(L"AIResearch", L"EnableStage15BranchTrace", 0) != 0)
            AppendStage14CodeWindow("F6A5D_POST_GUARD_BRANCH_REGION", kStage15F6A5DBranchRva, 0xA0u);

        g_Stage14ParentContinuation = g_gameBase + kStage14ParentEntryRva + 6u;
        g_Stage14F2F50ReturnContinuation = g_gameBase + kStage14F2F50ReturnRva + 9u;
        g_Stage14F2ED0ReturnContinuation = g_gameBase + kStage14F2ED0ReturnRva + 5u;
        InterlockedExchange(&g_stage14ProbeEnabled, 1);

        DWORD installedCount = 0;

        // Install post-call probes first, before enabling their callsite shims.
        if (ApplyStage14Patch(kStage14F2F50ReturnRva,
            reinterpret_cast<const void*>(&ArcanaStage14F2F50ReturnHook),
            sizeof(kStage14F2F50ReturnBytes), kStage14F2F50ReturnBytes, "F2F50_RETURN"))
            ++installedCount;

        if (ApplyStage14Patch(kStage14F2ED0ReturnRva,
            reinterpret_cast<const void*>(&ArcanaStage14F2ED0ReturnHook),
            sizeof(kStage14F2ED0ReturnBytes), kStage14F2ED0ReturnBytes, "F2ED0_RETURN"))
            ++installedCount;

        if (InstallStage14CallProbe(kStage14F2F50CallRva, kStage14F2F50Rva,
            kStage14F2F50CallBytes,
            reinterpret_cast<void*>(&ArcanaStage14F2F50CallHook),
            &g_Stage14F2F50Target, "F2F50_CALL"))
            ++installedCount;

        if (InstallStage14CallProbe(kStage14F2ED0CallRva, kStage14F2ED0Rva,
            kStage14F2ED0CallBytes,
            reinterpret_cast<void*>(&ArcanaStage14F2ED0CallHook),
            &g_Stage14F2ED0Target, "F2ED0_CALL"))
            ++installedCount;

        // Stage 15 branch probes must be installed before the parent-entry hook
        // is enabled, so every new F69F0 invocation sees the complete probe set.
        InterlockedExchange(&g_stage15Installed, 0);
        if (ReadIniInt(L"AIResearch", L"EnableStage15BranchTrace", 0) != 0)
        {
            const bool stage15Ready = InstallStage15BranchHooks();
            if (!stage15Ready)
                AppendStage14Log("WARNING: Stage 15 branch probes were not fully installed before enabling the F69F0 parent-entry hook.");
        }

        // Parent entry is enabled last, after all Stage 14 and Stage 15 interior probes are ready.
        if (ApplyStage14Patch(kStage14ParentEntryRva,
            reinterpret_cast<const void*>(&ArcanaStage14ParentEntryHook),
            sizeof(kStage14ParentEntryBytes), kStage14ParentEntryBytes, "F69F0_PARENT_ENTRY"))
            ++installedCount;

        char summary[256]{};
        sprintf_s(summary,
            "Stage 14 probes installed=%lu/5. F2F50 raw return and F2ED0 AL result are recorded separately.",
            static_cast<unsigned long>(installedCount));
        AppendStage14Log(summary);
        return installedCount != 0;
    }

    bool InstallStage15BranchHooks()
    {
        if (InterlockedCompareExchange(&g_stage14ProbeEnabled, 0, 0) == 0)
        {
            AppendStage14Log("ERROR: Stage 15 branch probes require the Stage 14 probe logger; Stage 15 was not installed.");
            return false;
        }

        AppendStage14Log("Stage 15 branch-flow trace requested. Five probes observe the post-F2ED0 branch, payload comparison, player bit 17, local guard, and secondary-helper test.");

        g_Stage15F6A5DTarget = g_gameBase + 0x000F6AE8u;
        g_Stage15F6A63Continuation = g_gameBase + 0x000F6A63u;
        g_Stage15F6A63Target = g_gameBase + 0x000F6AC8u;
        g_Stage15F6A68Continuation = g_gameBase + 0x000F6A68u;
        g_Stage15F6A68Target = g_gameBase + 0x000F6A9Du;
        g_Stage15F6A75Continuation = g_gameBase + 0x000F6A75u;
        g_Stage15F6A9DTarget = g_gameBase + 0x000F6AC8u;
        g_Stage15F6AA3Continuation = g_gameBase + 0x000F6AA3u;
        g_Stage15F6AACTarget = g_gameBase + 0x000F6AC8u;
        g_Stage15F6AB4Continuation = g_gameBase + 0x000F6AB4u;

        DWORD installedCount = 0;
        if (ApplyStage14Patch(kStage15F6A5DBranchRva,
            reinterpret_cast<const void*>(&ArcanaStage15F6A5DBranchHook),
            sizeof(kStage15F6A5DBranchBytes), kStage15F6A5DBranchBytes, "Stage15_F6A5D_F2ED0_BRANCH"))
            ++installedCount;

        if (ApplyStage14Patch(kStage15F6A63PayloadBranchRva,
            reinterpret_cast<const void*>(&ArcanaStage15F6A63PayloadBranchHook),
            sizeof(kStage15F6A63PayloadBranchBytes), kStage15F6A63PayloadBranchBytes, "Stage15_F6A63_PAYLOAD_BRANCH"))
            ++installedCount;

        if (ApplyStage14Patch(kStage15F6A68Bit17BranchRva,
            reinterpret_cast<const void*>(&ArcanaStage15F6A68Bit17BranchHook),
            sizeof(kStage15F6A68Bit17BranchBytes), kStage15F6A68Bit17BranchBytes, "Stage15_F6A68_BIT17_BRANCH"))
            ++installedCount;

        if (ApplyStage14Patch(kStage15F6A9DLocalBranchRva,
            reinterpret_cast<const void*>(&ArcanaStage15F6A9DLocalBranchHook),
            sizeof(kStage15F6A9DLocalBranchBytes), kStage15F6A9DLocalBranchBytes, "Stage15_F6A9D_LOCAL_GUARD_BRANCH"))
            ++installedCount;

        if (ApplyStage14Patch(kStage15F6AACHelperBranchRva,
            reinterpret_cast<const void*>(&ArcanaStage15F6AACHelperBranchHook),
            sizeof(kStage15F6AACHelperBranchBytes), kStage15F6AACHelperBranchBytes, "Stage15_F6AAC_SECONDARY_HELPER_BRANCH"))
            ++installedCount;

        char summary[256]{};
        sprintf_s(summary,
            "Stage 15 branch probes installed=%lu/5. Original Jcc outcomes and overwritten instructions are replayed by the probes.",
            static_cast<unsigned long>(installedCount));
        AppendStage14Log(summary);
        InterlockedExchange(&g_stage15Installed, installedCount != 0 ? 1 : 0);
        return installedCount != 0;
    }

    bool InstallStage17Bit113SetterHook()
    {
        if (kStage17Bit113SetterRva + sizeof(kStage17Bit113SetterBytes) >= g_gameImageSize)
        {
            AppendStage17Log("ERROR: setter RVA 0xFAA0 is outside the verified AALib.exe image.");
            return false;
        }
        BYTE* target = reinterpret_cast<BYTE*>(static_cast<std::uintptr_t>(g_gameBase + kStage17Bit113SetterRva));
        if (std::memcmp(target, kStage17Bit113SetterBytes, sizeof(kStage17Bit113SetterBytes)) != 0)
        {
            AppendStage17Log("ERROR: AALib.exe+0xFAA0 prologue does not match the verified bitfield setter; refusing to patch.");
            return false;
        }
        g_Stage17Bit113SetterContinuation = g_gameBase + kStage17Bit113SetterContinuationRva;
        InterlockedExchange(&g_stage14ProbeEnabled, 1);
        const bool installed = ApplyStage14Patch(kStage17Bit113SetterRva,
            reinterpret_cast<const void*>(&ArcanaStage17Bit113SetterEntryHook),
            sizeof(kStage17Bit113SetterBytes), kStage17Bit113SetterBytes,
            "Stage17_BIT113_SETTER_ENTRY");
        if (!installed)
        {
            AppendStage17Log("ERROR: bit-index 0x71 setter entry hook was not installed.");
            return false;
        }
        InterlockedExchange(&g_stage17Installed, 1);
        AppendStage17Log("Installed setter entry hook at RVA=0x0000FAA0; continuation=0x0000FAA7; seven-byte prologue validated.");
        AppendStage17Log("Only incoming bit-index 0x71 calls are recorded; each record decodes the original caller E8 instruction.");
        return true;
    }

    bool InstallFinalCommandCallHook()
    {
        BYTE* target = reinterpret_cast<BYTE*>(
            static_cast<std::uintptr_t>(g_gameBase + kFinalCommandCallRva));
        if (target == nullptr)
        {
            AppendLog("ERROR: final-command callsite address is null.");
            return false;
        }

        if (std::memcmp(target, kExpectedCallBytes, sizeof(kExpectedCallBytes)) != 0)
        {
            AppendLog("ERROR: callsite bytes at AALib.exe+0xF7C5D do not match E8 7E B1 FF FF; refusing to patch.");
            return false;
        }

        std::int32_t originalRelative = 0;
        std::memcpy(&originalRelative, target + 1, sizeof(originalRelative));
        const std::intptr_t originalDestination =
            reinterpret_cast<std::intptr_t>(target + 5) + originalRelative;
        if (originalDestination != static_cast<std::intptr_t>(g_gameBase + kQueueInsertRva))
        {
            AppendLog("ERROR: callsite destination is not AALib.exe+0xF2DE0; refusing to patch.");
            return false;
        }

        BYTE patch[5]{};
        if (!MakeRelativeCall(target, reinterpret_cast<const void*>(&ArcanaFinalCommandCallHook), patch))
        {
            AppendLog("ERROR: final-command hook is outside rel32 call range.");
            return false;
        }

        HANDLE suspended[512]{};
        const DWORD suspendedCapacity = static_cast<DWORD>(sizeof(suspended) / sizeof(suspended[0]));
        const DWORD suspendedCount = SuspendOtherProcessThreads(suspended, suspendedCapacity);

        DWORD oldProtect = 0;
        if (!VirtualProtect(target, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            ResumeThreads(suspended, suspendedCount);
            AppendLog("ERROR: VirtualProtect failed for final-command callsite.");
            return false;
        }

        // Publish the tail-jump destination before any game thread can execute
        // the newly patched CALL instruction.
        g_ArcanaQueueInsertTarget = g_gameBase + kQueueInsertRva;
        std::memcpy(target, patch, sizeof(patch));
        FlushInstructionCache(GetCurrentProcess(), target, sizeof(patch));
        InterlockedExchange(&g_hookInstalled, 1);

        DWORD ignored = 0;
        VirtualProtect(target, sizeof(patch), oldProtect, &ignored);
        ResumeThreads(suspended, suspendedCount);

        char message[320]{};
        sprintf_s(message,
            "Installed passive final-command callsite hook at RVA=0x%08lX; call destination remains queue helper RVA=0x%08lX; suspended=%lu thread(s).",
            static_cast<unsigned long>(kFinalCommandCallRva),
            static_cast<unsigned long>(kQueueInsertRva),
            static_cast<unsigned long>(suspendedCount));
        AppendLog(message);
        return true;
    }

    bool InstallAlternateQueueCallHook()
    {
        BYTE* target = reinterpret_cast<BYTE*>(
            static_cast<std::uintptr_t>(g_gameBase + kAlternateQueueCallRva));
        if (target == nullptr ||
            g_ArcanaQueueInsertTarget != g_gameBase + kQueueInsertRva)
        {
            AppendAlternateLog("ERROR: alternate callsite address or shared queue-helper target is invalid.");
            return false;
        }

        if (kAlternateQueueCallRva + 5u >= g_gameImageSize ||
            kAlternateQueueCallRva < kAlternateCodeBytesBefore)
        {
            AppendAlternateLog("ERROR: alternate callsite is outside the analyzed AALib.exe image bounds.");
            return false;
        }

        if (target[0] != 0xE8)
        {
            AppendAlternateLog("ERROR: AALib.exe+0xF6AD7 is not a five-byte near CALL; refusing to patch.");
            return false;
        }

        std::int32_t originalRelative = 0;
        std::memcpy(&originalRelative, target + 1, sizeof(originalRelative));
        const std::intptr_t originalDestination =
            reinterpret_cast<std::intptr_t>(target + 5) + originalRelative;
        if (originalDestination != static_cast<std::intptr_t>(g_gameBase + kQueueInsertRva))
        {
            AppendAlternateLog("ERROR: AALib.exe+0xF6AD7 does not call the known queue helper RVA 0xF2DE0; refusing to patch.");
            return false;
        }

        std::memcpy(g_alternateOriginalCallBytes, target, sizeof(g_alternateOriginalCallBytes));

        BYTE patch[5]{};
        if (!MakeRelativeCall(target, reinterpret_cast<const void*>(&ArcanaAlternateQueueCallHook), patch))
        {
            AppendAlternateLog("ERROR: alternate callsite hook is outside rel32 call range.");
            return false;
        }

        HANDLE suspended[512]{};
        const DWORD suspendedCapacity = static_cast<DWORD>(sizeof(suspended) / sizeof(suspended[0]));
        const DWORD suspendedCount = SuspendOtherProcessThreads(suspended, suspendedCapacity);

        DWORD oldProtect = 0;
        if (!VirtualProtect(target, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect))
        {
            ResumeThreads(suspended, suspendedCount);
            AppendAlternateLog("ERROR: VirtualProtect failed for alternate callsite.");
            return false;
        }

        std::memcpy(target, patch, sizeof(patch));
        FlushInstructionCache(GetCurrentProcess(), target, sizeof(patch));
        InterlockedExchange(&g_alternateHookInstalled, 1);

        DWORD ignored = 0;
        VirtualProtect(target, sizeof(patch), oldProtect, &ignored);
        ResumeThreads(suspended, suspendedCount);

        char message[512]{};
        sprintf_s(message,
            "Installed Stage 13 alternate queue callsite hook at RVA=0x%08lX (return RVA=0x%08lX); original destination=queue helper RVA=0x%08lX; original bytes=%02X %02X %02X %02X %02X; suspended=%lu thread(s).",
            static_cast<unsigned long>(kAlternateQueueCallRva),
            static_cast<unsigned long>(kAlternateQueueReturnRva),
            static_cast<unsigned long>(kQueueInsertRva),
            g_alternateOriginalCallBytes[0], g_alternateOriginalCallBytes[1],
            g_alternateOriginalCallBytes[2], g_alternateOriginalCallBytes[3],
            g_alternateOriginalCallBytes[4],
            static_cast<unsigned long>(suspendedCount));
        AppendAlternateLog(message);
        return true;
    }

    DWORD WINAPI FinalCommandTraceWorker(void*)
    {
        const int enabled = ReadIniInt(L"AIResearch", L"EnableFinalCommandTrace", 0);
        int flushMs = ReadIniInt(L"AIResearch", L"FinalCommandTraceFlushIntervalMs", kDefaultFlushIntervalMs);
        if (flushMs < static_cast<int>(kMinFlushIntervalMs)) flushMs = kMinFlushIntervalMs;
        if (flushMs > static_cast<int>(kMaxFlushIntervalMs)) flushMs = kMaxFlushIntervalMs;
        g_flushIntervalMs = static_cast<DWORD>(flushMs);

        const int alternateEnabled = ReadIniInt(
            L"AIResearch", L"EnableAlternateCallsiteTrace", 0);
        int alternateFlushMs = ReadIniInt(
            L"AIResearch", L"AlternateCallsiteTraceFlushIntervalMs", kDefaultFlushIntervalMs);
        if (alternateFlushMs < static_cast<int>(kMinFlushIntervalMs))
            alternateFlushMs = kMinFlushIntervalMs;
        if (alternateFlushMs > static_cast<int>(kMaxFlushIntervalMs))
            alternateFlushMs = kMaxFlushIntervalMs;
        g_alternateFlushIntervalMs = static_cast<DWORD>(alternateFlushMs);

        const int stage14Enabled = ReadIniInt(
            L"AIResearch", L"EnableStage14ProducerCallTrace", 0);
        const int stage15Enabled = ReadIniInt(
            L"AIResearch", L"EnableStage15BranchTrace", 0);
        const int stage17Enabled = ReadIniInt(
            L"AIResearch", L"EnableStage17Bit113SetterTrace", 0);
        int stage14FlushMs = ReadIniInt(
            L"AIResearch", L"Stage14ProducerCallTraceFlushIntervalMs", kStage14FlushDefaultMs);
        if (stage14FlushMs < static_cast<int>(kMinFlushIntervalMs))
            stage14FlushMs = kMinFlushIntervalMs;
        if (stage14FlushMs > static_cast<int>(kMaxFlushIntervalMs))
            stage14FlushMs = kMaxFlushIntervalMs;
        g_stage14FlushIntervalMs = static_cast<DWORD>(stage14FlushMs);

        if (enabled == 0 && alternateEnabled == 0 && stage14Enabled == 0 && stage15Enabled == 0 && stage17Enabled == 0)
        {
            AppendLog("Stage 11, Stage 13, Stage 14 and Stage 15 callsite traces are all disabled in ArcanaAI.ini. No Stage 7 callsite hooks were installed.");
            AppendAlternateLog("Stage 13 alternate-callsite trace is disabled.");
            AppendStage14Log("Stage 14 producer-call trace is disabled.");
            AppendStage14Log("Stage 15 branch-flow trace is disabled.");
            AppendStage17Log("Stage17 bit-113 setter trace is disabled in ArcanaAI.ini.");
            return 0;
        }

        for (int attempt = 0; attempt < 300; ++attempt)
        {
            g_gameModule = GetModuleHandleW(L"AALib.exe");
            if (g_gameModule != nullptr) break;
            Sleep(100);
        }
        if (g_gameModule == nullptr)
        {
            AppendLog("ERROR: AALib.exe was not found; final-command trace stopped.");
            if (stage17Enabled != 0) AppendStage17Log("ERROR: AALib.exe was not found; Stage17 stopped.");
            return 0;
        }

        g_gameBase = static_cast<DWORD>(reinterpret_cast<std::uintptr_t>(g_gameModule));
        DWORD timestamp = 0;
        if (!GetImageFingerprint32(g_gameModule, &g_gameImageSize, &timestamp) ||
            g_gameImageSize != kExpectedImageSize || timestamp != kExpectedTimeDateStamp ||
            kFinalCommandCallRva + sizeof(kExpectedCallBytes) >= g_gameImageSize ||
            kQueueInsertRva >= g_gameImageSize ||
            kP1ControlRva < kPlayerControlOffset || kP2ControlRva < kPlayerControlOffset)
        {
            AppendLog("ERROR: AALib.exe PE fingerprint does not match the analyzed binary; no callsite hook was installed.");
            if (stage17Enabled != 0) AppendStage17Log("ERROR: AALib.exe fingerprint mismatch; Stage17 stopped.");
            return 0;
        }

        g_playerP1 = g_gameBase + kP1ControlRva - kPlayerControlOffset;
        g_playerP2 = g_gameBase + kP2ControlRva - kPlayerControlOffset;

        if (enabled != 0)
        {
            char start[640]{};
            sprintf_s(start,
                "Stage 11 final-command trace requested. Player identity is derived from the exact queue pointer; original EDI is logged separately. Records type-1 events from callsite RVA 0xF7C62 for exact P1/P2 CPU queues.");
            AppendLog(start);
        }
        else
            AppendLog("Final-command trace is disabled; Stage 13 alternate-callsite trace may still run independently.");

        if (alternateEnabled != 0)
            AppendAlternateLog("Stage 13 alternate-callsite trace requested. Captures registers, stack arguments, EBP frame window, and original code bytes for the call at RVA 0xF6AD7 (return RVA 0xF6ADC).");
        else
            AppendAlternateLog("Stage 13 alternate-callsite trace is disabled in ArcanaAI.ini.");

        if (stage14Enabled != 0)
            AppendStage14Log("Stage 14 producer-call trace enabled in ArcanaAI.ini.");
        else
            AppendStage14Log("Stage 14 producer-call trace disabled in ArcanaAI.ini.");

        if (stage15Enabled != 0 && stage14Enabled != 0)
            AppendStage14Log("Stage 15 branch-flow trace enabled in ArcanaAI.ini; records are written into ArcanaAI_ProducerCallTrace.log.");
        else if (stage15Enabled != 0)
            AppendStage14Log("ERROR: Stage 15 branch-flow trace requires EnableStage14ProducerCallTrace=1; Stage 15 will not be installed.");
        else
            AppendStage14Log("Stage 15 branch-flow trace disabled in ArcanaAI.ini.");

        if (stage17Enabled != 0)
            AppendStage17Log("Stage17 bit-113 setter trace enabled; only incoming argument 0x71 is recorded.");
        else
            AppendStage17Log("Stage17 bit-113 setter trace disabled in ArcanaAI.ini.");

        // Stage 5 and Stage 7 both patch executable code. Use the same named
        // mutex so their brief thread-suspension/patch windows cannot overlap.
        HANDLE patchMutex = CreateMutexW(nullptr, FALSE, L"Local\\ArcanaAIOverhaul_PatchInstall_Mutex");
        if (patchMutex == nullptr)
        {
            AppendLog("ERROR: could not create shared patch-install mutex; Stage 7 hook was not installed.");
            return 0;
        }

        const DWORD mutexWait = WaitForSingleObject(patchMutex, INFINITE);
        if (mutexWait != WAIT_OBJECT_0 && mutexWait != WAIT_ABANDONED)
        {
            AppendLog("ERROR: could not acquire shared patch-install mutex; Stage 7 hook was not installed.");
            CloseHandle(patchMutex);
            return 0;
        }

        // Publish the verified helper address for either callsite shim. The
        // Stage 13 observer can run even when the Stage 11 F7C5D observer is off.
        g_ArcanaQueueInsertTarget = g_gameBase + kQueueInsertRva;
        const bool installed = enabled != 0 ? InstallFinalCommandCallHook() : false;
        const bool alternateInstalled =
            alternateEnabled != 0 ? InstallAlternateQueueCallHook() : false;
        const bool stage14Installed =
            stage14Enabled != 0 ? InstallStage14ProducerHooks() : false;
        const bool stage15Installed =
            (stage15Enabled != 0 && stage14Enabled != 0) ?
                (InterlockedCompareExchange(&g_stage15Installed, 0, 0) != 0) : false;
        const bool stage17Installed = stage17Enabled != 0 ? InstallStage17Bit113SetterHook() : false;
        ReleaseMutex(patchMutex);
        CloseHandle(patchMutex);
        if (enabled != 0 && !installed)
            AppendLog("WARNING: Stage 11 final-command callsite hook was not installed.");
        if (alternateEnabled != 0 && !alternateInstalled)
            AppendAlternateLog("WARNING: Stage 13 alternate callsite hook was not installed.");
        if (stage14Enabled != 0 && !stage14Installed)
            AppendStage14Log("WARNING: no Stage 14 producer-call probes were installed.");
        if (stage15Enabled != 0 && stage14Enabled != 0 && !stage15Installed)
            AppendStage14Log("WARNING: no Stage 15 branch-flow probes were installed.");
        if (stage17Enabled != 0 && !stage17Installed)
            AppendStage17Log("WARNING: Stage17 setter hook was not installed.");
        if (!installed && !alternateInstalled && !stage14Installed && !stage15Installed && !stage17Installed)
            return 0;

        DWORD lastAlternateFlushTick = GetTickCount();
        DWORD lastStage14FlushTick = GetTickCount();
        for (;;)
        {
            Sleep(g_flushIntervalMs);
            FlushReadyRecords();
            const DWORD currentTick = GetTickCount();

            if ((stage14Enabled != 0 || stage17Installed) &&
                currentTick - lastStage14FlushTick >= g_stage14FlushIntervalMs)
            {
                FlushStage14ProbeRecords();
                lastStage14FlushTick = currentTick;
            }

            if (alternateEnabled != 0 &&
                currentTick - lastAlternateFlushTick >= g_alternateFlushIntervalMs)
            {
                FlushAlternateReadyRecords();
                lastAlternateFlushTick = currentTick;
            }

            const LONG alternateDropped = InterlockedCompareExchange(&g_alternateDroppedRecords, 0, 0);
            const LONG alternateReported = InterlockedCompareExchange(&g_alternateReportedDroppedRecords, 0, 0);
            if (alternateDropped > alternateReported)
            {
                char alternateWarning[180]{};
                sprintf_s(alternateWarning,
                    "WARNING: alternate callsite record capacity reached; dropped records=%ld (capacity=%lu).",
                    static_cast<long>(alternateDropped),
                    static_cast<unsigned long>(kAlternateRecordCapacity));
                AppendAlternateLog(alternateWarning);
                InterlockedExchange(&g_alternateReportedDroppedRecords, alternateDropped);
            }

            const LONG stage14Dropped = InterlockedCompareExchange(&g_stage14ProbeDropped, 0, 0);
            const LONG stage14Reported = InterlockedCompareExchange(&g_stage14ProbeReportedDropped, 0, 0);
            if (stage14Dropped > stage14Reported)
            {
                char stage14Warning[192]{};
                sprintf_s(stage14Warning,
                    "WARNING: Stage 14 producer-probe ring full; dropped records=%ld (capacity=%lu).",
                    static_cast<long>(stage14Dropped),
                    static_cast<unsigned long>(kStage14ProbeRecordCapacity));
                AppendStage14Log(stage14Warning);
                InterlockedExchange(&g_stage14ProbeReportedDropped, stage14Dropped);
            }

            const LONG dropped = InterlockedCompareExchange(&g_droppedRecords, 0, 0);
            const LONG reported = InterlockedCompareExchange(&g_reportedDroppedRecords, 0, 0);
            if (dropped > reported)
            {
                char warning[160]{};
                sprintf_s(warning,
                    "WARNING: final-command trace record capacity reached; dropped records=%ld (capacity=%lu).",
                    static_cast<long>(dropped), static_cast<unsigned long>(kRecordCapacity));
                AppendLog(warning);
                InterlockedExchange(&g_reportedDroppedRecords, dropped);
            }
        }
    }

    BOOL CALLBACK StartFinalCommandTraceWorker(PINIT_ONCE, PVOID, PVOID*)
    {
        HANDLE thread = CreateThread(nullptr, 0, FinalCommandTraceWorker, nullptr, 0, nullptr);
        if (thread == nullptr)
        {
            AppendLog("ERROR: failed to create final-command trace worker thread.");
            return TRUE;
        }
        CloseHandle(thread);
        return TRUE;
    }
}

extern "C" void __cdecl ArcanaLogFinalCommandSite(
    DWORD parentEbp,
    DWORD playerObject,
    DWORD payloadRegister,
    DWORD queueAddress,
    DWORD eventType,
    DWORD payloadArgument,
    DWORD callReturnAddress)
{
    QueueFinalCommandRecord(
        parentEbp, playerObject, payloadRegister,
        queueAddress, eventType, payloadArgument, callReturnAddress);
}

extern "C" void __cdecl ArcanaLogAlternateQueueCallSite(
    DWORD stackBase,
    DWORD eaxRegister,
    DWORD ecxRegister,
    DWORD edxRegister,
    DWORD ebxRegister,
    DWORD ebpRegister,
    DWORD esiRegister,
    DWORD ediRegister,
    DWORD eflagsRegister)
{
    QueueAlternateCallRecord(
        stackBase, eaxRegister, ecxRegister, edxRegister, ebxRegister,
        ebpRegister, esiRegister, ediRegister, eflagsRegister);
}

extern "C" void __cdecl ArcanaLogStage14Probe(
    DWORD probeId,
    const DWORD* savedContext)
{
    QueueStage14ProbeRecord(probeId, savedContext);
}

void StartAICommandSiteTrace(HMODULE proxyModule)
{
    g_proxyModule = proxyModule;
    InitOnceExecuteOnce(&g_startOnce, StartFinalCommandTraceWorker, nullptr, nullptr);
}
