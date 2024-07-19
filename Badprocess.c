#include <stdio.h>
#include <Windows.h>

typedef long (WINAPI* RtlSetProcessIsCritical) (
    IN BOOLEAN    bNew, // / new status of process, TRUE for Critical, FALSE for Normal
    OUT BOOLEAN* pbOld,
    IN BOOLEAN    bNeedScb);

BOOL EnablePriv(LPCSTR lpszPriv)
{
    HANDLE hToken;
    LUID luid;//local identifier for priv management
    TOKEN_PRIVILEGES tkprivs;
    ZeroMemory(&tkprivs, sizeof(tkprivs));

    if (!OpenProcessToken(GetCurrentProcess(), (TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY), &hToken))//gets the handle for the token to use it for changing priv
        return FALSE;

    if (!LookupPrivilegeValue(NULL, lpszPriv, &luid)) { //passes the SE_DEBUG_NAME option to adjust
        CloseHandle(hToken); return FALSE;
    }

    tkprivs.PrivilegeCount = 1;
    tkprivs.Privileges[0].Luid = luid;
    tkprivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL bRet = AdjustTokenPrivileges(hToken, FALSE, &tkprivs, sizeof(tkprivs), NULL, NULL);
    CloseHandle(hToken);
    return bRet;
}


BOOL SetCriticalProcess()
{
    HANDLE hDLL;
    RtlSetProcessIsCritical fSetCriticalProcess;

    hDLL = LoadLibraryA("ntdll.dll"); //Gets the handle to ntdll.dll
    if (hDLL != NULL)
    {
        EnablePriv(SE_DEBUG_NAME);
        fSetCriticalProcess = (RtlSetProcessIsCritical)GetProcAddress((HINSTANCE)hDLL, "RtlSetProcessIsCritical"); //acesses ntdll with the help of GetProcAddress to use RtlSetProcessIsCritical

        if (!fSetCriticalProcess) return 0;
        fSetCriticalProcess(1, 0, 0);
        FreeLibrary(hDLL);
        return 1;
    }
    else
        FreeLibrary(hDLL);
    return 0;
}

int main()
{
    STARTUPINFOW si = { 0 };
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = { 0 };
    // Create the child process


    if (SetCriticalProcess()) {
        //crash-y Stuff
        //FreeConsole(); UNCOMMENT TO CRASH ON EXECUTION
       // MessageBoxA(NULL, "DON'T KILL ME", "WARNING!", MB_ICONHAND | MB_OK);



        DWORD dwPid = GetProcessId(pi.hProcess);
        printf("PID: %d\nTHIS PROCESS IS CRITICAL DO NOT TRY TO KILL IT", &dwPid);
        getch(); //keeps terminal open
    }


    else {
        MessageBoxA(NULL, "Unable to set the process as critical", "ERROR!", MB_ICONHAND | MB_OK);
    }

}
