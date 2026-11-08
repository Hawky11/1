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

        if (enabled == 0 && alternateEnabled == 0)
        {
            AppendLog("Final-command and alternate-callsite traces are both disabled in ArcanaAI.ini. No code hook was installed.");
            AppendAlternateLog("Both Stage 11 and Stage 13 callsite traces are disabled in ArcanaAI.ini.");
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
        ReleaseMutex(patchMutex);
        CloseHandle(patchMutex);
        if (enabled != 0 && !installed)
            AppendLog("WARNING: Stage 11 final-command callsite hook was not installed.");
        if (alternateEnabled != 0 && !alternateInstalled)
            AppendAlternateLog("WARNING: Stage 13 alternate callsite hook was not installed.");
        if (!installed && !alternateInstalled)
            return 0;

        DWORD lastAlternateFlushTick = GetTickCount();
        for (;;)
        {
            Sleep(g_flushIntervalMs);
            FlushReadyRecords();
            const DWORD currentTick = GetTickCount();
            if (currentTick - lastAlternateFlushTick >= g_alternateFlushIntervalMs)
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

void StartAICommandSiteTrace(HMODULE proxyModule)
{
    g_proxyModule = proxyModule;
    InitOnceExecuteOnce(&g_startOnce, StartFinalCommandTraceWorker, nullptr, nullptr);
}
