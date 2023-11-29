#include <windows.h>
#include "types.h"
#include "win_utils.c"
#include "gdiFont.c"
#include "editor.c"

MyBitmap bitmap = {0};
BITMAPINFO bitmapInfo = {0};

i32 isRunning = 0;
i32 isFullscreen = 0;

LRESULT OnEvent(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DESTROY)
    {
        isRunning = 0;
    }
    else if (message == WM_SIZE)
    {
        OnResize(window, &bitmapInfo, &bitmap);
        UpdateAndDrawApp(&bitmap);
    }
    else if (message == WM_PAINT)
    {
        PAINTSTRUCT paint = {0};
        HDC dc = BeginPaint(window, &paint);
        DrawBitmap(dc, &bitmapInfo, &bitmap);
        EndPaint(window, &paint);
    }

    return DefWindowProc(window, message, wParam, lParam);
}

int wWinMain(HINSTANCE instance, HINSTANCE prev, PWSTR cmdLine, int showCode)
{
    PreventWindowsDPIScaling();
    InitFontSystem(13);
    InitApp();

    HWND window = OpenGameWindow(instance, OnEvent);
    isRunning = 1;
    while (isRunning)
    {
        MSG msg;

        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_SPACE)
            {
                isFullscreen = isFullscreen ? 0 : 1;
                ToggleFullscreen(window, isFullscreen);
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}