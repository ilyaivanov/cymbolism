#include <windows.h>
#include <stdio.h>

int isClicking = 0;
int clickPerSecond = 30;
int isRunning = 1;
int isCtrlPressed = 0;

#define SCRIPT ".\\run.bat g"

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
                printf("Running '%s'...\n", SCRIPT);
                system(SCRIPT);
            }

            if (p->vkCode == 162)
            {
                isCtrlPressed = 1;
            }
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