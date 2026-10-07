/*
 * GetThreadId is a Vista API; on the systems it targets, the fallback calls
 * NtQueryInformationThread and answers ClientId.UniqueThread:
 * https://github.com/Chuyu-Team/YY-Thunks/blob/bb5ea67de93c323fd3c77d04946dd83497f06ea8/src/Thunks/api-ms-win-core-processthreads.hpp#L130-L172
 * https://github.com/Chuyu-Team/YY-Thunks/blob/bb5ea67de93c323fd3c77d04946dd83497f06ea8/ThunksList.md#L478
 * https://stackoverflow.com/questions/1514969/getthreadid-on-pre-vista-systems
 */
#include <windows.h>
#include <winternl.h>

typedef struct pppp_thread_basic_information {
    LONG ExitStatus;
    PVOID TebBaseAddress;
    CLIENT_ID ClientId;
    ULONG_PTR AffinityMask;
    LONG Priority;
    LONG BasePriority;
} pppp_thread_basic_information;

static DWORD WINAPI pppp_fallback_GetThreadId(HANDLE hThread)
{
    union {
        FARPROC raw;
        DWORD(WINAPI * get_thread_id)(HANDLE);
    } loader;
    union {
        FARPROC raw;
        NTSTATUS(NTAPI * query_information_thread)(HANDLE, THREADINFOCLASS, PVOID, ULONG, PULONG);
    } query;
    HMODULE module;
    pppp_thread_basic_information information;
    NTSTATUS status;

    module = GetModuleHandleW(L"kernel32.dll");
    if (module != NULL) {
        loader.raw = GetProcAddress(module, "GetThreadId");
        if (loader.get_thread_id != NULL) {
            return loader.get_thread_id(hThread);
        }
    }

    module = GetModuleHandleW(L"ntdll.dll");
    if (module == NULL) {
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return 0;
    }
    query.raw = GetProcAddress(module, "NtQueryInformationThread");
    if (query.query_information_thread == NULL) {
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return 0;
    }

    status = query.query_information_thread(hThread, ThreadBasicInformation, &information,
                                            sizeof(information), NULL);
    if (status < 0) {
        SetLastError(ERROR_INVALID_HANDLE);
        return 0;
    }

    return (DWORD)(ULONG_PTR)information.ClientId.UniqueThread;
}

#if defined(__x86_64__)
DWORD (WINAPI *const pppp_imp_GetThreadId)(HANDLE)
    __asm__("__imp_GetThreadId") = pppp_fallback_GetThreadId;
#else
DWORD (WINAPI *const pppp_imp_GetThreadId)(HANDLE)
    __asm__("__imp__GetThreadId@4") = pppp_fallback_GetThreadId;
#endif
