#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

int isClicking = 0;
int clickPerSecond = 30;
int isRunning = 1;
int isCtrlPressed = 0;

#define SCRIPT ".\\run.bat gl"
#define PROCESS_NAME "geometryWars.exe"

void EnableDebugPriv()
{
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkp;

    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, 0, &tkp, sizeof(tkp), NULL, NULL);

    CloseHandle(hToken); 
}

void CloseProcess(char *name){
    
    EnableDebugPriv();

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (stricmp(entry.szExeFile, name) == 0)
            {  
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
                TerminateProcess(hProcess, 1);
                CloseHandle(hProcess);
            }
        }
    }

    CloseHandle(snapshot);
}

DWORD WINAPI RunScript(LPVOID lpParam) {
    CloseProcess(PROCESS_NAME);

    printf("Running '%s'...\n", SCRIPT);
    system(SCRIPT);

    CloseHandle(GetCurrentThread());
    return 0;
}


LRESULT CALLBACK HookProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
        if (wParam == WM_KEYDOWN)
        {
            if (p->vkCode == 'D' && isCtrlPressed)
            {
                // I'm using a separate thread because for some reason closing the app via ALT+F4 takes some time
                // probably messasge queue is broken or I'm not using it properly. This doesn't happen if I click Cross on the app itself
                CreateThread(NULL, 0, RunScript, NULL, 0, NULL);
                isCtrlPressed = 0;
            }

            if (p->vkCode == 162)
                isCtrlPressed = 1;
        }

        if (wParam == WM_KEYUP && p->vkCode == 162)
        {
            isCtrlPressed = 0;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


int main()
{
    SetWindowsHookExA(WH_KEYBOARD_LL, HookProc, 0, 0);
    MSG msg;
    while (isRunning)
    {
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(14);
    }
    return 0;
}